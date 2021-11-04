/* FreeRTOS timer tests*/
#include <stdio.h>
#include "unity.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

static void TimerCallback(TimerHandle_t timer)
{
    volatile int *count;
    count = (volatile int *)pvTimerGetTimerID( timer );
    (*count)++;
    printf("Callback timer %p count %p = %d\n", timer, count, *count);
}

void OneshotTimersTest(void)
{
    volatile int count = 0;
    TimerHandle_t oneshot = xTimerCreate("oneshot", 100 / portTICK_PERIOD_MS, pdFALSE,
                                         (void *)&count, TimerCallback);
    TEST_ASSERT(oneshot);
    TEST_ASSERT_EQUAL(pdFALSE, xTimerIsTimerActive(oneshot));
    TEST_ASSERT_EQUAL(0, count);

    TEST_ASSERT( xTimerStart(oneshot, 1) );
    /* give the timer task a chance to process the message */
    vTaskDelay(2); 

    TEST_ASSERT_EQUAL(pdTRUE, xTimerIsTimerActive(oneshot));
    TEST_ASSERT_EQUAL(0, count);

    /* 2.5 timer periods*/
    vTaskDelay(250 / portTICK_PERIOD_MS); 

    TEST_ASSERT_EQUAL(1, count);
    TEST_ASSERT_EQUAL(pdFALSE, xTimerIsTimerActive(oneshot));

    TEST_ASSERT( xTimerDelete(oneshot, 1) );

    printf("OneshotTimersTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
}

void RecurringTimersTest(void)
{
    volatile int count = 0;
    TimerHandle_t recurring = xTimerCreate("oneshot", 100 / portTICK_PERIOD_MS, pdTRUE,
                                          (void *)&count, TimerCallback);
    TEST_ASSERT(recurring);
    TEST_ASSERT_EQUAL(pdFALSE, xTimerIsTimerActive(recurring));
    TEST_ASSERT_EQUAL(0, count);
    TEST_ASSERT( xTimerStart(recurring, 1) );

    /* let timer task process the queue*/
    vTaskDelay(2); 
    TEST_ASSERT_EQUAL(pdTRUE, xTimerIsTimerActive(recurring));
    TEST_ASSERT_EQUAL(0, count);

    /*2.5 timer periods*/
    vTaskDelay(250 / portTICK_PERIOD_MS); 

    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL(pdTRUE, xTimerIsTimerActive(recurring));

    TEST_ASSERT( xTimerStop(recurring, 1) );

    TEST_ASSERT_EQUAL(2, count);

    /* One more timer period*/
    vTaskDelay(100 / portTICK_PERIOD_MS); 

    /* hasn't gone up*/
    TEST_ASSERT_EQUAL(2, count);
    TEST_ASSERT_EQUAL(pdFALSE, xTimerIsTimerActive(recurring));
    TEST_ASSERT( xTimerDelete(recurring, 1) );

    printf("RecurringTimersTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
}

void StaticTimersTest(void)
{
    /*进入临界区*/
    taskENTER_CRITICAL(); 
    StaticTimer_t static_timer;
    TimerHandle_t created_timer;
    volatile int count = 0;

    created_timer = xTimerCreateStatic("oneshot", 1000 / portTICK_PERIOD_MS,
                                    pdTRUE,
                                    (void *)&count,
                                    TimerCallback,
                                    &static_timer);
    /*Start Timer*/                                    
    xTimerStart(created_timer, 1);      

    TEST_ASSERT_NOT_NULL(created_timer);
    /*退出临界区*/
    taskEXIT_CRITICAL(); 

    printf("StaticTimersTest end\r\n");
    while(1)
    {
        vTaskDelay(200);
    }
}
