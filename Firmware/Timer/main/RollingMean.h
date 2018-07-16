#pragma once
#include <array>

template <typename T, int SIZE>
class RollingMean
{
public:
	RollingMean()
	{
		m_buf.fill(0);
		m_index = 0;
		m_total = 0;
	}
	
	void Push(T a_val)
	{
		m_total -= m_buf[m_index];
		m_total += a_val;
		m_buf[m_index] = a_val;
		m_index = (m_index + 1) % SIZE;
	}
	
	T GetMean() const
	{
		return m_total / SIZE;
	}
	
	
private:
	std::array<T, SIZE> m_buf;
	int m_index;
	int m_total;
};