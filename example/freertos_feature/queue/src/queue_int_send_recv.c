/*
This example demonstrates a queue being created, data being sent to the queue from multiple
tasks, and data being received from the queue. The queue is created to hold data items of
type int. The tasks that send to the queue do not specify a block time, whereas the task
that receives from the queue does.
*/
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#define TASK_STACK_SIZE         2048
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

/* The tasks to be created.  Two instances are created of the sender task while
only a single instance is created of the receiver task. */
static void vSenderTask(void *pvParameters);
static void vReceiverTask(void *pvParameters);

/*-----------------------------------------------------------*/

/* Declare a variable of type QueueHandle_t.  This is used to store the queue
that is accessed by all three tasks. */
static QueueHandle_t xQueue;

static EventGroupHandle_t xEventGroup;
#define FIRST_SENDER_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by a task. */
#define SECOND_SENDER_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by a task. */

/*-----------------------------------------------------------*/

static void vSenderTask(void *pvParameters)
{
    int lValueToSend;
    BaseType_t xStatus;

    /* Two instances are created of this task so the value that is sent to the
    queue is passed in via the task parameter rather than be hard coded.  This way
    each instance can use a different value.  Cast the parameter to the required
    type. */
    lValueToSend = (int)(uintptr)pvParameters;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* The first parameter is the queue to which data is being sent.  The
        queue was created before the scheduler was started, so before this task
        started to execute.

        The second parameter is the address of the data to be sent.

        The third parameter is the Block time – the time the task should be kept
        in the Blocked state to wait for space to become available on the queue
        should the queue already be full.  In this case we don’t specify a block
        time because there should always be space in the queue. */
        xStatus = xQueueSendToBack(xQueue, &lValueToSend, 0);

        if (xStatus != pdPASS)
        {
            /* We could not write to the queue because it was full – this must
            be an error as the queue should never contain more than one item! */
            vPrintString("Could not send to the queue.\r\n");
        }

        vTaskDelay(500);
    }
}
/*-----------------------------------------------------------*/

static void vReceiverTask(void *pvParameters)
{
    /* Declare the variable that will hold the values received from the queue. */
    int32_t lReceivedValue;
    BaseType_t xStatus;

    /* This task is also defined within an infinite loop. */
    for (;;)
    {
        /* As this task unblocks immediately that data is written to the queue this
        call should always find the queue empty. */
        if (uxQueueMessagesWaiting(xQueue) != 0)
        {
            vPrintString("Queue should have been empty!\r\n");
        }

        /* The first parameter is the queue from which data is to be received.  The
        queue is created before the scheduler is started, and therefore before this
        task runs for the first time.

        The second parameter is the buffer into which the received data will be
        placed.  In this case the buffer is simply the address of a variable that
        has the required size to hold the received data.

        the last parameter is the block time – the maximum amount of time that the
        task should remain in the Blocked state to wait for data to be available should
        the queue already be empty. */
        xStatus = xQueueReceive(xQueue, &lReceivedValue, portMAX_DELAY);

        if (xStatus == pdPASS)
        {
            if (lReceivedValue == 100)
            {
                vTaskDelete(xtask1_handle);
                vPrintStringAndNumber("Received From Sender 1 = ", lReceivedValue);
                xEventGroupSetBits(xEventGroup, FIRST_SENDER_TASK_BIT);
            }
            else if (lReceivedValue == 200)
            {
                vTaskDelete(xtask2_handle);
                vPrintStringAndNumber("Received From Sender 2 = ", lReceivedValue);
                xEventGroupSetBits(xEventGroup, SECOND_SENDER_TASK_BIT);
            }
        }
    }
}

void DeleteIntTasks(void)
{
    if (xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("Int Receiver deletion.");
    }
}

BaseType_t CreateIntTasks(void)
{
    BaseType_t ret;
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = (FIRST_SENDER_TASK_BIT | SECOND_SENDER_TASK_BIT);

    xEventGroup = xEventGroupCreate();
    /* The queue is created to hold a maximum of 5 int values. */
    xQueue = xQueueCreate(5, sizeof(int));

    if (xQueue != NULL)
    {
        /* Create two instances of the task that will write to the queue.  The
        parameter is used to pass the value that the task should write to the queue,
        so one task will continuously write 100 to the queue while the other task
        will continuously write 200 to the queue.  Both tasks are created at
        priority 1. */
        ret = xTaskCreate(vSenderTask, "Queue Sender1", TASK_STACK_SIZE, (void *) 100, 2, &xtask1_handle);
        if (ret != pdPASS)
        {
            xtask1_handle = NULL;
            vPrintStringAndNumber("Sender 1 creation failed: ", ret);
            goto exit;
        }
        ret = xTaskCreate(vSenderTask, "Queue Sender2", TASK_STACK_SIZE, (void *) 200, 2, &xtask2_handle);
        if (ret != pdPASS)
        {
            xtask2_handle = NULL;
            vPrintStringAndNumber("Sender 2 creation failed: ", ret);
            goto exit;
        }
        /* Create the task that will read from the queue.  The task is created with
        priority 2, so above the priority of the sender tasks. */
        ret = xTaskCreate(vReceiverTask, "Queue Receiver", TASK_STACK_SIZE, NULL, 3, &xtask3_handle);
        if (ret != pdPASS)
        {
            xtask3_handle = NULL;
            vPrintStringAndNumber("Receiver creation failed: ", ret);
            goto exit;
        }

    }
    /* Block to wait for event bits to become set within the event group. */
    xEventGroupValue = xEventGroupWaitBits(xEventGroup, xBitsToWaitFor, pdTRUE, pdTRUE, TIMER_OUT);

exit:
    DeleteIntTasks();
    if (xEventGroupValue != xBitsToWaitFor)
    {
        vPrintf("%s@%d: Queue int send then recv example [failure].\r\n", __func__, __LINE__);
        return pdFAIL;
    }
    else
    {
        vPrintf("%s@%d: Queue int send then recv example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}