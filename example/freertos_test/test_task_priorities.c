/*
  Unit tests for FreeRTOS task priority get/set
*/

#include <stdio.h>
#include <strings.h>
#include "FreeRTOS.h"
#include "task.h"
#include "unity.h"
#include "ft_types.h"

static void CounterTask(void *param)
{
    TEST_ASSERT(param != NULL);
    volatile u32 *counter = (volatile u32 *)param;
    while (1) {
        (*counter)++;
    }
}

void GetAndSetPriorityTest(void)
{
    /* Two tasks per processor */
    TaskHandle_t tasks[2] = { 0 };
    unsigned volatile counters[2] = { 0 };
    
    TEST_ASSERT_EQUAL(UNITY_FREERTOS_PRIORITY, uxTaskPriorityGet(NULL));

    /* create a matrix of counter tasks on each core */
    for (int task = 0; task < 2; task++) {
        xTaskCreate(CounterTask, "count", 2048, (void *)&(counters[task]), UNITY_FREERTOS_PRIORITY - task, &(tasks[task]));
    }
    
    /* check they were created with the expected priorities */
    for (int task = 0; task < 2; task++) {
        TEST_ASSERT_EQUAL(UNITY_FREERTOS_PRIORITY - task, uxTaskPriorityGet(tasks[task]));
    }
    vTaskDelay(10);

    /* at this point, only the higher priority tasks (first index) should be counting */
    printf("counters=%d, %d\n", counters[0], counters[1]);

    /* swap priorities! */
    vTaskPrioritySet(tasks[0], UNITY_FREERTOS_PRIORITY - 1);
    vTaskPrioritySet(tasks[1], UNITY_FREERTOS_PRIORITY);

    /* check priorities have swapped... */
    TEST_ASSERT_EQUAL(UNITY_FREERTOS_PRIORITY-1, uxTaskPriorityGet(tasks[0]));
    TEST_ASSERT_EQUAL(UNITY_FREERTOS_PRIORITY, uxTaskPriorityGet(tasks[1]));

    /* check the tasks which are counting have also swapped now... */
    unsigned old_counters[2];
    old_counters[0] = counters[0];
    old_counters[1] = counters[1];

    vTaskDelay(10);
    TEST_ASSERT_EQUAL(old_counters[0], counters[0]);
    TEST_ASSERT_NOT_EQUAL(old_counters[1], counters[1]);


    /* clean up */
    for (int task = 0; task < 2; task++) {
        vTaskDelete(tasks[task]);
    }

    printf("GetAndSetPriorityTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }

}
