/*
 Test for FreeRTOS. This test whether the cpu debug error info.
*/

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "unity.h"

void PanicHandler(void)
{
    volatile int *i;
    i = (volatile int *)0x0;
    *i = 1;
    printf("PanicHandler end\n");
    while(1){
        vTaskDelay(200);
    }
}
