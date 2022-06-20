/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: main.c
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:01:13
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 */

#include <stdio.h>
#include "parameters.h"
#include "ft_types.h"
#include "FreeRTOS.h"
#include "task.h"
#include "unity.h"

#define ISR_FLOAT_TEST 1
#define BACKPORT_TEST 1
#define DEBUG_QUEUE_REGISTER_TEST 1
#define EVENT_GROUPS_TEST 1
#define TASK_GET_STATE_TEST 1
#define MUTEX_TEST 1
#define SCHEDULING_TIME_TEST 1
#define TASK_DELAY_UNTIL_TEST 1
#define DELETE_TASK_TEST 1
#define TASK_NOTIFY_TEST 1
#define TRACE_FACILITY_TEST 1
#define ISR_LATENCY_TEST 1
#define HOOK_TEST 1
#define NEWLIB_REENT_TEST 1
#define PANIC_TEST 0 /* D2000 aarch32 is not supported*/
#define QUEUE_SET_GET_TEST 1
#define STREAM_TEST 1
#define PRIORITY_TEST 1
#define THREAD_LOCAL_TEST 1
#define TIMER_TEST 1


#if ISR_FLOAT_TEST
extern BaseType_t TestFloatIsrEntry(void);
extern BaseType_t TestWdtIsrFloatEntry(void);
#endif

#if BACKPORT_TEST
extern void StaticTaskCreateTest(void);
extern void TimerFuncTest(void);
extern void QueueFuncTest(void);
#endif

#if DEBUG_QUEUE_REGISTER_TEST
extern void QueueRegistryDebugTest(void);
#endif

#if EVENT_GROUPS_TEST
extern void EventGroupsTest(void);
extern void EventGroupIsrTest(void);
#endif

#if NEWLIB_REENT_TEST
extern void NewLibTest(void);
#endif

#if SCHEDULING_TIME_TEST 
extern void ScheduleTimeTest(void);
#endif

#if TASK_GET_STATE_TEST 
extern void TaskGetStateTest(void);
#endif

#if MUTEX_TEST 
extern void MutexTest(void);
#endif

#if TASK_DELAY_UNTIL_TEST
extern void TaskDelayUntilTest(void);
#endif

#if DELETE_TASK_TEST 
extern void DeleteTaskTest(void);
extern void DeleteBlockTaskTest(void);
#endif

#if TASK_NOTIFY_TEST
extern void TaskNotifyTest(void);
#endif

#if TRACE_FACILITY_TEST
extern void TraceFacilityTest(void);
#endif 

#if ISR_LATENCY_TEST
extern void IsrLatencyTest(void);
#endif

#if HOOK_TEST 
extern void IdleHookTest(void);
extern void TickHookTest(void);
extern void CleanUpHookTest(void);
#endif

#if PANIC_TEST
extern void PanicHandler(void);
#endif

#if QUEUE_SET_GET_TEST
extern void TestQueueSetTest(void);
extern void QueueSetThreadSafetyTest(void);
#endif

#if STREAM_TEST
extern void SendRecvStreamTest(void);
#endif

#if PRIORITY_TEST
extern void GetAndSetPriorityTest(void);
#endif

#if THREAD_LOCAL_TEST
extern void ThreadLocalStorageTest(void);
#endif

#if TIMER_TEST
extern void OneshotTimersTest(void);
extern void RecurringTimersTest(void);
extern void StaticTimersTest(void);
#endif


int main(void)
{
    BaseType_t ret = pdPASS;
    
    #if ISR_FLOAT_TEST
    /* Floating point usage in timer ISR */
    TestFloatIsrEntry();
     /* Floating point usage in wdt ISR */
    /*  TestWdtIsrFloatEntry();*/
    #endif

    #if BACKPORT_TEST
      xTaskCreate((TaskFunction_t )TimerFuncTest, (const char* )"TimerFuncTest", 4096, (void* )NULL, 3, NULL);
      xTaskCreate((TaskFunction_t )QueueFuncTest, (const char* )"QueueFuncTest", 1024, (void* )NULL, 3, NULL);
      xTaskCreate((TaskFunction_t )StaticTaskCreateTest, (const char* )"StaticTaskCreateTest", 8192, (void* )NULL, 3, NULL);
    #endif
    
    #if DEBUG_QUEUE_REGISTER_TEST
    xTaskCreate((TaskFunction_t )QueueRegistryDebugTest, (const char* )"QueueRegistryDebugTest", 1024, (void* )NULL, 1, NULL);
    #endif 
    

    #if EVENT_GROUPS_TEST
    xTaskCreate((TaskFunction_t )EventGroupsTest, (const char* )"EventGroupsTest", 1024, (void* )NULL, 3, NULL);
    /* xTaskCreate((TaskFunction_t )EventGroupIsrTest, (const char* )"EventGroupIsrTest", 1024, (void* )NULL, 3, NULL);*/
    #endif


    #if TASK_GET_STATE_TEST 
    xTaskCreate((TaskFunction_t )TaskGetStateTest, (const char* )"TaskGetStateTest", 1024, (void* )NULL, 3, NULL);
    #endif


    #if SCHEDULING_TIME_TEST 
    xTaskCreate((TaskFunction_t )ScheduleTimeTest, (const char* )"ScheduleTimeTest", 1024, (void* )NULL, 4, NULL);
    #endif


    #if MUTEX_TEST
    xTaskCreate((TaskFunction_t )MutexTest, (const char* )"MutexTest", 1024, (void* )NULL, 3, NULL);
    #endif

    #if TASK_DELAY_UNTIL_TEST
    xTaskCreate((TaskFunction_t )TaskDelayUntilTest, (const char* )"TaskDelayUntilTest", 1024, (void* )NULL, 3, NULL);
    #endif

   #if DELETE_TASK_TEST
    xTaskCreate((TaskFunction_t )DeleteTaskTest, (const char* )"DeleteTaskTest", 1024, (void* )NULL, 1, NULL);
    /* xTaskCreate((TaskFunction_t )DeleteBlockTaskTest, (const char* )"DeleteBlockTaskTest", 1024, (void* )NULL, 1, NULL);*/
    #endif
  
    #if TASK_NOTIFY_TEST
    xTaskCreate((TaskFunction_t )TaskNotifyTest, (const char* )"TaskNotifyTest", 4096, (void* )NULL, 2, NULL);
    #endif 

    #if TRACE_FACILITY_TEST
    xTaskCreate((TaskFunction_t )TraceFacilityTest, (const char* )"TraceFacilityTest", 1024, (void* )NULL, 3, NULL);
    #endif

    #if ISR_LATENCY_TEST
    xTaskCreate((TaskFunction_t )IsrLatencyTest, (const char* )"IsrLatencyTest", 1024, (void* )NULL, 3, NULL);
    #endif

    #if HOOK_TEST
     xTaskCreate((TaskFunction_t )IdleHookTest, (const char* )"IdleHookTest", 1024, (void* )NULL, 1, NULL);
     xTaskCreate((TaskFunction_t )TickHookTest, (const char* )"TickHookTest", 1024, (void* )NULL, 1, NULL);
     xTaskCreate((TaskFunction_t )CleanUpHookTest, (const char* )"CleanUpHookTest", 1024, (void* )NULL, 1, NULL);
    #endif 

    #if NEWLIB_REENT_TEST
    /* new_lib_test reentrant  */
     xTaskCreate((TaskFunction_t )NewLibTest, (const char* )"NewLibTest", 1024, (void* )NULL, 3, NULL);
    #endif

    #if PANIC_TEST
    xTaskCreate((TaskFunction_t )PanicHandler, (const char* )"PanicHandler", 1024, (void* )NULL, 3, NULL);
    #endif

    #if QUEUE_SET_GET_TEST
    xTaskCreate((TaskFunction_t )TestQueueSetTest, (const char* )"TestQueueSetTest", 1024, (void* )NULL, 3, NULL);
    xTaskCreate((TaskFunction_t )QueueSetThreadSafetyTest, (const char* )"QueueSetThreadSafetyTest", 1024, (void* )NULL, 3, NULL);
    #endif

    #if STREAM_TEST
    xTaskCreate((TaskFunction_t )SendRecvStreamTest, (const char* )"SendRecvStreamTest", 1024, (void* )NULL, 3, NULL);
    #endif

    #if PRIORITY_TEST
     xTaskCreate((TaskFunction_t )GetAndSetPriorityTest, (const char* )"GetAndSetPriorityTest", 4096, (void* )NULL, UNITY_FREERTOS_PRIORITY, NULL);
    #endif

    #if THREAD_LOCAL_TEST
     xTaskCreate((TaskFunction_t )ThreadLocalStorageTest, (const char* )"ThreadLocalStorageTest", 8192, (void* )NULL, 3, NULL);
    #endif

    #if TIMER_TEST
        xTaskCreate((TaskFunction_t )OneshotTimersTest, (const char* )"OneshotTimersTest", 1024, (void* )NULL, UNITY_FREERTOS_PRIORITY, NULL);
        xTaskCreate((TaskFunction_t )RecurringTimersTest, (const char* )"RecurringTimersTest", 1024, (void* )NULL, UNITY_FREERTOS_PRIORITY, NULL);
        xTaskCreate((TaskFunction_t )StaticTimersTest, (const char* )"StaticTimersTest", 1024, (void* )NULL, UNITY_FREERTOS_PRIORITY, NULL);
    #endif

    vTaskStartScheduler(); /* 启动任务，开启调度 */   
    while (1); /* 正常不会执行到这里 */
    
FAIL_EXIT:
    printf("failed 0x%x \r\n", ret);  

    return 0;
}