#include "gesture.h"
#include "touchpad.h"

std::vector<Stroke> activeStroke(MAX_TOUCH_POINTS);

std::chrono::steady_clock::time_point gestureBeginTime = std::chrono::steady_clock::now();

void inputTouchPoints(std::vector<TouchData> touchPoints)
{
	int activePoints = 0;
	byte activePointsFlag = 0;
	int prevActivePoints = 0;
	byte prevActivePointsFlag = 0;

	for (int touchId = 0; touchId < MAX_TOUCH_POINTS; touchId++) {
		TouchData& touchData = touchPoints[touchId];
		Stroke& stroke = activeStroke[touchId];

		if (touchData.onSurface) {
			if (stroke.path.empty()) {
				// new stroke
				stroke.beginTime = std::chrono::steady_clock::now();
			}

			stroke.path.push_back(touchData);
		}
		else {
			stroke.path.clear();
			stroke.endTime = std::chrono::steady_clock::now();
		}
	}



	/*for (int i = 0; i < touchPoints.size(); i++) {
		if (touchPoints[i].onSurface) {
			activePoints++;
			activePointsFlag |= (1 << i);
		}
		if (activeStroke[i].path.size() > 0) {
			prevActivePoints++;
			prevActivePointsFlag |= (1 << i);
		}
	}

	if (prevActivePoints == 0 && activePoints > 0) {
		// begin of gesture
		gestureBeginTime = std::chrono::steady_clock::now();
	}

	for (int i = 0; i < touchPoints.size(); i++)
		activeStroke[i].path.push_back(touchPoints[i]);

	int elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - gestureBeginTime).count();

	//printf("cnt: %d  time: %.3f  ", activePoints, elapsedMillis / 1000.f);


	if (activePoints == 0) {
		// end of gesture

		// 		activeStroke.clear();
	}*/
}
