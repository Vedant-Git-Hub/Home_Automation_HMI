#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <unistd.h>

#include "network_discovery.h"
#include "freertos/idf_additions.h"
#include "lwip/sockets.h"
#include "portmacro.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "esp_task_wdt.h"
#include "device_info.h"


#define DISCOVERY_PORT				69

#define DISCOVERY_BUFF_SIZE			512


typedef enum{
	enum_READ_STATE = 0,
	enum_PARSE_STATE
}E_DISCOVERY_STATES;

static int discovery_sock;
static struct sockaddr_in discovery_server_add; 
static struct sockaddr_in discovery_client_add;
static socklen_t discovery_client_len = sizeof(discovery_client_add);
static E_DISCOVERY_STATES discovery_state;



static void discovery_ServerInit(void);
static int discovery_ParseFrame(uint8_t *recv_buff, uint16_t recv_len);



void task_DiscoveryServer(void *pvParameters)
{
	uint8_t discovery_receive_buff[DISCOVERY_BUFF_SIZE];
	uint16_t discovery_recv_len = 0;
	
	esp_task_wdt_add(NULL);  
	esp_task_wdt_delete(NULL);
	
	discovery_ServerInit();
	
	while(1)
	{
		
		switch(discovery_state)
		{
			case enum_READ_STATE:
				{
					discovery_recv_len = recvfrom(discovery_sock,
											  discovery_receive_buff,
											  DISCOVERY_BUFF_SIZE,
											  0, 
											  (struct sockaddr *)&discovery_client_add,
											  &discovery_client_len);
					if(discovery_recv_len < 0)
					{
						printf("UDP error receiving data:(\n");
					}
					
					discovery_state = enum_PARSE_STATE;
				}
				break;
				
			case enum_PARSE_STATE:
				{
					int ret = discovery_ParseFrame(discovery_receive_buff, discovery_recv_len);
					if(ret <= 0)
					{
						printf("UDP error sending:(\n");
					}
				
					discovery_state = enum_READ_STATE;
				}
				break;
				
			default:
				break;
		}
		
//        sendto(discovery_sock, "Hello from esp", sizeof("Hello from esp"), 0, (struct sockaddr *)&discovery_client_add, discovery_client_len);
		vTaskDelay(1/portTICK_PERIOD_MS);
	}
	
	close(discovery_sock);
}

static void discovery_ServerInit(void)
{
	discovery_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if(discovery_sock < 0)
	{
		printf("UDP Socket creation Failed:(\n");
		while(1);
	}
	printf("UDP Socket created successfully!\n");
	bzero(&discovery_server_add, sizeof(discovery_server_add));
	
	discovery_server_add.sin_family = AF_INET;
	discovery_server_add.sin_addr.s_addr = htonl(INADDR_ANY);
	discovery_server_add.sin_port = htons(DISCOVERY_PORT);
	
	if(bind(discovery_sock, (struct sockaddr *)&discovery_server_add, sizeof(discovery_server_add)) != 0)
	{
		printf("UDP Socket binding failed:(\n");
		while(1);
	}
	printf("UDP Socket successfully binded!\n");
	
	discovery_state = enum_READ_STATE;
}

static int discovery_ParseFrame(uint8_t *recv_buff, uint16_t recv_len)
{
	T_DISCOVERY_FRAME_REQ *resp_frame = (T_DISCOVERY_FRAME_REQ *)recv_buff;
	
	printf("Seq num: %d,frame id: %d, master id: %d, msg Len: %d, msg: %s", 
	resp_frame->seq_num, resp_frame->frame_id, resp_frame->master_id, resp_frame->msg_len, resp_frame->data); 
	
	return 1;
}