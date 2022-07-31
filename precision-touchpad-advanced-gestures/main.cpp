#include <Windows.h>
#include <vector>
#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "termcolor.h"
#include "utils.h"
#include "touchpad.h"
#include "touchevents.h"

static TCHAR szWindowClass[] = _T("DesktopApp");
static TCHAR szTitle[] = _T("F3: start writing - ESC: stop writing - C: clear - Q: close the application");

#define VK_C_KEY 0x43;
#define VK_Q_KEY 0x51;
#define VK_S_KEY 0x53;

std::vector<hidDeviceInfo> device_info_list;
std::vector<TouchData> previous_touches;
ULONG tracking_touch_id;
// flag to toggle drawing state
int is_drawing;

// TODO add option to change the key bindings
// TODO add option to add multiple key bindings (array of key bindings for each actions)

int turn_off_drawing_key_code;
int turn_on_drawing_key_code;
int export_writing_data_key_code;
int quit_application_key_code;
int clear_drawing_canvas_key_code;
int call_block_input_flag;
int call_unblock_input_flag;

LRESULT CALLBACK mBlockMouseInputHookProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644986(v=vs.85)
	if (nCode < 0)
	{
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	else
	{
		return -1;
	}
}

void mParseConnectedInputDevices()
{
	printf(FG_BLUE);
	printf("Parsing all HID devices...\n");
	printf(RESET_COLOR);

	// find number of connected devices

	UINT numDevices;
	RAWINPUTDEVICELIST* rawInputDeviceList = NULL;

	mGetRawInputDeviceList(&numDevices, &rawInputDeviceList);

	printf("Number of raw input devices: %d\n", numDevices);
	for (UINT deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
	{
		printf(BG_GREEN);
		printf("===== Device #%d =====\n", deviceIndex);
		printf(RESET_COLOR);
		RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList[deviceIndex];
		if (rawInputDevice.dwType != RIM_TYPEHID)
		{
			// skip keyboards and mouses
			continue;
		}

		// get preparsed data for HidP
		UINT cbDataSize = 0;
		PHIDP_PREPARSED_DATA preparsedData = NULL;

		mGetRawInputDevicePreparsedData(rawInputDevice.hDevice, &preparsedData, &cbDataSize);

		NTSTATUS hidpReturnCode;

		// find HID capabilities
		HIDP_CAPS caps;
		hidpReturnCode = HidP_GetCaps(preparsedData, &caps);
		if (hidpReturnCode != HIDP_STATUS_SUCCESS)
		{
			print_HidP_errors(hidpReturnCode);
			exit(-1);
		}

		printf("NumberInputValueCaps: %d\n", caps.NumberInputValueCaps);
		printf("NumberInputButtonCaps: %d\n", caps.NumberInputButtonCaps);

		int isButtonCapsEmpty = (caps.NumberInputButtonCaps == 0);

		if (!isButtonCapsEmpty)
		{
			UINT deviceNameLength;
			TCHAR* deviceName = NULL;
			unsigned int cbDeviceName;

			mGetRawInputDeviceName(rawInputDevice.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

			printf("Device name: ");
			wprintf(deviceName);
			printf("\n");

			printf(FG_GREEN);
			printf("Finding device in global list...\n");
			printf(RESET_COLOR);
			unsigned int foundHidIdx;
			int returnCode = findInputDeviceInList(device_info_list, *deviceName, cbDeviceName, preparsedData, cbDataSize, foundHidIdx);
			if (returnCode != 0)
			{
				printf("findInputDeviceInList failed\n");
				exit(-1);
			}

			printf(FG_GREEN);
			printf("foundHIDIdx: %d\n", foundHidIdx);
			printf(RESET_COLOR);

			printf(FG_BRIGHT_BLUE);
			printf("found device name: ");
			wprintf(device_info_list[foundHidIdx].Name);
			printf("\n");
			printf(RESET_COLOR);

			printf(FG_BRIGHT_BLUE);
			printf("current device name: ");
			wprintf(deviceName);
			printf("\n");
			printf(RESET_COLOR);

			if (caps.NumberInputValueCaps != 0)
			{
				const USHORT numValueCaps = caps.NumberInputValueCaps;
				USHORT _numValueCaps = numValueCaps;

				PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * numValueCaps);

				hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps, &_numValueCaps, preparsedData);
				if (hidpReturnCode != HIDP_STATUS_SUCCESS)
				{
					print_HidP_errors(hidpReturnCode);
				}

				// x check if numValueCaps value has been changed
				printf("NumberInputValueCaps: %d (old) vs %d (new)\n", numValueCaps, _numValueCaps);

				for (USHORT valueCapIndex = 0; valueCapIndex < numValueCaps; valueCapIndex++)
				{
					HIDP_VALUE_CAPS cap = valueCaps[valueCapIndex];

					if (cap.IsRange || !cap.IsAbsolute)
					{
						continue;
					}

					unsigned int foundLinkColIdx;
					int returnCode = findLinkCollectionInList(device_info_list[foundHidIdx].LinkColInfoList, cap.LinkCollection, foundLinkColIdx);
					if (returnCode != 0)
					{
						printf("findLinkCollectionInList failed\n");
						exit(-1);
					}

					printf(FG_GREEN);
					printf("[ValueCaps] foundLinkCollectionIndex: %d\n", foundLinkColIdx);
					printf(RESET_COLOR);

					if (cap.UsagePage == HID_USAGE_PAGE_GENERIC)
					{
						printf("=====================================================\n");
						printf("LinkCollection: %d\n", cap.LinkCollection);

						if (cap.NotRange.Usage == HID_USAGE_GENERIC_X)
						{
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].hasX = 1;
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].PhysicalRect.left = cap.PhysicalMin;
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].PhysicalRect.right = cap.PhysicalMax;
							printf("  Left: %d\n", cap.PhysicalMin);
							printf("  Right: %d\n", cap.PhysicalMax);
						}
						else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y)
						{
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].hasY = 1;
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].PhysicalRect.top = cap.PhysicalMin;
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].PhysicalRect.bottom = cap.PhysicalMax;
							printf("  Top: %d\n", cap.PhysicalMin);
							printf("  Bottom: %d\n", cap.PhysicalMax);
						}
					}
					else if (cap.UsagePage == HID_USAGE_PAGE_DIGITIZER)
					{
						if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_ID)
						{
							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].hasContactID = 1;
						}
						else if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_COUNT)
						{
							device_info_list[foundHidIdx].ContactCountLinkCollection = cap.LinkCollection;
						}
					}
				}

				free(valueCaps);
			}

			if (caps.NumberInputButtonCaps != 0)
			{
				const USHORT numButtonCaps = caps.NumberInputButtonCaps;
				USHORT _numButtonCaps = numButtonCaps;

				PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps);

				hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps, &_numButtonCaps, preparsedData);
				if (hidpReturnCode != HIDP_STATUS_SUCCESS)
				{
					print_HidP_errors(hidpReturnCode);
					exit(-1);
				}

				for (USHORT buttonCapIndex = 0; buttonCapIndex < numButtonCaps; buttonCapIndex++)
				{
					HIDP_BUTTON_CAPS buttonCap = buttonCaps[buttonCapIndex];

					if (buttonCap.IsRange)
					{
						continue;
					}

					printf(FG_BLUE);
					printf("[ButtonCaps] Index: %d, UsagePage: %d, Usage: %d, DIGITIZER: %d, IsRange: %d\n", buttonCapIndex, buttonCap.UsagePage, buttonCap.NotRange.Usage, buttonCap.UsagePage, buttonCap.IsRange);
					printf(RESET_COLOR);

					if (buttonCap.UsagePage == HID_USAGE_PAGE_DIGITIZER)
					{
						if (buttonCap.NotRange.Usage == HID_USAGE_DIGITIZER_TIP_SWITCH)
						{
							unsigned int foundLinkColIdx;
							int returnCode = findLinkCollectionInList(device_info_list[foundHidIdx].LinkColInfoList, buttonCap.LinkCollection, foundLinkColIdx);

							printf(FG_GREEN);
							printf("[ButtonCaps] foundLinkCollectionIndex: %d\n", foundLinkColIdx);
							printf(RESET_COLOR);

							device_info_list[foundHidIdx].LinkColInfoList[foundLinkColIdx].hasTipSwitch = 1;
						}
					}
				}

				free(buttonCaps);
			}

			free(deviceName);
		}

		free(preparsedData);
	}

	free(rawInputDeviceList);
}

void mRegisterRawInput(HWND hwnd)
{
	// register Windows Precision Touchpad top-level HID collection
	RAWINPUTDEVICE rid;
	clock_t ts = clock();

	rid.usUsagePage = HID_USAGE_PAGE_DIGITIZER;
	rid.usUsage = HID_USAGE_DIGITIZER_TOUCH_PAD;
	rid.dwFlags = RIDEV_INPUTSINK;
	rid.hwndTarget = hwnd;

	if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
	{
		printf(FG_GREEN);
		printf("[%d] Successfully register touchpad!\n", ts);
		printf(RESET_COLOR);
	}
	else
	{
		printf(FG_RED);
		printf("[%d] Failed to register touchpad\n", ts);
		printf(RESET_COLOR);
		mGetLastError();
		exit(-1);
	}
}

void mHandleCreateMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	mRegisterRawInput(hwnd);
}

void mHandleInputMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	clock_t ts = clock();

	// following guide: https://docs.microsoft.com/en-us/windows/win32/inputdev/using-raw-input#performing-a-standard-read-of-raw-input

	// Get the size of RAWINPUT by calling GetRawInputData() with pData = NULL

	if (is_drawing != 0)
	{
		UINT rawInputSize;
		PRAWINPUT rawInputData = NULL;

		mGetRawInputData((HRAWINPUT)lParam, &rawInputSize, (LPVOID*)(&rawInputData));

		// Parse the RAWINPUT data.
		if (rawInputData->header.dwType == RIM_TYPEHID)
		{
			// TODO what does `dwCount` represent?
			DWORD count = rawInputData->data.hid.dwCount;
			BYTE* rawData = rawInputData->data.hid.bRawData;

			if (count != 0)
			{
				UINT deviceNameLength;
				TCHAR* deviceName = NULL;
				unsigned int cbDeviceName;

				mGetRawInputDeviceName(rawInputData->header.hDevice, &deviceName, &deviceNameLength, &cbDeviceName);

				unsigned int foundHidIdx = (unsigned int)-1;

				if (device_info_list.empty())
				{
					// TODO parse new connected device data
				}
				else
				{
					for (unsigned int touchpadIndex = 0; touchpadIndex < device_info_list.size(); touchpadIndex++)
					{
						int compareNameResult = _tcscmp(deviceName, device_info_list[touchpadIndex].Name);
						if (compareNameResult == 0)
						{
							foundHidIdx = touchpadIndex;
							break;
						}
					}
				}

				if (foundHidIdx == (unsigned int)-1)
				{
					// TODO parse new connected device data
				}
				else
				{
					int isPreparsedDataNull = (device_info_list[foundHidIdx].PreparedData == NULL);

					if (device_info_list[foundHidIdx].LinkColInfoList.empty())
					{
						printf(FG_RED);
						printf("Cannot find any LinkCollection(s). Try parse the PREPARED_DATA may help. TODO\n");
						printf(RESET_COLOR);
					}
					else if (isPreparsedDataNull)
					{
						printf(FG_RED);
						printf("Cannot find PreparsedData\n");
						printf(RESET_COLOR);
					}
					else
					{

						printf("[%d]", ts);

						NTSTATUS hidpReturnCode;
						ULONG usageValue;

						PHIDP_PREPARSED_DATA preparsedHIDData = device_info_list[foundHidIdx].PreparedData;

						if (device_info_list[foundHidIdx].ContactCountLinkCollection == (USHORT)-1)
						{
							printf(FG_RED);
							printf("Cannot find contact count Link Collection!\n");
							printf(RESET_COLOR);
						}
						else
						{
							hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, device_info_list[foundHidIdx].ContactCountLinkCollection, HID_USAGE_DIGITIZER_CONTACT_COUNT, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

							if (hidpReturnCode != HIDP_STATUS_SUCCESS)
							{
								printf(FG_RED);
								printf("Failed to read number of contacts!\n");
								printf(RESET_COLOR);
								print_HidP_errors(hidpReturnCode);
								exit(-1);
							}

							ULONG numContacts = usageValue;

							if (numContacts > 0) {
								printf(FG_BRIGHT_BLUE);
								printf("numContacts: %d\n", numContacts);
								printf(RESET_COLOR);
							}

							hidTouchLinkCollectionInfo collectionInfo = device_info_list[foundHidIdx].LinkColInfoList[0];

							if (collectionInfo.hasX && collectionInfo.hasY && collectionInfo.hasContactID && collectionInfo.hasTipSwitch)
							{
								hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x30, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

								if (hidpReturnCode != HIDP_STATUS_SUCCESS)
								{
									printf(FG_RED);
									printf("Failed to read x position!\n");
									printf(RESET_COLOR);
									print_HidP_errors(hidpReturnCode);
									//exit(-1);
								}

								ULONG xPos = usageValue;

								hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.LinkColID, 0x31, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
								if (hidpReturnCode != HIDP_STATUS_SUCCESS)
								{
									printf(FG_RED);
									printf("Failed to read y position!\n");
									printf(RESET_COLOR);
									print_HidP_errors(hidpReturnCode);
									//exit(-1);
								}

								ULONG yPos = usageValue;

								hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
								if (hidpReturnCode != HIDP_STATUS_SUCCESS)
								{
									printf(FG_RED);
									printf("Failed to read touch ID!\n");
									printf(RESET_COLOR);
									print_HidP_errors(hidpReturnCode);
									//exit(-1);
								}

								ULONG touchId = usageValue;

								const ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);

								ULONG _maxNumButtons = maxNumButtons;

								USAGE* buttonUsageArray = (USAGE*)malloc(sizeof(USAGE) * maxNumButtons);

								hidpReturnCode = HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.LinkColID, buttonUsageArray, &_maxNumButtons, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

								if (hidpReturnCode != HIDP_STATUS_SUCCESS)
								{
									printf(FG_RED);
									printf("HidP_GetUsages failed!\n");
									printf(RESET_COLOR);
									print_HidP_errors(hidpReturnCode);
									//exit(-1);
								}

								int isContactOnSurface = 0;

								for (ULONG usageIdx = 0; usageIdx < maxNumButtons; usageIdx++)
								{
									if (buttonUsageArray[usageIdx] == HID_USAGE_DIGITIZER_TIP_SWITCH)
									{
										isContactOnSurface = 1;
										break;
									}
								}

								free(buttonUsageArray);

								TouchData curTouch;
								curTouch.touchID = touchId;
								curTouch.x = xPos;
								curTouch.y = yPos;
								curTouch.onSurface = isContactOnSurface;

								printf("Touch ID: %d | Pos: (%d, %d) | IsOnSurface: %d\n", curTouch.touchID, curTouch.x, curTouch.y, curTouch.onSurface);

								TouchEventType touchType = TouchEventType::TOUCH_DOWN;
								int cStyleFunctionReturnCode = interpretRawTouchInput(previous_touches, curTouch, &touchType);
								if (cStyleFunctionReturnCode != 0)
								{
									printf(FG_RED);
									printf("mInterpretRawTouchInput failed\n");
									printf(RESET_COLOR);
									//exit(-1);
								}

								// 3880, 2299

								HDC hdc = GetDC(hwnd);
								COLORREF col = RGB(255, 0, 0);
								if (touchId == 1)
									col = RGB(255, 255, 0);
								if (touchId == 2)
									col = RGB(0, 255, 0);
								if (touchId == 3)
									col = RGB(0, 255, 255);
								if (touchId == 4)
									col = RGB(0, 0, 255);
								HPEN strokePen = CreatePen(PS_SOLID, 20, col);
								SelectObject(hdc, strokePen);
								Ellipse(hdc, curTouch.x - 20, curTouch.y - 20, curTouch.x + 20, curTouch.y + 20);
								//MoveToEx(hdc, (int)stroke[stroke.Size - 2].X, (int)stroke[stroke.Size - 2].Y, (LPPOINT)NULL);
								//LineTo(hdc, (int)stroke[stroke.Size - 1].X, (int)stroke[stroke.Size - 1].Y);
								ReleaseDC(hwnd, hdc);

								if (tracking_touch_id == (ULONG)-1)
								{
									if (touchType == TouchEventType::TOUCH_DOWN)
									{
										tracking_touch_id = curTouch.touchID;
										// TODO create new stroke
										// TODO check return value for indication of errors
									}
									else
									{
										// wait for touch down event to register new stroke
									}
								}
								else if (curTouch.touchID == tracking_touch_id)
								{
									if (touchType == TouchEventType::TOUCH_MOVE)
									{
										// we skip TouchEventType::TOUCH_MOVE_UNCHANGED here
										// TODO append touch position to the last stroke
											// TODO check return value for indication of errors
										HDC hdc = GetDC(hwnd);
										HPEN strokePen = CreatePen(PS_SOLID, 20, RGB(255, 0, 0));
										SelectObject(hdc, strokePen);
										Ellipse(hdc, curTouch.x - 10, curTouch.y - 10, curTouch.x + 10, curTouch.y + 10);
										//MoveToEx(hdc, (int)stroke[stroke.Size - 2].X, (int)stroke[stroke.Size - 2].Y, (LPPOINT)NULL);
										//LineTo(hdc, (int)stroke[stroke.Size - 1].X, (int)stroke[stroke.Size - 1].Y);
										ReleaseDC(hwnd, hdc);
									}
									else if (touchType == TouchEventType::TOUCH_UP)
									{
										// I sure that the touch position is the same with the last touch position
										tracking_touch_id = (ULONG)-1;
									}
								}

								const char* touchTypeStr;

								if (touchType == TouchEventType::TOUCH_UP)
								{
									touchTypeStr = "touch up";
								}
								else if (touchType == TouchEventType::TOUCH_MOVE)
								{
									touchTypeStr = "touch move";
								}
								else if (touchType == TouchEventType::TOUCH_DOWN)
								{
									touchTypeStr = "touch down";
								}
								else if (touchType == TouchEventType::TOUCH_MOVE_UNCHANGED)
								{
									touchTypeStr = "touch move unchanged";
								}
								else
								{
									printf(FG_RED);
									printf("unknown event type: %d\n", touchType);
									printf(RESET_COLOR);
									exit(-1);
								}

								printf(FG_GREEN);
								printf("LinkColId: %d, touchID: %d, tipSwitch: %d, position: (%d, %d), eventType: %s\n", collectionInfo.LinkColID, touchId, isContactOnSurface, xPos, yPos, touchTypeStr);
								printf(RESET_COLOR);
							}
						}
					}

					free(deviceName);
				}
			}

			free(rawInputData);
		}
	}
}

void mHandleResizeMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	InvalidateRect(hwnd, NULL, FALSE);
}

void mHandlePaintMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	// clock_t ts = clock();
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
	HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdc, &rc, bgBrush);

	// TODO change color for every strokes
	HPEN strokePen = CreatePen(PS_SOLID, 20, RGB(255, 255, 255));
	SelectObject(hdc, strokePen);

	EndPaint(hwnd, &ps);
}

void mHandleKeyUpMessage(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	clock_t ts = clock();

	printf(FG_GREEN);
	printf("[%d] WM_KEYUP: 0x%x\n", ts, (unsigned int)wParam);
	printf(RESET_COLOR);

	int virtual_key_code = (int)wParam;
	if (virtual_key_code == turn_off_drawing_key_code)
	{
		is_drawing = 0;

		call_unblock_input_flag = 1;
	}
	else if (virtual_key_code == turn_on_drawing_key_code)
	{
		is_drawing = 1;

		call_block_input_flag = 1;
	}
	else if (virtual_key_code == clear_drawing_canvas_key_code)
	{
		tracking_touch_id = (ULONG)-1;

		if (!previous_touches.empty())
		{
			previous_touches.clear();
		}


		InvalidateRect(hwnd, NULL, FALSE);
	}
	else if (virtual_key_code == quit_application_key_code)
	{
		PostQuitMessage(0);
	}
}

LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	clock_t ts = clock();

	switch (uMsg)
	{
	case WM_CREATE:
	{
		mHandleCreateMessage(hwnd, uMsg, wParam, lParam);
		break;
	}
	case WM_INPUT:
	{
		mHandleInputMessage(hwnd, uMsg, wParam, lParam);
		break;
	}
	case WM_PAINT:
	{
		mHandlePaintMessage(hwnd, uMsg, wParam, lParam);
		break;
	}
	case WM_SIZE:
	{
		mHandleResizeMessage(hwnd, uMsg, wParam, lParam);
		break;
	}
	case WM_KEYUP:
	{
		mHandleKeyUpMessage(hwnd, uMsg, wParam, lParam);
		break;
	}
	case WM_SYSKEYUP:
	{
		printf(FG_GREEN);
		printf("[%d] WM_SYSKEYUP: 0x%x\n", ts, (unsigned int)wParam);
		printf(RESET_COLOR);
		break;
	}
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		break;
	}
	default:
	{
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	}

	return 0;
}

int CALLBACK wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	mParseConnectedInputDevices();

	// TODO detect and prompt user to select touchpad device and parse its width and height

	// default window width and height values

	int nWidth = 720;
	int nHeight = 480;

	if (!device_info_list.empty())
	{
		// TODO check for valid touchpad device
		for (unsigned int deviceIdx = 0; deviceIdx < device_info_list.size(); deviceIdx++)
		{
			hidDeviceInfo inputDevice = device_info_list[deviceIdx];

			if (inputDevice.LinkColInfoList.empty())
			{
				printf(FG_BRIGHT_YELLOW);
				printf("Skipping input device #%d!\n", deviceIdx);
				printf(RESET_COLOR);
				continue;
			}
			else
			{
				for (unsigned int linkColIdx = 0; linkColIdx < inputDevice.LinkColInfoList.size(); linkColIdx++)
				{
					hidTouchLinkCollectionInfo linkCollectionInfo = inputDevice.LinkColInfoList[linkColIdx];
					if (linkCollectionInfo.hasX && linkCollectionInfo.hasY)
					{
						// TODO Should we need to parse every single touch link collections? For now, I think one is sufficient.
						// TODO validate values (e.g. 0 or > screen size)
						if (linkCollectionInfo.PhysicalRect.right > nWidth)
						{
							nWidth = linkCollectionInfo.PhysicalRect.right;
						}

						if (linkCollectionInfo.PhysicalRect.bottom > nHeight)
						{
							nHeight = linkCollectionInfo.PhysicalRect.bottom;
						}

						break;
					}
				}
			}
		}
	}
	else
	{
		printf(FG_RED);
		printf("Failed to parse input devices!\n");
		printf(RESET_COLOR);
		//return -1;
	}

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
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		printf("RegisterClassEx failed\n");
		mGetLastError();
		return -1;
	}

	HWND hwnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_EX_LAYERED, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight, NULL, NULL, hInstance, NULL);

	if (!hwnd)
	{
		printf("CreateWindow failed\n");
		mGetLastError();
		return -1;
	}

	// make the window transparent
	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA | LWA_COLORKEY);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	HOOKPROC llMouseHookProc = mBlockMouseInputHookProc;
	HHOOK llMouseHookHandle = NULL;

	// BOOL block_input_retval;
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		/*if (call_block_input_flag != 0)
		{
			if (llMouseHookHandle == NULL)
			{
				llMouseHookHandle = SetWindowsHookEx(WH_MOUSE_LL, llMouseHookProc, NULL, 0);
			}

			SetCursor(NULL);

			call_block_input_flag = 0;
		}
		else if (call_unblock_input_flag != 0)
		{
			if (llMouseHookHandle != NULL)
			{
				UnhookWindowsHookEx(llMouseHookHandle);
				llMouseHookHandle = NULL;
			}

			call_unblock_input_flag = 0;
		}*/
	}

	return (int)msg.wParam;
}

int main()
{
	device_info_list.clear();
	tracking_touch_id = -1;
	is_drawing = 0;

	turn_off_drawing_key_code = VK_ESCAPE;
	turn_on_drawing_key_code = VK_F3;
	quit_application_key_code = VK_Q_KEY;
	export_writing_data_key_code = VK_S_KEY;
	clear_drawing_canvas_key_code = VK_C_KEY;

	call_block_input_flag = 0;
	call_unblock_input_flag = 0;

	return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLine(), SW_SHOWNORMAL);
};