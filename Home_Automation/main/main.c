#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include "wifi.h"
#include "relay_control.h"
#include "modbus_server.h"
#include "network_discovery.h"
#include "device_info.h"


EventGroupHandle_t wifi_event_group;


void app_main(void)
{
	esp_err_t err = nvs_flash_init();
	
	if(err == ESP_OK)
	{
		device_Init();
		
		wifi_Connection();
		
		wifi_event_group = xEventGroupCreate();
		if(wifi_event_group == NULL)
		{
			printf("Event group creation failed:(\n");
		}
		
		xTaskCreate(task_RelayControl, "relay_control", 4096, NULL, 5, NULL);
	
		EventBits_t event_bits = xEventGroupWaitBits(wifi_event_group, EVENT_IP_ASSIGNED, pdTRUE, pdFALSE, portMAX_DELAY);
		if((event_bits & EVENT_IP_ASSIGNED) == EVENT_IP_ASSIGNED)
		{
			printf("Starting modbus server task!\n");
			xTaskCreate(task_ModbusServer, "modbus_tcp_server", 4096, (void *)AF_INET, 5, NULL);
			xTaskCreate(task_DiscoveryServer, "discovery_udp_server", 4096, (void *)AF_INET, 5, NULL);
		}		
		
	}
	
	
    while (true) {
		
		vTaskDelay(1000/portTICK_PERIOD_MS);
		
    }
    
    device_DeInit();
}




