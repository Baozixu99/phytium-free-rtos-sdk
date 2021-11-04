/*
 * Test FreeRTOS debug functions and utilities.
 * - Queue registry functions vQueueAddToRegistry(), vQueueUnregisterQueue(),
 *   and pcQueueGetName(backported)
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "unity.h"

#define NO_OF_QUEUES_ON_CORE  2     /*Save space for some preallocated tasks*/
#define NO_OF_QUEUES_TOTAL    NO_OF_QUEUES_ON_CORE
#define QUEUE_NAME_MAX_LENGTH    30

static SemaphoreHandle_t start_sem;
static SemaphoreHandle_t done_sem = NULL;
static char *names[NO_OF_QUEUES_TOTAL];
static QueueHandle_t handles[NO_OF_QUEUES_TOTAL];

void TestQueueRegistryTask(void *arg)
{
    int i = 0;
    /*Create queues and accompanying queue names*/
    for(i = 0; i < NO_OF_QUEUES_ON_CORE; i++)
    {
        handles[i] = xQueueCreate(1,1);   /*Create queues*/
        names[i] = calloc(QUEUE_NAME_MAX_LENGTH, sizeof(char));
        sprintf(names[i], "Queue%d", i);
    }

    xSemaphoreTake(start_sem, portMAX_DELAY);   /*Wait for start vQueueAddToRegistry()*/
    for(i = 0; i < NO_OF_QUEUES_ON_CORE; i++)
    {
        vQueueAddToRegistry(handles[i] , names[i]);   /*Register queues to queue registry*/
    }
    xSemaphoreGive(done_sem);   /*Signal that vQueueAddToRegistry() has completed*/

    vTaskDelay(1);

    xSemaphoreTake(start_sem, portMAX_DELAY);   /*Wait to start vQueueUnregisterQueue()*/
    for(i = 0; i < NO_OF_QUEUES_ON_CORE; i++)
    {
        vQueueDelete(handles[i]);  /*Internally calls vQueueUnregisterQueue*/
    }
    xSemaphoreGive(done_sem);   /*Signal done*/

    vTaskDelete(NULL);  /*Delete self*/
}

void QueueRegistryDebugTest(void)
{
    int i = 0;
    /*Create synchronization semaphores and tasks to test queue registry*/
    done_sem = xSemaphoreCreateCounting(1, 0);
    if(done_sem == NULL)
    {
        printf("xSemaphoreCreateCounting failed\n");
        return;
    }

    start_sem = xSemaphoreCreateBinary();
    if(start_sem == NULL)
    {
        printf("xSemaphoreCreateBinary failed\n");
        return;
    }
    
    xTaskCreate(TestQueueRegistryTask, "testing task", 4096, NULL, 1, NULL);

    portDISABLE_INTERRUPTS();
    xSemaphoreGive(start_sem);  /*Trigger start*/
    
    portENABLE_INTERRUPTS();
    xSemaphoreTake(done_sem, portMAX_DELAY);    /*Wait for tasks to complete vQueueAddToRegistry*/
    
    for(i = 0; i < NO_OF_QUEUES_TOTAL; i++)
    {
        const char *addr = pcQueueGetName(handles[i]);

        TEST_ASSERT(addr == names[i]);   /*Check vQueueAddToRegistry was successful*/
    }

    portDISABLE_INTERRUPTS();
    xSemaphoreGive(start_sem);  /*Trigger start*/
    
    portENABLE_INTERRUPTS();
    xSemaphoreTake(done_sem, portMAX_DELAY);    /*Wait for tasks to complete vQueueUnregisterQueue*/
    
    for(i = 0; i < NO_OF_QUEUES_TOTAL; i++)
    {
        const char *addr = pcQueueGetName(handles[i]);
        TEST_ASSERT(addr == NULL);  /*Check vQueueUnregisterQueue was successful*/
        handles[i] = NULL;
    }

    /*Cleanup*/
    for(i = 0; i < NO_OF_QUEUES_TOTAL; i++)
    {
        free(names[i]);
        names[i] = NULL;
    }

    vSemaphoreDelete(start_sem);
    start_sem = NULL;
    
    vSemaphoreDelete(done_sem);
    done_sem = NULL;

    while(1)
    {
        vTaskDelay(200);
    }

}
