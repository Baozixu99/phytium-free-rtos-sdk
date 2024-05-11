/* This example demonstrates:
   creates and starts a one-shot timer and an auto-reload timer in diffirent period.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#define AUTO_RELOAD_TIMES         3
#define TIMER_TASK_NUM            2
#define EXAMPLE_TIMEOUT           (pdMS_TO_TICKS(30000UL))

/* The periods assigned to the one-shot and auto-reload timers respectively. */
#define ONE_SHOT_TIMER_PERIOD     (pdMS_TO_TICKS(3000UL))
#define AUTO_RELOAD_TIMER_PERIOD  (pdMS_TO_TICKS(3000UL))

#define ONE_SHOT_SUCCESS_BIT      (1UL << 0UL) /* bit 0, which is set by one-shot task. */
#define AUTO_RELOAD_SUCCESS_BIT   (1UL << 1UL) /* bit 1, which is set by auto-reload task. */

static QueueHandle_t xQueue = NULL;
static TimerHandle_t xOneShotTimer;
static TimerHandle_t xAutoReloadTimer;

/*-----------------------------------------------------------*/
/* The callback functions used by the one-shot and auto-reload timers respectively. */
static void prvOneShotTimerCallback(TimerHandle_t xTimer)
{   
    static TickType_t xTimeNow;
    BaseType_t xReturn = pdFAIL;
    u32 task_res = 0;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintf("One-shot timer callback executing once, and now_ticks: %d \r\n", xTimeNow);

    xReturn = xTimerDelete(xOneShotTimer, 0);
    if (xReturn != pdPASS)
    {
        vPrintf("DeleteSoftwareTimerTasks xTimerDelete OneShot failed.\r\n");
    }
    else
    {
        vPrintf("Delete software timer one-shot task.\r\n");
        task_res = ONE_SHOT_SUCCESS_BIT;
        xQueueSend(xQueue, &task_res, 0);
    }
}

/*-----------------------------------------------------------*/

static void prvAutoReloadTimerCallback(TimerHandle_t xTimer)
{
    static TickType_t xTimeNow;
    static int auto_timer_times = 0; /* Tracking the number of callback executions */
    BaseType_t xReturn = pdFAIL;
    u32 task_res = 0;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintf("Auto-reload timer callback executing for %d time(s), and now_ticks: %d \r\n", ++auto_timer_times, xTimeNow);

    if (auto_timer_times >= 3)
    {
        xReturn = xTimerDelete(xAutoReloadTimer, 0);
        if (xReturn != pdPASS)
        {
            vPrintf("DeleteSoftwareTimerTasks xTimerDelete AutoReload failed.\r\n");
        }
        else
        {
            vPrintf("Delete software timer auto-reload task.\r\n");
            task_res = AUTO_RELOAD_SUCCESS_BIT;
            xQueueSend(xQueue, &task_res, 0);
        }
    }
}

/*-----------------------------------------------------------*/

int CreateTimerTasks(void)
{
    BaseType_t xTimer1Started;
    BaseType_t xTimer2Started;
    BaseType_t xReturn = pdPASS;  /* Define a return value with a default of pdPASS */
    u32 task_res = 0;
    u32 task_flag = 0;

    xQueue = xQueueCreate(2, sizeof(u32)); /* Create Message Queue */
    if (xQueue == NULL)
    {
        vPrintf("xQueue create failed. \r\n");
        goto exit;
    }

    /* Create the one shot software timer, storing the handle to the created software timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("Create OneShot",         /* Text name for the software timer - not used by FreeRTOS. */
                                 ONE_SHOT_TIMER_PERIOD,    /* The software timer's period in ticks. */
                                 pdFALSE,                  /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
                                 0,                        /* This example does not use the timer id. */
                                 prvOneShotTimerCallback); /* The callback function to be used by the software timer being created. */

    /* Create the auto-reload software timer, storing the handle to the created software timer in xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate("Create AutoReload",         /* Text name for the software timer - not used by FreeRTOS. */
                                    AUTO_RELOAD_TIMER_PERIOD,    /* The software timer's period in ticks. */
                                    pdTRUE,                      /* Set uxAutoRealod to pdTRUE to create an auto-reload software timer. */
                                    0,                           /* This example does not use the timer id. */
                                    prvAutoReloadTimerCallback); /* The callback function to be used by the software timer being created. */

    /* Check the timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL))
    {
        /* Start the software timers, using a block time of 0 (no block time).
         * The scheduler has not been started yet so any block time specified here would be ignored anyway.
         */
        xTimer1Started = xTimerStart(xOneShotTimer, 0);
        xTimer2Started = xTimerStart(xAutoReloadTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue, and xTimerStart() will fail if the timer command queue gets full.
         * The timer service task does not get created until the scheduler is started, 
           so all commands sent to the command queue will stay in the queue until after the scheduler has been started.
         * Check both calls to xTimerStart() passed.
         */
        if ((xTimer1Started != pdPASS) || (xTimer2Started != pdPASS))
        {
            vPrintf("CreateSoftwareTimerTasks xTimerStart failed \r\n");
            goto exit;
        }
    }
    else
    {
        vPrintf("CreateSoftwareTimerTasks xTimerCreate failed \r\n");
        goto exit;
    }

    for (int loop = 0; loop < TIMER_TASK_NUM; loop++)
    {
        xReturn = xQueueReceive(xQueue, &task_res, EXAMPLE_TIMEOUT);
        if (xReturn == pdFAIL)
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

    if (task_flag != (ONE_SHOT_SUCCESS_BIT | AUTO_RELOAD_SUCCESS_BIT))
    {
        vPrintf("%s@%d: Software timer create start example [failure], task_flag = 0x%x \r\n", __func__, __LINE__, task_flag);
        return -1;
    }
    else
    {
        vPrintf("%s@%d: Software timer create start example [success].\r\n", __func__, __LINE__);
        return 0;
    }
}