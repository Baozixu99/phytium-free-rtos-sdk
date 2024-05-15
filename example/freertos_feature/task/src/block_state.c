/*
This example demonstrates:
how to make task block state;
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "feature_task.h"
#include <stdio.h>

#define TASK_STACK_SIZE         1024

#define SEMAPHORE_WAIT_TIME     pdMS_TO_TICKS(5000UL)
#define TASK_DELAY         pdMS_TO_TICKS(1000UL)


SemaphoreHandle_t xSemaCount = NULL;
/* The task function. */
static void vTaskFunction1(void *pvParameters);
static void vTaskFunction2(void *pvParameters);


/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Block Task 1 is running\r\n";
static const char *pcTextForTask2 = "Block Task 2 is running\r\n";

/*-----------------------------------------------------------*/

void CreateTasksForBlockTest(void)
{
    BaseType_t ret = pdPASS;
    xSemaCount = xSemaphoreCreateCounting(2,0);

    /* Create the first task at priority 1... */
    ret = xTaskCreate(vTaskFunction1, "Block Task 1", TASK_STACK_SIZE, (void *)pcTextForTask1, 1, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Block Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    ret = xTaskCreate(vTaskFunction2, "Block Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 2, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Block Task 2 create failed: ", ret);
        goto err_ret;
    }

    ret = xSemaphoreTake(xSemaCount,SEMAPHORE_WAIT_TIME);
    if(ret != pdPASS)
    {
        FTASK_ERROR("Failed to get semaphore.");
        goto err_ret;
    }

    ret = xSemaphoreTake(xSemaCount,SEMAPHORE_WAIT_TIME);
    if(ret != pdPASS)
    {
        FTASK_ERROR("Failed to get semaphore.");
        goto err_ret;
    }

    printf("%s@%d: Tasks for block [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks for block [failure].\r\n", __func__, __LINE__);

}

/*-----------------------------------------------------------*/

static void vTaskFunction1(void *pvParameters)
{
    char *pcTaskName;
    BaseType_t ret;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;
    /* Print out the name of this task. */
    vPrintString(pcTaskName);

    /* Delay for a period.  This time a call to vTaskDelay() is used which
    places the task into the Blocked state until the delay period has
    expired.  The parameter takes a time specified in 'ticks', and the
    pdMS_TO_TICKS() macro is used (where the xDelay constant is
    declared) to convert 3000 milliseconds into an equivalent time in
    ticks. */
    vTaskDelay(TASK_DELAY);
    xSemaphoreGive(xSemaCount);

err_ret:
    vTaskDelete(NULL);

}

static void vTaskFunction2(void *pvParameters)
{
    char *pcTaskName;

    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* Print out the name of this task. */
    vPrintString(pcTaskName);

    /* Delay for a period.  This time a call to vTaskDelay() is used which
    places the task into the Blocked state until the delay period has
    expired.  The parameter takes a time specified in 'ticks', and the
    pdMS_TO_TICKS() macro is used (where the xDelay constant is
    declared) to convert 3000 milliseconds into an equivalent time in
    ticks. */
    vTaskDelay(TASK_DELAY);
    xSemaphoreGive(xSemaCount);
    vTaskDelete(NULL);

}

