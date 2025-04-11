#ifndef H_NETWORK_DISCOVERY_H
#define H_NETWORK_DISCOVERY_H

#include <stdint.h>


#pragma pack(push, 1)
typedef struct{
	uint16_t seq_num;
	uint8_t frame_id;
	uint8_t master_id;
	uint16_t msg_len;
	uint8_t data[1];
}T_DISCOVERY_FRAME_REQ;
#pragma pack(pop)






void task_DiscoveryServer(void *pvParameters);




#endif