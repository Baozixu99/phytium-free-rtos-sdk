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
 * FilePath: fscmi_clock.c
 * Created Date: 2025-02-14 11:55:58
 * Last Modified: 2025-03-10 20:52:16
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0    Liusm       2025-02-14 11:55:58  First version
 */

#include "fscmi_clock.h"
#include <string.h>
#include "fscmi.h"
#include "fscmi_base.h"
#include "fio.h"
#include "fdrivers_port.h"
#include "ftypes.h"
#include "fswap.h"
#include "fassert.h"


#define FSCMI_CLOK_DEBUG_TAG "FSCMI_CLOK"
#define FSCMI_CLOK_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSCMI_CLOK_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_CLOK_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FSCMI_CLOK_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_CLOK_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSCMI_CLOK_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_CLOK_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSCMI_CLOK_DEBUG_TAG, format, ##__VA_ARGS__)

static struct FScmiTransferInfo *info = NULL;

static FError FScmiClockProtocolAttributesGet(FScmi *instance_p, struct FScmiClockInfo *pinfo)
{
    FError ret;
    struct FScmiMsgRespClockProtocolAttributes *attr;

    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: info is NULL,please check the initialization of the "
                         "protocol.\n",
                         __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, CLOCK_ATTRIBUTES_GET, SCMI_PROTOCOL_CLOCK, 0,
                           sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Prepare hdr packaging is error ,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    attr = (struct FScmiMsgRespClockProtocolAttributes *)info->rx.buf;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Transport package error,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    pinfo->num_clocks = attr->num_clocks;
    FSCMI_CLOK_INFO("num_clocks = %d.", pinfo->num_clocks);
    return FT_SUCCESS;
}

static FError FScmiClockAttributesGet(FScmi *instance_p, u32 domain, struct FClockDomainInfo *dom_info)
{
    FError ret;
    struct FScmiMsgRespClockAttributes *attr;

    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: info is NULL,please check the initialization of the "
                         "protocol.\n",
                         __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, CLOCK_ATTRIBUTES, SCMI_PROTOCOL_CLOCK,
                           sizeof(domain), sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Prepare hdr packaging is error ,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    *(u32 *)info->tx.buf = domain;
    attr = (struct FScmiMsgRespClockAttributes *)info->rx.buf;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Transport package error,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    strlcpy(dom_info->name, (char *)attr->name, FSCMI_MAX_STR_SIZE);
    FSCMI_CLOK_INFO("name = %s.", dom_info->name);

    return FT_SUCCESS;
}

_UNUSED static int rate_cmp_func(const void *_r1, const void *_r2)
{
    const u64 *r1 = _r1, *r2 = _r2;

    if (*r1 < *r2)
    {
        return -1;
    }
    else if (*r1 == *r2)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static FError FScmiClockDescribeRatesGet(FScmi *instance_p, u32 domain, struct FClockDomainInfo *dom_info)
{
    u64 *rate = NULL;
    FError ret;
    u32 cnt;
    boolean rate_discrete = FALSE;
    u32 tot_rate_cnt = 0, rates_flag;
    u16 num_returned, num_remaining;

    struct FScmiMsgClockDescribeRates *clk_desc;
    struct FScmiMsgRespClockDescribeRates *rlist;

    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: info is NULL,please check the initialization of the "
                         "protocol.\n",
                         __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, CLOCK_DESCRIBE_RATES, SCMI_PROTOCOL_CLOCK,
                           sizeof(*clk_desc), 0, info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Prepare hdr packaging is error ,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    clk_desc = (struct FScmiMsgClockDescribeRates *)info->tx.buf;
    rlist = (struct FScmiMsgRespClockDescribeRates *)info->rx.buf;
    do
    {
        clk_desc->id = domain;
        clk_desc->rate_index = tot_rate_cnt;

        ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
        if (ret != FT_SUCCESS)
        {
            FSCMI_CLOK_ERROR("[%s %d]: Transport package error,protocol id is %d\n",
                             __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
            return ret;
        }
        rates_flag = rlist->num_rates_flags;
        FSCMI_CLOK_INFO("rates_flag = 0x%x.", rates_flag);

        num_remaining = NUM_REMAINING(rates_flag);
        rate_discrete = RATE_DISCRETE(rates_flag);
        num_returned = NUM_RETURNED(rates_flag);

        if (tot_rate_cnt + num_returned > SCMI_MAX_NUM_RATES)
        {
            FSCMI_CLOK_INFO("[%s %d]:rates_flag:0x%x,Too many rates 0x%x, protocol id "
                            "is %d\n",
                            __func__, __LINE__, rates_flag, tot_rate_cnt + num_returned,
                            SCMI_PROTOCOL_CLOCK);
            ret = FSCMI_ERROR_RANGE;
            break;
        }

        if (!rate_discrete)
        {
            dom_info->range.min_rate = RATE_TO_U64(rlist->rate[0]);
            dom_info->range.max_rate = RATE_TO_U64(rlist->rate[1]);
            dom_info->range.step_size = RATE_TO_U64(rlist->rate[2]);
            FSCMI_CLOK_INFO("min_rate = %lld, max_rate = %lld, step_size = %lld.",
                            dom_info->range.min_rate, dom_info->range.max_rate,
                            dom_info->range.step_size);
            break;
        }

        rate = &dom_info->list.rates[tot_rate_cnt];
        for (cnt = 0; cnt < num_returned; cnt++, rate++)
        {
            *rate = RATE_TO_U64(rlist->rate[cnt]);
            FSCMI_CLOK_INFO("rate = %lld.", *rate);
        }
        tot_rate_cnt += num_returned;

    } while (num_returned && num_remaining);

    if (rate_discrete && rate)
    {
        dom_info->list.num_rates = tot_rate_cnt;
    }

    dom_info->rate_discrete = rate_discrete;

    return FT_SUCCESS;
}

FError FScmiClockRateGet(FScmi *instance_p, u32 domain, u64 *value)
{
    FError ret;

    if (instance_p->clock.num_clocks <= domain)
    {
        FSCMI_CLOK_ERROR("Invalid domain %d,please check the number of clocks or Init "
                         "the clock protocol first.\n",
                         domain);
        return FSCMI_ERROR_REQUEST;
    }

    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: info is NULL,please check the initialization of the "
                         "protocol.\n",
                         __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, CLOCK_RATE_GET, SCMI_PROTOCOL_CLOCK,
                           sizeof(domain), sizeof(u64), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Prepare hdr packaging is error ,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }
    *(u32 *)info->tx.buf = domain;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Transport package error,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    *value = *(u64 *)info->rx.buf;

    return FT_SUCCESS;
}

FError FScmiClockRateSet(FScmi *instance_p, u32 domain, u64 value)
{
    FError ret;
    u32 flag = 0; //CLOCK_SET_ASYNC
    struct FScmiClockSetRate *cfg;

    if (instance_p->clock.num_clocks <= domain)
    {
        FSCMI_CLOK_ERROR("Invalid domain %d,please check the number of clocks or Init "
                         "the clock protocol first.\n",
                         domain);
        return FSCMI_ERROR_REQUEST;
    }

    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: info is NULL,please check the initialization of the "
                         "protocol.\n",
                         __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, CLOCK_RATE_SET, SCMI_PROTOCOL_CLOCK,
                           sizeof(*cfg), 0, info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Prepare hdr packaging is error ,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    cfg = (struct FScmiClockSetRate *)info->tx.buf;
    cfg->flags = flag;
    cfg->id = domain;
    cfg->value_low = value & 0xFFFFFFFF;
    cfg->value_high = value >> 32;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("[%s %d]: Transport package error,protocol id is %d\n",
                         __func__, __LINE__, SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    return FT_SUCCESS;
}

static FError FScmiClockConfigSet(FScmi *instance_p, u32 domain, u32 config)
{
    FError ret;
    struct FScmiClockSetConfig *cfg;

    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: info is NULL,please check the initialization of the "
                         "protocol.\n",
                         __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, CLOCK_CONFIG_SET, SCMI_PROTOCOL_CLOCK,
                           sizeof(*cfg), 0, info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    cfg = (struct FScmiClockSetConfig *)info->tx.buf;
    cfg->id = domain;
    cfg->attributes = config;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    return FT_SUCCESS;
}


FError FScmiClockEnable(FScmi *instance_p, u32 domain)
{
    if (instance_p->clock.num_clocks <= domain)
    {
        FSCMI_CLOK_ERROR("Invalid domain %d,please check the number of clocks or Init "
                         "the clock protocol first.\n",
                         domain);
        return FSCMI_ERROR_REQUEST;
    }
    return FScmiClockConfigSet(instance_p, domain, CLOCK_ENABLE);
}

FError FScmiClockDisable(FScmi *instance_p, u32 domain)
{
    if (instance_p->clock.num_clocks <= domain)
    {
        FSCMI_CLOK_ERROR("Invalid domain %d,please check the number of clocks or Init "
                         "the clock protocol first.\n",
                         domain);
        return FSCMI_ERROR_REQUEST;
    }
    return FScmiClockConfigSet(instance_p, domain, 0);
}

u32 FScmiClockCountGet(FScmi *instance_p)
{
    struct FScmiClockInfo *ci = &instance_p->clock;

    return ci->num_clocks;
}

struct FClockDomainInfo *FScmiClockInfoGet(FScmi *instance_p, u32 domain)
{
    struct FScmiClockInfo *ci = &instance_p->clock;
    if (ci->num_clocks <= domain)
    {
        FSCMI_CLOK_ERROR("Invalid domain %d,please check the number of clocks or Init "
                         "the clock protocol first.\n",
                         domain);
        return NULL;
    }

    struct FClockDomainInfo *clk = ci->clock_info + domain;
    if (!clk->name[0])
    {
        return NULL;
    }

    return clk;
}

static FError FScmiClockGetVersion(FScmi *instance_p, u32 *rev_info)
{
    FError ret;

    ret = FScmiMessageInit(instance_p, CLOCK_VERSION_GET, SCMI_PROTOCOL_CLOCK, 0,
                           sizeof(*rev_info), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_CLOCK, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_CLOCK);
        return ret;
    }

    *rev_info = *(u32 *)info->rx.buf;

    return FT_SUCCESS;
}

/**
 * @name: FScmiClockInit
 * @msg: Clock protocol initialization
 * @return {FError} 返回值
 * @param {FScmi} *instance_p scmi协议实例
 * @param {boolean} poll_completion 轮询完成标志
 */
FError FScmiClockInit(FScmi *instance_p, boolean poll_completion)
{
    FError ret;
    u32 version;
    struct FScmiClockInfo *pinfo;
    pinfo = &instance_p->clock;

    info = FScmiGetInfo(instance_p, SCMI_PROTOCOL_CLOCK);
    if (info == NULL)
    {
        FSCMI_CLOK_ERROR("[%s %d]: FScmiGetInfo failed\n", __func__, __LINE__);
        return FSCMI_ERROR_NULL_POINTER;
    }
    info->poll_completion = poll_completion;

    ret = FScmiClockGetVersion(instance_p, &version);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("FScmiClockGetVersion failed\n");
        return ret;
    }
    FSCMI_CLOK_INFO("Clock protocol version 0x%x", version);

    ret = FScmiClockProtocolAttributesGet(instance_p, pinfo);
    if (ret != FT_SUCCESS)
    {
        FSCMI_CLOK_ERROR("FScmiClockProtocolAttributesGet failed\n");
        return ret;
    }
    for (u32 clk_id = 0; clk_id < pinfo->num_clocks; clk_id++)
    {
        struct FClockDomainInfo *clk = pinfo->clock_info + clk_id;

        ret = FScmiClockAttributesGet(instance_p, clk_id, clk);
        if (ret != FT_SUCCESS)
        {
            FSCMI_CLOK_ERROR("FScmiClockAttributesGet failed\n");
            return ret;
        }
        ret = FScmiClockDescribeRatesGet(instance_p, clk_id, clk);
        if (ret != FT_SUCCESS)
        {
            FSCMI_CLOK_ERROR("FScmiClockDescribeRatesGet failed\n");
            return ret;
        }
    }

    pinfo->version = version;
    instance_p->clock = *pinfo;

    return FT_SUCCESS;
}
