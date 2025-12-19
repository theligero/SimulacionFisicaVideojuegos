#ifndef HUD_H_
#define HUD_H_

#pragma once

#include <string>

class HUD {
public:
	static void Draw(int score, double time, const std::string& msg);
};

#endif // HUD_H_