#ifndef __TOUCHEVENTS_H__
#define __TOUCHEVENTS_H__
#include <Windows.h>
#include <vector>

enum TouchEventType {
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
};

int interpretRawTouchInput(std::vector<TouchData>& prevTouchPoints, TouchData& currentTouch, TouchEventType* eventType);

#endif  // __TOUCHEVENTS_H__
