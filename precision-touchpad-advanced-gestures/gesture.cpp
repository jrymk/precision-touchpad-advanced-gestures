#include "gesture.h"
#include "touchpad.h"
#include "config.h"
# define M_PI 3.14159265358979323846

std::vector<Stroke> activeStroke(config.maxTouchPoints);

std::chrono::steady_clock::time_point gestureBeginTime = std::chrono::steady_clock::now();

void inputTouchPoints(std::vector<TouchData> touchPoints)
{
	int activePoints = 0;
	byte activePointsFlag = 0;
	int prevActivePoints = 0;
	byte prevActivePointsFlag = 0;

	for (int touchId = 0; touchId < config.maxTouchPoints; touchId++) {
		TouchData& touchData = touchPoints[touchId];
		Stroke& stroke = activeStroke[touchId];

		// time point
		if (touchData.onSurface) {
			if (stroke.path.empty()) {
				// new stroke
				stroke.beginTime = std::chrono::steady_clock::now();
			}

			stroke.path.push_back(touchData);
		}
		else {
			activeStroke[touchId].path.clear();
			//activeStroke[touchId].path.shrink_to_fit();
			//std::vector<TouchData>().swap(activeStroke[touchId].path);
			stroke.endTime = std::chrono::steady_clock::now();
		}

		if (stroke.path.size() > 0) {
			// distance and angle
			double deltaX = touchData.x - stroke.path[0].x;
			double deltaY = touchData.y - stroke.path[0].y;
			stroke.distance = sqrt(deltaX * deltaX + deltaY * deltaY);
			stroke.angle = acos(deltaX / stroke.distance) * ((deltaY < 0) ? (double)1 : (double)-1);
			stroke.cardinal = angleToCardinal(stroke.angle);
			stroke.ordinal = angleToOrdinal(stroke.angle);
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

int angleToCardinal(double angle)
{
	return int((angle + M_PI / 4) / (M_PI / 2) + 4) % 4;
}

int angleToOrdinal(double angle)
{
	int temp = int((angle + M_PI / 8) / (M_PI / 4) + 8) % 8;
	return temp / 2 + (temp % 2 ? 4 : 0);
}
