#pragma once
#ifndef _TOUCHPAD_H_
#define _TOUCHPAD_H_
#include "utils.h"
#include "touchevents.h"
#include <Windows.h>

bool checkInput(UINT rawInputSize, PRAWINPUT rawInputData, hidDeviceInfo& deviceInfo);

bool readInput(UINT rawInputSize, PRAWINPUT rawInputData, hidDeviceInfo& deviceInfo, TouchData& touchData, int& setRemaining);

#endif