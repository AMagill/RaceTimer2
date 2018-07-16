#include <functional>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "Protocol.h"
#include "main.h"

static void sendCB(const uint8_t *a_macAddr, esp_now_send_status_t a_status);
static void recvCB(const uint8_t *a_macAddr, const uint8_t *a_data, int a_length);
	
static QueueHandle_t     g_eventQueue;
static SemaphoreHandle_t g_sendSemaphore;
static uint32_t          g_lastRxRtcTime = 0;


void Protocol::Init(QueueHandle_t a_queue, const uint8_t a_otherMac[6])
{
	g_eventQueue = a_queue;
	
	tcpip_adapter_init();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));
	
	// Initialize ESPNOW and register sending and receiving callback function.
	ESP_ERROR_CHECK(esp_now_init());
	ESP_ERROR_CHECK(esp_now_register_send_cb(sendCB));
	ESP_ERROR_CHECK(esp_now_register_recv_cb(recvCB));

	// Set primary master key.
	ESP_ERROR_CHECK(esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK));

	// Add broadcast peer information to peer list.
	esp_now_peer_info_t peer;
	memset(&peer, 0, sizeof(esp_now_peer_info_t));
	peer.channel = CONFIG_ESPNOW_CHANNEL;
	peer.ifidx   = ESP_IF_WIFI_STA;
	peer.encrypt = false;
	memcpy(peer.peer_addr, a_otherMac, ESP_NOW_ETH_ALEN);
	ESP_ERROR_CHECK(esp_now_add_peer(&peer));

	g_sendSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(g_sendSemaphore);
}


void Protocol::Send(uint32_t a_stateTime, uint8_t a_stateFlag)
{
	Message_t msg;
	msg.rtcTimeMS   = esp_timer_get_time() / 1000;
	msg.ackTimeMS   = g_lastRxRtcTime;
	msg.stateTimeMS = a_stateTime;
	msg.stateFlag   = a_stateFlag;
	
	xSemaphoreTake(g_sendSemaphore, portMAX_DELAY);
	ESP_ERROR_CHECK(esp_now_send(nullptr, reinterpret_cast<uint8_t*>(&msg), sizeof(msg)));
}


void sendCB(const uint8_t *a_macAddr, esp_now_send_status_t a_status)
{
	xSemaphoreGive(g_sendSemaphore);   // Allow another send
}


void recvCB(const uint8_t *a_macAddr, const uint8_t *a_data, int a_length)
{
	if (a_macAddr == NULL || a_data == NULL || a_length <= 0) 
	{
		ESP_LOGE("Protocol", "Receive cb arg error");
		return;
	}

	if (a_length != sizeof(Protocol::Message_t))
		return;
	
	const Protocol::Message_t* msg = reinterpret_cast<const Protocol::Message_t*>(a_data);	
	g_lastRxRtcTime = msg->rtcTimeMS;
	
	{
		Event_t event;
		event.type = EventType::RX_RTC;
		event.time = msg->rtcTimeMS;
		xQueueSend(g_eventQueue, &event, portMAX_DELAY);	
	}
	{	
		Event_t event;
		event.type = EventType::RX_ACK;
		event.time = msg->ackTimeMS;
		xQueueSend(g_eventQueue, &event, portMAX_DELAY);	
	}
	{	
		Event_t event;
		event.type = msg->stateFlag ? EventType::RX_BUTTON_DOWN : EventType::RX_BUTTON_UP;
		event.time = msg->stateTimeMS;
		xQueueSend(g_eventQueue, &event, portMAX_DELAY);	
	}
}
