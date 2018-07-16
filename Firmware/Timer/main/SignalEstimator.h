#pragma once
#include <array>

class SignalEstimator
{
static const int      SIZE   = 20;
static const uint32_t WINDOW = SIZE * 110;

public:
	SignalEstimator()
	{
		m_buffer.fill(0);
		m_curIndex = 0;
	}
	
	void Push(uint32_t a_val)
	{
		m_buffer[m_curIndex] = a_val;
		m_curIndex = (m_curIndex + 1) % SIZE;
	}
	
	int GetCount(uint32_t a_time) const
	{
		int sum = 0;
		uint32_t last = 0;
		for (auto item : m_buffer)
		{
			if (item != last && item > (a_time - WINDOW))
			{
				sum++;
				last = item;
			}
		}
		return sum;
	}
	
	int GetScaled(uint32_t a_time, uint8_t a_max)
	{
		const int val = (GetCount(a_time) + (SIZE/a_max) - 1) / (SIZE/a_max);
		if (val >= a_max)
			return a_max - 1;
		else if (val < 0)
			return 0;
		else
			return val;
	}
	
private:
	std::array<uint32_t, SIZE> m_buffer;
	int m_curIndex;
};

