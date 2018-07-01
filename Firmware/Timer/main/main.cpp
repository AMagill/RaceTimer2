#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "rom/ets_sys.h"
#include "rom/crc.h"
#include "Protocol.h"
#include "SSD1306.h"
#include "font_myriadProCond_6pt.h"

static uint8_t MASTER_MAC[6] = { 0x30, 0xAE, 0xA4, 0x1C, 0x0C, 0xB8 };  // I
static uint8_t SLAVE_MAC[6]  = { 0x30, 0xAE, 0xA4, 0x23, 0xCC, 0x10 };  // II

enum class EventType
{
	RX_ECHO,
	RX_OTHER,
	TIMER_SEND,
};
struct Event_t
{
	EventType type;
	int32_t   data;
};

TaskHandle_t  mainTask;
QueueHandle_t eventQueue;
TimerHandle_t sendTimer;

int lastSentMS    = 0;
int lastLatencyMS = 0;
int lastReceived  = 0;
int sentCount     = 0;
int echoCount     = 0;


static void onProtocolRx(const Protocol::Message_t a_msg)
{
	Event_t event;
	event.type = a_msg.isEcho ? EventType::RX_ECHO : EventType::RX_OTHER;
	event.data = a_msg.isEcho ? (a_msg.rxTimeMS - a_msg.timeMS) : a_msg.timeMS;
	xQueueSend(eventQueue, &event, portMAX_DELAY);	
}

static void onSendTimer(TimerHandle_t a_timer)
{
	Event_t event;
	event.type = EventType::TIMER_SEND;
	event.data = 0;
	xQueueSend(eventQueue, &event, portMAX_DELAY);
}


static void taskMain(void* a_context)
{
	SSD1306 oled(I2C_NUM_0);
	oled.Init();
	oled.SetFont(&myriadProCond_6ptFontInfo);
	
	Protocol& protocol = Protocol::GetSingle();
	protocol.Init(onProtocolRx);
	
	Event_t event;
	while (xQueueReceive(eventQueue, &event, portMAX_DELAY))
	{
		switch (event.type)
		{
		case EventType::RX_ECHO:
			lastLatencyMS = event.data;
			echoCount++;
			break;
		case EventType::RX_OTHER:
			lastReceived = event.data;
			break;
		case EventType::TIMER_SEND:
			lastSentMS    = protocol.Send();
			lastLatencyMS = 9999;
			sentCount++;
			break;
		}

		oled.Clear();
		oled.SetCursor(0, 0);
		oled.Printf("Sent %i", lastSentMS);
		oled.SetCursor(0, 1);
		oled.Printf("Latency %i", lastLatencyMS);
		oled.SetCursor(0, 2);
		oled.Printf("Got %i/%i", echoCount, sentCount);
		oled.SetCursor(0, 3);
		oled.Printf("Received %i", lastReceived);
		oled.SendFrame();
	}
}


extern "C"
void app_main()
{
	uint8_t myMac[6];
	esp_read_mac(myMac, ESP_MAC_WIFI_STA);

	if (memcmp(myMac, MASTER_MAC, sizeof(myMac)) == 0)
	{
		printf("I seem to be the master.\n");
	}
	else if (memcmp(myMac, SLAVE_MAC, sizeof(myMac)) == 0)
	{
		printf("I seem to be the slave.\n");
	}
	else
	{
		printf("No MAC match  %02X:%02X:%02X:%02X:%02X:%02X\n",
			myMac[0], myMac[1], myMac[2], myMac[3], myMac[4], myMac[5]);		
	}
	
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

	eventQueue = xQueueCreate(6, sizeof(Event_t));
	xTaskCreate(taskMain, "Main", 4096, nullptr, 1, &mainTask);
	sendTimer = xTimerCreate("Send", 1000 / portTICK_PERIOD_MS, pdTRUE, nullptr, onSendTimer);
	xTimerStart(sendTimer, 0);
}
