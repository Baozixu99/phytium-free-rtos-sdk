/*
This example creates two sending tasks and one receiving task. The sending tasks send data
to the receiving task on two separate queues, one queue for each task. The two queues are
added to a queue set, and the receiving task reads from the queue set to determine which of
the two queues contain data.
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#define TASK_STACK_SIZE         2048
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))

static xTaskHandle xtask1_handle;
static xTaskHandle xtask2_handle;
static xTaskHandle xtask3_handle;

static EventGroupHandle_t xEventGroup;
#define FIRST_SENDER_TASK_BIT  ( 1UL << 0UL ) /* Event bit 0, which is set by a task. */
#define SECOND_SENDER_TASK_BIT ( 1UL << 1UL ) /* Event bit 1, which is set by a task. */

/* The three sender task. */
static void vSenderTask1(void *pvParameters);
static void vSenderTask2(void *pvParameters);

/* The receiver task.  The receiver blocks on the queue set to received data
from both sender task. */
static void vReceiverTask(void *pvParameters);

/*-----------------------------------------------------------*/

/* Declare two variables of type QueueHandle_t.  Both queues are added to the
same queue set. */
static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL;
static int xQueue1_recv_success_flag = 0;
static int xQueue2_recv_success_flag = 0;
enum 
{
    XQUEUE1_MESSAGE = 1,  
    XQUEUE2_MESSAGE = 2,                       
};

/* Declare a variable of type QueueSetHandle_t.  This is the queue set to which
the two queues are added. */
static QueueSetHandle_t xQueueSet = NULL;

/*-----------------------------------------------------------*/

static void vSenderTask1(void *pvParameters)
{
    const TickType_t xBlockTime = pdMS_TO_TICKS(500);
    const int pcMessage = XQUEUE1_MESSAGE;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* Block for 200ms. */
        vTaskDelay(xBlockTime);

        /* Send this task's string to xQueue1. It is not necessary to use a
        block time, even though the queue can only hold one item.  This is
        because the priority of the task that reads from the queue is higher
        than the priority of this task; as soon as this task writes to the queue
        it will be pre-empted by the task that reads from the queue, so the
        queue will already be empty again by the time the call to xQueueSend()
        returns.  The block time is set to 0. */
        xQueueSend(xQueue1, &pcMessage, 0);
    }
}
/*-----------------------------------------------------------*/

static void vSenderTask2(void *pvParameters)
{
    const TickType_t xBlockTime = pdMS_TO_TICKS(500);
    const int pcMessage = XQUEUE2_MESSAGE;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* Block for 200ms. */
        vTaskDelay(xBlockTime);

        /* Send this task's string to xQueue1. It is not necessary to use a
        block time, even though the queue can only hold one item.  This is
        because the priority of the task that reads from the queue is higher
        than the priority of this task; as soon as this task writes to the queue
        it will be pre-empted by the task that reads from the queue, so the
        queue will already be empty again by the time the call to xQueueSend()
        returns.  The block time is set to 0. */
        xQueueSend(xQueue2, &pcMessage, 0);
    }
}
/*-----------------------------------------------------------*/

static void vReceiverTask(void *pvParameters)
{
    QueueHandle_t xQueueThatContainsData;
    int pcReceivedMessage;

    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;)
    {
        /* Block on the queue set to wait for one of the queues in the set to
        contain data.  Cast the QueueSetMemberHandle_t values returned from
        xQueueSelectFromSet() to a QueueHandle_t as it is known that all the
        items in the set are queues (as opposed to semaphores, which can also be
        members of a queue set). */
        xQueueThatContainsData = (QueueHandle_t) xQueueSelectFromSet(xQueueSet, portMAX_DELAY);

        /* An indefinite block time was used when reading from the set so
        xQueueSelectFromSet() will not have returned unless one of the queues in
        the set contained data, and xQueueThatContansData must be valid.  Read
        from the queue.  It is not necessary to specify a block time because it
        is known that the queue contains data.  The block time is set to 0. */
        xQueueReceive(xQueueThatContainsData, &pcReceivedMessage, 0);

        if (pcReceivedMessage == XQUEUE1_MESSAGE)
        {
            vTaskDelete(xtask1_handle);
            vPrintStringAndNumber("Received From xQueue1 = ", pcReceivedMessage);
            xEventGroupSetBits(xEventGroup, FIRST_SENDER_TASK_BIT);
        }
        else if (pcReceivedMessage == XQUEUE2_MESSAGE)
        {
            vTaskDelete(xtask2_handle);
            vPrintStringAndNumber("Received From xQueue2 = ", pcReceivedMessage);
            xEventGroupSetBits(xEventGroup, SECOND_SENDER_TASK_BIT);
        }
    }
}

void DeleteQueueSetTasks(void)
{
    if (xtask3_handle)
    {
        vTaskDelete(xtask3_handle);
        vPrintString("QueueSet Receiver deletion.");
    }
}

BaseType_t CreateQueueSetTasks(void)
{
    BaseType_t ret;
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = (FIRST_SENDER_TASK_BIT | SECOND_SENDER_TASK_BIT);
    /* Create the two queues.  Each queue sends character pointers.  The
    priority of the receiving task is above the priority of the sending tasks so
    the queues will never have more than one item in them at any one time. */
    xQueue1 = xQueueCreate(1, sizeof(int *));
    xQueue2 = xQueueCreate(1, sizeof(int *));

    xEventGroup = xEventGroupCreate();
    /* Create the queue set.  There are two queues both of which can contain
    1 item, so the maximum number of queue handle the queue set will ever have
    to hold is 2 (1 item multiplied by 2 sets). */
    xQueueSet = xQueueCreateSet(1 * 2);

    /* Add the two queues to the set. */
    ret = xQueueAddToSet(xQueue1, xQueueSet);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("xQueue1 add xQueueSet failed", ret);
        goto exit;
    }

    ret = xQueueAddToSet(xQueue2, xQueueSet);
    if (ret != pdPASS)
    {
        vPrintStringAndNumber("xQueue2 add xQueueSet failed", ret);
        goto exit;
    }
    /* Create the tasks that send to the queues. */
    ret = xTaskCreate(vSenderTask1, "QueueSet Sender1", TASK_STACK_SIZE, NULL, 2, &xtask1_handle);
    if (ret != pdPASS)
    {
        xtask1_handle = NULL;
        vPrintStringAndNumber("Sender 1 creation failed: ", ret);
        goto exit;
    }

    ret = xTaskCreate(vSenderTask2, "QueueSet Sender2", TASK_STACK_SIZE, NULL, 2, &xtask2_handle);
    if (ret != pdPASS)
    {
        xtask2_handle = NULL;
        vPrintStringAndNumber("Sender 2 creation failed: ", ret);
        goto exit;
    }

    /* Create the receiver task. */
    ret = xTaskCreate(vReceiverTask, "QueueSet Receiver", TASK_STACK_SIZE, NULL, 3, &xtask3_handle);
    if (ret != pdPASS)
    {
        xtask3_handle = NULL;
        vPrintStringAndNumber("Receiver creation failed: ", ret);
        goto exit;
    }
    /* Block to wait for event bits to become set within the event group. */
    xEventGroupValue = xEventGroupWaitBits(xEventGroup, xBitsToWaitFor, pdTRUE, pdTRUE, TIMER_OUT);

exit:
    DeleteQueueSetTasks(); 

    if (xEventGroupValue != xBitsToWaitFor)
    {
        vPrintf("%s@%d: Queue set example [failure].\r\n", __func__, __LINE__);
        return pdFAIL;
    }
    else
    {
        vPrintf("%s@%d: Queue set example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }

}