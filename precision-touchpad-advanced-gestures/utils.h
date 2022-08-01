#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_
#include <Windows.h>
#include <vector>
#include <string>
#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

struct hidTouchLinkCollectionInfo
{
	USHORT linkCollectionID;
	RECT physicalRect;

	// As we cannot identify which link collection contains which data, we need
	// all these flags to identify which data the link collection contains.
	// https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

	bool hasX;
	bool hasY;
	bool hasContactID;
	bool hasTipSwitch;
	bool hasConfidence;
	bool hasWidth;
	bool hasHeight;
	bool hasPressure;
};

struct hidDeviceInfo
{
	std::wstring name;
	std::vector<hidTouchLinkCollectionInfo> linkCollectionInfoList;
	PHIDP_PREPARSED_DATA preparsedData = nullptr;
	unsigned int preparsedDataSize = 0;
	unsigned int contactCountLinkCollection = 0;
};

void getLastError();

void printHidPErrors(NTSTATUS hidpReturnCode);

#endif