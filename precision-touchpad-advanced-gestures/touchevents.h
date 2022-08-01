#pragma once
#ifndef _TOUCHEVENTS_H_
#define _TOUCHEVENTS_H_
#include <Windows.h>
#include <vector>

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

int saveTouchInput(std::vector<TouchData>& touchPoints, TouchData& newTouch);

#endif
