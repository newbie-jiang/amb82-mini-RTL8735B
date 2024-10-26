#include "FreeRTOS.h"
#include "task.h"
#include <platform_stdlib.h>
#include "platform_opts.h"
#include "osdep_service.h"
#include "section_config.h"

#include "example_vl53l8cx.h"
#include "app_tof.h"





void example_vl53l8cx_thread(void *param)
{
   printf("-----------example_vl53l8cx_thread----------\r\n");
   
   MX_TOF_Init();
   

   while (1)
   {
	//   MX_TOF_Process();

   //  printf("example_vl53l8cx_thread\r\n");
   //  vTaskDelay(1000);
   }
   
}



void example_vl53l8cx(void)
{
	if (xTaskCreate(example_vl53l8cx_thread, ((const char *)"example_fatfs_thread"), 1024, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS) {
		printf("\n\r%s xTaskCreate(example_fatfs_thread) failed", __FUNCTION__);
	}
}
