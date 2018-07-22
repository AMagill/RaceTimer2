#include "Button.h"
#include "freertos/timers.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"

static const gpio_num_t INPUT_PIN      = GPIO_NUM_32;
static const TickType_t DEBOUNCE_TICKS = 15 / portTICK_PERIOD_MS;

static QueueHandle_t g_eventQueue;
static TimerHandle_t g_bounceTimer;
static bool          g_debouncing = false;
static uint32_t      g_lastIntMS  = 0;
static bool          g_lastState  = false;


static void IRAM_ATTR buttonISR(void* a_arg)
{
	BaseType_t xHigherPriorityTaskWoken = false;
	uint32_t now = esp_timer_get_time() / 1000;

	if (!g_debouncing)
	{	// We can accept the transition immediately
		g_lastState = !g_lastState;
		g_debouncing = true;
		
		Event_t event;
		event.type = g_lastState ? EventType::BUTTON_DOWN : EventType::BUTTON_UP;
		event.time = now;
		xQueueSendToBackFromISR(g_eventQueue, &event, &xHigherPriorityTaskWoken);		
	}
	
	// Otherwise the transition will be accepted retroactively when
	// transitions stop long enough for the debounce timer to expire.
	g_lastIntMS = now;
	xTimerStartFromISR(g_bounceTimer, &xHigherPriorityTaskWoken);
		
	if (xHigherPriorityTaskWoken)
		portYIELD_FROM_ISR();
}

static void onBounceTimer(TimerHandle_t a_timer)
{
	bool state = !gpio_get_level(INPUT_PIN);  // Low = pressed
	if (state != g_lastState)
	{	// Accept the transition
		g_lastState = state;
		
		Event_t event;
		event.type = state ? EventType::BUTTON_DOWN : EventType::BUTTON_UP;
		event.time = g_lastIntMS;
		xQueueSend(g_eventQueue, &event, 0);
	}
	g_debouncing = false;
}

void Button::Init(QueueHandle_t a_queue)
{
	g_eventQueue = a_queue;
	
	gpio_config_t gpioConfig;
	gpioConfig.pin_bit_mask = 1ULL << INPUT_PIN;
	gpioConfig.mode         = GPIO_MODE_INPUT;
	gpioConfig.pull_up_en   = GPIO_PULLUP_ENABLE;
	gpioConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpioConfig.intr_type    = GPIO_INTR_ANYEDGE;
	gpio_config(&gpioConfig);
	
	gpio_install_isr_service(0);
	gpio_isr_handler_add(INPUT_PIN, buttonISR, nullptr);
	
	g_bounceTimer = xTimerCreate("Debounce", DEBOUNCE_TICKS, pdFALSE, nullptr, onBounceTimer);
}