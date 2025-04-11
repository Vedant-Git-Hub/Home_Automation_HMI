#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "esp_err.h"
#include "nvs.h"
#include "esp_task_wdt.h"
#include "device_info.h"




static T_MY_INFO node_info;
static nvs_handle_t nvs_device_info;





void device_Init()
{
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_device_info);
	if(err != ESP_OK)
	{
		printf("NVS opening error:(\n");
		while(1);
	}
	
}

void device_SetVirginity(uint8_t virginity)
{
	esp_err_t err = nvs_set_u8(nvs_device_info, NVS_VIRGINITY_STAT, virginity);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		node_info.virginity_status = virginity;
	}
}

void device_SetID(uint8_t id)
{
	esp_err_t err = nvs_set_u8(nvs_device_info, NVS_ID, id);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		node_info.virginity_status = id;
	}
}

void device_SetName(char *name)
{
	esp_err_t err = nvs_set_str(nvs_device_info, NVS_NAME, name);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		strcpy(node_info.name, name);
	}
}

void device_SetPassword(char *pass)
{
	esp_err_t err = nvs_set_str(nvs_device_info, NVS_PASS, pass);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		strcpy(node_info.password, pass);
	}	
}

void device_SetHostName(char *host_name)
{
	esp_err_t err = nvs_set_str(nvs_device_info, NVS_HOST_NAME, host_name);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		strcpy(node_info.host_name, host_name);
	}		
}

void device_SetWifiSSID(char *ssid)
{
	esp_err_t err = nvs_set_str(nvs_device_info, NVS_WIFI_SSID, ssid);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		strcpy(node_info.wifi_ssid, ssid);
	}	
}

void device_SetWifiPass(char *pass)
{
	esp_err_t err = nvs_set_str(nvs_device_info, NVS_WIFI_PASS, pass);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		strcpy(node_info.wifi_pass, pass);
	}	
}

void device_SetRelayName(uint8_t relay_num, char *name)
{
	char *relay_name = NULL; 
	char *relay_name_key = NULL;
	
	switch(relay_num)
	{
		case 1:
			relay_name = (char *)&node_info.relay_1_name;
			relay_name_key = NVS_RELAY_1_NAME;
			break;
			
	    case 2:
			relay_name = (char *)&node_info.relay_2_name;
			relay_name_key = NVS_RELAY_2_NAME;
			break;
			
		case 3:
			relay_name = (char *)&node_info.relay_3_name;
			relay_name_key = NVS_RELAY_3_NAME;
			break;
			
		case 4:
			relay_name = (char *)&node_info.relay_4_name;
			relay_name_key = NVS_RELAY_4_NAME;
			break;
			
		case 5:
			relay_name = (char *)&node_info.relay_5_name;
			relay_name_key = NVS_RELAY_5_NAME;
			break;			
	}
	
	if(relay_name != NULL && relay_name_key != NULL)
	{
		esp_err_t err = nvs_set_str(nvs_device_info, relay_name_key, name);
		if(err != ESP_OK)
		{
			printf("Value not stored to NVS:(\n");
		}
		else
		{
			err = nvs_commit(nvs_device_info);
			if(err != ESP_OK)
			{
				printf("Failed to commit changes to nvs:(\n");
			}
		
			strcpy(relay_name, name);
		}			
	}
}

void device_SetRelayState(uint8_t relay_state)
{
	esp_err_t err = nvs_set_u8(nvs_device_info, NVS_RELAY_STATE, relay_state);
	if(err != ESP_OK)
	{
		printf("Value not stored to NVS:(\n");
	}
	else
	{
		err = nvs_commit(nvs_device_info);
		if(err != ESP_OK)
		{
			printf("Failed to commit changes to nvs:(\n");
		}
		
		node_info.relay_state.data = relay_state;
	}
}

uint8_t device_GetVirginity()
{
	uint8_t virginity_status;
	
	esp_err_t err = nvs_get_u8(nvs_device_info, NVS_VIRGINITY_STAT, &virginity_status);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
	
	node_info.virginity_status = virginity_status;
	
	return node_info.virginity_status;
}

uint8_t device_GetID()
{
	uint8_t id;
	
	esp_err_t err = nvs_get_u8(nvs_device_info, NVS_ID, &id);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
	
	node_info.id = id;
	
	return node_info.id;
}

char* device_GetName()
{
	size_t str_len = INFO_STR_LEN;
	esp_err_t err = nvs_get_str(nvs_device_info, NVS_NAME, (char *)&node_info.name, &str_len);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
	
	return (char *)&node_info.name;
}

char* device_GetPassword()
{
	size_t str_len = INFO_STR_LEN;
	esp_err_t err = nvs_get_str(nvs_device_info, NVS_PASS, (char *)&node_info.password, &str_len);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
	
	return (char *)&node_info.password;
}

char* device_GetHostName()
{
	size_t str_len = INFO_STR_LEN;
	
	esp_err_t err = nvs_get_str(nvs_device_info, NVS_HOST_NAME, (char *)&node_info.host_name, &str_len);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
		
	return (char *)&node_info.host_name;
}

char* device_GetWifiSSID()
{
	size_t str_len = INFO_STR_LEN;
	
	esp_err_t err = nvs_get_str(nvs_device_info, NVS_WIFI_SSID, (char *)&node_info.wifi_ssid, &str_len);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
		
	return (char *)&node_info.wifi_ssid;
}

char* device_GetWifiPass()
{
	size_t str_len = INFO_STR_LEN;
	
	esp_err_t err = nvs_get_str(nvs_device_info, NVS_WIFI_PASS, (char *)&node_info.wifi_pass, &str_len);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
		
	return (char *)&node_info.wifi_pass;
}

char* device_GetRelayName(uint8_t relay_num)
{
	char *relay_name = NULL; 
	size_t str_len = INFO_STR_LEN;
	char *relay_name_key = NULL;
	
	switch(relay_num)
	{
		case 1:
			relay_name = (char *)&node_info.relay_1_name;
			relay_name_key = NVS_RELAY_1_NAME;
			break;
			
	    case 2:
			relay_name = (char *)&node_info.relay_2_name;
			relay_name_key = NVS_RELAY_2_NAME;
			break;
			
		case 3:
			relay_name = (char *)&node_info.relay_3_name;
			relay_name_key = NVS_RELAY_3_NAME;
			break;
			
		case 4:
			relay_name = (char *)&node_info.relay_4_name;
			relay_name_key = NVS_RELAY_4_NAME;
			break;
			
		case 5:
			relay_name = (char *)&node_info.relay_5_name;
			relay_name_key = NVS_RELAY_5_NAME;
			break;			
	}
	
	if(relay_num != NULL && relay_name_key != NULL)
	{
		esp_err_t err = nvs_get_str(nvs_device_info, relay_name_key, (char *)&relay_name, &str_len);
		if(err != ESP_OK)
		{
			printf("Error retrieving prev value:(\n");
		}
	}

	return (char *)relay_name;
}

uint8_t device_GetRelayState()
{
	uint8_t data;
	
	esp_err_t err = nvs_get_u8(nvs_device_info, NVS_RELAY_STATE, &data);
	if(err != ESP_OK)
	{
		printf("Error retrieving prev value:(\n");
	}
	
	node_info.relay_state.data = data;
	
	return node_info.relay_state.data;
}

void device_DeInit()
{
	nvs_close(nvs_device_info);
}
