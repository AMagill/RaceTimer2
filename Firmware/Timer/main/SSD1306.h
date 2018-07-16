#pragma once
#include <array>
#include "driver/i2c.h"
#include "font.h"


class SSD1306
{
public:
	SSD1306(i2c_port_t a_port);
	void Init();
	void PowerOn();
	void PowerOff();
	void AllOn(bool a_on);
	void SendFrame();
	void Clear();
	void SetCursor(int a_x, int a_y);
	void SetFont(const FONT_INFO* a_font, int a_spacing = 0);
	void DrawChar(char a_char);
	void Print(const char* a_format);
	void Printf(const char* a_format, ...);
	void DrawScreen(const uint8_t* a_bitmap);
	
protected:
	i2c_port_t m_i2cPort;
	std::array<uint8_t, 128 * 32 / 8> m_frameBuf;
	int m_curX, m_curY;
	const FONT_INFO* m_curFont;
	int m_spacing;
	
	bool SendCommand(uint8_t a_cmd);
};