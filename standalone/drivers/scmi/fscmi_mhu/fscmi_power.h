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
 * FilePath: fscmi_power.h
 * Created Date: 2025-02-11 14:47:04
 * Last Modified: 2025-02-26 14:44:37
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef FSCMI_POWER_H
#define FSCMI_POWER_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fscmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct FScmiMsgRespPowerDomainAttributes
{
    u32 flags;
#define SUPPORTS_STATE_CHANGE_NOTIFY(x)     ((x)&BIT(31))
#define SUPPORTS_STATE_ASYN(x)              ((x)&BIT(30))
#define SUPPORTS_STATE_SYN(x)               ((x)&BIT(29))
#define SUPPORTS_STATE_CHANGE_REQ_NOTIFY(x) ((x)&BIT(28))
    char name[FSCMI_MAX_STR_SIZE];
};


struct FScmiMsgRespPowerAttributes
{
    u16 num_domains;
    u16 reserved;
    u32 stats_addr_low;
    u32 stats_addr_high;
    u32 stats_size;
}; /*the implementation details associated with this protocol.*/

#define POWER_ON  1
#define POWER_OFF 0

struct FScmiPowerSetState
{
    u32 flag;
#define POWER_FLAG_ASYNC(x) ((x)&BIT(0))
    u32 domain;
    s32 state;
#define POWER_STATE_TYPE(x) ((x)&BIT(30))
#if defined(SOC_TARGET_PD2308) || defined(SOC_TARGET_PD2408)
#define POWER_STATE_ON 0x8
#else
#define POWER_STATE_ON 0x1
#endif
#define POWER_STATE_OFF 0
#define POWER_STATE(x)  ((x)&0xfffffff)
};

/*msg_id: POWER_VERSION_GET, POWER_ATTRIBUTES_GET, POWER_MSG_ATTRIBUTES_GET, POWER_DOMAIN_ATTRIBUTES_GET, POWER_STATE_SET, POWER_STATE_GET*/
enum FScmiPowerProtocolCmd
{
    POWER_VERSION_GET = 0x0,           /* version of the protocol */
    POWER_ATTRIBUTES_GET = 0x1,        /* attributes of the power management */
    POWER_MSG_ATTRIBUTES_GET = 0x2,    /* attributes of the power management messages */
    POWER_DOMAIN_ATTRIBUTES_GET = 0x3, /* attributes of the power domains */
    POWER_STATE_SET = 0x4,             /* set the power state of a domain */
    POWER_STATE_GET = 0x5,             /* get the power state of a domain */
};

FError FScmiPowerInit(FScmi *instance_p, boolean poll_completion);

#ifdef __cplusplus
}
#endif

#endif /* FSCMI_POWER_H */
