#include "Battery.h"
#include "driver/adc.h"
#include "RollingMean.h"

static const adc1_channel_t CHANNEL = ADC1_CHANNEL_7;
static RollingMean<int, 100> g_mean;

void Battery::Init()
{
	ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
	ESP_ERROR_CHECK(adc1_config_channel_atten(CHANNEL, ADC_ATTEN_DB_6));  // 6dB => 2.2V full scale
}

int Battery::GetRaw()
{
	return adc1_get_raw(CHANNEL);	
}

uint8_t Battery::GetScaled(uint8_t a_max)
{
	// At 6dB attenuation and 12-bit width, 4095 = 2.2V
	// Battery voltage is electronically divided in half
	// Measured on two devices:  counts ~= 1090 * volts
	
	static const int minVal = 3597;  // 3.3V
	static const int maxVal = 4033;  // 3.7V
	
	g_mean.Push(adc1_get_raw(CHANNEL));
	int reading = g_mean.GetMean();
	
	if (reading > maxVal)
		return a_max - 1;
	else if (reading < minVal)
		return 0;
	else
		return (reading - minVal) * a_max / (maxVal - minVal);
}
