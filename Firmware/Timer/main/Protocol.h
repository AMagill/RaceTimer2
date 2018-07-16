#pragma once
#include "esp_now.h"
#include <memory>
#include "freertos/queue.h"


namespace Protocol
{
	struct Message_t
	{
		uint32_t rtcTimeMS;   // Current RTC time
		uint32_t ackTimeMS;   // Last RTC time received from other device
		uint32_t stateTimeMS; // From master: timer value,    from slave: button transition time
		uint8_t  stateFlag;   // From master: timmer running, from slave: button down
	};

	void Init(QueueHandle_t a_queue, const uint8_t a_otherMac[6]);
	void Send(uint32_t a_stateTime, uint8_t a_stateFlag);
}
