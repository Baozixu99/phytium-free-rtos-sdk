/*
This example demonstrates: 
Re-writing vPrintString() to use a gatekeeper task.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TASK_STACK_SIZE	    1024

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

#define MAX_MSG_LEN 100

static char *pcStringsToPrint[] = 
{
	"Gatekeeper Task 1 *********************************\n",
	"Gatekeeper Task 2 =================================\n"
	"Gatekeep Message printed from the tick hook #####\n"
};

xQueueHandle xPrintQueue;

static void prvStdioGatekeeperTask(void* pvParameters) 
{
	char* pcMessageToPrint;
	static char cBuffer[MAX_MSG_LEN];

	for (;;)
	{
		xQueueReceive(xPrintQueue, &pcMessageToPrint, portMAX_DELAY);
		strncpy(cBuffer, pcMessageToPrint, MAX_MSG_LEN);
		vPrintString(cBuffer);
	}
}

static void prvPrintTask(void *pvParameters) 
{
	int iIndexToString;
	iIndexToString = (int)(uintptr)pvParameters;

	for (;;) 
	{
		xQueueSendToBack(xPrintQueue, &(pcStringsToPrint[iIndexToString]), 0);

		vTaskDelay(5000);
	}
}

static void vApplicationTickHook(void) 
{
	static int iCount = 0;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	iCount++;
	if (iCount >= 2000) 
	{
		xQueueSendToFrontFromISR(xPrintQueue,
					&(pcStringsToPrint[2]),
					&xHigherPriorityTaskWoken); // not needed
		iCount = 0;
	}
}

void CreateGatekeeperTasks(void)
{
  xPrintQueue = xQueueCreate(5, sizeof(char*));

  srand(0x42);

  if (xPrintQueue != NULL) 
  {
	xTaskCreate(prvPrintTask, "Gatekeeper Print1", TASK_STACK_SIZE, (void*)0, 1, &xtask1_handle);
	xTaskCreate(prvPrintTask, "Gatekeeper Print2", TASK_STACK_SIZE, (void*)1, 2, &xtask2_handle);
	xTaskCreate(prvStdioGatekeeperTask, "Gatekeeper", TASK_STACK_SIZE, NULL, 0, &xtask3_handle);
  }
}

void DeleteGatekeeperTasks(void)
{
    if(xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
        vPrintString("Gatekeeper Print1 deletion \r\n");
    }

    if(xtask2_handle)
    {
        vTaskDelete(xtask2_handle);
        vPrintString("Gatekeeper Print2 deletion \r\n");
    }

	if(xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("Gatekeeper deletion \r\n");
    }
}