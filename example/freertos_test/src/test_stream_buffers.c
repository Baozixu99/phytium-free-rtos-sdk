#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stream_buffer.h"
#include "message_buffer.h"
#include "unity.h"
#include "ft_types.h"

typedef struct {
    StreamBufferHandle_t sb;
    SemaphoreHandle_t end_test;
    bool send_fail;
    bool receive_fail;
    bool produce_isr;
}test_context;

static void producer_task(void *arg)
{
    TEST_ASSERT(arg != NULL);
    test_context *tc  = arg;
    u8 produced = 0;
    printf("Starting sender task... \n");

    while(produced < 100) {

        if(!tc->produce_isr) {
            BaseType_t result = xStreamBufferSend(tc->sb, &produced, 1, 0);
            if(!result) {
                tc->send_fail = true;
                xSemaphoreGive(tc->end_test);
                vTaskDelete(NULL);
            } else {
                produced++;
            }
        }

        vTaskDelay(1);
    }

    tc->send_fail = false;
    vTaskDelete(NULL);
}

static void receiver_task(void *arg)
{
    TEST_ASSERT(arg != NULL);
    test_context *tc = arg;
    u8 expected_consumed = 0;
    printf("Starting receiver task... \n");

    for(;;){
         u8 read_byte = 0xFF;
         u32 result = xStreamBufferReceive(tc->sb, &read_byte, 1, 1000);

         if((read_byte != expected_consumed) || !result) {
            tc->receive_fail = true;
            xSemaphoreGive(tc->end_test);
            vTaskDelete(NULL);
         } else {
             expected_consumed++;
             if(expected_consumed == 99) {
                 break;
             }
         }
    }
    printf("expected_consumed=%d\r\n", expected_consumed);
    tc->receive_fail = false;
    xSemaphoreGive(tc->end_test);
    vTaskDelete(NULL);
}

void SendRecvStreamTest(void)
{
    BaseType_t result;
    test_context tc;

    tc.sb = xStreamBufferCreate(128, 1);
    tc.end_test = xSemaphoreCreateBinary();

    TEST_ASSERT(tc.sb);
    TEST_ASSERT(tc.end_test);

    tc.send_fail = false;
    tc.receive_fail = false;
    tc.produce_isr = false;
    
    result = xTaskCreate(producer_task, "sender", 4096, &tc, UNITY_FREERTOS_PRIORITY , NULL);
    TEST_ASSERT(result == pdTRUE);
    result = xTaskCreate(receiver_task, "receiver", 4096, &tc, UNITY_FREERTOS_PRIORITY, NULL);
    TEST_ASSERT(result == pdTRUE);
    
    result = xSemaphoreTake(tc.end_test, 2000);
    TEST_ASSERT(result == pdTRUE);

    vTaskDelay(1);

    TEST_ASSERT(tc.send_fail == false);
    TEST_ASSERT(tc.receive_fail == false);

    vStreamBufferDelete(tc.sb);
    vSemaphoreDelete(tc.end_test);
    
    printf("SendRecvStreamTest end\n");
    while(1){
        vTaskDelay(200);
    }
}
