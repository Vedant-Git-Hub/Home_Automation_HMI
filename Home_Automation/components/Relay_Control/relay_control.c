#include <stdbool.h>
#include <strings.h>
#include <unistd.h>

#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "relay_control.h"
#include "driver/gpio.h"
#include "hal/gpio_types.h"
#include "nvs.h"
#include "esp_task_wdt.h"
#include "device_info.h"


#define RELAY_1		GPIO_NUM_2
#define RELAY_2		GPIO_NUM_4
#define RELAY_3		GPIO_NUM_18
#define RELAY_4		GPIO_NUM_19
#define RELAY_5		GPIO_NUM_22

static void relay_Init(void);
static void relay_Control(void);


U_RELAY_STATE relay_states;
nvs_handle_t relay_nvs_handle;

static uint8_t prev_relay_states;



void task_RelayControl(void *pvParameters)
{
	esp_task_wdt_add(NULL);  
	esp_task_wdt_delete(NULL);
	
	relay_Init();
	
	while(1)
	{
		relay_Control();
	}
	
}

static void relay_Init(void)
{
	gpio_set_direction(RELAY_1, GPIO_MODE_OUTPUT);
	gpio_set_direction(RELAY_2, GPIO_MODE_OUTPUT);
	gpio_set_direction(RELAY_3, GPIO_MODE_OUTPUT);
	gpio_set_direction(RELAY_4, GPIO_MODE_OUTPUT);
	gpio_set_direction(RELAY_5, GPIO_MODE_OUTPUT);

	relay_states.data = device_GetRelayState();
}

static void relay_Control(void)
{	
	if(prev_relay_states != relay_states.data)
	{
		gpio_set_level(RELAY_1, relay_states.relay_state.relay_1);
		gpio_set_level(RELAY_2, relay_states.relay_state.relay_2);
		gpio_set_level(RELAY_3, relay_states.relay_state.relay_3);
		gpio_set_level(RELAY_4, relay_states.relay_state.relay_4);
		gpio_set_level(RELAY_5, relay_states.relay_state.relay_5);
		prev_relay_states = relay_states.data;
	}
	
	vTaskDelay(1/portTICK_PERIOD_MS);
}