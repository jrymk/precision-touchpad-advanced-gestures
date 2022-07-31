#include <Windows.h>
#include <tchar.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <stdio.h>

#include "touchpad.h"

#include "termcolor.h"
#include "utils.h"

int getRawInputDeviceName(_In_ HANDLE hDevice, _Out_ TCHAR** deviceName, _Out_ UINT* nameSize, _Out_ unsigned int* cbDeviceName)
{
	int ret = 0;
	UINT winReturnCode;

	winReturnCode = GetRawInputDeviceInfo(_In_ hDevice, RIDI_DEVICENAME, NULL, nameSize);
	if (winReturnCode == (UINT)-1)
	{
		ret = -1;
		printf(FG_RED);
		printf("GetRawInputDeviceInfo failed\n");
		printf(RESET_COLOR);
		mGetLastError();
		exit(-1);
	}
	else
	{
		(*cbDeviceName) = (unsigned int)(sizeof(TCHAR) * ((*nameSize) + 1));
		(*deviceName) = (TCHAR*)malloc((*cbDeviceName));

		(*deviceName)[(*nameSize)] = 0;

		winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, (*deviceName), nameSize);
		if (winReturnCode == (UINT)-1)
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputDeviceInfo failed\n");
			printf(RESET_COLOR);
			exit(-1);
		}
		else if (winReturnCode != (*nameSize))
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputDeviceInfo does not return the expected size %d (actual) vs %d (expected) at  %s:%d\n", winReturnCode, (*nameSize));
			printf(RESET_COLOR);
			exit(-1);
		}
	}
	return ret;
}

int getRawInputDeviceList(_Out_ UINT* numDevices, _Out_ RAWINPUTDEVICELIST** deviceList)
{
	int ret = 0;
	UINT winReturnCode;

	winReturnCode = GetRawInputDeviceList(NULL, numDevices, sizeof(RAWINPUTDEVICELIST));
	if (winReturnCode == (UINT)-1)
	{
		ret = -1;
		printf(FG_RED);
		printf("GetRawInputDeviceList failed\n");
		printf(RESET_COLOR);
		mGetLastError();
		exit(-1);
	}
	else
	{
		(*deviceList) = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * (*numDevices));
		winReturnCode = GetRawInputDeviceList((*deviceList), numDevices, sizeof(RAWINPUTDEVICELIST));
		if (winReturnCode == (UINT)-1)
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputDeviceList failed\n");
			printf(RESET_COLOR);
			mGetLastError();
			// TODO should we also free (*deviceList) here?
			exit(-1);
		}
	}

	return ret;
}

int getRawInputDevicePreparsedData(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ UINT* cbSize)
{
	int ret = 0;
	UINT winReturnCode;

	if (data == NULL)
	{
		ret = -1;
		printf(FG_RED);
		printf("(PHIDP_PREPARSED_DATA*) data parameter is NULL!\n");
		printf(RESET_COLOR);

		exit(-1);
	}
	else if (cbSize == NULL)
	{
		ret = -1;
		printf(FG_RED);
		printf("The cbSize parameter is NULL!\n");
		printf(RESET_COLOR);

		exit(-1);
	}
	else if ((*data) != NULL)
	{
		ret = -1;
		printf(FG_RED);
		printf("(PHIDP_PREPARSED_DATA) data parameter is not NULL! Please free your memory and set the point to NULL.\n");
		printf(RESET_COLOR);

		exit(-1);
	}
	else
	{
		winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, cbSize);
		if (winReturnCode == (UINT)-1)
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputDeviceInfo failed\n");
			printf(RESET_COLOR);
			mGetLastError();

			exit(-1);
		}
		else
		{
			(*data) = (PHIDP_PREPARSED_DATA)malloc((*cbSize));

			winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, (*data), cbSize);
			if (winReturnCode == (UINT)-1)
			{
				ret = -1;
				printf(FG_RED);
				printf("GetRawInputDeviceInfo failed\n");
				printf(RESET_COLOR);
				mGetLastError();

				exit(-1);
			}
		}
	}

	return ret;
}

int getRawInputData(_In_ HRAWINPUT hRawInput, _Out_ PUINT pcbSize, _Out_ LPVOID* pData)
{
	int ret = 0;
	UINT winReturnCode;

	if (pcbSize == NULL)
	{
		ret = -1;
		printf(FG_RED);
		printf("pcbSize parameter is NULL!\n");
		printf(RESET_COLOR);

		exit(-1);
	}
	else if (pData == NULL)
	{
		ret = -1;
		printf(FG_RED);
		printf("(LPVOID*) pData parameter is NULL!\n");
		printf(RESET_COLOR);

		exit(-1);
	}
	else if ((*pData) != NULL)
	{
		ret = -1;
		printf(FG_RED);
		printf("(LPVOID) pData value is not NULL! Please free your memory and set the pointer value to NULL.\n");
		printf(RESET_COLOR);

		exit(-1);
	}
	else
	{
		winReturnCode = GetRawInputData(hRawInput, RID_INPUT, NULL, pcbSize, sizeof(RAWINPUTHEADER));
		if (winReturnCode == (UINT)-1)
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputData failed\n");
			printf(RESET_COLOR);
			mGetLastError();

			exit(-1);
		}
		else
		{
			(*pData) = (LPVOID)malloc((*pcbSize));

			winReturnCode = GetRawInputData(hRawInput, RID_INPUT, (*pData), pcbSize, sizeof(RAWINPUTHEADER));
			if (winReturnCode == (UINT)-1)
			{
				ret = -1;
				printf(FG_RED);
				printf("GetRawInputData failed\n");
				printf(RESET_COLOR);
				mGetLastError();

				exit(-1);
			}
			else if (winReturnCode != (*pcbSize))
			{
				ret = -1;
				printf(FG_RED);
				printf("GetRawInputData failed.\n");
				printf("The return value - the number of byte(s) copied into pData (%d) is not equal the expected value (%d).\n", winReturnCode, (*pcbSize));
				printf(RESET_COLOR);
				exit(-1);
			}
		}
	}

	return ret;
}
