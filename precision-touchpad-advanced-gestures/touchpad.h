#pragma once
#ifndef _TOUCHPAD_H_
#define _TOUCHPAD_H_

#include "utils.h"
#include <Windows.h>

enum TouchEventType {
	RELEASED,
	TOUCH_DOWN,
	TOUCH_MOVE,
	TOUCH_UP,
	TOUCH_MOVE_UNCHANGED
};

struct TouchData
{
	int touchID = -1;
	int x = 0;
	int y = 0;
	bool onSurface = false;
	TouchEventType eventType = RELEASED;
};

bool checkInput(UINT rawInputSize, PRAWINPUT rawInputData, hidDeviceInfo& deviceInfo);

bool readInput(UINT rawInputSize, PRAWINPUT rawInputData, hidDeviceInfo& deviceInfo, TouchData& touchData, int& setRemaining);

int saveTouchInput(std::vector<TouchData>& touchPoints, TouchData& newTouch);

#endif