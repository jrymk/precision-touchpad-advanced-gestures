#include <Windows.h>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "utils.h"

void getLastError()
{
	DWORD errorCode = GetLastError();
	LPWSTR messageBuffer = NULL;
	size_t size = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

	printf(FG_RED);
	printf("Error Code: %d\n", errorCode);
	wprintf(L"%s\n", messageBuffer);
	printf(RESET_COLOR);
}

void printHidPErrors(NTSTATUS hidpReturnCode)
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