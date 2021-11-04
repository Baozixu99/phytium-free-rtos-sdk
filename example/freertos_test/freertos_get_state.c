#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"
#include "ft_types.h"

static TaskHandle_t blocked_task_handle;
static TaskHandle_t suspended_task_handle;
static SemaphoreHandle_t done_sem;

void TaskGetState(void* arg)
{
    /*Current task should return eRunning*/
    TEST_ASSERT(eTaskGetState(xTaskGetCurrentTaskHandle()) == eRunning);
    /*Idle task of current core should return eReady*/
    TEST_ASSERT(eTaskGetState(xTaskGetIdleTaskHandle()) == eReady);
    /*Blocked Task should return eBlocked*/
    TEST_ASSERT(eTaskGetState(blocked_task_handle) == /*eBlocked*/eSuspended);
    /*Suspended Task should return eSuspended*/
    TEST_ASSERT(eTaskGetState(suspended_task_handle) == eSuspended);

    xSemaphoreGive(done_sem);
    vTaskDelete(NULL);
}

void BlockedTask(void *arg)
{
    u32 notify_value;

    while(1)
    {
        xTaskNotifyWait(0, 0xFFFFFFFF, &notify_value, portMAX_DELAY);
    }
}

void SuspendedTask(void *arg)
{
    while(1)
    {
        vTaskSuspend(NULL);
    }
}

void TaskGetStateTest(void)
{
    done_sem = xQueueCreateCountingSemaphore(1, 0);
    /*Create blocked and suspended task*/
    xTaskCreate(BlockedTask, "Blocked task", 1024, NULL, TSK_PRIORITY, &blocked_task_handle);
    xTaskCreate(SuspendedTask, "Suspended task", 1024, NULL, TSK_PRIORITY, &suspended_task_handle);
    
    /*Create testing task*/
    xTaskCreate(TaskGetState, "Test task", 1024, NULL, TSK_PRIORITY, NULL);

    /*Wait until done*/
    xSemaphoreTake(done_sem, portMAX_DELAY);
    
    /*Clean up blocked and suspended tasks*/
    vTaskDelete(blocked_task_handle);
    blocked_task_handle = NULL;
    vTaskDelete(suspended_task_handle);
    suspended_task_handle = NULL;
    vSemaphoreDelete(done_sem);
    printf("end TaskGetStateTest \n");
    while (1) 
    {
        vTaskDelay(200);
    }
}
