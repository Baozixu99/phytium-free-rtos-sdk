/*
 Test of FreeRTOS task notifications. This test creates a sender and receiver
 task under different core permutations. For each permutation, the sender task
 will test the xTaskNotify(), xTaskNotifyGive(), xTaskNotifyFromISR(), and
 vTaskNotifyGiveFromISR(), whereas the receiver task will test
 xTaskNotifyWait() and ulTaskNotifyTake().
*/
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"
#include "parameters.h"
#include "ft_types.h"

#define NO_OF_NOTIFS    4
#define NO_OF_TASKS     2       
#define TIMER_DIVIDER 10000
#define TIMER_COUNT 100
#define MESSAGE 0xFF

static uint32_t send_core_message = 0;
static TaskHandle_t rec_task_handle;
static u8 isr_give = FALSE;

static SemaphoreHandle_t trigger_send_semphr;
static SemaphoreHandle_t task_delete_semphr;

/*Test tracking vars*/
static volatile uint32_t notifs_sent = 0;
static volatile uint32_t notifs_rec = 0;
static u8 wrong_core = FALSE;

static void SenderTask (void* arg){

    /*Test xTaskNotify*/
    xSemaphoreTake(trigger_send_semphr, portMAX_DELAY);
    notifs_sent++;

    xTaskNotify(rec_task_handle, (MESSAGE), eSetValueWithOverwrite);

    /*Test xTaskNotifyGive*/
    xSemaphoreTake(trigger_send_semphr, portMAX_DELAY);

    notifs_sent++;
    xTaskNotifyGive(rec_task_handle);

    /*Test xTaskNotifyFromISR*/
    xSemaphoreTake(trigger_send_semphr, portMAX_DELAY);
    isr_give = FALSE;
    vTaskDelay(20);

    /*Test vTaskNotifyGiveFromISR*/
    xSemaphoreTake(trigger_send_semphr, portMAX_DELAY);
    isr_give = TRUE;
    vTaskDelay(20);

    /*Delete Task and Semaphores*/
    xSemaphoreGive(task_delete_semphr);
    vTaskDelete(NULL);
}

static void ReceiverTask (void* arg){
    uint32_t notify_value;

    /*Test xTaskNotifyWait from task*/
    xTaskNotifyWait(0, 0xFFFFFFFF, &notify_value, portMAX_DELAY);
    if(notify_value != send_core_message)
    {
        wrong_core = TRUE;
    }
    notifs_rec++;

    /*Test ulTaskNotifyTake from task*/
    xSemaphoreGive(trigger_send_semphr);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    notifs_rec++;

    /*Test xTaskNotifyWait from ISR*/
    xSemaphoreGive(trigger_send_semphr);

    xTaskNotifyWait(0, 0xFFFFFFFF, &notify_value, portMAX_DELAY);
    if(notify_value != send_core_message)
    {
        wrong_core = TRUE;
    }
    notifs_rec++;

    /*Test ulTaskNotifyTake from ISR*/
    xSemaphoreGive(trigger_send_semphr);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    notifs_rec++;

    /*Test complete, stop timer and delete task*/
    xSemaphoreGive(task_delete_semphr);
    vTaskDelete(NULL);
}

static void sender_ISR(void *arg)
{
    while(1)
    {
        if(isr_give)
        {   
            /*Test vTaskNotifyGiveFromISR()*/
            notifs_sent++;
            xTaskNotifyGive(rec_task_handle);
        }
        else 
        {  
            /*Test xTaskNotifyFromISR()*/
            notifs_sent++;
            xTaskNotify(rec_task_handle, (MESSAGE), eSetValueWithOverwrite);
        }
        portYIELD();
    }
    
}

void TaskNotifyTest(void)
{
    static TaskHandle_t appTaskCreateHand;

    BaseType_t ret = xTaskCreate(sender_ISR, "tsk_call_resp", 4096, \
                                    NULL, 2, &appTaskCreateHand);

    trigger_send_semphr = xSemaphoreCreateBinary();
    task_delete_semphr = xQueueCreateCountingSemaphore(NO_OF_TASKS, 0);

    /*Reset Values*/
    notifs_sent = 0;
    notifs_rec = 0;
    wrong_core = FALSE;

    send_core_message = 0xFF;

    xTaskCreate(ReceiverTask, "rec task", 1000, NULL,  2, &rec_task_handle);
    xTaskCreate(SenderTask, "send task", 1000, NULL,  2, NULL);

    /*Wait for task creation to complete*/
    vTaskDelay(5);      

     /*Trigger sender task*/
    xSemaphoreGive(trigger_send_semphr);   
    for(int k = 0; k < NO_OF_TASKS; k++)
    {   
        /*Wait for sender and receiver task deletion*/
        TEST_ASSERT( xSemaphoreTake(task_delete_semphr, 1000 / portTICK_PERIOD_MS) );
    }
    /*Give time tasks to delete*/
    vTaskDelay(5);      
    
    /*Delete Semaphroes and timer ISRs*/
    vSemaphoreDelete(trigger_send_semphr);
    vSemaphoreDelete(task_delete_semphr);

    printf("TaskNotifyTest end\n");

    while(1)
    {
        vTaskDelay(200);
    }

}
