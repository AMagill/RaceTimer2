#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

enum class EventType
{
	RX_BUTTON_DOWN,
	RX_BUTTON_UP,
	RX_RTC,
	RX_ACK,
	TIMER_SEND,
	BUTTON_DOWN,
	BUTTON_UP,
};
struct Event_t
{
	EventType type;
	uint32_t  time;
};
