
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "unity.h"

SemaphoreHandle_t mutex;

static void MutexReleaseTask(void)
{
    xSemaphoreTake(mutex, portMAX_DELAY);
    xSemaphoreGive(mutex);
    vTaskDelete(NULL);
}

void MutexTest(void)
{
    mutex = xSemaphoreCreateMutex();

    xTaskCreate((TaskFunction_t )MutexReleaseTask, "mutex_release", 2048, (void* )NULL, UNITY_FREERTOS_PRIORITY, NULL);
    vTaskDelay(1);
    
    printf("MutexTest end\n");
    while(1)
    {
        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
}
