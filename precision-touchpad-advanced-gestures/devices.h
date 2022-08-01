#pragma once
#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <Windows.h>
#include <vector>
#include <string>

#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include "utils.h"

// https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers
// Digitizer Page (0x0D)
//
#define HID_USAGE_DIGITIZER_CONFIDENCE            ((USAGE)0x47)
#define HID_USAGE_DIGITIZER_WIDTH                 ((USAGE)0x48)
#define HID_USAGE_DIGITIZER_HEIGHT                ((USAGE)0x49)
#define HID_USAGE_DIGITIZER_CONTACT_ID            ((USAGE)0x51)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT         ((USAGE)0x54)
#define HID_USAGE_DIGITIZER_CONTACT_COUNT_MAXIMUM ((USAGE)0x55)

int getRawInputDeviceName(HANDLE hDevice, std::wstring& deviceName);
int getRawInputDevicePreparsedData(HANDLE hDevice, PHIDP_PREPARSED_DATA* data, unsigned int* cbSize);
int getRawInputDeviceList(unsigned int& numDevices, RAWINPUTDEVICELIST** deviceList);
int getRawInputData(HRAWINPUT hRawInput, PUINT pcbSize, LPVOID* pData);

extern std::vector<hidDeviceInfo> deviceInfoList;

void parseInputDevices();

int findInputDeviceInList(std::vector<hidDeviceInfo>& hidInfoList, std::wstring& deviceName, PHIDP_PREPARSED_DATA preparsedData, const unsigned int preparsedDataSize, unsigned int& foundHidIndex);

int findLinkCollectionInList(std::vector<hidTouchLinkCollectionInfo>& linkColInfoList, int linkCollection, unsigned int& foundLinkColIdx);

#endif