/*
 Test for FreeRTOS vTaskDelayUntil() function by comparing the delay period of
 the function to comparing it to ref clock.
*/

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"
#include "ft_types.h"

#define NO_OF_CYCLES    5
#define NO_OF_TASKS_PER_CORE 2
#define TICKS_TO_DELAY 10
#define TICK_RATE   configTICK_RATE_HZ
#define TICK_PERIOD_US (1000000/TICK_RATE)
#define IDEAL_DELAY_PERIOD_MS  ((1000*TICKS_TO_DELAY)/TICK_RATE)
#define IDEAL_DELAY_PERIOD_US   (IDEAL_DELAY_PERIOD_MS*1000)

#define TICKS_TO_MS(x)  (((x)*1000)/TICK_RATE)
#define REF_TO_ROUND_MS(x)    (((x)+500)/1000)

static SemaphoreHandle_t task_delete_semphr;

static void DelayingTask(void* arg)
{
    int i = 0;
    u64 ref_prev=0;
    u64 ref_current=0;
    TickType_t last_wake_time;
    TickType_t ticks_before_delay;

    vTaskDelay(1);  /*Delay until next tick to synchronize operations to tick boundaries*/

    last_wake_time = xTaskGetTickCount();
    ticks_before_delay = last_wake_time;
    
    for(i = 0; i < NO_OF_CYCLES; i++)
    {
        /*Delay of TICKS_TO_DELAY*/
        vTaskDelayUntil(&last_wake_time, TICKS_TO_DELAY);
        
        /*Get current ref clock*/
        TEST_ASSERT_EQUAL(IDEAL_DELAY_PERIOD_MS, TICKS_TO_MS(xTaskGetTickCount() - ticks_before_delay));

        ref_prev = ref_current;
        ticks_before_delay = last_wake_time;
    }

    /*Delete Task after prescribed number of cycles*/
    xSemaphoreGive(task_delete_semphr);
    vTaskDelete(NULL);
}

void TaskDelayUntilTest(void)
{
    int i = 0;
    task_delete_semphr = xQueueCreateCountingSemaphore(NO_OF_TASKS_PER_CORE, 0);

    xTaskCreate(DelayingTask, "delay_pinned", 1024, NULL, 1, NULL);
    xTaskCreate(DelayingTask, "delay_no_aff", 1024, NULL, 1, NULL);

    for(i = 0; i < NO_OF_TASKS_PER_CORE; i++)
    {
        xSemaphoreTake(task_delete_semphr, portMAX_DELAY);
    }

    /*Cleanup*/
    vSemaphoreDelete(task_delete_semphr);
    
    printf("TaskDelayUntilTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
    
}
