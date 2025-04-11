#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>

#include "modbus_server.h"
#include "device_info.h"
#include "relay_control.h"
#include "lwip/sockets.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "esp_task_wdt.h"


#define MODBUS_PORT		502

#define FUNC_READ_MULTIPLE_COILS		0x01
#define FUNC_WRITE_SINGLE_COIL			0x05
#define FUNC_WRITE_MULTIPLE_COILS		0x0F


typedef enum{
	enum_ACCEPT_STATE = 0,
	enum_READ_STATE,
	enum_PARSE_STATE,
}E_MODBUS_SERVER_STATE;


static void modbus_ServerInit();
static int modbus_ParseFrame(uint8_t *mod_rx_data);
static uint16_t modbus_ToBigEndian16(uint16_t val);


extern U_RELAY_STATE relay_states;
static int modbus_sock, conn_fd;
static socklen_t len;
static struct sockaddr_in mod_server_add; 
static struct sockaddr_in mod_client_add;
static E_MODBUS_SERVER_STATE server_state;


void task_ModbusServer(void *pvParameters)
{
	uint8_t mod_rx_data[256];
	
	esp_task_wdt_add(NULL);  
	esp_task_wdt_delete(NULL);
	
	modbus_ServerInit(); 
	
	while(1)
	{
		
		switch(server_state)
		{
			case enum_ACCEPT_STATE:
				{
					conn_fd = accept(modbus_sock, (struct sockaddr *)&mod_client_add, &len);
					if(conn_fd < 0)
					{
						printf("Server accept failed:( %d\n", conn_fd);
						while(1);
					}
					printf("Server has accepted the client!\n");
					
					server_state = enum_READ_STATE;
				}
				break;
			
			case enum_READ_STATE:
				{
					int ret = read(conn_fd, &mod_rx_data, sizeof(mod_rx_data));
					if(ret <= 0)
					{
						printf("Read error:(\n");
						server_state = enum_ACCEPT_STATE;
					}
					else if(ret > 0)
					{
						server_state = enum_PARSE_STATE;
					}
				}
				break;
				
			case enum_PARSE_STATE:
				{
					int ret = modbus_ParseFrame((uint8_t *)&mod_rx_data);
					if(ret <= 0)
					{
						printf("Write error was occurred:( %d\n", ret);
						server_state = enum_ACCEPT_STATE;
					}
					else if(ret > 0)
					{
						server_state = enum_READ_STATE;
					}
				}
				break;
				
			default:
				server_state = enum_ACCEPT_STATE;
				break;
		}
	}
}

static void modbus_ServerInit()
{
	modbus_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(modbus_sock == -1)
	{
		printf("Socket creation Failed:(\n");
		while(1);
	}
	printf("Socket created successfully!\n");
	bzero(&mod_server_add, sizeof(mod_server_add));
	
	mod_server_add.sin_family = AF_INET;
	mod_server_add.sin_addr.s_addr = htonl(INADDR_ANY);
	mod_server_add.sin_port = htons(MODBUS_PORT);
	
	if(bind(modbus_sock, (struct sockaddr *)&mod_server_add, sizeof(mod_server_add)) != 0)
	{
		printf("Socket binding failed:(\n");
		while(1);
	}
	printf("Socket successfully binded!\n");
	
	if(listen(modbus_sock, 5) != 0)
	{
		printf("Listen failed:(\n");
		while(1);
	}
	printf("Listen successful!\n");
	len = sizeof(mod_client_add);
	
	server_state = enum_ACCEPT_STATE;
}

static int modbus_ParseFrame(uint8_t *mod_rx_data)
{
	uint8_t resp_message_frame[256];
	uint8_t resp_frame_len = 0;
	T_MODBUS_TCP_HEADER *rx_header = (T_MODBUS_TCP_HEADER *)mod_rx_data;
	T_MODBUS_TCP_HEADER tx_header;

	
	switch(rx_header->func_code)
	{
		case FUNC_READ_MULTIPLE_COILS:
			{
				T_MODBUS_TCP_RES_RMC tx_body;
				T_MODBUS_TCP_REQ_RMC *rx_body = (T_MODBUS_TCP_REQ_RMC *)((uint8_t *)mod_rx_data + (uint8_t)sizeof(T_MODBUS_TCP_HEADER));
				
				if(modbus_ToBigEndian16(rx_body->start_add) >= 0 && modbus_ToBigEndian16(rx_body->start_add) <= 4)
				{
					tx_header.trans_ident = rx_header->trans_ident;
					tx_header.protocol = rx_header->protocol;
					tx_header.msg_len = 0x0400;
					tx_header.unit_id = rx_header->unit_id;
					tx_header.func_code = rx_header->func_code;
					
					tx_body.data_bytes = 0x01;
					tx_body.data[0] = relay_states.data;
					
					memcpy(resp_message_frame, (uint8_t *)&tx_header, sizeof(T_MODBUS_TCP_HEADER));
					memcpy((resp_message_frame + (uint8_t)sizeof(T_MODBUS_TCP_HEADER)), (uint8_t *)&tx_body, sizeof(T_MODBUS_TCP_RES_RMC));
					resp_frame_len = ((uint8_t)sizeof(T_MODBUS_TCP_HEADER) + (uint8_t)sizeof(T_MODBUS_TCP_RES_RMC));
				}
			}
			break;
			
		case FUNC_WRITE_SINGLE_COIL:
			{
				T_MODBUS_TCP_RES_FSC tx_body;
				T_MODBUS_TCP_REQ_FSC *rx_body = (T_MODBUS_TCP_REQ_FSC *)((uint8_t *)mod_rx_data + (uint8_t)sizeof(T_MODBUS_TCP_HEADER));
				
				if(modbus_ToBigEndian16(rx_body->write_add) >= 0 && modbus_ToBigEndian16(rx_body->write_add) <= 4)
				{
					tx_header.trans_ident = rx_header->trans_ident;
					tx_header.protocol = rx_header->protocol;
					tx_header.msg_len = rx_header->msg_len;
					tx_header.unit_id = rx_header->unit_id;
					tx_header.func_code = rx_header->func_code;
					
					switch(modbus_ToBigEndian16(rx_body->write_add))
					{
						case 0:
							relay_states.relay_state.relay_1 = (modbus_ToBigEndian16(rx_body->status) == 0xFF00 ? 1 : 0); 
							break;
							
						case 1:
							relay_states.relay_state.relay_2 = (modbus_ToBigEndian16(rx_body->status) == 0xFF00 ? 1 : 0); 
							break;
							
						case 2:
							relay_states.relay_state.relay_3 = (modbus_ToBigEndian16(rx_body->status) == 0xFF00 ? 1 : 0); 
							break;
							
						case 3:
							relay_states.relay_state.relay_4 = (modbus_ToBigEndian16(rx_body->status) == 0xFF00 ? 1 : 0); 
							break;
							
						case 4:
							relay_states.relay_state.relay_5 = (modbus_ToBigEndian16(rx_body->status) == 0xFF00 ? 1 : 0); 
							break;
					}
					
					tx_body.add_written = rx_body->write_add;
					tx_body.status_written = rx_body->status;
					
					memcpy(resp_message_frame, (uint8_t *)&tx_header, sizeof(T_MODBUS_TCP_HEADER));
					memcpy((resp_message_frame + (uint8_t)sizeof(T_MODBUS_TCP_HEADER)), (uint8_t *)&tx_body, sizeof(T_MODBUS_TCP_RES_FSC));
					resp_frame_len = ((uint8_t)sizeof(T_MODBUS_TCP_HEADER) + (uint8_t)sizeof(T_MODBUS_TCP_RES_FSC));
					
					device_SetRelayState(relay_states.data);
				}
			}
			break;
			
		case FUNC_WRITE_MULTIPLE_COILS:
			{
				T_MODBUS_TCP_RES_FMC tx_body;
				T_MODBUS_TCP_REQ_FMC *rx_body = (T_MODBUS_TCP_REQ_FMC *)((uint8_t *)mod_rx_data + (uint8_t)sizeof(T_MODBUS_TCP_HEADER));
				
				if(modbus_ToBigEndian16(rx_body->num_of_coils) == 5)
				{
					tx_header.trans_ident = rx_header->trans_ident;
					tx_header.protocol = rx_header->protocol;
					tx_header.msg_len = 0x0600;
					tx_header.unit_id = rx_header->unit_id;
					tx_header.func_code = rx_header->func_code;
				
					relay_states.data = rx_body->data[0];
					
					tx_body.start_add = rx_body->start_add;
					tx_body.num_of_coils = rx_body->num_of_coils;
					
					memcpy(resp_message_frame, (uint8_t *)&tx_header, sizeof(T_MODBUS_TCP_HEADER));
					memcpy((resp_message_frame + (uint8_t)sizeof(T_MODBUS_TCP_HEADER)), (uint8_t *)&tx_body, sizeof(T_MODBUS_TCP_RES_FMC));
					resp_frame_len = ((uint8_t)sizeof(T_MODBUS_TCP_HEADER) + (uint8_t)sizeof(T_MODBUS_TCP_RES_FMC));
					
					device_SetRelayState(relay_states.data);
				}

			}
			break;
			
		default:
			break;	
		
	}
	
	if(resp_frame_len != 0)
	{
		int ret = write(conn_fd, &resp_message_frame, resp_frame_len);
		
		return ret;
	}
	
	return 0;
}

static uint16_t modbus_ToBigEndian16 (uint16_t val)
{
	
	return ((val << 8 & 0xFF00) |
		    (val >> 8 & 0x00FF));
}