#include "touchpad.h"
#include "devices.h"
#include "termcolor.h"

#include <string>

bool checkInput(UINT rawInputSize, PRAWINPUT rawInputData, hidDeviceInfo& deviceInfo) {
	if (rawInputData->header.dwType != RIM_TYPEHID)
		return false;

	DWORD count = rawInputData->data.hid.dwCount;
	BYTE* rawData = rawInputData->data.hid.bRawData;

	if (count == 0)
		return false;

	std::wstring deviceName;
	getRawInputDeviceName(rawInputData->header.hDevice, deviceName);

	unsigned int foundHidIdx = (unsigned int)-1;

	for (unsigned int touchpadIndex = 0; touchpadIndex < deviceInfoList.size(); touchpadIndex++)
	{
		int compareNameResult = deviceName.compare(deviceInfoList[touchpadIndex].name);
		if (compareNameResult == 0)
		{
			//wprintf(deviceInfoList[touchpadIndex].name.c_str());
			foundHidIdx = touchpadIndex;
			deviceInfo = deviceInfoList[foundHidIdx];

			if (deviceInfo.linkCollectionInfoList.empty())
			{
				printf(FG_RED);
				printf("Cannot find any LinkCollection(s). Try parse the PREPARED_DATA may help. TODO\n");
				printf(RESET_COLOR);
				return false;
			}
			else if (deviceInfo.preparsedData == nullptr)
			{
				printf(FG_RED);
				printf("Cannot find PreparsedData\n");
				printf(RESET_COLOR);
				return false;
			}
			return true;
		}
	}

	return false;
}

bool readInput(UINT rawInputSize, PRAWINPUT rawInputData, hidDeviceInfo& deviceInfo, TouchData& touchData, int& setRemaining) {
	NTSTATUS hidpReturnCode;
	ULONG usageValue;
	PHIDP_PREPARSED_DATA preparsedHIDData = deviceInfo.preparsedData;

	if (deviceInfo.contactCountLinkCollection == (USHORT)-1)
	{
		printf(FG_RED);
		printf("Cannot find contact count Link Collection!\n");
		printf(RESET_COLOR);
		return false;
	}

	hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, deviceInfo.contactCountLinkCollection, HID_USAGE_DIGITIZER_CONTACT_COUNT, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

	if (hidpReturnCode != HIDP_STATUS_SUCCESS)
	{
		printf(FG_RED);
		printf("Failed to read number of contacts!\n");
		printf(RESET_COLOR);
		printHidPErrors(hidpReturnCode);
		return false;
	}

	ULONG numContacts = usageValue;

	if (numContacts > 0 || setRemaining == 0) {
		setRemaining = numContacts - 1;
		/*printf(FG_BRIGHT_BLUE);
		printf("numContacts: %d\n", numContacts);
		printf(RESET_COLOR);*/
	}
	else
		setRemaining--;

	hidTouchLinkCollectionInfo collectionInfo = deviceInfo.linkCollectionInfoList[0];

	if (collectionInfo.hasX && collectionInfo.hasY && collectionInfo.hasContactID && collectionInfo.hasTipSwitch)
	{
		hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.linkCollectionID, 0x30, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

		if (hidpReturnCode != HIDP_STATUS_SUCCESS)
		{
			printf(FG_RED);
			printf("Failed to read x position\n");
			printf(RESET_COLOR);
			printHidPErrors(hidpReturnCode);
			return false;
		}

		ULONG xPos = usageValue;

		hidpReturnCode = HidP_GetUsageValue(HidP_Input, 0x01, collectionInfo.linkCollectionID, 0x31, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
		if (hidpReturnCode != HIDP_STATUS_SUCCESS)
		{
			printf(FG_RED);
			printf("Failed to read y position\n");
			printf(RESET_COLOR);
			printHidPErrors(hidpReturnCode);
			return false;
		}

		ULONG yPos = usageValue;

		hidpReturnCode = HidP_GetUsageValue(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.linkCollectionID, HID_USAGE_DIGITIZER_CONTACT_ID, &usageValue, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);
		if (hidpReturnCode != HIDP_STATUS_SUCCESS)
		{
			printf(FG_RED);
			printf("Failed to read touch ID\n");
			printf(RESET_COLOR);
			printHidPErrors(hidpReturnCode);
			return false;
		}

		ULONG touchId = usageValue;
		const ULONG maxNumButtons = HidP_MaxUsageListLength(HidP_Input, HID_USAGE_PAGE_DIGITIZER, preparsedHIDData);
		ULONG _maxNumButtons = maxNumButtons;
		USAGE* buttonUsageArray = (USAGE*)malloc(sizeof(USAGE) * maxNumButtons);

		hidpReturnCode = HidP_GetUsages(HidP_Input, HID_USAGE_PAGE_DIGITIZER, collectionInfo.linkCollectionID, buttonUsageArray, &_maxNumButtons, preparsedHIDData, (PCHAR)rawInputData->data.hid.bRawData, rawInputData->data.hid.dwSizeHid);

		if (hidpReturnCode != HIDP_STATUS_SUCCESS)
		{
			printf(FG_RED);
			printf("HidP_GetUsages failed!\n");
			printf(RESET_COLOR);
			printHidPErrors(hidpReturnCode);
			return false;
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
	
		touchData.touchID = touchId;
		touchData.x = xPos;
		touchData.y = yPos;
		touchData.onSurface = isContactOnSurface;
		return true;
	}
	return false;
}