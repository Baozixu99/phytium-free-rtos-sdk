#include <stdio.h>
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"
#include "finterrupt.h"
#include "fcpu_info.h"

#define TASK_STACK_SIZE            1024
#define INTEGER_GENERATOR_TASK_PRIORITY    3
#define STRING_PRINTER_TAKE_TASK_PRIORITY  4
#define QUEUE_INTR_RECV_TIMES      5
#define QUEUE_INTR_EXAMPLE_TIMEOUT pdMS_TO_TICKS(10000U)

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
    QUEUE_INTR_EXAMPLE_SUCCESS = 0,
    QUEUE_INTR_EXAMPLE_UNKNOWN_STATE,
    QUEUE_INTR_EXAMPLE_FAILURE,          
};

static QueueHandle_t xQueue = NULL;

/* Macro to force an interrupt. */
static void vTriggerInterrupt(void);

static u32 cpu_id = 0;

static xQueueHandle xIntegerQueue;
static xQueueHandle xStringQueue;

static void vIntegerGenerator(void *pvParameters)
{
    int task_res = QUEUE_INTR_EXAMPLE_UNKNOWN_STATE;
    portTickType xLastExecutionTime;
    unsigned long ulValueToSend = 0;

    xLastExecutionTime = xTaskGetTickCount();

    for (;;)
    {
        vTaskDelayUntil(&xLastExecutionTime, 500 / portTICK_RATE_MS);

        for (int i = 0; i < QUEUE_INTR_RECV_TIMES; ++i)
        {
            xQueueSendToBack(xIntegerQueue, &ulValueToSend, 0);
            ++ulValueToSend;
        }

        vPrintf("Queue Periodic task - About to generate an interrupt.\n");
        vTriggerInterrupt();
        vPrintf("Queue Periodic task - Interrupt generated.\n");

        if ( uxQueueMessagesWaiting(xIntegerQueue) == 0 &&  uxQueueMessagesWaiting(xStringQueue) == 0)
        {
            task_res = QUEUE_INTR_EXAMPLE_SUCCESS;
            xQueueSend(xQueue, &task_res, 0);
            break;
        }
    }

    vTaskDelete(NULL);
}

static void vInterruptHandler(s32 vector, void *param)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    static unsigned long ulReceivedNumber;

    static const char *pcStrings[] =
    {
        "String 0\n",
        "String 1\n",
        "String 2\n",
        "String 3\n"
    };

    while (xQueueReceiveFromISR(xIntegerQueue, &ulReceivedNumber,
                                &xHigherPriorityTaskWoken) != errQUEUE_EMPTY)
    {

        /* last 2 bits: values 0-3 */
        ulReceivedNumber &= 0x03;
        xQueueSendToBackFromISR(xStringQueue, &pcStrings[ulReceivedNumber],
                                &xHigherPriorityTaskWoken);
    }

    /* never call taskYIELD() form ISR! */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

static void vStringPrinter(void *pvParameters)
{
    char *pcString;
    int queue_intr_recv_times = 0;

    for (;;)
    {
        xQueueReceive(xStringQueue, &pcString, portMAX_DELAY);
        vPrintf("pcString = %s\n", pcString);
        queue_intr_recv_times++;

        if (queue_intr_recv_times == QUEUE_INTR_RECV_TIMES)
        {
            break;
        }
    }

    vTaskDelete(NULL);
}

static void prvSetupSoftwareInterrupt(void)
{
    GetCpuId(&cpu_id);

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

int CreateQueueTasks(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    int task_res = QUEUE_INTR_EXAMPLE_UNKNOWN_STATE;

    xQueue = xQueueCreate(1, sizeof(int)); /* 创建消息队列 */
    if (xQueue == NULL)
    {
        vPrintString("xQueue create failed.");
        goto exit;
    }

    xIntegerQueue = xQueueCreate(10, sizeof(unsigned long));
    xStringQueue = xQueueCreate(10, sizeof(char *));

    prvSetupSoftwareInterrupt();

    xTaskCreate(vIntegerGenerator, "QueueIntGen", TASK_STACK_SIZE, NULL, INTEGER_GENERATOR_TASK_PRIORITY, NULL);
    xTaskCreate(vStringPrinter,    "QueueString", TASK_STACK_SIZE, NULL, STRING_PRINTER_TAKE_TASK_PRIORITY, NULL);

    xReturn = xQueueReceive(xQueue, &task_res, QUEUE_INTR_EXAMPLE_TIMEOUT);
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

    if (task_res != QUEUE_INTR_EXAMPLE_SUCCESS)
    {
        vPrintString("Queue from interrupt feature example [failure]");
        return task_res;
    }
    else
    {
        vPrintString("Queue from interrupt feature example [success].");
        return task_res;
    }
}