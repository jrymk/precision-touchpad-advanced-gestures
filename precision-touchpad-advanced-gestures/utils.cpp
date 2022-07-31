#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "termcolor.h"
#include "utils.h"

void mGetLastError()
{
	DWORD errorCode = GetLastError();
	LPWSTR messageBuffer = NULL;
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	printf(FG_RED);
	printf("Error Code: %d\n", errorCode);
	wprintf(L"%s\n", messageBuffer);
	printf(RESET_COLOR);
	// TODO check to see if we don't free the messageBuffer pointer, will it lead to memory leaking?
}

void print_HidP_errors(NTSTATUS hidpReturnCode)
{
	printf(FG_RED);

	if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_LENGTH)
	{
		printf("The report length is not valid. HidP function failed at ");
	}
	else if (hidpReturnCode == HIDP_STATUS_INVALID_REPORT_TYPE)
	{
		printf("The specified report type is not valid. HidP function failed at ");
	}
	else if (hidpReturnCode == HIDP_STATUS_INCOMPATIBLE_REPORT_ID)
	{
		printf("The collection contains a value on the specified usage page in a report of the specified type, but there are no such usages in the specified report. HidP function failed at ");
	}
	else if (hidpReturnCode == HIDP_STATUS_INVALID_PREPARSED_DATA)
	{
		printf("The preparsed data is not valid. HidP function failed at ");
	}
	else if (hidpReturnCode == HIDP_STATUS_USAGE_NOT_FOUND)
	{
		printf("The collection does not contain a value on the specified usage page in any report of the specified report type. HidP function failed at ");
	}
	else
	{
		printf("Unknown error code: %d. HidP function failed at ", hidpReturnCode);
	}

	printf(RESET_COLOR);
}

int findInputDeviceInList(std::vector<hidDeviceInfo>& hidInfoList, TCHAR& deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int& foundHidIndex)
{
	foundHidIndex = (unsigned int)-1;
	if (hidInfoList.empty())
	{
		foundHidIndex = 0;
		hidInfoList.resize(1);
		// TODO recursive free pointers inside struct
		// but we can't know the size of array pointer here
		// because the size is recorded as 0

		hidInfoList[0].cbName = cbDeviceName;
		hidInfoList[0].LinkColInfoList.clear();
		hidInfoList[0].PreparedData = preparsedData;
		hidInfoList[0].cbPreparsedData = cbPreparsedData;
		hidInfoList[0].Name = (TCHAR*)malloc(cbDeviceName);
		hidInfoList[0].ContactCountLinkCollection = (USHORT)-1;
		memcpy(hidInfoList[0].Name, &deviceName, cbDeviceName);
		hidInfoList[0].PreparedData = (PHIDP_PREPARSED_DATA)malloc(cbPreparsedData);
		memcpy(hidInfoList[0].PreparedData, preparsedData, cbPreparsedData);
	}
	else
	{
		for (unsigned int touchpadIndex = 0; touchpadIndex < hidInfoList.size(); touchpadIndex++)
		{
			int compareNameResult = _tcscmp(&deviceName, hidInfoList[touchpadIndex].Name);
			if (compareNameResult == 0)
			{
				foundHidIndex = touchpadIndex;
				break;
			}
		}
	}

	if (foundHidIndex == (unsigned int)-1)
	{
		// the array/list/dictionary is not empty
		// but we cannot find any entry with the same name

		// allocate memory and create a new entry at the end of array
		foundHidIndex = hidInfoList.size();
		hidInfoList.resize(foundHidIndex + 1);

		hidInfoList[foundHidIndex].cbName = cbDeviceName;
		hidInfoList[foundHidIndex].LinkColInfoList.clear();
		hidInfoList[foundHidIndex].cbPreparsedData = cbPreparsedData;

		hidInfoList[foundHidIndex].Name = (TCHAR*)malloc(cbDeviceName);
		memcpy(hidInfoList[foundHidIndex].Name, &deviceName, cbDeviceName);
		hidInfoList[foundHidIndex].PreparedData = (PHIDP_PREPARSED_DATA)malloc(cbPreparsedData);
		memcpy(hidInfoList[foundHidIndex].PreparedData, preparsedData, cbPreparsedData);
	}

	return 0;
}

int findLinkCollectionInList(std::vector<hidTouchLinkCollectionInfo>& linkColInfoList, USHORT linkCollection, unsigned int& foundLinkColIdx)
{
	foundLinkColIdx = (unsigned int)-1;

	if (linkColInfoList.empty())
	{
		foundLinkColIdx = 0;
		linkColInfoList.resize(1);

		linkColInfoList[foundLinkColIdx].linkCollectionID = linkCollection;
		linkColInfoList[foundLinkColIdx].hasX = 0;
		linkColInfoList[foundLinkColIdx].hasY = 0;
		linkColInfoList[foundLinkColIdx].hasTipSwitch = 0;
		linkColInfoList[foundLinkColIdx].hasContactID = 0;
		linkColInfoList[foundLinkColIdx].hasConfidence = 0;
		linkColInfoList[foundLinkColIdx].hasWidth = 0;
		linkColInfoList[foundLinkColIdx].hasHeight = 0;
		linkColInfoList[foundLinkColIdx].hasPressure = 0;
	}
	else
	{
		for (unsigned int linkColIdx = 0; linkColIdx < linkColInfoList.size(); linkColIdx++)
		{
			if (linkColInfoList[linkColIdx].linkCollectionID == linkCollection)
			{
				foundLinkColIdx = linkColIdx;
				break;
			}
		}
	}

	if (foundLinkColIdx == (unsigned int)-1)
	{
		foundLinkColIdx = linkColInfoList.size();
		linkColInfoList.resize(foundLinkColIdx + 1);

		linkColInfoList[foundLinkColIdx].linkCollectionID = linkCollection;
		linkColInfoList[foundLinkColIdx].hasX = 0;
		linkColInfoList[foundLinkColIdx].hasY = 0;
		linkColInfoList[foundLinkColIdx].hasTipSwitch = 0;
		linkColInfoList[foundLinkColIdx].hasContactID = 0;
		linkColInfoList[foundLinkColIdx].hasConfidence = 0;
		linkColInfoList[foundLinkColIdx].hasWidth = 0;
		linkColInfoList[foundLinkColIdx].hasHeight = 0;
		linkColInfoList[foundLinkColIdx].hasPressure = 0;
	}

	return 0;
}

