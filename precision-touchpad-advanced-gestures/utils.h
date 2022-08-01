#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_

#include <Windows.h>
#include <vector>
#include <string>
#include <hidusage.h>
#include <hidpi.h>
#pragma comment(lib, "hid.lib")

// https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
static const char RESET_COLOR[] = "\033[0m";

static const char FG_BLACK[] = "\033[30m";
static const char FG_RED[] = "\033[31m";
static const char FG_GREEN[] = "\033[32m";
static const char FG_YELLOW[] = "\033[33m";
static const char FG_BLUE[] = "\033[34m";
static const char FG_MAGENTA[] = "\033[35m";
static const char FG_CYAN[] = "\033[36m";
static const char FG_WHITE[] = "\033[37m";
static const char FG_BRIGHT_BLACK[] = "\033[90m";
static const char FG_BRIGHT_RED[] = "\033[91m";
static const char FG_BRIGHT_GREEN[] = "\033[92m";
static const char FG_BRIGHT_YELLOW[] = "\033[93m";
static const char FG_BRIGHT_BLUE[] = "\033[94m";
static const char FG_BRIGHT_MAGENTA[] = "\033[95m";
static const char FG_BRIGHT_CYAN[] = "\033[96m";
static const char FG_BRIGHT_WHITE[] = "\033[97m";

static const char BG_BLACK[] = "\033[40m";
static const char BG_RED[] = "\033[41m";
static const char BG_GREEN[] = "\033[42m";
static const char BG_YELLOW[] = "\033[43m";
static const char BG_BLUE[] = "\033[44m";
static const char BG_MAGENTA[] = "\033[45m";
static const char BG_CYAN[] = "\033[46m";
static const char BG_WHITE[] = "\033[47m";
static const char BG_BRIGHT_BLACK[] = "\033[100m";
static const char BG_BRIGHT_RED[] = "\033[101m";
static const char BG_BRIGHT_GREEN[] = "\033[102m";
static const char BG_BRIGHT_YELLOW[] = "\033[103m";
static const char BG_BRIGHT_BLUE[] = "\033[104m";
static const char BG_BRIGHT_MAGENTA[] = "\033[105m";
static const char BG_BRIGHT_CYAN[] = "\033[106m";
static const char BG_BRIGHT_WHITE[] = "\033[107m";

static const char SGR_RESET = 0;
static const char SGR_BOLD = 1;
static const char SGR_FAINT = 2;
static const char SGR_ITALIC = 3;
static const char SGR_UNDERLINE = 4;
static const char SGR_SLOW_BLINK = 5;
static const char SGR_RAPID_BLINK = 6;
static const char SGR_INVERT = 7;
static const char SGR_HIDE = 8;
static const char SGR_STRIKE = 9;
static const char SGR_NORMAL = 22;
static const char SGR_UNDERLINE_OFF = 24;
static const char SGR_BLINK_OFF = 25;
static const char SGR_INVERT_OFF = 27;
static const char SGR_HIDE_OFF = 28;
static const char SGR_STRIKE_OFF = 29;

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