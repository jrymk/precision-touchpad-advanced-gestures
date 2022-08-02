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
std::vector<TouchData> touchPoints(config.maxTouchPoints);

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

	HDC hdc = GetDC(hwnd);
	RECT rc;

	GetClientRect(hwnd, &rc);

	if (rc.bottom == 0)
	{
		return;
	}

	bool haveSurface = false;
	for (int i = 0; i < 5; i++) {
		if (touchPoints[i].onSurface)
			haveSurface = true;
	}
	
	if (!haveSurface) {
		// draw black background
		HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &rc, bgBrush);

		//ShowWindow(hwnd, 0);
		//UpdateWindow(hwnd);
	}
	ReleaseDC(hwnd, hdc);


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
					printf(" (%d, %d) ", touchPoints[i].x, touchPoints[i].y);
					printf(" %.3f", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - activeStroke[i].beginTime).count() / 1000.f);
					printf(" %.3f", activeStroke[i].distance);
					printf(" %.3f", activeStroke[i].angle);
					printf(" %d", activeStroke[i].cardinal);
					printf(" %d", activeStroke[i].ordinal);

					printf("                       \n");

					if (touchPoints[i].eventType == TOUCH_UP)
						touchPoints[i].eventType = RELEASED;

					Position.X = int(touchPoints[i].x / 3880.f * 120);
					Position.Y = int(touchPoints[i].y / 3880.f * 49);
					SetConsoleCursorPosition(hOut, Position);

					if (touchPoints[i].onSurface) {
						if (i == 0)
							printf(BG_RED);
						if (i == 1)
							printf(BG_YELLOW);
						if (i == 2)
							printf(BG_GREEN);
						if (i == 3)
							printf(BG_CYAN);
						if (i == 4)
							printf(BG_BRIGHT_BLUE);
						printf("*");
						printf(RESET_COLOR);
					}
				}

				printf("\n");

				if (touchPoints[0].onSurface + touchPoints[1].onSurface + touchPoints[2].onSurface + touchPoints[3].onSurface + touchPoints[4].onSurface == 0)
					system("cls");

				HDC hdc = GetDC(hwnd);

				// TODO change color for every strokes

				for (int i = 0; i < config.maxTouchPoints; i++) {
					COLORREF color = RGB(150, 150, 150);
					if (i == 0)
						color = RGB(255, 107, 107);
					if (i == 1)
						color = RGB(254, 202, 87);
					if (i == 2)
						color = RGB(29, 209, 161);
					if (i == 3)
						color = RGB(72, 219, 251);
					if (i == 4)
						color = RGB(84, 160, 255);

					//HPEN strokePen = CreatePen(PS_DASH, 2, color);
					HPEN strokePen = CreatePen(PS_SOLID, 2, color);
					SelectObject(hdc, strokePen);


					if (touchPoints[i].onSurface) {
						Ellipse(hdc, touchPoints[i].x / 5 - 2, touchPoints[i].y / 5 - 2, touchPoints[i].x / 5 + 2, touchPoints[i].y / 5 + 2);
						//ShowWindow(hwnd, SW_SHOW);

						if (i == 4)
							MoveWindow(hwnd, int(touchPoints[i].x / 3880.f * 2560.f), int(touchPoints[i].y / 3880.f * 1600), 776, 460, false);
					}


					DeleteObject(strokePen);
				}

				ReleaseDC(hwnd, hdc);
				DeleteObject(hdc);

			}
			//printf(FG_GREEN);
			//printf("LinkColId: %d, touchID: %d, tipSwitch: %d, position: (%d, %d), eventType: %s\n", deviceInfo.linkCollectionInfoList[0].linkCollectionID, currentTouch.touchID, currentTouch.onSurface, currentTouch.x, currentTouch.y, touchTypeStr);
			//printf(RESET_COLOR);
		}
	}
	free(rawInputData);
}

void mHandlePaintMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;

	GetClientRect(hwnd, &rc);

	if (rc.bottom == 0)
	{
		return;
	}

	hdc = BeginPaint(hwnd, &ps);

	// draw black background
	/*HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdc, &rc, bgBrush);
	DeleteObject(bgBrush);*/

	for (int i = 0; i < config.maxTouchPoints; i++) {
		COLORREF color = RGB(150, 150, 150);
		if (i == 0)
			color = RGB(255, 107, 107);
		if (i == 1)
			color = RGB(254, 202, 87);
		if (i == 2)
			color = RGB(29, 209, 161);
		if (i == 3)
			color = RGB(72, 219, 251);
		if (i == 4)
			color = RGB(84, 160, 255);

		//HPEN strokePen = CreatePen(PS_DASH, 2, color);
		//SelectObject(hdc, strokePen);

		if (touchPoints[i].onSurface) {
			Ellipse(hdc, touchPoints[i].x / 5 - 2, touchPoints[i].y / 5 - 2, touchPoints[i].x / 5 + 2, touchPoints[i].y / 5 + 2);
			//ShowWindow(hwnd, SW_SHOW);
		}
		//DeleteObject(strokePen);
	}

	EndPaint(hwnd, &ps);
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
	case WM_PAINT:
	{
		mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
		break;
	}
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
	wcex.lpszClassName = _T("OverlayWindowClass");
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		printf("RegisterClassEx failed\n");
		getLastError();
		return -1;
	}

	// create a window that will never show for input handling
	HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED , _T("OverlayWindowClass"), _T("precision-touchpad-advanced-gestures"), WS_VISIBLE | WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, 776, 460, NULL, NULL, hInstance, NULL);
	if (!hwnd)
	{
		printf("CreateWindow failed\n");
		getLastError();
		return -1;
	}

	//SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);



	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);


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