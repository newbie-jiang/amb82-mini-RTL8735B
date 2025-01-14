/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lcd.h"
#include "lcd_init.h"
#include "spi_api.h"


extern spi_t  spi_master;


static void prvST7789_lcd_Task(void *pvParameters)
{
      LCD_Init();

	  LCD_Fill(0,0,240,320,WHITE);

      Draw_Circle(100,100,50,RED);

      LCD_ShowString(50,50,"DISPLAY",BLUE,WHITE,24,1);

	  vTaskDelay(1000);

	while (1)
	{
	    // printf("ST7789 Task run\r\n");
        //  LCD_Fill(0,0,240,320,RED);


		vTaskDelay(100);
	}
}


void vStart_ST7789_Tasks(uint16_t usTaskStackSize, UBaseType_t uxTaskPriority)
{
	BaseType_t x = 0L;
	printf("\nExample: ST7789 \n");

	xTaskCreate(prvST7789_lcd_Task,	/* The function that implements the task. */
				"ST7789",			/* Just a text name for the task to aid debugging. */
				usTaskStackSize + 128,	/* The stack size is defined in FreeRTOSIPConfig.h. */
				(void *)x,		    /* The task parameter, not used in this case. */
				uxTaskPriority,		/* The priority assigned to the task is defined in FreeRTOSConfig.h. */
				NULL);				/* The task handle is not used. */
}

void example_st7789_lcd(void)
{
	vStart_ST7789_Tasks(4096, tskIDLE_PRIORITY + 4);
}
/*-----------------------------------------------------------*/


