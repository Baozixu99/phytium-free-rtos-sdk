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
 * FilePath: ftimer_tacho_os.c
 * Date: 2022-08-23 17:20:51
 * LastEditTime: 2022-08-23 17:20:51
 * Description:  This file is for required function implementations of timer tacho driver used in FreeRTOS.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/11/25  first commit
 */

#include <string.h>
#include "fkernel.h"
#include "ftimer_tacho.h"
#include "ftimer_tacho_os.h"
#include "fparameters.h"
#include "finterrupt.h"
#include "fsleep.h"
#include "fassert.h"



#define FTACHO_OS_DEBUG_TAG "FFreeRTOSTacho"
#define FTACHO_OS_ERROR(format, ...) FT_DEBUG_PRINT_E(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTACHO_OS_WARN(format, ...)  FT_DEBUG_PRINT_W(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTACHO_OS_INFO(format, ...)  FT_DEBUG_PRINT_I(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTACHO_OS_DEBUG(format, ...) FT_DEBUG_PRINT_D(FTACHO_OS_DEBUG_TAG, format, ##__VA_ARGS__)

#define FTIMER_OS_DEBUG_TAG "FFreeRTOSTimer"
#define FTIMER_OS_ERROR(format, ...) FT_DEBUG_PRINT_E(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_OS_WARN(format, ...)  FT_DEBUG_PRINT_W(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_OS_INFO(format, ...)  FT_DEBUG_PRINT_I(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FTIMER_OS_DEBUG(format, ...) FT_DEBUG_PRINT_D(FTIMER_OS_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Constant Definitions *****************************/
/*Notice:timer are 38U,tacho nums only FTACHO_NUM = 16U,but they use the same controller num 0~15. */
static FFreeRTOSTimerTacho os_timer_tacho[38] = {0};

#define MAX_32_VAL GENMASK(31, 0)
#define MAX_64_VAL GENMASK_ULL(63, 0)
#define TACHO_MAX   10000
#define TACHO_MIN   10
#define TACHO_PERIOD 1000000 /* 1000000/50000000 = 0.02s  20ms ticks period at 50Mhz pclk*/

#define US2TICKS(us) ((FTIMER_CLK_FREQ_HZ * (us) / 1000000ULL ) + 1ULL)
#define MS2TICKS(ms) (US2TICKS(1000ULL) * (ms))

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/**
 * @name: FTimerFunctionInit
 * @msg:  timer init.
 * @param {u8}id :use 0~37 timer
 * @param {boolean}timer_mode:单次定时还是循环定时
 * @param {u64}times:定时时间，单位us
 * @return {FFreeRTOSSpim *} return
 */
FFreeRTOSTimerTacho *FFreeRTOSTimerInit(u32 id, boolean timer_mode, u64 times)
{
    FASSERT_MSG(id < FTIMER_NUM, "Invalid timer id.");
    FASSERT_MSG(FT_COMPONENT_IS_READY != os_timer_tacho[id].ctrl.isready, "timer_tacho ready.");
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;
    u64 cnttick = 0;
    u32 startcnt = 0;
    FFreeRTOSTimerTacho *instance = &os_timer_tacho[id];
     
    FTimerTachoCtrl *timer = &instance->ctrl;
    FASSERT(FT_COMPONENT_IS_READY != os_timer_tacho[id].ctrl.isready);
    FASSERT((instance->locker = xSemaphoreCreateMutex()) != NULL);

    FTimerTachoConfig *timercfg = &timer->config;
    memset(timer, 0, sizeof(timer));
        /* tacho  */
    FTimerGetDefConfig(id, timercfg);
    timercfg->id = id;
    timercfg->cmp_type = timer_mode;
    timercfg->timer_mode = FTIMER_RESTART;

    cnttick = US2TICKS(times);
    FTIMER_OS_INFO("\n***cnttick:%llu.", cnttick);
    if (cnttick > 0xffffffff)
    {
        timercfg->timer_bits = FTIMER_64_BITS;
        startcnt = MAX_64_VAL - cnttick;
    }
    else
    {
        timercfg->timer_bits = FTIMER_32_BITS;
        startcnt = MAX_32_VAL - cnttick;
    }
    ret = FTimerInit(timer, timercfg);

    if (timercfg->timer_bits == FTIMER_32_BITS)
    {
        ret |= FTimerSetPeriod32(timer, MAX_32_VAL);
    }
    else
    {
        ret |= FTimerSetPeriod64(timer, MAX_64_VAL);
    }


  /*将时间参数us装换成计时器的ticks，我们设置StartTick，将CmpTick设置为最大*/
    ret = FTimerSetStartVal(timer,startcnt);
        if (FREERTOS_TIMER_TACHO_SUCCESS != ret)
    {
        return NULL;
    }
        FTIMER_OS_INFO("Timer Init finished.");

    return (&os_timer_tacho[id]);
}

/**
 * @name: FFTimerStartTest
 * @msg:  start timer.
 * @param {u64 times,boolean forceLoad}
 * @return {FError}
 */
FError FFreeRTOSTimerStart(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p->locker);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;

    FTimerTachoCtrl *timer = &os_timer_p->ctrl;

    if (pdFALSE == xSemaphoreTake(os_timer_p->locker, portMAX_DELAY))
    {
        FTIMER_OS_ERROR("Timer xSemaphoreTake failed.");
        /* We could not take the semaphore, exit with 0 data received */
        return FREERTOS_TIMER_TACHO_SEMA_ERROR;
    }

    ret = FTimerStart(timer);
    if (FREERTOS_TIMER_TACHO_SUCCESS != ret)
    {
        return ret;
    }
    return ret;
}

/**
 * @name: FFreeRTOSTimerStop
 * @msg:
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_timer_p
 */
FError FFreeRTOSTimerStop(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p->locker);

    if (pdFALSE == xSemaphoreGive(os_timer_p->locker))
    {
        /* We could not post the semaphore, exit with error */
        FTIMER_OS_ERROR("Timer xSemaphoreGive failed.");
        return FREERTOS_TIMER_TACHO_SEMA_ERROR;
    }
    FTimerTachoCtrl *timer = &os_timer_p->ctrl;

    return FTimerStop(timer);
}


/**
 * @name: FFreeRTOSTimerDeinit
 * @msg:
 * @return {*} void
 * @param {FFreeRTOSTimerTacho} *os_timer_p
 */
void FFreeRTOSTimerDeinit(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p->locker);

    FTimerDeInit(&os_timer_p->ctrl);
    vSemaphoreDelete(os_timer_p->locker);
    memset(os_timer_p, 0, sizeof(*os_timer_p));
    return;
}


/**
 * @name: FFreeRTOSTimerDebug
 * @msg: Dump timer reg message
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_timer_p
 */
void FFreeRTOSTimerDebug(FFreeRTOSTimerTacho *os_timer_p)
{
    FASSERT(NULL != os_timer_p);
    FTimeSettingDump(&os_timer_p->ctrl);
    return;
}



/**********************************************************************************************************/
/***********************************************tacho******************************************************/
/**********************************************************************************************************/

/**
 * @name: FFreeRTOSTachoInit
 * @msg: tacho or capture init function
 * @return {*}
 * @param {u32} id
 * @param {boolean} tacho_mode
 */
FFreeRTOSTimerTacho *FFreeRTOSTachoInit(u32 id, boolean tacho_mode)
{
    FASSERT_MSG(id < FTACHO_NUM, "Invalid timer id.");
    FASSERT_MSG(FT_COMPONENT_IS_READY != os_timer_tacho[id].ctrl.isready, "timer_tacho ready.");

    FFreeRTOSTimerTacho *instance = &os_timer_tacho[id];
    FTimerTachoCtrl *tacho = &os_timer_tacho[id].ctrl;

    FTimerTachoConfig *tachocfg = &tacho->config;
    memset(tacho, 0, sizeof(tacho));
    /* tacho  */
    FTachoGetDefConfig(id, tachocfg);

    FASSERT((instance->locker = xSemaphoreCreateMutex()) != NULL);

    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;
    tachocfg->id = id;
    tachocfg->work_mode = FTIMER_WORK_MODE_TACHO;
    tachocfg->edge_mode = FTACHO_RISING_EDGE;/* Not open operation interface for cmd */
    tachocfg->jitter_level = FTACHO_JITTER_LEVEL0;/* Not open operation interface for cmd */
    tachocfg->timer_bits  = FTIMER_32_BITS;/* Use capture mode, Not open operation interface for cmd.*/
    tachocfg->timer_mode  = FTIMER_RESTART;/* Use capture mode, Not open operation interface for cmd.*/
    tachocfg->plus_num = TACHO_PERIOD;

    if (tacho_mode == FTIMER_WORK_MODE_TACHO)
    {
        /* Not open operation interface for cmd */
        FTachoSetOverLimit(tacho, TACHO_MAX);
        FTachoSetUnderLimit(tacho, TACHO_MIN);
        tachocfg->captue_cnt = 0x7f;/* 边沿检测计数默认值 */
    }
    ret = FTachoInit(tacho, tachocfg);

    return (&os_timer_tacho[id]);
}

/**
 * @name: FFreeRTOSTachoGetRPM
 * @msg: get tacho RPM
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_tacho_p
 * @param {u32} *rpm
 */
FError FFreeRTOSTachoGetRPM(FFreeRTOSTimerTacho *os_tacho_p, u32 *rpm)
{
    FASSERT(NULL != os_tacho_p->locker);
    FError ret = FREERTOS_TIMER_TACHO_SUCCESS;

    FTimerTachoCtrl *tacho = &os_tacho_p->ctrl;


    ret = FTachoGetFanRPM(tacho, rpm);
    if (ret != FREERTOS_TIMER_TACHO_SUCCESS)
    {
        FTIMER_OS_ERROR("Tachometer get error,please check init.");
        return ret;
    }
    return ret;
}

/**
 * @name: FFreeRTOSTachoGetCNT
 * @msg: get capture value
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_tacho_p
 */
u32 FFreeRTOSTachoGetCNT(FFreeRTOSTimerTacho *os_tacho_p)
{
    FASSERT(NULL != os_tacho_p->locker);

    FTimerTachoCtrl *tacho = &os_tacho_p->ctrl;

    return FTachoGetCaptureCnt(tacho);
}

/**
 * @name: FFreeRTOSTachoDeinit
 * @msg: deinit tacho or capture
 * @return {*}
 * @param {FFreeRTOSTimerTacho} *os_tacho_p
 */
void FFreeRTOSTachoDeinit(FFreeRTOSTimerTacho *os_tacho_p)
{
    FASSERT(NULL != os_tacho_p->locker);

    FTachoDeInit(&os_tacho_p->ctrl);
    vSemaphoreDelete(os_tacho_p->locker);
    memset(os_tacho_p, 0, sizeof(*os_tacho_p));
    return;
}

