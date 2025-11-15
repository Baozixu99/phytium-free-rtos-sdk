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
 * FilePath: fscmi.h
 * scmi.h
 * Date: 2022-12-30 18:32:33
 * LastEditTime: 2022-12-30 18:32:33
 * Description:  This file is for scmi protocol struct and application interface
 * 
 * Modify History: 
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0 liushengming 2023/01/31 init
 */


#ifndef FSCMI_H
#define FSCMI_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fmhu.h"
#include "fparameters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* error code */

#define FSCMI_ERROR_TYPE              FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x1u)
#define FSCMI_ERROR_RANGE             FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x2u)
#define FSCMI_ERROR_NOT_FOUND         FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x3u)
#define FSCMI_ERROR_NULL_POINTER      FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x4u)
#define FSCMI_ERROR_WAIT_MBOX_TIMEOUT FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x5u)
#define FSCMI_ERROR_WAIT_MEM_TIMEOUT  FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x6u)
#define FSCMI_ERROR_FETCH_RESPONSE    FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x7u)
#define FSCMI_ERROR_REQUEST           FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x8u)
#define FSCMI_ERROR_VERSION           FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0x9u)
#define FSCMI_ERROR_INIT              FT_CODE_ERR(ErrModBsp, ErrBspScmi, 0xau)

#define FSCMI_MBOX_MHU_TYPE           0

#define FSCMI_VERSION1                0x20001
#define FSCMI_VERSION0                0x20000
#define FSCMI_VERSION01               0x10000
/*
 * SCMI specification requires all parameters, message headers, return
 * arguments or any protocol data to be expressed in little endian
 * format only.
 */
struct FScmiSharedMem
{
    u32 reserved;
    u32 channel_status;
#define SCMI_SHMEM_CHAN_STAT_CHANNEL_ERROR BIT(1)
#define SCMI_SHMEM_CHAN_STAT_CHANNEL_FREE  BIT(0)
    u32 reserved1[2];
    u32 flags;
#define SCMI_SHMEM_FLAG_INTR_ENABLED BIT(0)
    u32 length;
    u32 msg_header;
    u8 msg_payload[0];
};

enum FScmiStdProtocol
{
    SCMI_PROTOCOL_BASE = 0x10,
    SCMI_PROTOCOL_POWER = 0x11,
    SCMI_PROTOCOL_SYSTEM = 0x12,
    SCMI_PROTOCOL_PERF = 0x13,
    SCMI_PROTOCOL_CLOCK = 0x14,
    SCMI_PROTOCOL_SENSOR = 0x15,
    SCMI_PROTOCOL_RESET = 0x16,
};

enum FScmiProtocolIndex
{
    BASE_PROTOCOL_INDEX,
    PERF_DOMAIN_INDEX,
    SENSOR_MANAGEMENT_INDEX,
    POWER_DOMAIN_INDEX,
    CLOCK_DOMAIN_INDEX,
    RESET_DOMAIN_INDEX,
    FSCMI_SUPPORT_PROTOCOL_NUM,
};

/* base protocol */
struct FScmiRevisionInfo
{
    u32 version;      /* version of the SCMI specification */
    u16 major_ver;    /* major version of the SCMI specification */
    u16 minor_ver;    /* minor version of the SCMI specification */
    u8 num_protocols; /* number of protocols implemented */
    u8 num_agents;    /* number of agents implemented */
    u32 impl_ver;
    char vendor_id[FSCMI_MAX_STR_SIZE];
    char sub_vendor_id[FSCMI_MAX_STR_SIZE];
};

/* Sensor protocol  */
struct FScmiSensorInfo
{
    u32 id;
    u8 type;
    char name[FSCMI_MAX_STR_SIZE];
};

struct FScmiSensorsInfo
{
    u32 version;      /* version of the SCMI sensor specification */
    u16 major_ver;    /* major version of the SCMI sensor specification */
    u16 minor_ver;    /* minor version of the SCMI sensor specification */
    u32 num_sensors;  /* number of sensors implemented */
    u32 max_requests; /* maximum number of requests supported */
    u64 reg_addr;     /* address of the sensor register */
    u32 reg_size;     /* size of the sensor register */
    struct FScmiSensorInfo sensor_info[FSCMI_MAX_NUM_SENSOR]; /* TS0 TS1 */
};

/* Performance domain protocol */

struct FScmiOpp
{
    u32 perf_val;
    u32 power;
    u32 trans_latency_us;
};

struct FPerfDomInfo
{
    boolean set_limits;
    boolean set_perf;
    boolean perf_limit_notify;
    boolean perf_level_notify;
    boolean fast_channel;
    u32 opp_count;
    u32 sustained_freq;
    u32 sustained_perf_level;
    u32 mult_factor;
    char name[FSCMI_MAX_STR_SIZE];
    struct FScmiOpp opp[FSCMI_MAX_OPPS];
};

struct FScmiPerfInfo
{
    u32 version;
    u16 major_ver;
    u16 minor_ver;
    u32 num_domains;
    boolean power_scale_mw;
    u64 stats_addr;
    u32 stats_size;
    struct FPerfDomInfo dom_info[FSCMI_MAX_PERF_DOMAINS];
};

/* Power domain protocol */
struct FPowerDomInfo
{
    boolean state_change_notify;     /*Power state change notifications support.*/
    boolean state_asyn;              /*Power state asynchronous support.*/
    boolean state_syn;               /*Power state synchronous support.*/
    boolean state_change_req_notify; /*Power state change requested notifications support.*/
    char name[FSCMI_MAX_STR_SIZE];
};

struct FScmiPowerInfo
{
    u32 version;     /* version of the SCMI power specification */
    u16 major_ver;   /* major version of the SCMI power specification */
    u16 minor_ver;   /* minor version of the SCMI power specification */
    u32 num_domains; /* Number of domains for which statistics are collected */
    u64 stats_addr;  /* Address of the power domain statistics */
    u32 stats_size;  /* The length in bytes of the statistics shared memory region. */
    struct FPowerDomInfo dom_info[FSCMI_MAX_POWER_DOMAINS];
};

/* Clock domain protocol */
struct FClockDomainInfo
{
    char name[FSCMI_MAX_STR_SIZE];
    boolean rate_discrete;
    union
    {
        struct
        {
            int num_rates;
            u64 rates[FSCMI_MAX_CLOCK_RATES];
        } list;
        struct
        {
            u64 min_rate;
            u64 max_rate;
            u64 step_size;
        } range;
    };
};

struct FScmiClockInfo
{
    u32 version;    /* version of the SCMI clock specification */
    u32 num_clocks; /* number of clocks implemented */
    struct FClockDomainInfo clock_info[FSCMI_MAX_CLOCK_DOMAINS];
};

/* Reset domain protocol */
struct FResetDomainInfo
{
    boolean async_reset;
    boolean reset_notify;
    u32 latency_us;
    char name[FSCMI_MAX_STR_SIZE];
};

struct FScmiResetInfo
{
    u32 version;     /* version of the SCMI reset specification */
    u32 num_domains; /* number of reset domains implemented */
    struct FResetDomainInfo dom_info[FSCMI_MAX_RESET_DOMAINS];
};

/* Scmi massage package */
struct FScmiMsgHdr
{
    u8 id;          /* message id   */
    u8 protocol_id; /* protocol id */
    u16 seq;        /* message token */
    u32 status;     /* Status of the transfer once it's complete */
};

struct FScmiMsg
{
    u8 buf[FSCMI_MSG_SIZE]; /* buffer in normal memory */
    u32 len;                /* buffer length */
};

struct FScmiTransferInfo
{
    struct FScmiMsgHdr hdr; /* Message(Tx/Rx) header */
    struct FScmiMsg tx;
    struct FScmiMsg rx;
    boolean poll_completion;
};

/* Scmi protocol struct */
struct FScmiConfig
{
    uintptr share_mem; /* Chan transport protocol shared memory */
    u32 mbox_type;     /* select mbox driver */
};

typedef struct
{
    u32 is_ready; /* Device is ininitialized and ready*/
    struct FScmiConfig config;
    struct FScmiRevisionInfo revision; /* Base protocol */
    struct FScmiSensorsInfo sensors;   /* Sensor protocol */
    struct FScmiPerfInfo perf;         /* Performance domain protocol */
    struct FScmiPowerInfo power;       /* Power domain protocol */
    struct FScmiClockInfo clock;       /* Clock domain protocol */
    struct FScmiResetInfo reset;       /* Reset domain protocol */
    struct FScmiTransferInfo info[FSCMI_SUPPORT_PROTOCOL_NUM];
    u8 protocols_imp[FSCMI_MAX_PROTOCOLS_IMP]; /* List of protocols implemented, currently maximum of FSCMI_MAX_PROTOCOLS_IMP elements allocated by the base protocol */
    FScmiMhu scmi_mhu;
} FScmi;

/* Base protocol function */
FError FScmiCfgInitialize(FScmi *instance_p, const struct FScmiConfig *config, boolean poll_completion);
FError FScmiMessageInit(FScmi *instance_p, u8 msg_id, u8 pro_id, u32 tx_size,
                        u32 rx_size, u8 *tx_buffer);
FError FScmiProtocolTxPrepare(FScmi *instance_p, u8 pro_id);
FError FScmiProtocolPollDone(FScmi *instance_p, u8 pro_id);
FError FScmiFetchResponse(FScmi *instance_p, u8 pro_id);
struct FScmiTransferInfo *FScmiGetInfo(FScmi *instance_p, u8 pro_id);
FError FScmiDoTransport(FScmi *instance_p, struct FScmiTransferInfo *info,
                        u32 protocol_index, boolean secu_flag);
/* Sensor protocol function */
FError FScmiSensorGetTemp(FScmi *instance_p, u32 sensor_id, s64 *temp);
/* Performance domain function */
FError FScmiDvfsFreqSet(FScmi *instance_p, u32 domain, u64 freq);
FError FScmiDvfsFreqGet(FScmi *instance_p, u32 domain, u64 *freq);
u64 FScmiPerfGetOppFreq(FScmi *instance_p, u32 domain, u32 opp_num);
/* Power domain function */
FError FPowerInit(FScmi *instance_p);
FError FScmiPowerStateSet(FScmi *instance_p, u32 asyn, u32 domain, u32 state);
FError FScmiPowerStateGet(FScmi *instance_p, u32 domain, u32 *state);
FError FPowerSetByName(FScmi *instance_p, const char *name, u32 state);
/* Clock domain function */
FError FScmiClockRateSet(FScmi *instance_p, u32 domain, u64 value);
FError FScmiClockRateGet(FScmi *instance_p, u32 domain, u64 *value);
/* Reset domain function */
FError FScmiResetReset(FScmi *instance_p, u32 domain);
FError FScmiResetAssert(FScmi *instance_p, u32 domain);
FError FScmiResetDeassert(FScmi *instance_p, u32 domain);

#ifdef __cplusplus
}
#endif


#endif
