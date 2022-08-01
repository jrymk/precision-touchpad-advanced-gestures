#include <Windows.h>
#include <vector>
#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "utils.h"
#include "touchpad.h"
#include "devices.h"
#include "gesture.h"
#include "config.h"

int setRemaining = 0;
std::vector<TouchData> touchPoints(MAX_TOUCH_POINTS);

LRESULT CALLBACK mBlockMouseInputHookProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644986(v=vs.85)
	if (nCode < 0)
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	else
		return -1;
}

void registerRawInput(HWND hwnd)
{
	// register Windows Precision Touchpad top-level HID collection
	RAWINPUTDEVICE rid;

	rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
	rid.usUsage = HID_USAGE_DIGITIZER_TOUCH_PAD;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;

	if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
	{
		printf(FG_GREEN);
		printf("Successfully register touchpad\n");
		printf(RESET_COLOR);
	}
	else
	{
		printf(FG_RED);
		printf("Failed to register touchpad\n");
		printf(RESET_COLOR);
		getLastError();
	}
}

void handleInputMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// following guide: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input
	// Get the size of RAWINPUT by calling GetRawInputData() with pData = NULL

	UINT rawInputSize;
	PRAWINPUT rawInputData = NULL;
	hidDeviceInfo deviceInfo;

	getRawInputData((HRAWINPUT)lParam, &rawInputSize, (LPVOID*)(&rawInputData));

	// check if this messsage is for a touchpad
	if (checkInput(rawInputSize, rawInputData, deviceInfo)) {
		TouchData currentTouch;
		// try to read touch data to currentTouch
		if (readInput(rawInputSize, rawInputData, deviceInfo, currentTouch, setRemaining)) {
			TouchEventType touchType = TouchEventType::TOUCH_DOWN;
			saveTouchInput(touchPoints, currentTouch);
			// 3880, 2299

			// when data comes in, it comes in <contact count> messages. in order to get data from the same time, only read data when setRemaining is 0. the last data read should be "touch_up"
			if (!setRemaining) {
				inputTouchPoints(touchPoints);

				HANDLE hOut;
				COORD Position;

				hOut = GetStdHandle(STD_OUTPUT_HANDLE);

				Position.X = 0;
				Position.Y = 0;
				SetConsoleCursorPosition(hOut, Position);

				for (int i = 0; i < touchPoints.size(); i++) {
					std::string str = "---- ";
					switch (touchPoints[i].eventType) {
					case RELEASED:
						str = "---- ";
						break;
					case TOUCH_DOWN:
						str = "down ";
						break;
					case TOUCH_MOVE:
						str = "move ";
						break;
					case TOUCH_UP:
						str = "up   ";
						break;
					case TOUCH_MOVE_UNCHANGED:
						str = "unch ";
						break;

					}

					Position.X = 0;
					Position.Y = i;
					SetConsoleCursorPosition(hOut, Position);

					printf("[%d] ", i);
					printf(str.c_str());
					printf(" %.3f", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - activeStroke[i].beginTime).count() / 1000.f);

					printf("\n");

					if (touchPoints[i].eventType == TOUCH_UP)
						touchPoints[i].eventType = RELEASED;

					Position.X = int(touchPoints[i].x / 3880.f * 120);
					Position.Y = int(touchPoints[i].y / 3880.f * 50);
					SetConsoleCursorPosition(hOut, Position);
					
					if (touchPoints[i].onSurface) {
						if (i == 0)
							printf(BG_RED);
						if (i == 1)
							printf(BG_YELLOW);
						if (i == 2)
							printf(BG_GREEN);
						if (i == 3)
							printf(BG_BRIGHT_BLUE);
						if (i == 4)
							printf(BG_CYAN);
						printf("*");
						printf(RESET_COLOR);
					}
				}

				printf("\n");

				if (touchPoints[0].onSurface + touchPoints[1].onSurface + touchPoints[2].onSurface + touchPoints[3].onSurface + touchPoints[4].onSurface == 0)
					system("cls");
			}
			//printf(FG_GREEN);
			//printf("LinkColId: %d, touchID: %d, tipSwitch: %d, position: (%d, %d), eventType: %s\n", deviceInfo.linkCollectionInfoList[0].linkCollectionID, currentTouch.touchID, currentTouch.onSurface, currentTouch.x, currentTouch.y, touchTypeStr);
			//printf(RESET_COLOR);
		}
	}
	free(rawInputData);
}

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		registerRawInput(hwnd);
		break;
	case WM_INPUT:
		handleInputMessage(hwnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	parseInputDevices();

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(0);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("DesktopApp");
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		printf("RegisterClassEx failed\n");
		getLastError();
		return -1;
	}

	// create a window that will never show for input handling
	HWND hwnd = CreateWindow(_T("DesktopApp"), _T("precision-touchpad-advanced-gestures"), 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL, hInstance, NULL);
	if (!hwnd)
	{
		printf("CreateWindow failed\n");
		getLastError();
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

int main()
{
	wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
	return 0;
};