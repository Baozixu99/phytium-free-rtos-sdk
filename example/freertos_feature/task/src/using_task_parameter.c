/*
This example demonstrates:
how to create and delete tasks use parameters;
*/
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "fkernel.h"
#include "task.h"
#include "string.h"
#include "feature_task.h"

/* Used as a loop counter to create a very crude delay. */
#define DELAY_LOOP_COUNT        ( 0x1fffffff )

/* task stack size set. */
#define TASK_STACK_SIZE         1024

#define TASK_DELAY              pdMS_TO_TICKS(1000UL)
#define TASK_WAIT_TIME          pdMS_TO_TICKS(5000UL)


/* The task function. */
static void vTaskFunction1(void *pvParameters);
static void vTaskFunction2(void *pvParameters);


static TaskHandle_t xTaskNotifyHandle = NULL;
/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
const char *pcTextForTask1 = "Parameter Task 1 is running\r\n";
const char *pcTextForTask2 = "Parameter Task 2 is running\r\n";

/*-----------------------------------------------------------*/

void CreateTasksForParamterTest(void)
{
    BaseType_t ret = pdPASS;
    xTaskNotifyHandle = xTaskGetCurrentTaskHandle();
    uint32_t task_ret = 0;
    /* Create one of the two tasks. */
    ret = xTaskCreate(vTaskFunction1,           /* Pointer to the function that implements the task. */
                "Parameter Task 1",             /* Text name for the task.  This is to facilitate debugging only. */
                TASK_STACK_SIZE,                    /* Stack depth - most small microcontrollers will use much less stack than this. */
                (void *)pcTextForTask1, /* Pass the text to be printed in as the task parameter. */
                1,                      /* This task will run at priority 1. */
                NULL);                    /* We are not using the task handle. */
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Parameter Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* Create the other task in exactly the same way.  Note this time that we
    are creating the SAME task, but passing in a different parameter.  We are
    creating two instances of a single task implementation. */
    ret = xTaskCreate(vTaskFunction2, "Parameter Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, 1, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Parameter Task 2 create failed: ", ret);
        goto err_ret;
    }
    ret = xTaskNotifyWait(0, 0, &task_ret, TASK_WAIT_TIME);
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }
    ret = xTaskNotifyWait(0, 0, &task_ret, TASK_WAIT_TIME);
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }
    printf("%s@%d: Tasks for parameter [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks for parameter [failure].\r\n", __func__, __LINE__);

}
/*-----------------------------------------------------------*/

static void vTaskFunction1(void *pvParameters)
{
    char *pcTaskName;
    volatile uint32_t ul;
    uint32_t task_ret;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;
    
    /* Print out the name of this task. */
    vPrintString(pcTaskName);

    if(!strcmp(pcTaskName, pcTextForTask1))
    {
        task_ret = 1;
    }
    else{
        task_ret = 0;
    }

    /* Delay for a period. */
    vTaskDelay(TASK_DELAY);
    xTaskNotify(xTaskNotifyHandle, task_ret, eSetValueWithoutOverwrite);
    vTaskDelete(NULL);
}

static void vTaskFunction2(void *pvParameters)
{
    char *pcTaskName;
    volatile uint32_t ul;
    uint32_t task_ret = 0;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;
    
    /* Print out the name of this task. */
    vPrintString(pcTaskName);
    if(!strcmp(pcTaskName, pcTextForTask2))
    {
        task_ret = 1;
    }
    else{
        task_ret = 0;
    }

    /* Delay for a period. */
    vTaskDelay(TASK_DELAY);
    xTaskNotify(xTaskNotifyHandle, task_ret, eSetValueWithoutOverwrite);
    vTaskDelete(NULL);
}