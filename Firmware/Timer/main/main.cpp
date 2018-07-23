#include "main.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_pm.h"
#include "rom/ets_sys.h"
#include "rom/crc.h"
#include "tcpip_adapter.h"
#include "Protocol.h"
#include "Button.h"
#include "Battery.h"
#include "SSD1306.h"
#include "SignalEstimator.h"
#include "font_openSansCondensed_11pt.h"
#include "font_openSansCondensed_32pt.h"
#include "font_myriadProCond_6pt.h"
#include "font_symbols.h"
#include "Bitmaps.h"

static const uint8_t MASTER_MAC[6] = { 0x30, 0xAE, 0xA4, 0x1C, 0x0C, 0xB8 };  // I  - Red   button
static const uint8_t SLAVE_MAC[6]  = { 0x30, 0xAE, 0xA4, 0x23, 0xCC, 0x10 };  // II - Green button
static const TickType_t DISPLAY_IDLE_TICKS   = 100 / portTICK_PERIOD_MS;
static const TickType_t DISPLAY_ACTIVE_TICKS = 31  / portTICK_PERIOD_MS;
static const TickType_t SEND_TICKS           = 100 / portTICK_PERIOD_MS;

TaskHandle_t  mainTask;
QueueHandle_t eventQueue;
TimerHandle_t sendTimer;
TimerHandle_t displayTimer;


static void onSendTimer(TimerHandle_t a_timer)
{
	Event_t event;
	event.type = EventType::TIMER_SEND;
	xQueueSend(eventQueue, &event, portMAX_DELAY);
}

static void onDisplayTimer(TimerHandle_t a_timer)
{
	Event_t event;
	event.type = EventType::TIMER_DISPLAY;
	xQueueSend(eventQueue, &event, portMAX_DELAY);
}


static void taskMain(void* a_context)
{
	bool     isMaster          = false;
	uint32_t timerValueMS      = 0;
	bool     timerRunning      = false;
	uint32_t lastRxStateTimeMS = 0;
	uint32_t buttonTimeMS      = 0;
	bool     buttonDown        = false;
	int32_t  clockSkew         = 0;
	uint32_t lastEventTimeMS   = 0;
	SignalEstimator signalEstimate;
	
	{
		ESP_LOGI("Main", "Configured master:  %02X:%02X:%02X:%02X:%02X:%02X",
			MASTER_MAC[0], MASTER_MAC[1], MASTER_MAC[2], MASTER_MAC[3], MASTER_MAC[4], MASTER_MAC[5]);		
		ESP_LOGI("Main", "Configured slave:   %02X:%02X:%02X:%02X:%02X:%02X",
			SLAVE_MAC[0], SLAVE_MAC[1], SLAVE_MAC[2], SLAVE_MAC[3], SLAVE_MAC[4], SLAVE_MAC[5]);		

		uint8_t myMac[6];
		esp_read_mac(myMac, ESP_MAC_WIFI_STA);
		ESP_LOGI("Main", "My MAC:  %02X:%02X:%02X:%02X:%02X:%02X",
			myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);		

		if (memcmp(myMac, MASTER_MAC, sizeof(myMac)) == 0)
		{
			ESP_LOGI("Main", "MAC matches master.");
			isMaster = true;
		}
		else if (memcmp(myMac, SLAVE_MAC, sizeof(myMac)) == 0)
		{
			ESP_LOGI("Main", "MAC matches slave.");
		}
		else
		{
			ESP_LOGE("Main", "MAC doesn't match!");
		}
	}
	
	SSD1306 oled(I2C_NUM_0);
	oled.Init();
	oled.PowerOn();
	
	Protocol::Init(eventQueue, isMaster ? SLAVE_MAC : MASTER_MAC);
	Button::Init(eventQueue);
	Battery::Init();
	
	// Main loop
	while (true)
	{
		Event_t event;
		while (xQueueReceive(eventQueue, &event, portMAX_DELAY))
		{
			bool lastTimerRunning = timerRunning;
			uint32_t now = esp_timer_get_time() / 1000;
		
			switch (event.type)
			{
			case EventType::RX_RTC:
				clockSkew = now - event.time;
				if (lastRxStateTimeMS > event.time) // Our timestamp for green's last button event is in green's future.  It probably restarted.
					lastRxStateTimeMS = 0;
				break;
			case EventType::RX_ACK:
				signalEstimate.Push(event.time);
				break;
			case EventType::RX_BUTTON_DOWN:
				if (isMaster)
				{
					if (lastRxStateTimeMS != 0 && event.time > lastRxStateTimeMS)
					{
						 // The green button's state changed to DOWN.  (First message doesn't count)
						timerRunning = false;
						timerValueMS = 0;
						lastEventTimeMS = now;
					}
					lastRxStateTimeMS = event.time;
				}
				else  // Slave
					{
						timerRunning = true;
						timerValueMS = event.time + clockSkew;
					}
				break;
			case EventType::RX_BUTTON_UP:
				if (isMaster)
				{
					if (lastRxStateTimeMS != 0 && event.time > lastRxStateTimeMS)
					{
						 // The green button's state changed to UP.  (First message doesn't count)
						timerRunning = true;
						timerValueMS = now;
						lastEventTimeMS = now;
					}
					lastRxStateTimeMS = event.time;
				}
				else  // Slave
					{
						if (timerRunning)
							lastEventTimeMS = now;
						timerRunning = false;
						timerValueMS = event.time;
					}
				break;
			case EventType::BUTTON_DOWN:
				ESP_LOGI("Main", "Button down");
				lastEventTimeMS = now;
				buttonDown   = true;
				buttonTimeMS = event.time;   // Not 'now', because the debounce algorithm can send events from the past
				if(isMaster && timerRunning)  // Press down stops
				{
					timerRunning = false;
					timerValueMS = buttonTimeMS - timerValueMS;
				}
				if (isMaster)
					oled.PowerOn();
				break;
			case EventType::BUTTON_UP:
				ESP_LOGI("Main", "Button up");
				lastEventTimeMS = now;
				buttonDown   = false;
				buttonTimeMS = event.time;   // Not 'now', because the debounce algorithm can send events from the past
				if (!isMaster)
					oled.PowerOn();
				break;
			case EventType::TIMER_SEND:
				if (isMaster)
					Protocol::Send(timerValueMS, timerRunning);
				else
					Protocol::Send(buttonTimeMS, buttonDown);
				break;
			case EventType::TIMER_DISPLAY:
				{
					const int totalMS     = timerRunning ? (now - timerValueMS) : timerValueMS;
					const int min         = (totalMS / 60000) % 100;
					const int sec         = (totalMS / 1000)  % 60;
					const int cs          = (totalMS / 10)    % 100;    // centiseconds
					const int battery     = Battery::GetScaled(8);
					const int signal      = signalEstimate.GetScaled(now, 5);
					const int idleTimeSec = (now - lastEventTimeMS) / 1000;
					const int idleTimeRemaining = (timerRunning ? 3600 : 600) - idleTimeSec - 1;
	
					bool drawStatusIcons = true;
					oled.Clear();
					if (now < 2000)
					{
						drawStatusIcons = false;
						oled.DrawScreen(splashScreenBitmap);
					}
					else if (signal == 0 && now % 1000 > 500)
					{
						oled.SetFont(&openSansCondensed_11ptFontInfo, 1);
						oled.SetCursor(48, 1);
						oled.Print("Not connected!");
					}
					else if (min > 9)   // 00:00.00
					{
						drawStatusIcons = false;    // Not enough room!
						oled.SetFont(&openSansCondensed_32ptFontInfo);
						oled.SetCursor(2, 0);
						oled.Printf("%i:%.2i.%.2i", min, sec, cs);
					}
					else if (min > 0)   //  0:00.00
					{
						oled.SetFont(&openSansCondensed_32ptFontInfo);
						oled.SetCursor(20, 0);
						oled.Printf("%i:%.2i.%.2i", min, sec, cs);
					}
					else                //    00.00
					{
						oled.SetFont(&openSansCondensed_32ptFontInfo);
						oled.SetCursor(47, 0);
						oled.Printf("%.2i.%.2i", sec, cs);
					}

					if (drawStatusIcons)
					{
						oled.SetFont(&symbols_FontInfo);
						oled.SetCursor(0, 0);
						oled.DrawChar('b' + battery);    // Battery
						oled.SetCursor(0, 1);
						oled.DrawChar('s' + signal);     // Signal
			
						oled.SetFont(&myriadProCond_6ptFontInfo, 1);
						oled.SetCursor(0, 3);
						if (idleTimeRemaining > 60)
							oled.Printf("Z:%im", idleTimeRemaining / 60 + 1);
						else
							oled.Printf("Z:%is", idleTimeRemaining);
					}
					oled.SendFrame();

					if (idleTimeRemaining < 0)
					{
						esp_sleep_enable_ext0_wakeup(GPIO_NUM_32, 0);
						oled.PowerOff();
						esp_deep_sleep_start();
					}
				}
				break;
			}  // switch event.type
		
			if(lastTimerRunning != timerRunning)
				xTimerChangePeriod(displayTimer, timerRunning ? DISPLAY_ACTIVE_TICKS : DISPLAY_IDLE_TICKS, portMAX_DELAY);
		} // while xQueueReceive
	} // while true
}


extern "C"
void app_main()
{
	esp_event_loop_init(nullptr, nullptr);
	
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) 
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	
	esp_timer_init();

	eventQueue = xQueueCreate(32, sizeof(Event_t));
	xTaskCreate(taskMain, "Main", 32768, nullptr, 1, &mainTask);
	sendTimer = xTimerCreate("Send", SEND_TICKS, pdTRUE, nullptr, onSendTimer);
	xTimerStart(sendTimer, portMAX_DELAY);
	displayTimer = xTimerCreate("Display", DISPLAY_IDLE_TICKS, pdTRUE, nullptr, onDisplayTimer);
	xTimerStart(displayTimer, portMAX_DELAY);
}
