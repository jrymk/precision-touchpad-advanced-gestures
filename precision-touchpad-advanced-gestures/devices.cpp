#include "devices.h"

#include <stdio.h>

std::vector<hidDeviceInfo> deviceInfoList;

int getRawInputDeviceName(HANDLE hDevice, std::wstring& deviceName)
{
	int ret = 0;
	unsigned int winReturnCode;
	unsigned int deviceNameSize;
	winReturnCode = GetRawInputDeviceInfo(_In_ hDevice, RIDI_DEVICENAME, nullptr, &deviceNameSize);
	if (winReturnCode == (unsigned int)-1)
	{
		getLastError();
		exit(-1);
	}
	else
	{
		wchar_t* deviceNamePtr = (wchar_t*)malloc(sizeof(wchar_t) * (deviceNameSize + 1));
		winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_DEVICENAME, deviceNamePtr, &deviceNameSize);
		deviceName.clear();
		deviceName.append(deviceNamePtr, deviceNameSize);

		if (winReturnCode == (unsigned int)-1 || winReturnCode != deviceNameSize)
			exit(-1);

		free(deviceNamePtr);
	}
	return ret;
}

int getRawInputDeviceList(unsigned int& numDevices, RAWINPUTDEVICELIST** deviceList)
{
	int ret = 0;
	unsigned int winReturnCode;

	winReturnCode = GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if (winReturnCode == (unsigned int)-1)
	{
		getLastError();
		exit(-1);
	}
	else
	{
		*deviceList = (RAWINPUTDEVICELIST*)malloc(sizeof(RAWINPUTDEVICELIST) * numDevices);
		winReturnCode = GetRawInputDeviceList(*deviceList, &numDevices, sizeof(RAWINPUTDEVICELIST));
		if (winReturnCode == (unsigned int)-1)
		{
			getLastError();
			exit(-1);
		}
	}

	return ret;
}

int getRawInputDevicePreparsedData(_In_ HANDLE hDevice, _Out_ PHIDP_PREPARSED_DATA* data, _Out_ unsigned int* cbSize)
{
	int ret = 0;
	unsigned int winReturnCode;

	winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, cbSize);
	if (winReturnCode == (unsigned int)-1)
	{
		ret = -1;
		printf(FG_RED);
		printf("GetRawInputDeviceInfo failed\n");
		printf(RESET_COLOR);
		getLastError();

		exit(-1);
	}
	else
	{
		(*data) = (PHIDP_PREPARSED_DATA)malloc((*cbSize));

		winReturnCode = GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, (*data), cbSize);
		if (winReturnCode == (unsigned int)-1)
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputDeviceInfo failed\n");
			printf(RESET_COLOR);
			getLastError();

			exit(-1);
		}
	}
	return ret;
}

int getRawInputData(_In_ HRAWINPUT hRawInput, PUINT pcbSize, _Out_ LPVOID* pData)
{
	int ret = 0;
	unsigned int winReturnCode;

	winReturnCode = GetRawInputData(hRawInput, RID_INPUT, NULL, pcbSize, sizeof(RAWINPUTHEADER));
	if (winReturnCode == (unsigned int)-1)
	{
		ret = -1;
		printf(FG_RED);
		printf("GetRawInputData failed\n");
		printf(RESET_COLOR);
		getLastError();

		exit(-1);
	}
	else
	{
		(*pData) = (LPVOID)malloc((*pcbSize));
		winReturnCode = GetRawInputData(hRawInput, RID_INPUT, (*pData), pcbSize, sizeof(RAWINPUTHEADER));

		if (winReturnCode == (unsigned int)-1)
		{
			ret = -1;
			printf(FG_RED);
			printf("GetRawInputData failed\n");
			printf(RESET_COLOR);
			getLastError();

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
	return ret;
}

void parseInputDevices()
{
	printf(FG_BLUE);
	printf("Scanning HID devices...\n");
	printf(RESET_COLOR);

	unsigned int numDevices;
	RAWINPUTDEVICELIST* rawInputDeviceList = nullptr;

	getRawInputDeviceList(numDevices, &rawInputDeviceList);

	printf("Number of raw input devices: %d\n", numDevices);
	for (UINT deviceIndex = 0; deviceIndex < numDevices; deviceIndex++)
	{
		printf(BG_GREEN);
		printf("Device #%d:", deviceIndex);
		printf(RESET_COLOR);
		printf(" ");
		RAWINPUTDEVICELIST rawInputDevice = rawInputDeviceList[deviceIndex];
		if (rawInputDevice.dwType != RIM_TYPEHID)
		{
			printf("Not HID device\n");
			continue;
		}

		// get preparsed data for HidP
		UINT cbDataSize = 0;
		PHIDP_PREPARSED_DATA preparsedData = NULL;

		getRawInputDevicePreparsedData(rawInputDevice.hDevice, &preparsedData, &cbDataSize);

		NTSTATUS hidpReturnCode;

		HIDP_CAPS capability;
		hidpReturnCode = HidP_GetCaps(preparsedData, &capability);
		if (hidpReturnCode != HIDP_STATUS_SUCCESS)
		{
			printHidPErrors(hidpReturnCode);
			exit(-1);
		}

		int isButtonCapsEmpty = (capability.NumberInputButtonCaps == 0);

		if (!isButtonCapsEmpty)
		{
			std::wstring deviceName;

			getRawInputDeviceName(rawInputDevice.hDevice, deviceName);

			wprintf(deviceName.c_str());

			printf("\nvalue capabilities:  %d  ", capability.NumberInputValueCaps);
			if (capability.NumberInputValueCaps >= 2) {
				printf(BG_YELLOW);
				printf(FG_BLACK);
				printf("Nice! That looks like a touchpad");
				printf(RESET_COLOR);
			}

			printf("\nbutton capabilities: %d  \n", capability.NumberInputButtonCaps);

			unsigned int foundHidIdx;
			int returnCode = findInputDeviceInList(deviceInfoList, deviceName, preparsedData, cbDataSize, foundHidIdx);
			if (returnCode != 0)
			{
				printf("findInputDeviceInList failed\n");
				exit(-1);
			}

			// contains value as input
			if (capability.NumberInputValueCaps != 0)
			{
				const USHORT numValueCaps = capability.NumberInputValueCaps;
				USHORT _numValueCaps = numValueCaps;

				PHIDP_VALUE_CAPS valueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * numValueCaps);

				hidpReturnCode = HidP_GetValueCaps(HidP_Input, valueCaps, &_numValueCaps, preparsedData);
				if (hidpReturnCode != HIDP_STATUS_SUCCESS)
					printHidPErrors(hidpReturnCode);

				for (USHORT valueCapIndex = 0; valueCapIndex < numValueCaps; valueCapIndex++)
				{
					HIDP_VALUE_CAPS cap = valueCaps[valueCapIndex];

					if (cap.IsRange || !cap.IsAbsolute)
						continue;

					unsigned int foundLinkColIdx;
					int returnCode = findLinkCollectionInList(deviceInfoList[foundHidIdx].linkCollectionInfoList, cap.LinkCollection, foundLinkColIdx);
					if (returnCode != 0)
					{
						printf("findLinkCollectionInList failed\n");
						exit(-1);
					}

					printf(FG_CYAN);
					printf("[value capability]  foundLinkCollectionIndex: %d\n", foundLinkColIdx);
					printf(RESET_COLOR);

					if (cap.UsagePage == HID_USAGE_PAGE_GENERIC)
					{
						printf(FG_BRIGHT_MAGENTA);
						printf("Link collection: %d\n", cap.LinkCollection);

						if (cap.NotRange.Usage == HID_USAGE_GENERIC_X)
						{
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].hasX = true;
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].physicalRect.left = cap.PhysicalMin;
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].physicalRect.right = cap.PhysicalMax;
							printf("  X min: %d  max: %d\n", cap.PhysicalMin, cap.PhysicalMax);
						}
						else if (cap.NotRange.Usage == HID_USAGE_GENERIC_Y)
						{
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].hasY = true;
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].physicalRect.top = cap.PhysicalMin;
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].physicalRect.bottom = cap.PhysicalMax;
							printf("  Y min: %d  max: %d\n", cap.PhysicalMin, cap.PhysicalMax);
						}
					}
					else if (cap.UsagePage == HID_USAGE_PAGE_DIGITIZER)
					{
						if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_ID)
							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].hasContactID = true;
						else if (cap.NotRange.Usage == HID_USAGE_DIGITIZER_CONTACT_COUNT)
							deviceInfoList[foundHidIdx].contactCountLinkCollection = cap.LinkCollection;
					}
				}
				free(valueCaps);
			}

			if (capability.NumberInputButtonCaps != 0)
			{
				const USHORT numButtonCaps = capability.NumberInputButtonCaps;
				USHORT _numButtonCaps = numButtonCaps;

				PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * numButtonCaps);

				hidpReturnCode = HidP_GetButtonCaps(HidP_Input, buttonCaps, &_numButtonCaps, preparsedData);
				if (hidpReturnCode != HIDP_STATUS_SUCCESS)
				{
					printHidPErrors(hidpReturnCode);
					exit(-1);
				}

				for (USHORT buttonCapIndex = 0; buttonCapIndex < numButtonCaps; buttonCapIndex++)
				{
					HIDP_BUTTON_CAPS buttonCap = buttonCaps[buttonCapIndex];

					if (buttonCap.IsRange)
						continue;

					printf(FG_BLUE);
					printf("[button capability] Index: %d, UsagePage: %d, Usage: %d, DIGITIZER: %d, IsRange: %d\n", buttonCapIndex, buttonCap.UsagePage, buttonCap.NotRange.Usage, buttonCap.UsagePage, buttonCap.IsRange);
					printf(RESET_COLOR);

					if (buttonCap.UsagePage == HID_USAGE_PAGE_DIGITIZER)
					{
						if (buttonCap.NotRange.Usage == HID_USAGE_DIGITIZER_TIP_SWITCH)
						{
							unsigned int foundLinkColIdx;
							int returnCode = findLinkCollectionInList(deviceInfoList[foundHidIdx].linkCollectionInfoList, buttonCap.LinkCollection, foundLinkColIdx);

							printf(FG_GREEN);
							printf("[button capability] foundLinkCollectionIndex: %d\n", foundLinkColIdx);
							printf(RESET_COLOR);

							deviceInfoList[foundHidIdx].linkCollectionInfoList[foundLinkColIdx].hasTipSwitch = 1;
						}
					}
				}
				free(buttonCaps);
			}
		}
		free(preparsedData);
	}
	free(rawInputDeviceList);

	if (deviceInfoList.empty())
	{
		printf(FG_RED);
		printf("Failed to parse input devices!\n");
		printf(RESET_COLOR);
		//return -1;
	}
}

int findInputDeviceInList(std::vector<hidDeviceInfo>& hidInfoList, std::wstring& deviceName, PHIDP_PREPARSED_DATA preparsedData, const unsigned int preparsedDataSize, unsigned int& foundHidIndex)
{
	foundHidIndex = (unsigned int)-1;
	if (hidInfoList.empty())
	{
		foundHidIndex = 0;
		hidInfoList.resize(1);

		hidInfoList[0].preparsedData = preparsedData;
		hidInfoList[0].name = deviceName;
		hidInfoList[0].contactCountLinkCollection = (unsigned int)-1;
		hidInfoList[0].preparsedDataSize = preparsedDataSize;
		hidInfoList[0].preparsedData = (PHIDP_PREPARSED_DATA)malloc(preparsedDataSize);
		memcpy(hidInfoList[0].preparsedData, preparsedData, preparsedDataSize);
	}
	else
	{
		for (unsigned int touchpadIndex = 0; touchpadIndex < hidInfoList.size(); touchpadIndex++)
		{
			int compareNameResult = deviceName.compare(hidInfoList[touchpadIndex].name);
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

		hidInfoList[foundHidIndex].linkCollectionInfoList.clear();
		hidInfoList[foundHidIndex].preparsedDataSize = preparsedDataSize;

		hidInfoList[foundHidIndex].name = deviceName;
		hidInfoList[foundHidIndex].preparsedData = (PHIDP_PREPARSED_DATA)malloc(preparsedDataSize);
		memcpy(hidInfoList[foundHidIndex].preparsedData, preparsedData, preparsedDataSize);
	}

	return 0;
}

int findLinkCollectionInList(std::vector<hidTouchLinkCollectionInfo>& linkColInfoList, int linkCollection, unsigned int& foundLinkColIdx)
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