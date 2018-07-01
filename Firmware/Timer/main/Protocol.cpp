#include <functional>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "tcpip_adapter.h"
#include "Protocol.h"

static const char *TAG = "EspNow";
static const uint8_t BROADCAST_MAC[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static SemaphoreHandle_t sendSemaphore;

struct airMessage_t
{
	char     magic[4] = {'R', 'a', 'c', 'e'};
	uint32_t timeMS;
	bool     isEcho;
};


//    buf->crc = crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);


Protocol::Protocol()
{
}


void Protocol::Init(ProtocolRxCB_t* a_rxCB)
{
	m_rxCallback = a_rxCB;
	
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
	memcpy(peer.peer_addr, BROADCAST_MAC, ESP_NOW_ETH_ALEN);
	ESP_ERROR_CHECK(esp_now_add_peer(&peer));

	sendSemaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(sendSemaphore);
}


int32_t Protocol::Send()
{
	airMessage_t msg;
	msg.timeMS = esp_timer_get_time() / 1000;
	
	xSemaphoreTake(sendSemaphore, portMAX_DELAY);
	ESP_ERROR_CHECK(esp_now_send(BROADCAST_MAC, reinterpret_cast<uint8_t*>(&msg), sizeof(msg)));
	
	return msg.timeMS;
}


void Protocol::sendCB(const uint8_t *a_macAddr, esp_now_send_status_t a_status)
{
	xSemaphoreGive(sendSemaphore);   // Allow another send
}


void Protocol::recvCB(const uint8_t *a_macAddr, const uint8_t *a_data, int a_length)
{
	if (a_macAddr == NULL || a_data == NULL || a_length <= 0) 
	{
		ESP_LOGE(TAG, "Receive cb arg error");
		return;
	}

	if (a_length != sizeof(airMessage_t))
		return;
	
	const airMessage_t* airMsg = reinterpret_cast<const airMessage_t*>(a_data);
	if (strncmp(airMsg->magic, "Race", 4) != 0)
		return;
		
	// If it's not an echo, then echo it back
	if(!airMsg->isEcho)
	{
		airMessage_t response = *airMsg;
		response.isEcho = true;
		xSemaphoreTake(sendSemaphore, portMAX_DELAY);
		ESP_ERROR_CHECK(esp_now_send(BROADCAST_MAC, reinterpret_cast<uint8_t*>(&response), sizeof(response)));
	}

	Message_t msg;
	msg.rxTimeMS = esp_timer_get_time() / 1000;
	msg.timeMS   = airMsg->timeMS;
	msg.isEcho   = airMsg->isEcho;
	GetSingle().m_rxCallback(msg);
}
