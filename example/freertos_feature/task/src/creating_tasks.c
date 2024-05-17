/*
This example demonstrates:
how to create and delete tasks;
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "fkernel.h"
#include "task.h"
#include <stdio.h>

#define DELAY_LOOP_COUNT        ( 0x1ffffff )
#define TASK_STACK_SIZE         1024

#define SEMAPHORE_WAIT_TIME     pdMS_TO_TICKS(5000UL)

/* The task functions. */
static void vTask1(void *pvParameters);
static void vTask2(void *pvParameters);

static SemaphoreHandle_t xSemCount = NULL;
/*-----------------------------------------------------------*/

void CreateTasks(void)
{
    /* Create one of the two tasks. */
    BaseType_t ret = pdPASS;
    
    xSemCount = xSemaphoreCreateCounting(2,0);

    ret = xTaskCreate(vTask1,      /* Pointer to the function that implements the task. */
                "Create Task 1",    /* Text name for the task.  This is to facilitate debugging only. */
                TASK_STACK_SIZE,        /* Stack depth - most small microcontrollers will use much less stack than this. */
                NULL,       /* We are not using the task parameter. */
                1,          /* This task will run at priority 1. */
                NULL);        /* We are using the task handle to delete this task. */
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Create Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* Create the other task in exactly the same way. */
    ret = xTaskCreate(vTask2, "Create Task 2", TASK_STACK_SIZE, NULL, 1, NULL);
    if (ret != pdPASS)
    {

        vPrintStringAndNumber("Create Task 2 create failed: ", ret);
        goto err_ret;
    }

    ret = xSemaphoreTake(xSemCount, SEMAPHORE_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }
    ret = xSemaphoreTake(xSemCount, SEMAPHORE_WAIT_TIME);
    if(ret != pdPASS)
    {
        goto err_ret;
    }
    printf("%s@%d: Tasks create [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks create [failure].\r\n", __func__, __LINE__);
}

/*-----------------------------------------------------------*/

static void vTask1(void *pvParameters)
{
    const char *pcTaskName = "Create Task 1 is running\r\n";
    volatile uint32_t ul;

    /* Print out the name of this task. */
    vPrintString(pcTaskName);

        /* Delay for a period. */
    for (ul = 0; ul < DELAY_LOOP_COUNT; ul++)
    {
        /* This loop is just a very crude delay implementation.  There is
        nothing to do in here.  Later exercises will replace this crude
        loop with a proper delay/sleep function. */
    }
    xSemaphoreGive(xSemCount);
    vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

static void vTask2(void *pvParameters)
{
    const char *pcTaskName = "Create Task 2 is running\r\n";
    volatile uint32_t ul;
    /* Print out the name of this task. */
    vPrintString(pcTaskName);

        /* Delay for a period. */
    for (ul = 0; ul < DELAY_LOOP_COUNT; ul++)
    {
        /* This loop is just a very crude delay implementation.  There is
        nothing to do in here.  Later exercises will replace this crude
        loop with a proper delay/sleep function. */
    }
    xSemaphoreGive(xSemCount);
    vTaskDelete(NULL);
}