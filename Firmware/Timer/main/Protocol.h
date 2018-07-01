#pragma once
#include "esp_now.h"


class Protocol
{
public:
	struct Message_t
	{
		uint32_t timeMS;
		bool     isEcho;
		uint32_t rxTimeMS;
	};

	using ProtocolRxCB_t = void(const Message_t a_msg);

	static Protocol& GetSingle()
	{
		static Protocol instance;
		return instance;
	}
	
	void Init(ProtocolRxCB_t* a_rxCB);
	int32_t Send();
	
private:
	Protocol();  // Private constructor
	Protocol(const Protocol&)       = delete;  // No copy
	void operator=(const Protocol&) = delete;  // No copy
	
	static void sendCB(const uint8_t *a_macAddr, esp_now_send_status_t a_status);
	static void recvCB(const uint8_t *a_macAddr, const uint8_t *a_data, int a_length);
	
	ProtocolRxCB_t* m_rxCallback;
};
