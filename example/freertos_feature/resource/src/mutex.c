/*
This example demonstrates:
Rewriting vPrintString() to use a mutex semaphore.
*/
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

#define TASK_STACK_SIZE             1024
#define QUEUE_LENGTH                1
#define PRINT_1_TASK_PARAM          (void *)0
#define PRINT_2_TASK_PARAM          (void *)1
#define PRINT_1_TASK_PRIORITY       3
#define PRINT_2_TASK_PRIORITY       4
#define MUTEX_PRINT_TIMES           4
#define MUTEX_EXAMPLE_TIMEOUT       pdMS_TO_TICKS(5000U)
#define MUTEX_TAKE_TIMEOUT          pdMS_TO_TICKS(2000U)

enum
{
    MUTEX_EXAMPLE_SUCCESS = 0,
    MUTEX_EXAMPLE_UNKNOWN_STATE,
    MUTEX_EXAMPLE_QUEUE_CREATE_FAILURE,
    MUTEX_EXAMPLE_FAILURE,          
};

static char *pcStringsToPrint[] =
{
    "Task 1 ******************************\n",
    "Task 2 ==============================\n"
};

static xSemaphoreHandle xMutex;
static QueueHandle_t xQueue = NULL;

static void prvMutexPrintString(const char *pcString)
{
    xSemaphoreTake(xMutex, MUTEX_TAKE_TIMEOUT);
    printf("Mutex pcString = %s", pcString);
    xSemaphoreGive(xMutex);
}

static void prvPrintTask(void *pvParameters)
{
    int task_res = MUTEX_EXAMPLE_UNKNOWN_STATE;
    int print_time = 0;
    int iIndexToString = (int)(uintptr)pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(1000UL);

    for (int loop = 0; loop < MUTEX_PRINT_TIMES; loop++)
    {
        prvMutexPrintString(pcStringsToPrint[iIndexToString]);
        print_time++;

        /* Just delay with random time,
        Don't use rand() in secure applications. It's not reentrant!*/
        vTaskDelay(/*rand() & 0x3FF*/xDelay);
    }

    if ((print_time == MUTEX_PRINT_TIMES) && (pvParameters == PRINT_2_TASK_PARAM))
    {
        task_res = MUTEX_EXAMPLE_SUCCESS;
        xQueueSend(xQueue, &task_res, 0);
    }

    vTaskDelete(NULL);
}

int CreateResourceTasks(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = MUTEX_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(QUEUE_LENGTH, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        vPrintString("xQueue create failed.");
        task_res = MUTEX_EXAMPLE_QUEUE_CREATE_FAILURE;
        goto exit;
    }

    xMutex = xSemaphoreCreateMutex();

    if (xMutex != NULL)
    {
        xTaskCreate(prvPrintTask, "Mutex Print1", TASK_STACK_SIZE, PRINT_1_TASK_PARAM, PRINT_1_TASK_PRIORITY, NULL);
        xTaskCreate(prvPrintTask, "Mutex Print2", TASK_STACK_SIZE, PRINT_2_TASK_PARAM, PRINT_2_TASK_PRIORITY, NULL);
    }

    xReturn = xQueueReceive(xQueue, &task_res, MUTEX_EXAMPLE_TIMEOUT);
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

    if (xMutex != NULL)
    {
        vSemaphoreDelete(xMutex);
    }

    if (task_res != MUTEX_EXAMPLE_SUCCESS)
    {
        vPrintString("Resource mutex feature example [failure]");
        return task_res;
    }
    else
    {
        vPrintString("Resource mutex feature example [success].");
        return task_res;
    }
}