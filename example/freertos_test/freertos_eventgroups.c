#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "timers.h"
#include "ft_debug.h"
#include "unity.h"
#include "interrupt.h"
#include "gicv3.h"
#include "parameters.h"
#include "fwdt.h"
#include "fwdt_hw.h"

static const int NUM_TASKS = 8;
static const int COUNT = 1000;
static EventGroupHandle_t eg;
static SemaphoreHandle_t done_sem;

#define BIT_CALL (1 << 0)
#define BIT_RESPONSE(TASK) (1 << (TASK+1))
#define ALL_RESPONSE_BITS (((1 << NUM_TASKS) - 1) << 1)

static void TaskEventGroupResponse(void *param)
{
    TEST_ASSERT(param != NULL);
    int task_num = *(intptr*)param;
    int i = 0;

    for (i = 0; i < COUNT; i++) 
    {
        /* Wait until the common "call" bit is set, starts off all tasks
           (clear on return) */
        TEST_ASSERT( xEventGroupWaitBits(eg, BIT_CALL, TRUE, FALSE, portMAX_DELAY) );
        /* Set our individual "response" bit */
        xEventGroupSetBits(eg, BIT_RESPONSE(task_num));
        vTaskDelay(10);
    }
    xSemaphoreGive(done_sem);
    vTaskDelete(NULL);
}

void EventGroupsTest(void)
{
    int i = 0;
    eg = xEventGroupCreate();
    done_sem = xSemaphoreCreateCounting(NUM_TASKS, 0);

    /* Note: task_event_group_call_response all have higher priority than this task, so on this core
       they will always preempt this task.
       This is important because we need to know all tasks have blocked on BIT_CALL each time we signal it,
       or they get out of sync.
     */
    for (i = 0; i < NUM_TASKS; i++) 
    {
        xTaskCreate(TaskEventGroupResponse, "tsk_call_resp", 4096, (void *)&i, UNITY_FREERTOS_PRIORITY - 1, NULL);
    }

    /* Tasks all start instantly, but this task will resume running at the same time as the higher priority tasks on the
       other processor may still be setting up, so allow time for them to also block on BIT_CALL... */
    vTaskDelay(10);

    for (i = 0; i < COUNT; i++) 
    {
        /* signal all tasks with "CALL" bit... */
        xEventGroupSetBits(eg, BIT_CALL);

        /* Only wait for 1 tick, the wakeup should be immediate... */
        TEST_ASSERT_EQUAL_HEX16(ALL_RESPONSE_BITS, xEventGroupWaitBits(eg, ALL_RESPONSE_BITS, TRUE, TRUE, 1));
        vTaskDelay(10);
    }

    /* Ensure all tasks cleaned up correctly */
    for (i = 0; i < NUM_TASKS; i++) 
    {
        TEST_ASSERT( xSemaphoreTake(done_sem, 100/portTICK_PERIOD_MS) );
    }

    vSemaphoreDelete(done_sem);
    vEventGroupDelete(eg);
    printf("--------------------------------------------------------\n");
    while (1) 
    {
        vTaskDelay(200);
    }
}


/*-----------------Test case for event group trace facilities-----------------*/
/*
 * Test event group Trace Facility functions such as
 * xEventGroupClearBitsFromISR(), xEventGroupSetBitsFromISR()
 */

//Use a wdt to trigger an ISr
#define BITS            0xAA

static bool test_set_bits;
static bool test_clear_bits;
static FWdtCtrl wdt_ctrl;
/* wdt num: 0 or 1 */
static u8 wdt_id = 0;

/**
 * @name: WdtInterrupt
 * @msg:  This function handle wdt timeout interrupt, use it to refresh wdt.
 * @return {void} 
 * @param  {s32} vector, the interrupt number
 * @param  {void} *param, pointer to a WdtCtrl structure that contains
  *                the configuration information for the specified wdt module.
 */
static void FWdtInterrupt(s32 vector, void *param)
{
    FWdtRefresh((FWdtCtrl *)param);
    printf("FWdtInterrupt\n");

    portBASE_TYPE task_woken = pdFALSE;

    if(test_set_bits)
    {
        xEventGroupSetBitsFromISR(eg, BITS, &task_woken);
        test_set_bits = FALSE;
    } 
    else if (test_clear_bits)
    {
        xEventGroupClearBitsFromISR(eg, BITS);
        xSemaphoreGiveFromISR(done_sem, &task_woken);
        test_clear_bits = FALSE;
    }

    //Switch context if necessary
    if(task_woken ==  pdTRUE)
    {
        portYIELD_FROM_ISR(task_woken);
    }
}

/**
 * @name: WdtRefreshTest
 * @msg:  Set wdt interrupt to refresh wdt, set timeout value, start wdt.
 * @return {void} 
 * @param {WdtCtrl} *pctrl, pointer to a WdtCtrl structure that contains
  *                the configuration information for the specified wdt module.
 */

static void FWdtRefreshTest(FWdtCtrl *pctrl)
{
    FWdtConfig *pconfig = &pctrl->config;
    /* interrupt init */
    InterruptSetPriority(pconfig->irq_num, /*pconfig->irq_prority*/((0x8 << 4) + 4 * 16));
    InterruptInstall(pconfig->irq_num, FWdtInterrupt, (void*)pctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_num);
    FWdtSetTimeout(pctrl, 3);
    FWdtStart(pctrl);
}

/**
 * @name: WdtNoRefreshTest
 * @msg:  Set wdt timeout value, start wdt, no refresh.
 * @return {void} 
 * @param {WdtCtrl} *pctrl,  pointer to a WdtCtrl structure that contains
  *                the configuration information for the specified wdt module.
 */
static void WdtTaskCreate(void)
{
    FWdtCtrl *pctrl = &wdt_ctrl;
    pctrl->config = *FWdtLookupConfig(wdt_id);
    FWdtRefreshTest(pctrl);
}

void EventGroupIsrTest(void)
{ 
    done_sem = xSemaphoreCreateBinary();
    eg = xEventGroupCreate();
    test_set_bits = FALSE;
    test_clear_bits = FALSE;

    WdtTaskCreate();
    //Test set bits
    test_set_bits = TRUE;

    TEST_ASSERT(BITS==xEventGroupWaitBits(eg, BITS, pdFALSE, pdTRUE, portMAX_DELAY));     //Let ISR set event group bits

    //Test clear bits
    xEventGroupSetBits(eg, BITS);                   //Set bits to be cleared
    test_clear_bits = TRUE;

    xSemaphoreTake(done_sem, portMAX_DELAY);        //Wait for ISR to clear bits
    vTaskDelay(10);                                 //Event group clear bits runs via daemon task, delay so daemon can run
    TEST_ASSERT(0==xEventGroupGetBits(eg));   //Check bits are cleared

    //Clean up
    vEventGroupDelete(eg);
    vSemaphoreDelete(done_sem);
    vTaskDelay(10);     //Give time for idle task to clear up deleted tasks

    printf("EventGroupIsrTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
}
