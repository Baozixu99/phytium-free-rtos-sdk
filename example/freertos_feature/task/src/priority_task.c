/*
This example demonstrates:
how to create and delete tasks use priority;
*/
#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "feature_task.h"

/* Used as a loop counter to create a very crude delay. */
#define DELAY_LOOP_COUNT        ( 0x1fffffff )
#define QUEUE_WAIT_TIME        pdMS_TO_TICKS(10000U)
#define LOOP_TIMES              2

#define TASK_STACK_SIZE         1024

#define HIGH_PRIORITY           2
#define LOW_PRIORITY            1

static xTaskHandle xtask1_handle;




/* The task function. */
static void vTaskFunction(void *pvParameters);

/* Define the strings that will be passed in as the task parameters.  These are
defined const and off the stack to ensure they remain valid when the tasks are
executing. */
static const char *pcTextForTask1 = "Priority Task 1 is running\r\n";
static const char *pcTextForTask2 = "Priority Task 2 is running\r\n";

static QueueHandle_t xQueue = NULL;
/*-----------------------------------------------------------*/


void CreateTasksForPriorityTest(void)
{
    BaseType_t ret = pdPASS;
    int task_ret = 0;
    xQueue = xQueueCreate(1,sizeof(int));
    if(xQueue == NULL)
    {
        FTASK_ERROR("xQueue create failed.");
        goto err_ret;
    }
    /* Create the first task at priority 1... */
    ret = xTaskCreate(vTaskFunction, "Priority Task 1", TASK_STACK_SIZE, (void *)pcTextForTask1, LOW_PRIORITY, &xtask1_handle);
    if (ret != pdPASS)
    {
        xtask1_handle = NULL;
        vPrintStringAndNumber("Priority Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* ... and the second task at priority 2.  The priority is the second to
    last parameter. */
    ret = xTaskCreate(vTaskFunction, "Priority Task 2", TASK_STACK_SIZE, (void *)pcTextForTask2, HIGH_PRIORITY, NULL);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("Priority Task 2 create failed: ", ret);
        goto err_ret;
    }
    ret = xQueueReceive(xQueue, &task_ret, QUEUE_WAIT_TIME);
    /* task 1 will not excute, delete directly.*/
    if (xtask1_handle)
    {
        vTaskDelete(xtask1_handle);
    }
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }

    printf("%s@%d: Tasks for priority [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    
    printf("%s@%d: Tasks for priority [failure].\r\n", __func__, __LINE__);
}


/*-----------------------------------------------------------*/

static void vTaskFunction(void *pvParameters)
{
    char *pcTaskName;
    volatile uint32_t ul;
    UBaseType_t uxPriority;
    int i = 0;
    int task_ret = 1;
    /* The string to print out is passed in via the parameter.  Cast this to a
    character pointer. */
    pcTaskName = (char *) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Print out the name of this task. */
        uxPriority = uxTaskPriorityGet(NULL);
        printf("Current task's priority is %d\r\n",uxPriority);

        if(uxPriority != HIGH_PRIORITY)
        {
            task_ret = 0;
        }
        /* Delay for a period. */
        for (ul = 0; ul < DELAY_LOOP_COUNT; ul++)
        {
            /* This loop is just a very crude delay implementation.  There is
            nothing to do in here.  Later exercises will replace this crude
            loop with a proper delay/sleep function. */
        }
        
        if(++i > LOOP_TIMES)
        {
            break;
        }
    }
ret:
    xQueueSend(xQueue, &task_ret, 0);
    vTaskDelete(NULL);

}
