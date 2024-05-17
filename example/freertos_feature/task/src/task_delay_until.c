/*
This example demonstrates:
how to use delay until in tasks;
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#define TASK_STACK_SIZE         1024

#define DELAY_UNTIL             pdMS_TO_TICKS(1000UL)

#define TASK_WAIT_TIME     pdMS_TO_TICKS(5000UL)

static TaskHandle_t xTaskNotifyHandle = NULL;
/* The task function. */
static void vTaskFunction(void *pvParameters);

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Delay Until Task 1 is running\r\n";
static const char *pcTextForTask2 = "Delay Until Task 2 is running\r\n";

/*-----------------------------------------------------------*/
void CreateTasksForDelayUntilTest(void)
{
    BaseType_t ret = pdPASS;
    int task_ret = 0;
   
    xTaskNotifyHandle = xTaskGetCurrentTaskHandle();

    /* Create the first task at priority 1... */
    ret = xTaskCreate(vTaskFunction, "Delay Until Task 1", TASK_STACK_SIZE, (void *)pcTextForTask1, 1, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Delay Until Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    xTaskCreate(vTaskFunction, "Delay Until Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 2, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Delay Until Task 2 create failed: ", ret);
        goto err_ret;
    }

    ret = ulTaskNotifyTake(pdTRUE,TASK_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }
    ret = ulTaskNotifyTake(pdTRUE,TASK_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }

    printf("%s@%d: Tasks for delay until [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks for delay until [failure].\r\n", __func__, __LINE__);
}

/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    TickType_t xLastWakeTime;
    TickType_t xLastWakeTimeTemp;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* Print out the name of this task. */
    vPrintString(pcTaskName);
    /* The xLastWakeTime variable needs to be initialized with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();
    xLastWakeTimeTemp = xLastWakeTime;

    /* We want this task to execute exactly every 1000 milliseconds.  As per
    the vTaskDelay() function, time is measured in ticks, and the
    pdMS_TO_TICKS() macro is used to convert this to milliseconds.
    xLastWakeTime is automatically updated within vTaskDelayUntil() so does not
    have to be updated by this task code. */
    vTaskDelayUntil(&xLastWakeTime, DELAY_UNTIL);

    if(xLastWakeTime == xLastWakeTimeTemp + DELAY_UNTIL)
    {
        xTaskNotifyGive(xTaskNotifyHandle);
    }
    vTaskDelete(NULL);
}
