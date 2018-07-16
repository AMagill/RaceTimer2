#pragma once
#include <stdint.h>

namespace Battery
{
	void    Init();
	int     GetRaw();
	uint8_t GetScaled(uint8_t a_max);
}