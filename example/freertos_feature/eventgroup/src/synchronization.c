/* This example demonstrates how to:
   uses xEventGroupSync() to synchronize three instances of a single task implementation.
 * The task parameter is used to pass into each instance the event bit the task will set when it calls xEventGroupSync().
 */
#include <time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#define TASK_STACK_SIZE           1024
#define TASK_PRIORITY             3
#define SYNCHRONIZATION_TASK_NUM  3
#define WAIT_BITS_TIMEOUT         pdMS_TO_TICKS(10000U)
#define EXAMPLE_TIMEOUT           pdMS_TO_TICKS(30000U)


/* Definitions for the event bits in the event group. */
#define FIRST_TASK_BIT            (1UL << 0UL) /* Event bit 0, which is set by the first task. */
#define SECOND_TASK_BIT           (1UL << 1UL) /* Event bit 1, which is set by the second task. */
#define THIRD_TASK_BIT            (1UL << 2UL) /* Event bit 2, which is set by the third task. */
#define TASK_ERROR_BIT            (1UL << 3UL) /* Bit 3, which means sync task failed. */
#define ALL_SYNC_BITS             (FIRST_TASK_BIT | SECOND_TASK_BIT | THIRD_TASK_BIT)

enum
{
    EVENTGROUP_SYNCHRONIZATION_SUCCESS = 0,
    EVENTGROUP_SYNCHRONIZATION_UNKNOWN_STATE,
};

static QueueHandle_t xQueue = NULL;

/* Use by the pseudo random number generator. */
static uint32_t ulNextRand;

/* Declare the event group used to synchronize the three tasks. */
static EventGroupHandle_t xEventGroup;

/*-----------------------------------------------------------*/
/* Pseudo random number generation functions - implemented in this file as the MSVC rand() function has unexpected consequences. */
static uint32_t prvRand(void)
{
    const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;
    uint32_t ulReturn;

    /* Utility function to generate a pseudo random number as the MSVC rand() function has unexpected consequences. */
    taskENTER_CRITICAL();
    ulNextRand = (ulMultiplier * ulNextRand) + ulIncrement;
    ulReturn = (ulNextRand >> 16UL) & 0x7fffUL;
    taskEXIT_CRITICAL();
    return ulReturn;
}

static void prvSRand(uint32_t ulSeed)
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}

/*-----------------------------------------------------------*/
/* Three instances of this task are created. */
static void vSyncingTask(void *pvParameters)
{
    const EventBits_t uxAllSyncBits = ALL_SYNC_BITS;
    const TickType_t xMaxDelay = pdMS_TO_TICKS(500UL);
    const TickType_t xMinDelay = pdMS_TO_TICKS(4000UL);
    TickType_t xDelayTime;
    EventBits_t uxThisTasksSyncBit;
    EventBits_t uxReturn;
    u32 task_res = 0;

    /* Three instances of this task are created - each task uses a different event bit in the synchronization.
     * The event bit to use by this task instance is passed into the task using the task's parameter.
     * Store it in the uxThisTasksSyncBit variable.
     */
    uxThisTasksSyncBit = (EventBits_t) pvParameters;

    /* Simulate this task taking some time to perform an action by delaying for a pseudo random time.
     * This prevents all three instances of this task from reaching the synchronization point at the same time, 
       and allows the example's behavior to be observed more easily.
     */
    xDelayTime = (prvRand() % xMaxDelay) + xMinDelay;
    vTaskDelay(xDelayTime);

    /* Print out a message to show this task has reached its synchronization point.
     * pcTaskGetTaskName() is an API function that returns the name assigned to the task when the task was created.
     */
    vPrintString(pcTaskGetTaskName(NULL));
    vPrintString(" reached sync point");

    /* Wait for all the tasks to have reached their respective synchronization points. */
    uxReturn = xEventGroupSync(xEventGroup,        /* The event group used to synchronize. */
                               uxThisTasksSyncBit, /* The bit set by this task to indicate it has reached the synchronization point. */
                               uxAllSyncBits,      /* The bits to wait for, one bit for each task taking part in the synchronization. */
                               WAIT_BITS_TIMEOUT); /* Wait indefinitely for all three tasks to reach the synchronization point. */

    /* Print out a message to show this task has passed its synchronization point.
     * As an indefinite delay was used the following line will only be reached after all the tasks reached their respective synchronization points.
     */
    if((uxReturn & uxAllSyncBits) == uxAllSyncBits) /* All three tasks reached the synchronisation point before the call to xEventGroupSync() timed out. */
    {
        vPrintString(pcTaskGetTaskName(NULL));
        vPrintString(" exited sync point");

        vPrintString(pcTaskGetTaskName(NULL));
        vPrintString(" deletion");

        task_res = uxThisTasksSyncBit;
        xQueueSend(xQueue, &task_res, 0);
    }
    else
    {
        task_res = TASK_ERROR_BIT;
        xQueueSend(xQueue, &task_res, 0);
    }

    vTaskDelete(NULL);
}

/*-----------------------------------------------------------*/

int CreateSyncTasks(void)
{
    BaseType_t xReturn = pdPASS; /* Define a return value with a default of pdPASS */
    u32 task_res = 0;
    u32 task_flag = 0; /* Flag for whether receive enough task_res */

    xQueue = xQueueCreate(3, sizeof(u32)); /* Creat Message Queue */
    if (xQueue == NULL)
    {
        vPrintString("xQueue create failed.");
        goto exit;
    }

    /* The tasks created in this example block for a random time. 
     * The block time is generated using rand() - seed the random number generator.
     */
    prvSRand((uint32_t) time(NULL));

    /* Before an event group can be used it must first be created. */
    xEventGroup = xEventGroupCreate();

    /* Create three instances of the task.  Each task is given a different name,
       which is later printed out to give a visual indication of which task is executing.
     * The event bit to use when the task reaches its synchronization point is passed into the task using the task parameter.
     */
    xTaskCreate(vSyncingTask, "Sync Task 1", TASK_STACK_SIZE, (void *)FIRST_TASK_BIT, TASK_PRIORITY, NULL);
    xTaskCreate(vSyncingTask, "Sync Task 2", TASK_STACK_SIZE, (void *)SECOND_TASK_BIT, TASK_PRIORITY, NULL);
    xTaskCreate(vSyncingTask, "Sync Task 3", TASK_STACK_SIZE, (void *)THIRD_TASK_BIT, TASK_PRIORITY, NULL);

    for (int loop = 0; loop < SYNCHRONIZATION_TASK_NUM; loop++)
    {   
        xReturn = xQueueReceive(xQueue, &task_res, EXAMPLE_TIMEOUT);
        if (xReturn == pdFAIL || task_res == TASK_ERROR_BIT)
        {
            vPrintString("xQueue receive timeout or task err.");
            goto exit;
        }
        task_flag |= task_res;
    }

exit:
    if (xQueue != NULL)
    {
        vQueueDelete(xQueue);
    }

    if (task_flag != ALL_SYNC_BITS)
    {
        vPrintf("Event group feature management example [failure], task_flag = 0x%x \r\n", task_flag);
        return -1;
    }
    else
    {
        vPrintString("Event group feature management example [success].");
        return 0;
    }
}