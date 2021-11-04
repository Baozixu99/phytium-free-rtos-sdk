/*
 * Test backported deletion behavior by creating tasks of various affinities and
 * check if the task memory is freed immediately under the correct conditions.
 *
 * The behavior of vTaskDelete() results in the immediate freeing of task memory
 * and the immediate execution of deletion callbacks for tasks which are not
 * running, provided they are not pinned to the other core (due to FPU cleanup
 * requirements).
 *
 * If the condition is not met, freeing of task memory and execution of
 * deletion callbacks will still be carried out by the Idle Task.
 */
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"
#include "fsleep.h"
#include "math.h"

#define NO_OF_TSKS  3
#define DELAY_TICKS 2
/* Caps of all memory which is allocated from when a task is created */
#define HEAP_CAPS   (portTcbMemoryCaps | portStackMemoryCaps)

#define DELAY_US_ITERATIONS	1000

/*Deleting self means deleting currently running task*/
static void TaskSelfDel(void *param)
{
    vTaskDelete(NULL);  
}
/*Await external deletion*/
static void TaskExternDel(void *param)
{
    vTaskDelay(portMAX_DELAY);  
}

static void TaskSelfDelDelay(void *param)
{
    u32 delay = *((uint32_t*)param);
    fsleep_microsec(delay);
    vTaskDelete(NULL);
}

void DeleteTaskTest(void)
{
    /*  Test vTaskDelete() on currently running tasks*/
    u32 before_count = uxTaskGetNumberOfTasks();
    int i = 0;
    
    for(i = 0; i < NO_OF_TSKS; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xTaskCreate(TaskSelfDel, "tsk_self", 1024, NULL, configMAX_PRIORITIES - 1, NULL));
    }
    
    /*Minimal delay to see if Idle task cleans up all tasks awaiting deletion in a single tick*/
    vTaskDelay(DELAY_TICKS);    

    /*  Test vTaskDelete() on not currently running tasks */
    TaskHandle_t handles[NO_OF_TSKS];
  
    for(i = 0 ; i < NO_OF_TSKS; i++)
    {
        TEST_ASSERT_EQUAL(pdTRUE, xTaskCreate(TaskExternDel, "tsk_extern", 4096, NULL, configMAX_PRIORITIES - 1, &handles[i] ));
    }

    /*Delete the tasks, memory should be freed immediately*/
    for(i = 0; i < NO_OF_TSKS; i++)
    {
        vTaskDelete(handles[i]);
    }

    /* Test self deleting no affinity task is not removed by idle task of other core before context switch */
    for(i = 0; i < DELAY_US_ITERATIONS; i+= 10)
    {
        /*Sync to next tick interrupt*/
        vTaskDelay(1);                          
        xTaskCreate(TaskSelfDelDelay, "delay", 1024, (void *)&i, configMAX_PRIORITIES - 1, NULL);
         /*Busy wait to ensure no affinity task runs*/
        fsleep_microsec(10);                      
    }

    printf("DeleteTaskTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }

}

typedef struct 
{
    SemaphoreHandle_t sem;
    /* Check the deleted task doesn't keep running after being deleted*/
    volatile u8 deleted; 
} TaskBlocksParam_t;

/* Task blocks as often as possible
   (two or more of these can share the same semaphore and "juggle" it around)
*/
static void TaskBlocksFrequently(void *param)
{
    TEST_ASSERT(param != NULL);
    TaskBlocksParam_t *p = (TaskBlocksParam_t *)param;
    SemaphoreHandle_t sem = p->sem;
    srand(xTaskGetTickCount() ^ (*(TickType_t*)xTaskGetCurrentTaskHandle()));
    while (1) 
    {
        TEST_ASSERT(!p->deleted);
        fsleep_microsec(rand() % 10);
        TEST_ASSERT(!p->deleted);
        xSemaphoreTake(sem, portMAX_DELAY);
        TEST_ASSERT(!p->deleted);
        fsleep_microsec(rand() % 10);
        TEST_ASSERT(!p->deleted);
        xSemaphoreGive(sem);
    }
}

void DeleteBlockTaskTest(void)
{
    TaskHandle_t blocking_tasks;
    TaskBlocksParam_t params = { 0 };
    u8 iter = 0;
    for( iter = 0; iter < 10; iter++) 
    {
        SemaphoreHandle_t sem = xSemaphoreCreateMutex();
        params.deleted = FALSE;
        params.sem = sem;
        TEST_ASSERT_EQUAL(pdTRUE, xTaskCreate(TaskBlocksFrequently, "tsk_block", 4096, &params,
                                                    configMAX_PRIORITIES - 1, &blocking_tasks));
        /*Let the tasks juggle the mutex for a bit*/
        vTaskDelay(5); 
        vTaskDelete(blocking_tasks);
        params.deleted = TRUE;
        
        /* Yield to the idle task for cleanup */
        vTaskDelay(4); 
        vSemaphoreDelete(sem);
    }

    while(1)
    {
        vTaskDelay(200);
    }
}
