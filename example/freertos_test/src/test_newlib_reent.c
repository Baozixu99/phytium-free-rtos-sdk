/*
 Test for multicore FreeRTOS. This test spins up threads, fiddles with queues etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "unity.h"

static int done;
static int error;
int l = 0;

static void tskTestRand(void)
{
    srand(0x1234);
    vTaskDelay(10);
    l = rand();
    if (l != 869320854) {
       error++;
    }
    vTaskDelay(2);
    l = rand();
    if (l != 1148737841) {
       error++;
    }
    done++;

    while (1) {
        vTaskDelay(1000);
    }
}

/* split this thing into separate orthogonal tests*/
void NewLibTest(void)
{
    
    xTaskCreate((TaskFunction_t )tskTestRand, (const char* )"tsk1", 1024, (void* )NULL, 3, NULL);
    xTaskCreate((TaskFunction_t )tskTestRand, (const char* )"tsk2", 1024, (void* )NULL, 3, NULL);
    xTaskCreate((TaskFunction_t )tskTestRand, (const char* )"tsk3", 1024, (void* )NULL, 3, NULL);
    xTaskCreate((TaskFunction_t )tskTestRand, (const char* )"tsk4", 1024, (void* )NULL, 3, NULL);
    vTaskDelay(1000);
    printf("NewLibTest end\n");
    while (1) {
        vTaskDelay(1000);
    }
}
