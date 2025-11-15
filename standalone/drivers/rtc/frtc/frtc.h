/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: frtc.h
 * Date: 2021-08-25 14:53:42
 * LastEditTime: 2021-08-26 09:02:51
 * Description:  This file is for user API definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   Wangxiaodong 2021/8/26  first commit
 */

#ifndef DRIVERS_RTC_FRTC_H
#define DRIVERS_RTC_FRTC_H

#include <time.h>

#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uintptr control_base_addr; /* rtc控制寄存器基地址 */
    const char *instance_name; /* instance name */
} FRtcConfig;                  /* rtc配置 */

typedef struct
{
    FRtcConfig config; /* rtc配置 */
    u32 is_ready;      /* rtc初始化完成标志 */
} FRtcCtrl;

typedef struct
{
    u16 year;  /*Specifies the RTC Date Year.
                This parameter must be a number between Min_Data = 2000 and Max_Data = 2099 */
    u8 month;  /*Specifies the RTC Date Month.
                This parameter must be a number between Min_Data = 1 and Max_Data = 12 */
    u8 mday;   /*Specifies the RTC day of Month.
                This parameter must be a number between Min_Data = 1 and Max_Data = 31 */
    u8 hour;   /*Specifies the RTC Time Hour.
                This parameter must be a number between Min_Data = 0 and Max_Data = 23 */
    u8 minute; /*Specifies the RTC Time Minute.
                This parameter must be a number between Min_Data = 0 and Max_Data = 59 */
    u8 second; /*Specifies the RTC Time Second.
                This parameter must be a number between Min_Data = 0 and Max_Data = 59 */
} FRtcDateTime;

#define FRTC_SUCCESS          FT_SUCCESS
#define FRTC_ERR_DATE_INVALID FT_MAKE_ERRCODE(ErrModBsp, ErrBspRtc, BIT(1))
#define FRTC_ERR_TIME_INVALID FT_MAKE_ERRCODE(ErrModBsp, ErrBspRtc, BIT(2))

/* rtc config init */
const FRtcConfig *FRtcLookupConfig(void);

/* initialize rtc ctrl */
FError FRtcCfgInitialize(FRtcCtrl *instance_p, const FRtcConfig *config_p);

/* deinitialize rtc ctrl */
void FRtcCfgDeInitialize(FRtcCtrl *pctrl);

/* set rtc time */
FError FRtcSetDateTime(FRtcCtrl *pctrl, const FRtcDateTime *date_time);

/* get rtc time */
FError FRtcGetDateTime(FRtcCtrl *pctrl, FRtcDateTime *date_time);

/* read rtc time in secs and mesc */
void FRtcReadTimeStamp(FRtcCtrl *pctrl, time_t *sec, time_t *msec);

#ifdef __cplusplus
}
#endif

#endif