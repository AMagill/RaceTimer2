#include "SSD1306.h"
#include <cstdarg>
#include <string.h>

static const uint8_t LCD_WIDTH              = 128;
static const uint8_t LCD_HEIGHT             = 32;

static const uint8_t I2C_ADDR               = 0x3C << 1;
static const uint8_t CMD_DISPLAYOFF         = 0xAE;
static const uint8_t CMD_DISPLAYON          = 0xAF;
static const uint8_t CMD_SETDISPLAYCLOCKDIV = 0xD5;
static const uint8_t CMD_SETMULTIPLEX       = 0xA8;
static const uint8_t CMD_CHARGEPUMP         = 0x8D;
static const uint8_t CMD_MEMORYMODE         = 0x20;
static const uint8_t CMD_SEGREMAP           = 0xA0;
static const uint8_t CMD_COMSCANDEC         = 0xC8;
static const uint8_t CMD_SETCOMPINS         = 0xDA;
static const uint8_t CMD_SETCONTRAST        = 0x81;
static const uint8_t CMD_SETPRECHARGE       = 0xD9;
static const uint8_t CMD_SETVCOMDETECT      = 0xDB;
static const uint8_t CMD_COLUMNADDR         = 0x21;
static const uint8_t CMD_PAGEADDR           = 0x22;


SSD1306::SSD1306(i2c_port_t a_port) : m_i2cPort(a_port)
{
}


void SSD1306::Init()
{
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = GPIO_NUM_23;
	conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
	conf.scl_io_num = GPIO_NUM_22;
	conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
	conf.master.clk_speed = 1000000;
	i2c_param_config(m_i2cPort, &conf);
	i2c_driver_install(m_i2cPort, conf.mode, 0, 0, 0);
}


void SSD1306::PowerOn()
{
	SendCommand(CMD_DISPLAYOFF);
	
	SendCommand(CMD_SETDISPLAYCLOCKDIV);
	SendCommand(0x80);

	SendCommand(CMD_SETMULTIPLEX);
	SendCommand(LCD_HEIGHT - 1);

	SendCommand(CMD_CHARGEPUMP);
	SendCommand(0x14);  // Enable = 1
	
	SendCommand(CMD_MEMORYMODE);
	SendCommand(0x00);  // Horizontal mode
	
	SendCommand(CMD_SEGREMAP | 0x01);
	
	SendCommand(CMD_COMSCANDEC);

	SendCommand(CMD_SETCOMPINS);
	SendCommand(0x02);

	SendCommand(CMD_SETCONTRAST);
	SendCommand(0x8F);

	SendCommand(CMD_SETPRECHARGE);
	SendCommand(0xF1);

	SendCommand(CMD_SETVCOMDETECT);
	SendCommand(0x40);
		
	SendCommand(CMD_DISPLAYON);
}


void SSD1306::PowerOff()
{
	SendCommand(CMD_DISPLAYOFF);
	
	SendCommand(CMD_CHARGEPUMP);
	SendCommand(0x10);  // Enable = 0
}


bool SSD1306::SendCommand(uint8_t a_cmd)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, I2C_ADDR | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, 0x00,  true);  // Continuation = 0, Data/#Command = 0
	i2c_master_write_byte(cmd, a_cmd, true);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(m_i2cPort, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret == ESP_OK;
}


void SSD1306::AllOn(bool a_on)
{
	SendCommand(a_on ? 0xA5 : 0xA4);
}


void SSD1306::SendFrame()
{
	SendCommand(CMD_COLUMNADDR);
	SendCommand(0);              // Column start address
	SendCommand(LCD_WIDTH - 1);  // Column end   address

	SendCommand(CMD_PAGEADDR);
	SendCommand(0);              // Page start address
	SendCommand(3);              // Page end   address
	
	for (uint16_t i=0; i<(LCD_WIDTH*LCD_HEIGHT/8); i++) 
	{
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, I2C_ADDR | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, 0x40, true);   // Continuation = 0, Data/#Command = 1
		for (uint8_t x=0; x<16; x++) 
		{
			i2c_master_write_byte(cmd, m_frameBuf[i], true);
			i++;
		}
		i--;
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(m_i2cPort, cmd, 1000 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
	}

}


void SSD1306::Clear()
{
	m_frameBuf.fill(0);
}


void SSD1306::SetCursor(int a_x, int a_y)
{
	m_curX = a_x;
	m_curY = a_y;
}


void SSD1306::SetFont(const FONT_INFO* a_font, int a_spacing)
{
	m_curFont = a_font;
	m_spacing = a_spacing;
}


void SSD1306::DrawChar(char a_char)
{
	if (a_char == ' ')
	{
		m_curX += m_curFont->spacePixels;    // Drawing a space is easy!
		return;
	}

	if (a_char < m_curFont->startChar || a_char > m_curFont->endChar)
		return;  // Not in font!
	
	const FONT_CHAR_INFO& charInfo = m_curFont->charInfo[a_char - m_curFont->startChar];
	if (charInfo.widthBits == 0)
		return;  // Not in font or empty
	
	const uint8_t* charBits = &m_curFont->data[charInfo.offset];
	for (int page = 0; page < m_curFont->heightPages; page++)
	{
		if ((m_curY + page) > (LCD_HEIGHT / 8))
			break;  // Don't run off the bottom
		
		uint8_t* bufBits = &m_frameBuf[m_curX + (m_curY + page) * LCD_WIDTH];
		for (int col = 0; col < charInfo.widthBits; col++)
		{
			if ((m_curX + col) >= LCD_WIDTH)
				break;  // Don't run off the side
			
			*bufBits++ |= *charBits++;
		}
	}
	
	m_curX += charInfo.widthBits + m_spacing;
}

void SSD1306::Print(const char* a_format)
{
	for (int i = 0; a_format[i] != 0 && m_curX < LCD_WIDTH; i++)
		DrawChar(a_format[i]);	
}

void SSD1306::Printf(const char* a_format, ...)
{
	static char buf[128];
	
	va_list argptr;
	va_start(argptr, a_format);
	vsnprintf(buf, sizeof(buf), a_format, argptr);
	va_end(argptr);
	
	Print(buf);
}

void SSD1306::DrawScreen(const uint8_t* a_bitmap)
{
	memcpy(m_frameBuf.data(), a_bitmap, LCD_WIDTH*LCD_HEIGHT / 8);
}