#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "croutine.h"
#include "finterrupt.h"
#include "fcpu_info.h"

#define TASK_STACK_SIZE         1024
#define PERIODIC_TASK_PRIORITY  3
#define SEM_TAKE_TASK_PRIORITY  4
#define BIN_SEM_EXAMPLE_TIMEOUT pdMS_TO_TICKS(10000U)

/* The interrupt number to use for the software interrupt generation.  This
could be any unused number.  In this case the first chip level (non system)
interrupt is used */
#define INTERRUPT_ID        0

/* The priority of the software interrupt.  The interrupt service routine uses
an (interrupt safe) FreeRTOS API function, so the priority of the interrupt must
be equal to or lower than the priority set by
configMAX_SYSCALL_INTERRUPT_PRIORITY - remembering that on the Cortex M3 high
numeric values represent low priority values, which can be confusing as it is
counter intuitive. */
#define INTERRUPT_PRIORITY  IRQ_PRIORITY_VALUE_12

enum
{
    BIN_SEM_EXAMPLE_SUCCESS = 0,
    BIN_SEM_EXAMPLE_UNKNOWN_STATE,
    BIN_SEM_EXAMPLE_FAILURE,          
};

static QueueHandle_t xQueue = NULL;

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void);

static u32 cpu_id = 0;

xSemaphoreHandle xBinarySemaphore;

void vPeriodicTask(void *pvParameters)
{
    int task_res = BIN_SEM_EXAMPLE_UNKNOWN_STATE;

    vTaskDelay(500 / portTICK_RATE_MS);
    printf("Bin Periodic task - Generate an interrupt.\n");
    vTriggerInterrupt();
    printf("Bin Periodic task - Interrupt generated.\n");
    
    if (uxSemaphoreGetCount(xBinarySemaphore) == 0)
    {
        task_res = BIN_SEM_EXAMPLE_SUCCESS;
        xQueueSend(xQueue, &task_res, 0);
    }

    vTaskDelete(NULL);
}

void vSemTakeTask(void *pvParameters)
{
    xSemaphoreTake(xBinarySemaphore, 0);
    for (;;)
    {
        xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);
        printf("Bin Handler task - Processing event.\n");

        if (uxSemaphoreGetCount(xBinarySemaphore) == 0)
        {
            break;
        }
    }

    vTaskDelete(NULL);
}

static void vInterruptHandler(s32 vector, void *param)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);

    /* never call taskYIELD() form ISR! */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void prvSetupSoftwareInterrupt()
{
    GetCpuId(&cpu_id);

    /* The interrupt service routine uses an (interrupt safe) FreeRTOS API
    function so the interrupt priority must be at or below the priority defined
    by configSYSCALL_INTERRUPT_PRIORITY. */
    InterruptSetPriority(INTERRUPT_ID, INTERRUPT_PRIORITY);

    InterruptInstall(INTERRUPT_ID, vInterruptHandler, NULL, NULL);

    /* Enable the interrupt. */
    InterruptUmask(INTERRUPT_ID);
}

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void)
{
    InterruptCoreInterSend(INTERRUPT_ID, (1 << cpu_id));
}


int CreateBinarySemTasks(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = BIN_SEM_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        vPrintString("xQueue create failed.");
        goto exit;
    }

    vSemaphoreCreateBinary(xBinarySemaphore);

    if (xBinarySemaphore != NULL)
    {
        prvSetupSoftwareInterrupt();
        xTaskCreate(vSemTakeTask,  "BinHandler",  TASK_STACK_SIZE, NULL, SEM_TAKE_TASK_PRIORITY, NULL);
        xTaskCreate(vPeriodicTask, "BinPeriodic", TASK_STACK_SIZE, NULL, PERIODIC_TASK_PRIORITY, NULL);
    }

    xReturn = xQueueReceive(xQueue, &task_res, BIN_SEM_EXAMPLE_TIMEOUT);
    if (xReturn == pdFAIL)
    {
        vPrintString("xQueue receive timeout.");
        goto exit;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_res != BIN_SEM_EXAMPLE_SUCCESS)
    {
        vPrintString("Binary semaphore feature example [failure]");
        return task_res;
    }
    else
    {
        vPrintString("Binary semaphore feature example [success].");
        return task_res;
    }
}