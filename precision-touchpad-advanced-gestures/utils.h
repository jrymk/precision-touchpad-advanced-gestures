#ifndef __UTILS_H__
#define __UTILS_H__
#include <Windows.h>
#include <vector>
#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

#include <tchar.h>

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
	TCHAR* Name;
	unsigned int cbName;
	std::vector<hidTouchLinkCollectionInfo> LinkColInfoList;
	PHIDP_PREPARSED_DATA PreparedData;
	UINT cbPreparsedData;
	USHORT ContactCountLinkCollection;
};

void mGetLastError();

void print_HidP_errors(NTSTATUS hidpReturnCode);

int findInputDeviceInList(std::vector<hidDeviceInfo>& hidInfoList, TCHAR& deviceName, const unsigned int cbDeviceName, PHIDP_PREPARSED_DATA preparsedData, const UINT cbPreparsedData, unsigned int& foundHidIndex);

int findLinkCollectionInList(std::vector<hidTouchLinkCollectionInfo>& linkColInfoList, USHORT linkCollection, unsigned int& foundLinkColIdx);

#endif  // __UTILS_H__
