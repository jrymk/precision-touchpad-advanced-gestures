#pragma once
#ifndef _GESTURE_H_
#define _GESTURE_H_

#include <vector>
#include <chrono>
#include "touchpad.h"
#include "config.h"

struct Stroke {
	std::chrono::steady_clock::time_point beginTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
	std::vector<TouchData> path;
};

extern std::vector<Stroke> activeStroke;

void inputTouchPoints(std::vector<TouchData> touchPoints);


#endif