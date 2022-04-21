/*
 * Test FreeRTOS trace facility functions. These following functions are enabled
 * when configUSE_TRACE_FACILITY is defined 1 in FreeRTOS.
 * Tasks:           uxTaskGetTaskNumber(), uxTaskSetTaskNumber()
 * Queues:          ucQueueGetQueueType(), vQueueSetQueueNumber(), uxQueueGetQueueNumber()
 * Event Groups:    xEventGroupSetBitsFromISR(), xEventGroupClearBitsFromISR(), uxEventGroupGetNumber()
 *
 * Note: uxTaskGetSystemState() is tested in a separate unit test
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "unity.h"
#include "ft_types.h"

 /*Expected Queue Type*/
#define BIN_SEM_QUEUE_TYPE queueQUEUE_TYPE_BINARY_SEMAPHORE    

static QueueHandle_t test_queues;
static TaskHandle_t task_handles;

void taskTraceUtilities(void *arg)
{
    
    TEST_ASSERT(arg != NULL);
    u32 id = (u32)(intptr)arg;

    TaskHandle_t handle = xTaskGetCurrentTaskHandle();

    /*cast and store id as task number*/
    vTaskSetTaskNumber(handle, (UBaseType_t)id);    
    /*store id as queue number*/
    vQueueSetQueueNumber(test_queues, id);    

    /*Wait to start*/
    xSemaphoreTake(test_queues, portMAX_DELAY);

    TEST_ASSERT(uxTaskGetTaskNumber(task_handles) == (UBaseType_t)(0x0F));
    TEST_ASSERT(uxQueueGetQueueNumber(test_queues) == (UBaseType_t)(0x0F));
    TEST_ASSERT(ucQueueGetQueueType(test_queues) == BIN_SEM_QUEUE_TYPE);

    /*Signal done*/
    xSemaphoreGive(test_queues);      
    vTaskDelete(NULL);
}

void TraceFacilityTest(void)
{
    /*Create a queue as binary semaphore */
    test_queues = xSemaphoreCreateBinary();   
    xTaskCreate(taskTraceUtilities, "Test Task", 4096, (void *)(0x0F), TSK_PRIORITY, &task_handles);
    vTaskDelay(10);

    /*Start the tasks*/
    xSemaphoreGive(test_queues);

     /*Small delay to ensure semaphores are taken*/
    vTaskDelay(10);

    /*Wait for done*/
    xSemaphoreTake(test_queues, portMAX_DELAY);
    vSemaphoreDelete(test_queues);

    printf("TraceFacilityTest end\n");
    while(1){
        vTaskDelay(200);
    }
}


#define MAX_TASKS           15
#define TASKS_TO_CREATE     5

static TaskHandle_t created_handles[TASKS_TO_CREATE];
static TaskStatus_t *tsk_status_array;

void CreatedTask(void* arg)
{
    while(1){
        vTaskDelay(100);
    }
}

void TaskGetSystemState(void)
{
    tsk_status_array = calloc(MAX_TASKS, sizeof(TaskStatus_t));
    for(int i = 0; i < TASKS_TO_CREATE; i++){
        xTaskCreate(CreatedTask, "Created Task", 1024, NULL, TSK_PRIORITY, &created_handles[i]);
    }

    /*Get System states*/
    int no_of_tasks = uxTaskGetSystemState(tsk_status_array, MAX_TASKS, NULL);
    TEST_ASSERT((no_of_tasks > 0) && (no_of_tasks <= MAX_TASKS));

    /*Check if get system state has got all created tasks*/
    u8 not_found = FALSE;
    for(int i = 0; i < TASKS_TO_CREATE; i++){
        u8 found = FALSE;
        for(int j = 0; j < MAX_TASKS; j++){
            if(tsk_status_array[j].xHandle == created_handles[i]){
                found = TRUE;
                break;
            }
        }
        if(!found){
            not_found = TRUE;
            break;
        }
    }
    TEST_ASSERT(not_found == FALSE);

    /*Cleanup*/
    for(int i = 0; i < TASKS_TO_CREATE; i++){
        vTaskDelete(created_handles[i]);
    }
    free(tsk_status_array);
    vTaskDelay(10);
}
