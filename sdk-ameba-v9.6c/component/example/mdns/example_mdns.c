/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#include "wifi_conf.h"
#include "lwip_netconf.h"


//This example is original and cannot restart if failed. To use this example, define WAIT_FOR_ACK and not define MQTT_TASK in MQTTClient.h
void prvmdnsTask(void *pvParameters)
{
	
}


void vStartMDNSTasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
	BaseType_t x = 0L;
	printf("\nExample: mdns \n");

	xTaskCreate(prvmdnsTask,	/* The function that implements the task. */
				"mdns",			/* Just a text name for the task to aid debugging. */
				usTaskStackSize + 128,	/* The stack size is defined in FreeRTOSIPConfig.h. */
				(void *)x,		/* The task parameter, not used in this case. */
				uxTaskPriority,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
				NULL);				/* The task handle is not used. */
}

void example_mdns(void)
{
	vStartMDNSTasks(4096, tskIDLE_PRIORITY + 4);
}
/*-----------------------------------------------------------*/


