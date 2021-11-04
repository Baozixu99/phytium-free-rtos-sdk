
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "unity.h"
#include "portmacro.h"
#include "ft_types.h"

#define NUMBER_OF_ITERATIONS 10
#define SCHEDULING_TIME 10

typedef struct 
{
    SemaphoreHandle_t end_sema;
    u32 before_sched;
    u32 cycles_to_sched;
    TaskHandle_t t1_handle;
} test_context_t;

static void test_task_1(void *arg) 
{
    TEST_ASSERT(arg != NULL);
    test_context_t *context = (test_context_t *)arg;

    for( ;; ) {
        context->before_sched = portGET_RUN_TIME_COUNTER_VALUE();
        taskYIELD();
    }
    vTaskDelete(NULL);
}

static void test_task_2(void *arg) 
{
    TEST_ASSERT(arg != NULL);
    int i = 0;
    test_context_t *context = (test_context_t *)arg;
    u64 accumulator = 0;

    vTaskPrioritySet(NULL, 5);
    vTaskPrioritySet(context->t1_handle, 5);
    
    taskYIELD();
    
    for(i = 0; i < NUMBER_OF_ITERATIONS; i++) 
    {
        accumulator += (portGET_RUN_TIME_COUNTER_VALUE() - context->before_sched);
        taskYIELD();
    }

    context->cycles_to_sched = accumulator / NUMBER_OF_ITERATIONS;
    vTaskDelete(context->t1_handle);
    xSemaphoreGive(context->end_sema);
    vTaskDelete(NULL);
}


void ScheduleTimeTest(void)
{
    test_context_t context;

    context.end_sema = xSemaphoreCreateBinary();
    TEST_ASSERT(context.end_sema != NULL);

    xTaskCreate(test_task_1, "test1" , 4096, &context, 1, &context.t1_handle);
    xTaskCreate(test_task_2, "test2" , 4096, &context, 1, NULL);

    BaseType_t result = xSemaphoreTake(context.end_sema, portMAX_DELAY);
    TEST_ASSERT_EQUAL_HEX32(pdTRUE, result);

    TEST_ASSERT(SCHEDULING_TIME > context.cycles_to_sched);

    printf("ScheduleTimeTest end\n");
    while (1) 
    {
        vTaskDelay(200);
    }

}
