#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#define TASK_STACK_SIZE         1024
#define TASK_WAIT_TIME    pdMS_TO_TICKS(5000UL)

/* The task function. */
static void vTaskFunction(void *pvParameters);

/* A variable that is incremented by the idle task hook function. */
static uint32_t ulIdleCycleCount = 0UL;

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Idle Task 1 is running";
static const char *pcTextForTask2 = "Idle Task 2 is running";

static TaskHandle_t xTaskNotifyHandle = NULL;
/*-----------------------------------------------------------*/

void CreateTasksForIdleTask(void)
{
    BaseType_t ret;
    xTaskNotifyHandle = xTaskGetCurrentTaskHandle();
    uint32_t task_ret;
    /* Create the first task at priority 1... */
    ret = xTaskCreate(vTaskFunction, "Idle Task 1",
                      TASK_STACK_SIZE, (void *)pcTextForTask1, 1, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Idle Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    ret = xTaskCreate(vTaskFunction, "Idle Task 2",
                      TASK_STACK_SIZE, (void *)pcTextForTask2, 2, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Idle Task 2 create failed: ", ret);
        goto err_ret;
    }

    ret = xTaskNotifyWait(0,0, &task_ret, TASK_WAIT_TIME);
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }
    ret = xTaskNotifyWait(0,0, &task_ret, TASK_WAIT_TIME);
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }
    printf("%s@%d: Tasks for idle [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks for idle [failure].\r\n", __func__, __LINE__);

}

/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    const TickType_t xDelay = pdMS_TO_TICKS(1000UL);
    int ulIdleCycleCountTemp;
    uint32_t task_ret = 1;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* Print out the name of this task AND the number of times ulIdleCycleCount
    has been incremented. */
    vPrintf("%s, count: %lu\n", pcTaskName, ulIdleCycleCount);
    ulIdleCycleCountTemp = ulIdleCycleCount;
    /* Delay for a period.  This time we use a call to vTaskDelay() which
    puts the task into the Blocked state until the delay period has expired.
    The delay period is specified in 'ticks'. */
    vTaskDelay(xDelay);

    if(ulIdleCycleCount <= ulIdleCycleCountTemp)
    {
        task_ret = 0;
    }

    xTaskNotify(xTaskNotifyHandle, task_ret, eSetValueWithoutOverwrite);
    vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

/* Idle hook functions MUST be called vApplicationIdleHook(), take no parameters,
and return void. */
void vApplicationIdleHook(void)
{
    /* This hook function does nothing but increment a counter. */
    ulIdleCycleCount++;
}
