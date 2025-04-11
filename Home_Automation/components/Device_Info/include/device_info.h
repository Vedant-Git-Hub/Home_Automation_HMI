#ifndef H_DEVICE_INFO_H
#define H_DEVICE_INFO_H

#include <stdint.h>
#include "relay_control.h"

#define INFO_STR_LEN					32

#define NVS_VIRGINITY_STAT				"VS"
#define NVS_ID							"ID"
#define NVS_NAME						"NAME"
#define NVS_PASS						"PASS"
#define NVS_HOST_NAME					"HN"
#define NVS_WIFI_SSID					"WSSID"
#define NVS_WIFI_PASS					"WPASS"
#define NVS_RELAY_1_NAME				"R1NAME"
#define NVS_RELAY_2_NAME				"R2NAME"
#define NVS_RELAY_3_NAME				"R3NAME"
#define NVS_RELAY_4_NAME				"R4NAME"
#define NVS_RELAY_5_NAME				"R5NAME"
#define NVS_RELAY_STATE					"RS"


#pragma pack(push, 1)
typedef struct{
	uint8_t virginity_status;
	uint8_t id;
	char name[INFO_STR_LEN];
	char password[INFO_STR_LEN];
	char host_name[INFO_STR_LEN];	
	char wifi_ssid[INFO_STR_LEN];
	char wifi_pass[INFO_STR_LEN];
	char relay_1_name[INFO_STR_LEN];
	char relay_2_name[INFO_STR_LEN];
	char relay_3_name[INFO_STR_LEN];
	char relay_4_name[INFO_STR_LEN];
	char relay_5_name[INFO_STR_LEN];
	U_RELAY_STATE relay_state;
}T_MY_INFO;
#pragma pack(pop)


void device_Init();
void device_SetVirginity(uint8_t virginity);
void device_SetID(uint8_t id);
void device_SetName(char *name);
void device_SetPassword(char *pass);
void device_SetHostName(char *host_name);
void device_SetWifiSSID(char *ssid);
void device_SetWifiPass(char *pass);
void device_SetRelayName(uint8_t relay_num, char *name);
void device_SetRelayState(uint8_t relay_state);
uint8_t device_GetVirginity();
uint8_t device_GetID();
char* device_GetName();
char* device_GetPassword();
char* device_GetHostName();
char* device_GetWifiSSID();
char* device_GetWifiPass();
char* device_GetRelayName(uint8_t relay_num);
uint8_t device_GetRelayState();
void device_DeInit();


#endif