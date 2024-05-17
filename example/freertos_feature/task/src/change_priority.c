/*
This example demonstrates:
how to change task priority;
*/
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "feature_task.h"

#define TASK_STACK_SIZE         1024
#define LOOP_TIMES              2

#define HIGH_PRIORITY           2
#define LOW_PRIORITY            1

#define TASK_WAIT_TIME     pdMS_TO_TICKS(5000UL)
#define TASK_DELAY         pdMS_TO_TICKS(1000UL)
/* The two task functions. */
static void vTask1(void *pvParameters);
static void vTask2(void *pvParameters);

/* Used to hold the handle. */
TaskHandle_t xtask1_handle;
TaskHandle_t xtask2_handle;

TaskHandle_t xTaskNotifyHandle = NULL;


/*-----------------------------------------------------------*/

void CreateTasksForChangePriorityTest(void)
{
    BaseType_t ret = pdPASS;
    xTaskNotifyHandle = xTaskGetCurrentTaskHandle();
    u32 task_ret;
    /* Create the first task at priority 2.  This time the task parameter is
    not used and is set to NULL.  The task handle is also not used so likewise
    is also set to NULL. */
    ret = xTaskCreate(vTask1, "ChangePriority Task 1", TASK_STACK_SIZE, NULL, HIGH_PRIORITY, &xtask1_handle);
    if (ret != pdPASS)
    {
        xtask1_handle = NULL;
        vPrintStringAndNumber("ChangePriority Task 1 create failed: ", ret);
        goto err_ret;
    }
    /* The task is created at priority 2 ^. */

    /* Create the second task at priority 1 - which is lower than the priority
    given to Task1.  Again the task parameter is not used so is set to NULL -
    BUT this time we want to obtain a handle to the task so pass in the address
    of the xtask2_handle variable. */
    ret = xTaskCreate(vTask2, "ChangePriority Task 2", TASK_STACK_SIZE, NULL, LOW_PRIORITY, &xtask2_handle);
    if (ret != pdPASS)
    {
        xtask2_handle = NULL;
        vPrintStringAndNumber("ChangePriority Task 2 create failed: ", ret);
        goto err_ret;
    }
    /* The task handle is the last parameter ^^^^^^^^^^^^^ */

    ret = xTaskNotifyWait(0,0,&task_ret,TASK_WAIT_TIME);
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }
    ret = xTaskNotifyWait(0,0,&task_ret,TASK_WAIT_TIME);
    if(ret != pdPASS || task_ret != 1)
    {
        goto err_ret;
    }
    printf("%s@%d: Tasks for change priority [success].\r\n", __func__, __LINE__);
    return;

err_ret:
    printf("%s@%d: Tasks for change priority [failure].\r\n", __func__, __LINE__);
}

/*-----------------------------------------------------------*/

void vTask1(void *pvParameters)
{
    UBaseType_t uxMyPriority;
    UBaseType_t uxTask2Priority;
    uint32_t task_ret = 1;
    /* This task will always run before Task2 as it has the higher priority.
    Neither Task1 nor Task2 ever block so both will always be in either the
    Running or the Ready state.

    Query the priority at which this task is running - passing in NULL means
    "return our own priority". */
    uxMyPriority = uxTaskPriorityGet(NULL);
    uxTask2Priority = uxTaskPriorityGet(xtask2_handle);

    /* Setting the Task2 priority above the Task1 priority will cause
    Task2 to immediately start running (as then Task2 will have the higher
    priority of the    two created tasks). */
    vPrintString("Change Task2 priority to Task1 priority\r\n");
    vTaskPrioritySet(xtask2_handle, uxMyPriority);
    /* Print out the name of this task. */
    vPrintf("ChangePriority Task1 is running, Task1 Priority=%d\r\n", uxMyPriority);

    /* Task1 will only run when it has a priority higher than Task2.
    Therefore, for this task to reach this point Task2 must already have
    executed and set its priority back down to 0. */
    vTaskDelay(TASK_DELAY);
    /*in Task2, Task1 priority have been changed to low priority.*/
    uxMyPriority = uxTaskPriorityGet(NULL);
    if(uxMyPriority != LOW_PRIORITY)
    {
        task_ret = 0;
        FTASK_ERROR("Task1 priority set failed. priority= %d", uxMyPriority);
    }

    xTaskNotify(xTaskNotifyHandle, task_ret, eSetValueWithoutOverwrite);
    vTaskDelete(NULL);
}

/*-----------------------------------------------------------*/

void vTask2(void *pvParameters)
{
    UBaseType_t uxMyPriority;
    UBaseType_t uxTask1Priority;
    uxMyPriority = uxTaskPriorityGet(NULL);
    uxTask1Priority = uxTaskPriorityGet(xtask1_handle);
    u32 task_ret = 1;

    /* Set our priority back down to its original value.  Passing in NULL
    as the task handle means "change our own priority".  Setting the
    priority below that of Task1 will cause Task1 to immediately start
    running again. */
    vPrintString("Change the Task1 priority to low priority\r\n");
    vTaskPrioritySet(xtask1_handle, LOW_PRIORITY);
    /* Print out the name of this task. */
    vPrintf("ChangePriority Task2 is running, Task2 Priority=%d\r\n", uxMyPriority);
    vTaskDelay(TASK_DELAY);
    /* in Task1, Task2 priority have been set Task1 priority */
    if(uxMyPriority != uxTask1Priority)
    {
        task_ret = 0;
        FTASK_ERROR("Task2 priority set failed. priority= %d", uxMyPriority);
    }
    xTaskNotify(xTaskNotifyHandle, task_ret, eSetValueWithoutOverwrite);

    vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/




