#ifndef H_MODBUS_SERVER_H
#define H_MODBUS_SERVER_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct{
	uint16_t trans_ident;
	uint16_t protocol;
	uint16_t msg_len;
	uint8_t unit_id;
	uint8_t func_code;
}T_MODBUS_TCP_HEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	uint16_t start_add;
	uint16_t add_count;	
}T_MODBUS_TCP_REQ_RMC; 
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	uint8_t data_bytes;
	uint8_t data[1];	
}T_MODBUS_TCP_RES_RMC; 
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	uint16_t write_add;
	uint16_t status;
}T_MODBUS_TCP_REQ_FSC;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	uint16_t add_written;
	uint16_t status_written;
}T_MODBUS_TCP_RES_FSC;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	uint16_t start_add;
	uint16_t num_of_coils;
	uint8_t data_bytes;
	uint8_t data[1];
}T_MODBUS_TCP_REQ_FMC;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	uint16_t start_add;
	uint16_t num_of_coils;
}T_MODBUS_TCP_RES_FMC;
#pragma pack(pop)

void task_ModbusServer(void *pvParameters);


#endif