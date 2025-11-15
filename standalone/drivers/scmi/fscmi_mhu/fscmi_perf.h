/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fscmi_perf.h
 * Date: 2022-12-29 16:40:54
 * LastEditTime: 2022-12-29 16:40:55
 * Description:  This file is for performance message struct
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/01/31 init
 */
#ifndef FSCMI_PERF_H
#define FSCMI_PERF_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fscmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum FScmiPerformanceProtocolCmd
{
    PERF_VERSION_GET = 0x0,
    PERF_ATTRIBUTES_GET = 0x1,
    PERF_DOMAIN_ATTRIBUTES = 0x3,
    PERF_DESCRIBE_LEVELS = 0x4,
    PERF_LIMITS_SET = 0x5,
    PERF_LIMITS_GET = 0x6,
    PERF_LEVEL_SET = 0x7,
    PERF_LEVEL_GET = 0x8,
    PERF_NOTIFY_LIMITS = 0x9,
    PERF_NOTIFY_LEVEL = 0xa,
};

struct FScmiMsgRespPerfAttributes
{
    u16 num_domains;
    u16 flags;
#define POWER_SCALE_IN_MILLIWATT(x) ((x)&BIT(0))
    u32 stats_addr_low;
    u32 stats_addr_high;
    u32 stats_size;
};

struct FScmiMsgRespPerfDomainAttributes
{
    u32 flags;
#define SUPPORTS_SET_LIMITS(x)        ((x)&BIT(31))
#define SUPPORTS_SET_PERF_LVL(x)      ((x)&BIT(30))
#define SUPPORTS_PERF_LIMIT_NOTIFY(x) ((x)&BIT(29))
#define SUPPORTS_PERF_LEVEL_NOTIFY(x) ((x)&BIT(28))
#define SUPPORTS_FAST_CHANNEL(x)      ((x)&BIT(27))
    u32 rate_limit_us;           /* 连续请求的最小时间间隔 当前为1000us */
    u32 sustained_freq;          /* 持续频率 */
    u32 sustained_perf_level;    /* 持续性能等级 */
    u8 name[FSCMI_MAX_STR_SIZE]; /* Domain name */
};

struct FScmiMsgPerfDescribeLevels
{
    u32 domain;
    u32 level_index;
};

struct FScmiPerfSetLimits
{
    u32 domain;
    u32 max_level;
    u32 min_level;
};

struct FScmiPerfGetLimits
{
    u32 max_level;
    u32 min_level;
};

struct FScmiPerfSetLevel
{
    u32 domain;
    u32 level;
};

struct FScmiPerfNotifyLevelOrLimits
{
    u32 domain;
    u32 notify_enable;
};

struct FScmiMsgRespPerfDescribeLevels
{
    u16 num_returned;
    u16 num_remaining;
    struct
    {
        u32 perf_val;              /* Performance level value */
        u32 power;                 /* Power cost */
        u16 transition_latency_us; /* Worst-case transition latency in microseconds to move from any supported performance to the level indicated by this entry in the array */
        u16 reserved;
    } opp[0];
};

FError FScmiPerfInit(FScmi *instance_p, boolean poll_completion);

#ifdef __cplusplus
}
#endif

#endif
