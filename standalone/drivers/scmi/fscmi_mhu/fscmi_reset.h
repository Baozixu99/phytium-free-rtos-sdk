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
 * FilePath: fscmi_reset.h
 * Created Date: 2025-02-14 14:05:37
 * Last Modified: 2025-02-24 19:58:51
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef FSCMI_RESET_H
#define FSCMI_RESET_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fparameters.h"
#include "fscmi.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum FScmiResetProtocolCmd
{
    RESET_VERSION_GET = 0x0,    /* version of the protocol */
    RESET_ATTRIBUTES_GET = 0x1, /* attributes of the reset domain */
    RESET_MSG_ATTRIBUTES_GET = 0x2,
    RESET_DOMAIN_ATTRIBUTES = 0x3,
    RESET = 0x4,
    RESET_NOTIFY = 0x5,
};

#define NUM_RESET_DOMAIN_MASK 0xffff
#define RESET_NOTIFY_ENABLE   BIT(0)

struct FScmiMsgRespResetDomainAttributes
{
    u32 attributes;
#define SUPPORTS_ASYNC_RESET(x)  ((x)&BIT(31))
#define SUPPORTS_NOTIFY_RESET(x) ((x)&BIT(30))
    u32 latency;
    char name[FSCMI_MAX_STR_SIZE];
};

struct FScmiMsgResetDomainReset
{
    u32 domain_id;
    u32 flags;
#define AUTONOMOUS_RESET      BIT(0)
#define EXPLICIT_RESET_ASSERT BIT(1)
#define ASYNCHRONOUS_RESET    BIT(2)
    u32 reset_state;
#define ARCH_COLD_RESET 0
};

struct FScmiMsgResetNotify
{
    u32 id;
    u32 event_control;
#define RESET_TP_NOTIFY_ALL BIT(0)
};

struct FScmiResetIssuedNotifyPayld
{
    u32 agent_id;
    u32 domain_id;
    u32 reset_state;
};

FError FScmiResetInit(FScmi *instance_p, boolean poll_completion);

#ifdef __cplusplus
}
#endif

#endif /* FSCMI_RESET_H */
