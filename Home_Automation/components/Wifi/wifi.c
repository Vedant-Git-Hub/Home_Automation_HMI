#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>

#include "wifi.h"
#include "esp_event_base.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_wifi_default.h"
#include "esp_wifi_types_generic.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/idf_additions.h"
#include "freertos/event_groups.h"
#include "device_info.h"




extern EventGroupHandle_t wifi_event_group;

static void wifi_EventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);



void wifi_Connection(void)
{
	esp_netif_init();
	esp_event_loop_create_default();
	esp_netif_t *netif = esp_netif_create_default_wifi_sta();
	esp_netif_set_hostname(netif, device_GetHostName());
	
	wifi_init_config_t wifi_init = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&wifi_init);
	
	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_EventHandler, NULL);
	esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, wifi_EventHandler, NULL);
	
	wifi_config_t wifi_config = {
		.sta = {
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
		},
	};
	strcpy((char *)&wifi_config.sta.ssid, device_GetWifiSSID());
	strcpy((char *)&wifi_config.sta.password, device_GetWifiPass());
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	
	esp_wifi_start();
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_connect();
	printf("Wifi init sta finished-> SSID: %s, PASS: %s\n", device_GetWifiSSID(), device_GetWifiPass());
}

static void wifi_EventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	static uint8_t retry_num = 0;
	
	switch(event_id)
	{
		case WIFI_EVENT_STA_START:
			printf("WIFI Connecting...\n");
			break;
			
		case WIFI_EVENT_STA_CONNECTED:
			printf("WIFI Connected!\n");
			retry_num = 0;
			break;
			
		case WIFI_EVENT_STA_DISCONNECTED:
			{
				printf("WIFI Connection Lost :(\n"); 
				if(retry_num < 5)
				{
					esp_wifi_connect();
					retry_num++;
					printf("Retrying to connect...\n");
				}
			}
			break;
			
		case IP_EVENT_STA_GOT_IP:
			{
				printf("WIFI IP assigned!\n");
				xEventGroupSetBits(wifi_event_group, EVENT_IP_ASSIGNED);
			}
			
	}
}