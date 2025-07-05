#include "LCD.h"
#include <freertos/FreeRTOS.h>














void app_main(void)
{	
	
	xTaskCreate(task_LCD, "LCD_Touch_Initialization", 20480, NULL, 0, NULL);
	
	while(1)
	{
		vTaskDelay(10/portTICK_PERIOD_MS);
	}
}

