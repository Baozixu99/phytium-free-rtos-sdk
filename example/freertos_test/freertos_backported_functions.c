/*
 * Test features that are backported from version FreeRTOS 9.0.0.
 *
 * 1) Test backported timer functions
 *      - xTimerCreateStatic(), vTimerSetTimerId(), xTimerGetPeriod(), xTimerGetExpiryTime()
 * 2) Test backported queue/semaphore functions
 *      - xQueueCreateStatic()
 *      - xSemaphoreCreateBinaryStatic(), xSemaphoreCreateCountingStatic(), uxSemaphoreGetCount()
 *      - xSemaphoreCreateMutexStatic(), xSemaphoreCreateRecursiveMutexStatic()
 * 3) Test static allocation of tasks
 *      - xTaskCreateStaticPinnedToCore()
 * 4) Test static allocation of event group
 *      - xEventGroupCreateStatic()
 * 5) Test Thread Local Storage Pointers and Deletion Callbacks
 *      - vTaskSetThreadLocalStoragePointerAndDelCallback()
 *      - pvTaskGetThreadLocalStoragePointer()
 *
 * Note: The *pcQueueGetName() function is also backported, but is not tested in
 *       the following test cases (see Queue Registry test cases instead)
 */
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "ft_assert.h"

/* ---------------------Test 1: Backported Timer functions-----------------------
 * Test xTimerCreateStatic(), vTimerSetTimerId(), xTimerGetPeriod(), xTimerGetExpiryTime()
 *
 * This test creates a one-shot static timer, sets/checks the timer's id and period. Then ensures
 * the timer cb is executed in a timely fashion.
 */
#define TMR_PERIOD_TICKS  1000
#define TIMER_ID    0xFF
#define TICK_DELTA  5

static StaticTimer_t timer_buffer;
static TickType_t tmr_ideal_exp;

static void TimerCallback(TimerHandle_t xtimer)
{
    /*Check cb is called in timely fashion*/
    printf("xTaskGetTickCount()=%d\n", xTaskGetTickCount());
}

void TimerFuncTest(void)
{
    /*进入临界区*/
    taskENTER_CRITICAL(); 
    /*Create one shot static timer with period TMR_PERIOD_TICKS*/
    TimerHandle_t tmr_handle = xTimerCreateStatic("static_tmr", TMR_PERIOD_TICKS, pdTRUE, NULL, TimerCallback, &timer_buffer);
    printf("xTimerGetPeriod(tmr_handle)=%d\n", xTimerGetPeriod(tmr_handle));

    vTimerSetTimerID(tmr_handle, (void *)TIMER_ID);
    printf("pvTimerGetTimerID(tmr_handle)=%d\n", pvTimerGetTimerID(tmr_handle));

    /*Start Timer*/
    xTimerStart(tmr_handle, 1);

     /*Calculate ideal expiration time*/
    tmr_ideal_exp = xTaskGetTickCount() + TMR_PERIOD_TICKS;    

    /*Need to yield to allow daemon task to process start command, or else expiration time will be NULL*/ 
    vTaskDelay(2);  

    printf("xTimerGetExpiryTime(tmr_handle)=%d\n", xTimerGetExpiryTime(tmr_handle));
    /*Delay until one shot timer has triggered*/
    vTaskDelay(2*TMR_PERIOD_TICKS);     
    
    vTaskDelete(NULL);
    /*退出临界区*/
    taskEXIT_CRITICAL(); 

    while(1)
    {
        vTaskDelay(200);
    }

}


/* ---------------Test backported queue/semaphore functions-------------------
 * xQueueCreateStatic()
 * xSemaphoreCreateBinaryStatic(), xSemaphoreCreateCountingStatic()
 * xSemaphoreCreateMutexStatic(), xSemaphoreCreateRecursiveMutexStatic()
 * uxSemaphoreGetCount() is also tested on the static counting semaphore
 *
 * This test creates various static queue/semphrs listed above and tests them by
 * doing a simple send/give and rec/take.
 */

#define ITEM_SIZE       3
#define NO_OF_ITEMS     3
#define DELAY_TICKS     2
/*Queues, Semaphores, and Mutex use the same queue structure*/
static StaticQueue_t queue_buffer;     
/*Queue storage provided in separate buffer to queue struct*/
static u8 queue_storage_area[(ITEM_SIZE*NO_OF_ITEMS)];   

void QueueFuncTest(void)
{
    /*Test static queue*/
    uint8_t queue_item_to_send[ITEM_SIZE];
    uint8_t queue_item_received[ITEM_SIZE];
    int i = 0;

    for(i = 0; i < ITEM_SIZE; i++)
    {
        queue_item_to_send[i] = (0xF << i);
    }

    QueueHandle_t handle = xQueueCreateStatic(NO_OF_ITEMS, ITEM_SIZE,(uint8_t*) &queue_storage_area, &queue_buffer);
    
    FT_ASSERTVOID(pdTRUE==xQueueSendToBack(handle, &queue_item_to_send, DELAY_TICKS));
    vTaskDelay(1);
    
    FT_ASSERTVOID(pdTRUE==xQueueReceive(handle, queue_item_received, DELAY_TICKS));
    vTaskDelay(1);

    for(i = 0; i < ITEM_SIZE; i++)
    {
         /*Check received contents are correct*/
        FT_ASSERTVOID(queue_item_to_send[i]==queue_item_received[i]);  
    }

    /*Technically not needed as deleting static queue/semphr doesn't clear static memory*/
    vQueueDelete(handle);   

    /*Test static binary semaphore*/
    handle = xSemaphoreCreateBinaryStatic(&queue_buffer);   

    /*Queue and Semphr handles are the same*/
    FT_ASSERTVOID(pdTRUE==xSemaphoreGive(handle));
    vTaskDelay(1);

    FT_ASSERTVOID(pdTRUE==xSemaphoreTake(handle, DELAY_TICKS));
    vTaskDelay(1);

    vSemaphoreDelete(handle);

    /*Test static counting semaphore and uxSemaphoreGetCount()*/
    handle = xSemaphoreCreateCountingStatic(NO_OF_ITEMS, 0, &queue_buffer);
    for(i = 0; i < NO_OF_ITEMS; i++)
    {
        FT_ASSERTVOID(pdTRUE==xSemaphoreGive(handle));
    }
    vTaskDelay(1);
    /*Test uxSemaphoreGetCount()*/
    FT_ASSERTVOID(NO_OF_ITEMS== uxSemaphoreGetCount(handle));    
    for(i = 0; i < NO_OF_ITEMS; i++)
    {
        FT_ASSERTVOID(pdTRUE==xSemaphoreTake(handle, DELAY_TICKS));
    }
    vTaskDelay(1);
    FT_ASSERTVOID(0==uxSemaphoreGetCount(handle));
    vSemaphoreDelete(handle);

    /*Test static mutex*/
    handle = xSemaphoreCreateMutexStatic(&queue_buffer);
    FT_ASSERTVOID(pdTRUE==xSemaphoreTake(handle, DELAY_TICKS));
    vTaskDelay(1);

    /*Current task should now hold mutex*/
    FT_ASSERTVOID(((void *)xTaskGetCurrentTaskHandle()) == xSemaphoreGetMutexHolder(handle)); 
    FT_ASSERTVOID(pdTRUE==xSemaphoreGive(handle));
    vTaskDelay(1);
     /*Mutex should have been released*/
    FT_ASSERTVOID(NULL==xSemaphoreGetMutexHolder(handle));     
    vSemaphoreDelete(handle);

    /*Test static mutex recursive*/
    handle = xSemaphoreCreateRecursiveMutexStatic(&queue_buffer);
    for(i = 0; i < NO_OF_ITEMS; i++)
    {
        FT_ASSERTVOID(pdTRUE==xSemaphoreTakeRecursive(handle, DELAY_TICKS));
    }
    vTaskDelay(1);
     /*Current task should hold mutex*/
    FT_ASSERTVOID(((void *)xTaskGetCurrentTaskHandle())==xSemaphoreGetMutexHolder(handle));  
    for(i = 0; i < NO_OF_ITEMS; i++)
    {
        FT_ASSERTVOID(pdTRUE==xSemaphoreGiveRecursive(handle));
    }
    vTaskDelay(1);
    /*Mutex should have been released*/
    FT_ASSERTVOID(NULL==xSemaphoreGetMutexHolder(handle));      
    vSemaphoreDelete(handle);

    while(1)
    {
        vTaskDelay(200);
    }
    
}

/* -----------------Test backported static task allocation  -------------------
 * Test xTaskCreateStaticPinnedToCore() but creating static task on each core
 * and checking the task cb has run successfully.
 */

#define STACK_SIZE  1024    /*Task stack size*/
static StackType_t task_stack[STACK_SIZE];  /*Static buffer for task stack*/
static StaticTask_t task_buffer;            /*Static buffer for TCB*/
static u8 has_run;

static void Task(void *arg)
{
    has_run = TRUE;       /*Signify static task cb has run*/
    vTaskDelete(NULL);
}

void StaticTaskCreateTest(void)
{
    has_run = FALSE;     /*Clear has_run flag*/

    TaskHandle_t handle = xTaskCreateStatic(Task, "static task", STACK_SIZE, NULL,
                                                        3, (StackType_t *)&task_stack,
                                                        (StaticTask_t *)&task_buffer);

    vTaskDelay(5); /*Allow for static task to run, delete, and idle to clean up*/
    
    FT_ASSERTVOID(NULL!= handle);    /*Check static task was successfully allocated*/
    FT_ASSERTVOID(has_run);    /*Check static task has run*/

    while(1)
    {
        vTaskDelay(200);
    }
}


/* ------------- Test backported static event group allocation -------------------
 * Test xEventGroupCreateStatic() but creating static event group then waiting
 * for an event.
 */

#define WAIT_BITS   0x01    /*Wait for first bit*/

static StaticEventGroup_t event_group;
static EventGroupHandle_t eg_handle;

void StaticEventGroupTest(void)
{
    eg_handle = xEventGroupCreateStatic((StaticEventGroup_t *)&event_group);
    xEventGroupSetBits(eg_handle, WAIT_BITS);
    FT_ASSERTVOID(WAIT_BITS==xEventGroupWaitBits(eg_handle, WAIT_BITS, pdTRUE, pdTRUE, portMAX_DELAY));
    /*Cleanup static event*/
    vEventGroupDelete(eg_handle);
}
