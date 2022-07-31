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
  USHORT LinkColID;
  RECT PhysicalRect;

  // As we cannot identify which link collection contains which data, we need
  // all these flags to identify which data the link collection contains.
  // https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/supporting-usages-in-multitouch-digitizer-drivers

  int hasX;
  int hasY;
  int hasContactID;
  int hasTipSwitch;
  int hasConfidence;
  int hasWidth;
  int hasHeight;
  int hasPressure;
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
