/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fscmi_clock.h
 * Created Date: 2025-02-14 11:56:17
 * Last Modified: 2025-03-10 20:46:54
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef FSCMI_CLOCK_H
#define FSCMI_CLOCK_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fscmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SCMI_MAX_NUM_RATES 16

enum FScmiClockProtocolCmd
{
    CLOCK_VERSION_GET = 0x0,    /* version of the protocol */
    CLOCK_ATTRIBUTES_GET = 0x1, /* attributes of the clock */
    CLOCK_MSG_ATTRIBUTES_GET = 0x2,
    CLOCK_ATTRIBUTES = 0x3,
    CLOCK_DESCRIBE_RATES = 0x4,
    CLOCK_RATE_SET = 0x5,
    CLOCK_RATE_GET = 0x6,
    CLOCK_CONFIG_SET = 0x7,
};

struct FScmiMsgRespClockProtocolAttributes
{
    u16 num_clocks;
    u8 max_async_req;
    u8 reserved;
};

struct FScmiMsgRespClockAttributes
{
    u32 attributes;
#define CLOCK_ENABLE BIT(0)
    u8 name[FSCMI_MAX_STR_SIZE];
};

struct FScmiClockSetConfig
{
    u32 id;
    u32 attributes;
};

struct FScmiMsgClockDescribeRates
{
    u32 id;
    u32 rate_index;
};

struct FScmiMsgRespClockDescribeRates
{
    u32 num_rates_flags;
#define NUM_RETURNED(x)  ((x)&0xfff)
#define RATE_DISCRETE(x) !((x)&BIT(12))
#define NUM_REMAINING(x) ((x) >> 16)
    struct
    {
        u32 value_low;
        u32 value_high;
    } rate[0];
#define RATE_TO_U64(X)                             \
    ({                                             \
        typeof(X) x = (X);                         \
        (x).value_low | (u64)(x).value_high << 32; \
    })
};

struct FScmiClockSetRate
{
    u32 flags;
#define CLOCK_SET_ASYNC       BIT(0)
#define CLOCK_SET_IGNORE_RESP BIT(1)
#define CLOCK_SET_ROUND_UP    BIT(2)
#define CLOCK_SET_ROUND_AUTO  BIT(3)
    u32 id;
    u32 value_low;
    u32 value_high;
};

FError FScmiClockInit(FScmi *instance_p, boolean poll_completion);
struct FClockDomainInfo *FScmiClockInfoGet(FScmi *instance_p, u32 domain);
u32 FScmiClockCountGet(FScmi *instance_p);
FError FScmiClockDisable(FScmi *instance_p, u32 domain);
FError FScmiClockEnable(FScmi *instance_p, u32 domain);

#ifdef __cplusplus
}
#endif

#endif /* FSCMI_CLOCK_H */
