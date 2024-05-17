#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#define TASK_STACK_SIZE         1024

#define TASK_WAIT_TIME     pdMS_TO_TICKS(8000UL)
#define TASK_DELAY              pdMS_TO_TICKS(1000UL)
/* The task functions. */
static void vContinuousProcessingTask(void *pvParameters);
static void vPeriodicTask(void *pvParameters);

static TaskHandle_t xTaskNotifyHandle = NULL;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "BlockingOrNone Continuous task 1 running\r\n";
static const char *pcTextForTask2 = "BlockingOrNone Continuous task 2 running\r\n";
static const char *pcTextForPeriodicTask = "BlockingOrNone Periodic task is running\r\n";

/*-----------------------------------------------------------*/
void CreateTasksForBlockingOrNone(void)
{   
    BaseType_t ret = pdPASS;
    uint32_t task_ret;
    xTaskNotifyHandle = xTaskGetCurrentTaskHandle();
    /* Create two instances of the continuous processing task, both at priority 1. */
    ret = xTaskCreate(vContinuousProcessingTask, "BlockingOrNone Task 1",
                TASK_STACK_SIZE, (void *)pcTextForTask1, 1, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("BlockingOrNone Task 1 create failed: ", ret);
        goto err_ret;
    }

    ret = xTaskCreate(vContinuousProcessingTask, "BlockingOrNone Task 2",
                TASK_STACK_SIZE, (void *)pcTextForTask2, 1, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("BlockingOrNone Task 2 create failed: ", ret);
        goto err_ret;
    }

    /* Create one instance of the periodic task at priority 2. */
    ret = xTaskCreate(vPeriodicTask, "BlockingOrNone Task 3",
                TASK_STACK_SIZE, (void *)pcTextForPeriodicTask, 2, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("BlockingOrNone Task 3 create failed: ", ret);
        goto err_ret;
    }

    ret = ulTaskNotifyTake(pdTRUE, TASK_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }
    ret = ulTaskNotifyTake(pdTRUE, TASK_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }
    ret = ulTaskNotifyTake(pdTRUE, TASK_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }
    printf("%s@%d: Tasks for blocking or none [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks for blocking or none [failure].\r\n", __func__, __LINE__);
}
/*-----------------------------------------------------------*/

void vContinuousProcessingTask(void *pvParameters)
{
    char *pcTaskName;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* Print out the name of this task.  This task just does this repeatedly
    without ever blocking or delaying. */
    vPrintString(pcTaskName);

    vTaskDelay(TASK_DELAY);
  
    xTaskNotifyGive(xTaskNotifyHandle);
    vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vPeriodicTask(void *pvParameters)
{
    TickType_t xLastWakeTime;
   
    /* Print out the name of this task. */
    vPrintString("Periodic task is running\r\n");

    /* The xLastWakeTime variable needs to be initialized with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    
    /* We want this task to execute exactly every 100 milliseconds. */
    vTaskDelayUntil(&xLastWakeTime, TASK_DELAY);
    /*xLastWakeTime have been updated*/

    xTaskNotifyGive(xTaskNotifyHandle);
    vTaskDelete(NULL);
}