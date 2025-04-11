#ifndef H_RELAY_CONTROL_H
#define H_RELAY_CONTROL_H

#include <stdint.h>


#define NVS_RELAY_STATE_KEY		"PRS"    //Previous relay state


#pragma pack(push, 1)
typedef struct{
	uint8_t relay_1:1;
	uint8_t relay_2:1;
	uint8_t relay_3:1;
	uint8_t relay_4:1;
	uint8_t relay_5:1;
	uint8_t reserved:3;
}T_RELAY_STATE;
#pragma pack(pop)

typedef union{
	uint8_t data;
	T_RELAY_STATE relay_state;
}U_RELAY_STATE;

void task_RelayControl(void *pvParameters);


#endif