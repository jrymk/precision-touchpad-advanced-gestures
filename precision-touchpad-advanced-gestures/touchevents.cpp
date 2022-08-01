#include <stdio.h>

#include "touchevents.h"
#include "termcolor.h"
#include "utils.h"

int saveTouchInput(std::vector<TouchData>& touchPoints, TouchData& newTouch)
{
	// make sure touch points fits the new touch id
	if (newTouch.touchID >= touchPoints.size())
		touchPoints.resize(newTouch.touchID + 1);

	if (touchPoints.empty())
	{
		touchPoints.resize(1);
		if (newTouch.onSurface)
			newTouch.eventType = TOUCH_DOWN;
		else
			newTouch.eventType = TOUCH_UP;

		return 0;
	}
	else
	{
		for (auto& prevTouch : touchPoints)
		{
			if (prevTouch.touchID == newTouch.touchID)
			{
				if (prevTouch.onSurface && newTouch.onSurface)
				{
					if ((prevTouch.x == newTouch.x) && (prevTouch.y == newTouch.y))
						newTouch.eventType = TouchEventType::TOUCH_MOVE_UNCHANGED;
					else
						newTouch.eventType = TouchEventType::TOUCH_MOVE;
				}
				else if (prevTouch.onSurface && !newTouch.onSurface)
					newTouch.eventType = TouchEventType::TOUCH_UP;
				else if (!prevTouch.onSurface && newTouch.onSurface)
					newTouch.eventType = TouchEventType::TOUCH_DOWN;
				else
					newTouch.eventType = TouchEventType::RELEASED; // this shouldn't be the correct way to do so

				// update touch data
				prevTouch = newTouch;

				return 0;
			}
		}
	}

	touchPoints[newTouch.touchID] = newTouch;

	return 0;
}
