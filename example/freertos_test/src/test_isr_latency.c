#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <fsleep.h>
#include <generic_timer.h>
#include "interrupt.h"
#include "parameters.h"
#include "fwdt.h"
#include "fwdt_hw.h"
#include "ft_types.h"
#include "math.h"
#include "unity.h"

#define SW_ISR_LEVEL_1  7
#define ISR_ENTER_CYCLES 10 
#define ISR_EXIT_CYCLES 10

static SemaphoreHandle_t sync;
static SemaphoreHandle_t end_sema;
static u32 cycle_before_trigger;
static u32 cycle_before_exit;
static u32 delta_enter_cycles = 0;
static u32 delta_exit_cycles = 0;

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
    BaseType_t yield;
    delta_enter_cycles += portGET_RUN_TIME_COUNTER_VALUE() - cycle_before_trigger;
    printf("test_task: delta_enter_cycles=%d\n", delta_enter_cycles);

    xSemaphoreGiveFromISR(sync, &yield);
    portYIELD_FROM_ISR(yield);

    cycle_before_exit = portGET_RUN_TIME_COUNTER_VALUE();

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
    InterruptSetPriority(pconfig->irq_num, IRQ_PRIORITY_VALUE_12);
    InterruptInstall(pconfig->irq_num, FWdtInterrupt, (void*)pctrl, pconfig->instance_name);
    InterruptUmask(pconfig->irq_num);
    FWdtSetTimeout(pctrl, 1);
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
    GenericTimerStart();

    memset(&wdt_ctrl, 0, sizeof(wdt_ctrl));

    FWdtConfig pconfig = *FWdtLookupConfig(wdt_id);
   
    /* wdt init, include reset and read */
    FWdtCfgInitialize(&wdt_ctrl, &pconfig);

    FWdtRefreshTest(&wdt_ctrl);

}

static void TestTask(void *arg) {

    for(int i = 0; i < 20; i++) 
    {
        cycle_before_trigger = portGET_RUN_TIME_COUNTER_VALUE();
        xSemaphoreTake(sync, portMAX_DELAY);
        delta_exit_cycles += portGET_RUN_TIME_COUNTER_VALUE() - cycle_before_exit;
        
    }
    delta_enter_cycles /= 20;
    delta_exit_cycles /= 20;

    xSemaphoreGive(end_sema);
    vTaskDelete(NULL);
}


void IsrLatencyTest(void)
{
    WdtTaskCreate();
    sync = xSemaphoreCreateBinary();
    TEST_ASSERT(sync != NULL);
    end_sema = xSemaphoreCreateBinary();
    TEST_ASSERT(end_sema != NULL);
    xTaskCreate(TestTask, "tst" , 4096, NULL,  1, NULL);
    vTaskDelay(100);
    BaseType_t result = xSemaphoreTake(end_sema, portMAX_DELAY);
    TEST_ASSERT_EQUAL_HEX32(pdTRUE, result);
    printf("delta_enter_cycles=%d\n", delta_enter_cycles);
    printf("delta_exit_cycles=%d\n", delta_exit_cycles);
    printf("IsrLatencyTest end\n");

    while(1)
    {
        vTaskDelay(200);
    }
} 
