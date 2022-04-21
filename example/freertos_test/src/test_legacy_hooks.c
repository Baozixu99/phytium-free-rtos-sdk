/*
 Test of FreeRTOS "legacy" hook functions, and delete hook

 Only compiled in if the relevant options are enabled.
*/
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"

static volatile unsigned idle_count;

void IdleHookTest(void)
{
    idle_count = 0;
    vTaskDelay(10);
    /* The legacy idle hook should be called at least once*/
    TEST_ASSERT_NOT_EQUAL(0, idle_count); 
    
    printf("IdleHookTest end\n");
    while(1){
        vTaskDelay(200);
    }
}

static volatile unsigned tick_count;

void TickHookTest(void)
{
    unsigned before = xTaskGetTickCount();
    const unsigned SLEEP_FOR = 20;
    tick_count = before;
    vTaskDelay(SLEEP_FOR);
    printf("TickHookTest end\n");
    while(1){
        vTaskDelay(200);
    }
}

static volatile void *deleted_tcb;

static void taskDeletesItself(void *ignored)
{
    vTaskDelete(NULL);
}

void vPortCleanUpTCB(void *pxTCB)
{
    deleted_tcb = pxTCB;
}

void CleanUpHookTest(void)
{

    TaskHandle_t new_task = NULL;
    deleted_tcb = NULL;
    xTaskCreate(taskDeletesItself, "delete", 2048, NULL, 1, &new_task);

    /* xTaskCreate(vPortCleanUpTCB, "vPortCleanUpTCB", 2048, &new_task, 1, NULL);*/
    vPortCleanUpTCB((void *)new_task);
    vTaskDelay(5);

    /* TCB & TaskHandle are the same in FreeRTOS*/
    TEST_ASSERT_EQUAL_PTR(deleted_tcb, new_task); 

    printf("CleanUpHookTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
}
