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

#define TASK_STACK_SIZE                 1024
#define QUEUE_LENGTH                    1
#define PRINT_QUEUE_LENGTH              5
#define PRINT_1_TASK_PARAM              (void *)0
#define PRINT_2_TASK_PARAM              (void *)1
#define PRINT_1_TASK_PRIORITY           3
#define PRINT_2_TASK_PRIORITY           4
#define PRINT_GATEKEEPER_TASK_PRIORITY  1
#define PRINT_TIMES                     4
#define GATEKEEPER_RECV_TIMES           10
#define GATEKEEPER_EXAMPLE_TIMEOUT      pdMS_TO_TICKS(30000U)
#define GATEKEEPER_RECV_TIMEOUT         pdMS_TO_TICKS(3000U)

enum
{
    GATEKEEPER_EXAMPLE_SUCCESS = 0,
    GATEKEEPER_EXAMPLE_UNKNOWN_STATE,
    GATEKEEPER_EXAMPLE_QUEUE_CREATE_FAILURE,
    GATEKEEPER_EXAMPLE_FAILURE,          
};

static char *pcStringsToPrint[] =
{
    "Gatekeeper Task 1 *********************************\n",
    "Gatekeeper Task 2 =================================\n",
    "Gatekeep Message printed from the tick hook #####\n"
};

static xQueueHandle xPrintQueue;
static QueueHandle_t xQueue = NULL;

static void prvStdioGatekeeperTask(void *pvParameters)
{
    int task_res = GATEKEEPER_EXAMPLE_UNKNOWN_STATE;
    BaseType_t xReturn = pdPASS;
    int iRecvTimes = 0;
    char *pcMessageToPrint;
    for (;;)
    {
        xQueueReceive(xPrintQueue, &pcMessageToPrint, GATEKEEPER_RECV_TIMEOUT);
        if (xReturn == pdFAIL)
        {
            vPrintString("xPrintQueue receive timeout.");
            break;
        }
        else
        {
            iRecvTimes++;
        }
        printf( "%s", pcMessageToPrint );
        if (iRecvTimes == GATEKEEPER_RECV_TIMES)
        {
            task_res = GATEKEEPER_EXAMPLE_SUCCESS;
            xQueueSend(xQueue, &task_res, 0);
            break;  
        }
    }

    vTaskDelete(NULL);
}

static void prvPrintTask(void *pvParameters)
{
    int iIndexToString;
    iIndexToString = (int)(uintptr)pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(1000UL);

    for (int loop = 0; loop < PRINT_TIMES; loop++)
    {
        xQueueSendToBack(xPrintQueue, &(pcStringsToPrint[iIndexToString]), 0);
        vTaskDelay(xDelay);
    }

    vTaskDelete(NULL);
}

void vApplicationTickHook(void)
{
    static int iCount = 0;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Ensure the xPrintQueue has been created, otherwise the xQueueSendToFrontFromISR function will fail */
    if (xPrintQueue == NULL)
        return;
    iCount++;
    if (iCount >= 2000)
    {
        xQueueSendToFrontFromISR(xPrintQueue,
                                 &(pcStringsToPrint[2]),
                                 &xHigherPriorityTaskWoken); /* not needed */
        iCount = 0;
    }
}

int CreateGatekeeperTasks(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = GATEKEEPER_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        vPrintString("xQueue create failed.");
        task_res = GATEKEEPER_EXAMPLE_QUEUE_CREATE_FAILURE;
        goto exit;
    }

    xPrintQueue = xQueueCreate(PRINT_QUEUE_LENGTH, sizeof(char *));
    if (xPrintQueue == NULL)
    {
        vPrintString("xPrintQueue create failed.");
        task_res = GATEKEEPER_EXAMPLE_QUEUE_CREATE_FAILURE;
        goto exit;
    }
    else
    {
        xTaskCreate(prvPrintTask, "Gatekeeper Print1", TASK_STACK_SIZE, PRINT_1_TASK_PARAM, PRINT_1_TASK_PRIORITY, NULL);
        xTaskCreate(prvPrintTask, "Gatekeeper Print2", TASK_STACK_SIZE, PRINT_2_TASK_PARAM, PRINT_2_TASK_PRIORITY, NULL);
        xTaskCreate(prvStdioGatekeeperTask, "Gatekeeper", TASK_STACK_SIZE, NULL, PRINT_GATEKEEPER_TASK_PRIORITY, NULL);
    }

    xReturn = xQueueReceive(xQueue, &task_res, GATEKEEPER_EXAMPLE_TIMEOUT);
    if (xReturn == pdFAIL)
    {
        vPrintString("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (xPrintQueue != NULL)
    {
        vQueueDelete(xPrintQueue);
    }

    if (task_res != GATEKEEPER_EXAMPLE_SUCCESS)
    {
        vPrintString("Resource Gatekeeper feature example [failure]");
        return task_res;
    }
    else
    {
        vPrintString("Resource Gatekeeper feature example [success].");
        return task_res;
    }
}