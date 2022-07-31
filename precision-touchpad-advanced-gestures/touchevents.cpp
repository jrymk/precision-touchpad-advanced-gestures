#include <stdio.h>

#include "touchevents.h"
#include "termcolor.h"
#include "utils.h"

int interpretRawTouchInput(std::vector<TouchData>& prevTouchPoints, TouchData& currentTouch, TouchEventType* eventType)
{
	if (prevTouchPoints.empty())
	{
		prevTouchPoints.resize(1);
		if (currentTouch.onSurface)
			(*eventType) = TouchEventType::TOUCH_DOWN;
		else
			(*eventType) = TouchEventType::TOUCH_UP;

		return 0;
	}
	else
	{
		for (auto& prevTouch : prevTouchPoints)
		{
			if (prevTouch.touchID == currentTouch.touchID)
			{
				if (prevTouch.onSurface && currentTouch.onSurface)
				{
					if ((prevTouch.x == currentTouch.x) && (prevTouch.y == currentTouch.y))
						(*eventType) = TouchEventType::TOUCH_MOVE_UNCHANGED;
					else
						(*eventType) = TouchEventType::TOUCH_MOVE;
				}
				else if ((prevTouch.onSurface != 0) && (currentTouch.onSurface == 0))
					(*eventType) = TouchEventType::TOUCH_UP;
				else if ((prevTouch.onSurface == 0) && (currentTouch.onSurface != 0))
					(*eventType) = TouchEventType::TOUCH_DOWN;
				else
					(*eventType) = TouchEventType::TOUCH_UP;

				// update touch data
				prevTouch = currentTouch;

				return 0;
			}
		}
	}

	prevTouchPoints.resize(prevTouchPoints.size() + 1, currentTouch);

	if (currentTouch.onSurface)
		(*eventType) = TouchEventType::TOUCH_MOVE;
	else
		(*eventType) = TouchEventType::TOUCH_UP;

	return 0;
}
