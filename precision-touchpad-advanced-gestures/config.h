#pragma once
#ifndef _CONFIG_H_
#define _CONFIG_H_

struct Config {
	int maxTouchPoints = 5;
	int fingerReadyTimeLimit = 100;
	int fingerReleaseDebouce = 20; // how long can each finger release for it to count as a single stroke
	int allFingerReleaseDebounce = 10; // how long can you release all fingers for it to count as a continious gesture
};

extern Config config;

#endif
