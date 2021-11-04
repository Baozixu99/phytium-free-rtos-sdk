#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "unity.h"
#include "ft_types.h"

/*
 * Basic queue set tests. Multiple queues are added to a queue set then each
 * queue is filled in a sequential order. The members returned from the queue
 * set must adhered to the order in which the queues were filled.
 */
#define NO_OF_QUEUES            5
#define QUEUE_LEN               4
#define ITEM_SIZE               sizeof(uint32_t)

static QueueHandle_t handles[NO_OF_QUEUES];
static QueueSetHandle_t set_handle;

void TestQueueSetTest(void)
{
    /*Create queue set, queues, and add queues to queue set*/
    set_handle = xQueueCreateSet(NO_OF_QUEUES * QUEUE_LEN);
    for (int i = 0; i < NO_OF_QUEUES; i++) {
        handles[i] = xQueueCreate(QUEUE_LEN, ITEM_SIZE);
        TEST_ASSERT_MESSAGE(handles[i] != NULL, "Failed to create queue");
        TEST_ASSERT_MESSAGE(xQueueAddToSet(handles[i], set_handle) == pdPASS, "Failed to add to queue set");
    }

    /*Fill queue set via filling each queue*/
    for (int i = 0; i < NO_OF_QUEUES; i++) {
        for (int j = 0; j < QUEUE_LEN; j++) {
            uint32_t item_num = (i * QUEUE_LEN) + j;
            TEST_ASSERT_MESSAGE(xQueueSendToBack(handles[i], &item_num, portMAX_DELAY) == pdTRUE, "Failed to send to queue");
        }
    }

    /*Check queue set is notified in correct order*/
    for (int i = 0; i < NO_OF_QUEUES; i++) {
        for (int j = 0; j < QUEUE_LEN; j++) {
            QueueSetMemberHandle_t member = xQueueSelectFromSet(set_handle, portMAX_DELAY);
            TEST_ASSERT_EQUAL_MESSAGE(handles[i], member, "Incorrect queue set member returned");
            uint32_t item;
            xQueueReceive((QueueHandle_t)member, &item, 0);
            TEST_ASSERT_EQUAL_MESSAGE(((i * QUEUE_LEN) + j), item, "Incorrect item value");
        }
    }

    /*Remove queues from queue set and delete queues*/
    for (int i = 0; i < NO_OF_QUEUES; i++) {
        TEST_ASSERT_MESSAGE(xQueueRemoveFromSet(handles[i], set_handle), "Failed to remove from queue set");
        vQueueDelete(handles[i]);
    }
    vQueueDelete(set_handle);

    printf("TestQueueSetTest end\n");
    while(1){
        vTaskDelay(200);
    }
}

/*
 * Queue set thread safety test. Test the SMP thread safety by adding two queues
 * to a queue set and have a task on each core send to the queues simultaneously.
 * Check returned queue set members are valid.
 */

static volatile bool sync_flags;
static SemaphoreHandle_t sync_sem;

static void send_task(void *arg)
{
    TEST_ASSERT(arg != NULL);
    QueueHandle_t queue = (QueueHandle_t)arg;

    /*Wait until task on the other core starts running*/
    xSemaphoreTake(sync_sem, portMAX_DELAY);
    sync_flags = true;

    /*Fill queue*/
    for (int i = 0; i < QUEUE_LEN; i++) {
        u32 item = i;
        xQueueSendToBack(queue, &item, portMAX_DELAY);
    }

    xSemaphoreGive(sync_sem);
    vTaskDelete(NULL);
}

void QueueSetThreadSafetyTest(void)
{
    /*Create queue set, queues, and a send task*/
    sync_sem = xSemaphoreCreateCounting(1, 0);
    QueueHandle_t queue_handles;
    QueueSetHandle_t queueset_handle = xQueueCreateSet(QUEUE_LEN);
    
    sync_flags = false;
    queue_handles = xQueueCreate(QUEUE_LEN, ITEM_SIZE);
    TEST_ASSERT_MESSAGE(xQueueAddToSet(queue_handles, queueset_handle) == pdPASS, "Failed to add to queue set");
    xTaskCreate(send_task, "send", 1024, (void *)queue_handles, 1, NULL);

    vTaskDelay(2);
    xSemaphoreGive(sync_sem);
    vTaskDelay(2);

    /*Check returned queue set members are valid*/
    u32 expect_0 = 0;
    u32 expect_1 = 0;
    for (int i = 0; i < (QUEUE_LEN); i++) {
        QueueSetMemberHandle_t member = xQueueSelectFromSet(queueset_handle, portMAX_DELAY);
        u32 item;
        if (member == queue_handles) {
            xQueueReceive((QueueHandle_t)member, &item, 0);
            TEST_ASSERT_EQUAL_MESSAGE(expect_0, item, "Incorrect item value");
            expect_0++;
        } else {
            TEST_ASSERT_MESSAGE(0, "Incorrect queue set member returned");
        }
    }

    xSemaphoreTake(sync_sem, portMAX_DELAY);
    
    xQueueRemoveFromSet(queueset_handle, handles);
    vQueueDelete(queue_handles);
    vQueueDelete(queueset_handle);

    printf("QueueSetThreadSafetyTest end\n");
    while(1)
    {
        vTaskDelay(200);
    }
}

