/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * FilePath: posix_example.c
 * Date: 2023-10-12 10:41:45
 * LastEditTime: 2023-10-12 10:41:45
 * Description:  This file is for freertos posix demo test
 *
 * Modify History:
 *  Ver   Who       Date        Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 wangxiaodong 2023/10/12 first commit
 */

/* System headers. */
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* FreeRTOS includes. */
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fdebug.h"
#include "fassert.h"
#include "posix_example.h"

/* FreeRTOS+POSIX. */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/fcntl.h"
#include "FreeRTOS_POSIX/errno.h"

#define FPOSIX_DEBUG_TAG "POSIX_TEST"
#define FPOSIX_ERROR(format, ...) FT_DEBUG_PRINT_E(FPOSIX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPOSIX_WARN(format, ...) FT_DEBUG_PRINT_W(FPOSIX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPOSIX_INFO(format, ...) FT_DEBUG_PRINT_I(FPOSIX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FPOSIX_DEBUG(format, ...) FT_DEBUG_PRINT_D(FPOSIX_DEBUG_TAG, format, ##__VA_ARGS__)

enum
{
    POSIX_TEST_SUCCESS = 0,
    POSIX_TEST_UNKNOWN = 1,
    POSIX_TEST_FAILURE = 2,
};
#define TIMER_OUT (pdMS_TO_TICKS(5000UL))
static QueueHandle_t xQueue = NULL;
static xTaskHandle xtask_handle;

#define TASK_STACK_SIZE 1024
#define LINE_BREAK "\r\n" /* Constants. */

/**
 * @brief Control messages.
 *
 * uint8_t is sufficient for this enum, that we are going to cast to char directly.
 * If ever needed, implement a function to properly typecast.
 */
/**@{ */
typedef enum ControlMessage
{
    eMSG_LOWER_INAVLID = 0x00,        /**< Guard, let's not use 0x00 for messages. */
    eWORKER_CTRL_MSG_CONTINUE = 0x01, /**< Dispatcher to worker, distributing another job. */
    eWORKER_CTRL_MSG_EXIT = 0x02,     /**< Dispatcher to worker, all jobs are finished and the worker receiving such can exit. */

    /* define additional messages here */
    eMSG_UPPER_INVALID = 0xFF /**< Guard, additional tasks shall be defined above. */
} eControlMessage;
/**@} */

/**
 * @defgroup Configuration constants for the dispatcher-worker demo.
 */
/**@{ */
#define MQUEUE_NUMBER_OF_WORKERS (4) /**< The number of worker threads, each thread has one queue which is used as income box. */

#if (MQUEUE_NUMBER_OF_WORKERS > 10)
#error "Please keep MQUEUE_NUMBER_OF_WORKERS < 10."
#endif

#define MQUEUE_WORKER_QNAME_BASE "/qNode0" /**< Queue name base. */
#define MQUEUE_WORKER_QNAME_BASE_LEN (6)   /** Queue name base length. */

#define MQUEUE_TIMEOUT_SECONDS (1)               /**< Relative timeout for mqueue functions. */
#define MQUEUE_MAX_NUMBER_OF_MESSAGES_WORKER (1) /**< Maximum number of messages in a queue. */

#define MQUEUE_MSG_WORKER_CTRL_MSG_SIZE sizeof(uint8_t) /**< Control message size. */
#define MESSAGE_TRAN_SUCCESS 0
#define MESSAGE_TRAN_FAILED -1
#define POSIX_DEMO__TASK_PRIORITY 3

static int message_correct_flag = MESSAGE_TRAN_SUCCESS;
/**@} */

/**
 * @brief Structure used by Worker thread.
 */
/**@{ */
typedef struct WorkerThreadResources
{
    pthread_t pxID; /**< thread ID. */
    mqd_t xInboxID; /**< mqueue inbox ID. */
} WorkerThreadResources_t;
/**@} */

/**
 * @brief Structure used by Dispatcher thread.
 */
/**@{ */
typedef struct DispatcherThreadResources
{
    pthread_t pxID;   /**< thread ID. */
    mqd_t *pOutboxID; /**< a list of mqueue outbox ID. */
} DispatcherThreadResources_t;
/**@} */

/*-----------------------------------------------------------*/

static void *prvWorkerThread(void *pvArgs)
{
    if (message_correct_flag != MESSAGE_TRAN_SUCCESS)
    {
        FPOSIX_ERROR("prvWorkerThread check message_correct_flag error.");

        return NULL;
    }
    WorkerThreadResources_t pArgList = *(WorkerThreadResources_t *)pvArgs;

    FPOSIX_DEBUG("Worker thread #[%d] - start %s", (int)pArgList.pxID, LINE_BREAK);

    struct timespec xReceiveTimeout = {0};

    ssize_t xMessageSize = 0;
    char pcReceiveBuffer[MQUEUE_MSG_WORKER_CTRL_MSG_SIZE] = {0};

    /* This is a worker thread that reacts based on what is sent to its inbox (mqueue). */
    while (true)
    {
        clock_gettime(CLOCK_REALTIME, &xReceiveTimeout);
        xReceiveTimeout.tv_sec += MQUEUE_TIMEOUT_SECONDS;

        xMessageSize = mq_receive(pArgList.xInboxID,
                                  pcReceiveBuffer,
                                  MQUEUE_MSG_WORKER_CTRL_MSG_SIZE,
                                  0);

        /* Parse messages */
        if (xMessageSize == MQUEUE_MSG_WORKER_CTRL_MSG_SIZE)
        {
            switch ((int)pcReceiveBuffer[0])
            {
            case eWORKER_CTRL_MSG_CONTINUE:
                /* Task branch, currently only prints message to screen. */
                /* Could perform tasks here. Could also notify dispatcher upon completion, if desired. */
                FPOSIX_DEBUG("Worker thread #[%d] -- Received eWORKER_CTRL_MSG_CONTINUE %s", (int)pArgList.pxID, LINE_BREAK);
                break;

            case eWORKER_CTRL_MSG_EXIT:
                FPOSIX_DEBUG("Worker thread #[%d] -- Finished. Exit now. %s", (int)pArgList.pxID, LINE_BREAK);

                return NULL;

            default:
                /* Received a message that we don't care or not defined. */
                break;
            }
        }
        else
        {
            FPOSIX_ERROR("prvWorkerThread mq_receive error.");
            message_correct_flag = MESSAGE_TRAN_FAILED;

            return NULL;
        }
    }

    /* You should never hit here. */
    /* return NULL; */
}

/*-----------------------------------------------------------*/

static void *prvDispatcherThread(void *pvArgs)
{
    DispatcherThreadResources_t pArgList = *(DispatcherThreadResources_t *)pvArgs;

    FPOSIX_DEBUG("Dispatcher thread - start %s", LINE_BREAK);

    struct timespec xSendTimeout = {0};

    int iStatus = 0;
    char pcSendBuffer[MQUEUE_MSG_WORKER_CTRL_MSG_SIZE] = {0};

    /* Just for fun, let threads do a total of 100 independent tasks. */
    int i = 0;
    const int totalNumOfJobsPerThread = 100;

    /* Distribute 1000 independent tasks to workers, in round-robin fashion. */
    pcSendBuffer[0] = (char)eWORKER_CTRL_MSG_CONTINUE;

    for (i = 0; i < totalNumOfJobsPerThread; i++)
    {
        if (message_correct_flag != MESSAGE_TRAN_SUCCESS)
        {
            FPOSIX_ERROR("prvDispatcherThread check message_correct_flag error.");

            return NULL;
        }
        
        clock_gettime(CLOCK_REALTIME, &xSendTimeout);
        xSendTimeout.tv_sec += MQUEUE_TIMEOUT_SECONDS;
        FPOSIX_DEBUG("Dispatcher iteration #[%d] -- Sending msg to worker thread #[%d]. %s", i, (int)pArgList.pOutboxID[i % MQUEUE_NUMBER_OF_WORKERS], LINE_BREAK);

        iStatus = mq_timedsend(pArgList.pOutboxID[i % MQUEUE_NUMBER_OF_WORKERS],
                                    pcSendBuffer,
                                    MQUEUE_MSG_WORKER_CTRL_MSG_SIZE,
                                    0,
                                    &xSendTimeout);
        if (iStatus != 0)
        {
            /* This error is acceptable in our setup.
             * Since inbox for each thread fits only one message.
             * In reality, balance inbox size, message arrival rate, and message drop rate. */
            FPOSIX_ERROR("An acceptable failure -- dispatcher failed to send eWORKER_CTRL_MSG_CONTINUE to outbox ID: %x. errno %d %s",
                   (int)pArgList.pOutboxID[i % MQUEUE_NUMBER_OF_WORKERS], errno, LINE_BREAK);

            return NULL;
        }
    }

    /* Control thread is now done with distributing jobs. Tell workers they are done. */
    pcSendBuffer[0] = (char)eWORKER_CTRL_MSG_EXIT;

    for (i = 0; i < MQUEUE_NUMBER_OF_WORKERS; i++)
    {
        FPOSIX_DEBUG("Dispatcher [%d] -- Sending eWORKER_CTRL_MSG_EXIT to worker thread #[%d]. %s", i, (int)pArgList.pOutboxID[i % MQUEUE_NUMBER_OF_WORKERS], LINE_BREAK);
        /* This is a blocking call, to guarantee worker thread exits. */
        iStatus = mq_send(pArgList.pOutboxID[i % MQUEUE_NUMBER_OF_WORKERS],
                               pcSendBuffer,
                               MQUEUE_MSG_WORKER_CTRL_MSG_SIZE,
                               0);
    }

    return NULL;
}

/*-----------------------------------------------------------*/

/**
 * @brief Job distribution with actor model.
 *
 * See the top of this file for detailed description.
 */
void POSIXDemoTask(void)
{
    int i = 0;
    int iStatus = 0;
    int task_res = POSIX_TEST_SUCCESS;

    /* Handles of the threads and related resources. */
    DispatcherThreadResources_t pxDispatcher = {0};
    WorkerThreadResources_t pxWorkers[MQUEUE_NUMBER_OF_WORKERS] = {{0}};
    mqd_t workerMqueues[MQUEUE_NUMBER_OF_WORKERS] = {0};

    struct mq_attr xQueueAttributesWorker =
        {
            .mq_flags = 0,
            .mq_maxmsg = MQUEUE_MAX_NUMBER_OF_MESSAGES_WORKER,
            .mq_msgsize = MQUEUE_MSG_WORKER_CTRL_MSG_SIZE,
            .mq_curmsgs = 0
        };

    pxDispatcher.pOutboxID = workerMqueues;

    /* Create message queues for each worker thread. */
    for (i = 0; i < MQUEUE_NUMBER_OF_WORKERS; i++)
    {
        /* Prepare a unique queue name for each worker. */
        char qName[] = MQUEUE_WORKER_QNAME_BASE;
        qName[MQUEUE_WORKER_QNAME_BASE_LEN - 1] = qName[MQUEUE_WORKER_QNAME_BASE_LEN - 1] + i;

        /* Open a queue with --
         * O_CREAT -- create a message queue.
         * O_RDWR -- both receiving and sending messages.
         */
        pxWorkers[i].xInboxID = mq_open(qName,
                                        O_CREAT | O_RDWR,
                                        (mode_t)0,
                                        &xQueueAttributesWorker);

        if (pxWorkers[i].xInboxID == (mqd_t)-1)
        {
            FPOSIX_ERROR("Invalid inbox (mqueue) for worker. %s", LINE_BREAK);
            iStatus = -1;
            task_res = POSIX_TEST_FAILURE;
            goto task_exit;
        }

        /* Outboxes of dispatcher thread is the inboxes of all worker threads. */
        pxDispatcher.pOutboxID[i] = pxWorkers[i].xInboxID;
    }

    /* Create and start Worker threads. */
    if (iStatus == 0)
    {
        for (i = 0; i < MQUEUE_NUMBER_OF_WORKERS; i++)
        {
            iStatus = pthread_create(&(pxWorkers[i].pxID), NULL, prvWorkerThread, &pxWorkers[i]);
            if (iStatus != 0)
            {
                task_res = POSIX_TEST_FAILURE;
                goto task_exit;
            }
        }

        /* Create and start dispatcher thread. */
        iStatus = pthread_create(&(pxDispatcher.pxID), NULL, prvDispatcherThread, &pxDispatcher);
        if (iStatus != 0)
        {
            task_res = POSIX_TEST_FAILURE;
            goto task_exit;
        }

        /* Actors will do predefined tasks in threads. Current implementation is that
         * dispatcher actor notifies worker actors to terminate upon finishing distributing tasks. */

        /* Wait for worker threads to join. */
        for (i = 0; i < MQUEUE_NUMBER_OF_WORKERS; i++)
        {
            (void)pthread_join(pxWorkers[i].pxID, NULL);
        }

        /* Wait for dispatcher thread to join. */
        (void)pthread_join(pxDispatcher.pxID, NULL);
    }

    /* Close and unlink worker message queues. */
    for (i = 0; i < MQUEUE_NUMBER_OF_WORKERS; i++)
    {
        char qName[] = MQUEUE_WORKER_QNAME_BASE;
        qName[MQUEUE_WORKER_QNAME_BASE_LEN - 1] = qName[MQUEUE_WORKER_QNAME_BASE_LEN - 1] + i;

        if (pxWorkers[i].xInboxID != NULL)
        {
            (void)mq_close(pxWorkers[i].xInboxID);
            (void)mq_unlink(qName);
        }
    }
    /* Have something on console. */
    if (iStatus == 0 && message_correct_flag == MESSAGE_TRAN_SUCCESS)
    {
        printf("All threads finished. %s", LINE_BREAK);
    }
    else
    {
        FPOSIX_ERROR("Queues did not get initialized properly. Did not run demo. %s", LINE_BREAK);
        task_res = POSIX_TEST_FAILURE;
    }

task_exit:
    xQueueSend(xQueue, &task_res, 0);
    /* This task was created with the native xTaskCreate() API function, so
    must not run off the end of its implementing thread. */
    vTaskDelete(NULL);
}

BaseType_t CreatePOSIXDemoTasks(void)
{
    BaseType_t xReturn = pdPASS; /* 定义一个创建信息返回值，默认为 pdPASS */
    BaseType_t xTimerStarted = pdPASS;
    int task_res = POSIX_TEST_UNKNOWN;

    xQueue = xQueueCreate(1, sizeof(int));
    if (xQueue == NULL)
    {
        FPOSIX_ERROR("xQueue create failed.\r\n");
        goto exit;
    }

    xReturn = xTaskCreate((TaskFunction_t)POSIXDemoTask, /* 任务入口函数 */
                          (const char *)"POSIXDemoTask",       /* 任务名字 */
                          TASK_STACK_SIZE,       /* 任务栈大小 */
                          (void *)NULL,                    /* 任务入口函数参数 */
                          (UBaseType_t)POSIX_DEMO__TASK_PRIORITY,                  /* 任务的优先级 */
                          &xtask_handle);                  /* 任务控制 */
    FASSERT_MSG(xReturn == pdPASS, "POSIXDemoTask creation is failed.");

    xReturn = xQueueReceive(xQueue, &task_res, TIMER_OUT);
    if (xReturn == pdFAIL)
    {
        FPOSIX_ERROR("xQueue receive timeout.\r\n");
        goto exit;
    }

exit:
    if (task_res != POSIX_TEST_SUCCESS || message_correct_flag != MESSAGE_TRAN_SUCCESS)
    {
        printf("%s@%d: Posix test example [failure], task_res = %d.\r\n", __func__, __LINE__, task_res);
        return pdFAIL;
    }
    else
    {
        printf("%s@%d: Posix test example [success].\r\n", __func__, __LINE__);
        return pdTRUE;
    }
}
