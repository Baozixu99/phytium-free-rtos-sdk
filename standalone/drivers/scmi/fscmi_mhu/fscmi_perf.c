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
 * FilePath: fscmi_perf.c
 * Date: 2023-01-18 15:38:15
 * LastEditTime: 2023-01-18 15:38:17
 * Description:  This file is for performance domain control
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/01/31 init
 */

#include "fscmi_perf.h"
#include <string.h>
#include "fscmi.h"
#include "fscmi_base.h"
#include "fio.h"
#include "fdrivers_port.h"
#include "ftypes.h"
#include "fswap.h"
#include "fassert.h"

#define FSCMI_PERF_DEBUG_TAG "FSCMI_PERF"
#define FSCMI_PERF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSCMI_PERF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_PERF_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FSCMI_PERF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_PERF_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSCMI_PERF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_PERF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSCMI_PERF_DEBUG_TAG, format, ##__VA_ARGS__)

#define KHZ_TO_HZ 1000
#define MHZ_TO_HZ 1000000

static struct FScmiTransferInfo *info = NULL;

static FError FScmiPerfGetVersion(FScmi *instance_p, u32 *rev_info)
{
    FError ret;

    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d",
                         SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, PERF_VERSION_GET, SCMI_PROTOCOL_PERF, 0,
                           sizeof(*rev_info), info->tx.buf);

    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Prepare hdr packaging is error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_PERF, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Transport package error,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    /* Fill in the obtained parameters */
    *rev_info = FtIn32((uintptr)info->rx.buf);

    return FT_SUCCESS;
}

static FError FScmiPerfAttributesGet(FScmi *instance_p, struct FScmiPerfInfo *pinfo)
{
    FError ret;
    struct FScmiMsgRespPerfAttributes *attr;

    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d",
                         SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, PERF_ATTRIBUTES_GET, SCMI_PROTOCOL_PERF, 0,
                           sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Prepare hdr packaging is error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    attr = (struct FScmiMsgRespPerfAttributes *)info->rx.buf;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_PERF, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Transport package error,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    u16 flags = attr->flags;
    pinfo->num_domains = attr->num_domains;
    pinfo->power_scale_mw = POWER_SCALE_IN_MILLIWATT(flags);
    pinfo->stats_addr = (u32)attr->stats_addr_low | ((u64)(attr->stats_addr_high) << 32);
    pinfo->stats_size = attr->stats_size;

    return FT_SUCCESS;
}

static FError FScmiPerfDomainAttributesGet(FScmi *instance_p, u32 domain, struct FPerfDomInfo *dom_info)
{
    FError ret;
    struct FScmiMsgRespPerfDomainAttributes *attr;

    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d",
                         SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, PERF_DOMAIN_ATTRIBUTES, SCMI_PROTOCOL_PERF,
                           sizeof(domain), sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Prepare hdr packaging is error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    *(u32 *)info->tx.buf = domain;
    attr = (struct FScmiMsgRespPerfDomainAttributes *)info->rx.buf;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_PERF, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Transport package error,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    u32 flags = attr->flags;

    dom_info->set_limits = SUPPORTS_SET_LIMITS(flags);
    dom_info->set_perf = SUPPORTS_SET_PERF_LVL(flags);
    dom_info->perf_limit_notify = SUPPORTS_PERF_LIMIT_NOTIFY(flags);
    dom_info->perf_level_notify = SUPPORTS_PERF_LEVEL_NOTIFY(flags);
    dom_info->fast_channel = SUPPORTS_FAST_CHANNEL(flags);
    dom_info->sustained_freq = attr->sustained_freq;
    dom_info->sustained_perf_level = attr->sustained_perf_level;
    if (instance_p->revision.major_ver == 2 && instance_p->revision.minor_ver == 1)
    {
        dom_info->mult_factor = 1;
    }
    else
    {
        /* CPUFreq converts to kHz, hence default 1000 */
        dom_info->mult_factor = (dom_info->sustained_freq / dom_info->sustained_perf_level) * KHZ_TO_HZ;
    }
    FSCMI_PERF_INFO("mult_factor %u", dom_info->mult_factor);
    strlcpy(dom_info->name, (char *)attr->name, FSCMI_MAX_STR_SIZE);

    return FT_SUCCESS;
}

_UNUSED static int opp_cmp_func(const void *opp1, const void *opp2)
{
    const struct FScmiOpp *t1 = opp1, *t2 = opp2;

    return t1->perf_val - t2->perf_val;
}

static FError FScmiPerfDescribeLevelsGet(FScmi *instance_p, u32 domain, struct FPerfDomInfo *perf_dom)
{
    FError ret;
    u32 cnt;
    u32 tot_opp_cnt = 0;
    u16 num_returned, num_remaining;
    struct FScmiOpp *opp;
    struct FScmiMsgPerfDescribeLevels *dom_info;
    struct FScmiMsgRespPerfDescribeLevels *level_info;

    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d",
                         SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, PERF_DESCRIBE_LEVELS, SCMI_PROTOCOL_PERF,
                           sizeof(*dom_info), 0, info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Prepare hdr packaging is error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    dom_info = (struct FScmiMsgPerfDescribeLevels *)info->tx.buf;
    level_info = (struct FScmiMsgRespPerfDescribeLevels *)info->rx.buf;

    do
    {
        dom_info->domain = domain;
        /* Set the number of OPPs to be skipped/already read */
        dom_info->level_index = tot_opp_cnt;

        ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_PERF, FALSE);
        if (ret != FT_SUCCESS)
        {
            FSCMI_PERF_ERROR("Transport package error,protocol id is %d", SCMI_PROTOCOL_PERF);
            return ret;
        }

        num_returned = level_info->num_returned;
        num_remaining = level_info->num_remaining;
        if (tot_opp_cnt + num_returned > FSCMI_MAX_OPPS)
        {
            FSCMI_PERF_ERROR("No. of OPPs exceeded MAX_OPPS");
            break;
        }

        opp = &perf_dom->opp[tot_opp_cnt];
        for (cnt = 0; cnt < num_returned; cnt++, opp++)
        {
            opp->perf_val = level_info->opp[cnt].perf_val;
            opp->power = level_info->opp[cnt].power;
            opp->trans_latency_us = level_info->opp[cnt].transition_latency_us;
            if (perf_dom->mult_factor == KHZ_TO_HZ)
            {
                FSCMI_PERF_INFO("Level %d: %u KHz Power cost %d Latency %uus\n", cnt,
                                opp->perf_val, opp->power, opp->trans_latency_us);
            }
            else
            {
                FSCMI_PERF_INFO("Level %d: %u Hz Power cost %d Latency %uus\n", cnt,
                                opp->perf_val, opp->power, opp->trans_latency_us);
            }
        }

        tot_opp_cnt += num_returned;
        /*
		 * check for both returned and remaining to avoid infinite
		 * loop due to buggy firmware
		 */
    } while (num_returned && num_remaining);

    perf_dom->opp_count = tot_opp_cnt;
    //sort(perf_dom->opp, tot_opp_cnt, sizeof(*opp), opp_cmp_func, NULL);

    return FT_SUCCESS;
}

static FError FScmiPerfLevelSet(FScmi *instance_p, u32 domain, u32 level)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FASSERT(domain < instance_p->perf.num_domains);

    FError ret;
    struct FScmiPerfSetLevel *lvl;
    struct FScmiPerfInfo *pi = &instance_p->perf;
    struct FPerfDomInfo *dom = pi->dom_info + domain;

    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d",
                         SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, PERF_LEVEL_SET, SCMI_PROTOCOL_PERF, sizeof(*lvl),
                           0, info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Prepare hdr packaging is error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    lvl = (struct FScmiPerfSetLevel *)info->tx.buf;
    lvl->domain = domain;
    lvl->level = level;
    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_PERF, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Transport package error,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }
    if (dom->mult_factor == KHZ_TO_HZ)
    {
        FSCMI_PERF_INFO("FScmiPerfLevelSet domain:%u level:%u KHz.", lvl->domain, lvl->level);
    }
    else
    {
        FSCMI_PERF_INFO("FScmiPerfLevelSet domain:%u level:%u Hz.", lvl->domain, lvl->level);
    }
    return FT_SUCCESS;
}

static FError FScmiPerfLevelGet(FScmi *instance_p, u32 domain, u32 *level)
{
    FASSERT(instance_p != NULL);
    FASSERT(level != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FASSERT(domain < instance_p->perf.num_domains);
    struct FScmiPerfInfo *pi = &instance_p->perf;
    struct FPerfDomInfo *dom = pi->dom_info + domain;

    FError ret;

    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d",
                         SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, PERF_LEVEL_GET, SCMI_PROTOCOL_PERF, sizeof(u32),
                           sizeof(u32), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Prepare hdr packaging is error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    *(u32 *)info->tx.buf = domain;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_PERF, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Transport package error,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }

    *level = *(u32 *)info->rx.buf;
    if (dom->mult_factor == KHZ_TO_HZ)
    {
        FSCMI_PERF_INFO("FScmiPerfLevelGet domain:%d level:%u KHz.", domain, *level);
    }
    else
    {
        FSCMI_PERF_INFO("FScmiPerfLevelGet domain:%d level:%u Hz.", domain, *level);
    }
    return FT_SUCCESS;
}

/**
 * @name: FScmiDvfsFreqSet
 * @msg: Dynamic frequency scaling set
 * @return {FError}             返回值
 * @param {FScmi} *instance_p   scmi协议实例
 * @param {u32} domain          性能域id
 * @param {u64} freq            频率值Hz,依据初始化获得的level对应的值写入，不支持任意调频
 */
FError FScmiDvfsFreqSet(FScmi *instance_p, u32 domain, u64 freq_Hz)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FASSERT(domain < instance_p->perf.num_domains);

    FError ret;
    struct FScmiPerfInfo *pi = &instance_p->perf;
    struct FPerfDomInfo *dom = pi->dom_info + domain;

    ret = FScmiPerfLevelSet(instance_p, domain, freq_Hz / dom->mult_factor);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("FScmiPerfLevelSet error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }
    return FT_SUCCESS;
}

/**
 * @name: FScmiDvfsFreqGet
 * @msg: Dynamic frequency scaling get
 * @return {FError}             返回值
 * @param {FScmi} *instance_p   scmi协议实例
 * @param {u32} domain          性能域id
 * @param {u64} *freq           当前性能域的频率值
 */
FError FScmiDvfsFreqGet(FScmi *instance_p, u32 domain, u64 *freq)
{
    FASSERT(instance_p != NULL);
    FASSERT(freq != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FASSERT(domain < instance_p->perf.num_domains);

    FError ret;
    u32 level;
    struct FScmiPerfInfo *pi = &instance_p->perf;
    struct FPerfDomInfo *dom = pi->dom_info + domain;

    ret = FScmiPerfLevelGet(instance_p, domain, &level);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("FScmiPerfLevelGet error ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return ret;
    }
    *freq = level * dom->mult_factor;
    return FT_SUCCESS;
}

u64 FScmiPerfGetOppFreq(FScmi *instance_p, u32 domain, u32 opp_num)
{
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    FASSERT(domain < instance_p->perf.num_domains);

    struct FPerfDomInfo *dom = &instance_p->perf.dom_info[domain];

    if (opp_num > dom->opp_count)
    {
        FSCMI_PERF_ERROR("Can't find opp_num,please check.");
        return FSCMI_ERROR_REQUEST;
    }

    return dom->opp[opp_num].perf_val * dom->mult_factor;
}

/**
 * @name: FScmiPerfInit
 * @msg: Performance domain initialization
 * @return {FError} 返回值
 * @param {FScmi} *instance_p scmi协议实例
 * @param {boolean} poll_completion 轮询完成标志
 */
FError FScmiPerfInit(FScmi *instance_p, boolean poll_completion)
{
    FError ret;
    struct FScmiPerfInfo *pinfo;

    info = FScmiGetInfo(instance_p, SCMI_PROTOCOL_PERF);
    if (info == NULL)
    {
        FSCMI_PERF_ERROR("Info data structure not found ,protocol id is %d", SCMI_PROTOCOL_PERF);
        return FSCMI_ERROR_NULL_POINTER;
    }
    info->poll_completion = poll_completion;
    pinfo = &instance_p->perf;

    ret = FScmiPerfGetVersion(instance_p, &instance_p->perf.version);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Can't get version,please check mem_address or chan_id.Error "
                         "code:0x%x.",
                         ret);
        return FSCMI_ERROR_REQUEST;
    }
    instance_p->perf.major_ver = (instance_p->perf.version >> 16);
    instance_p->perf.minor_ver = (instance_p->perf.version & 0xffff);
    FSCMI_PERF_INFO("Perf Version %d.%d\n", instance_p->perf.major_ver, instance_p->perf.minor_ver);

    ret = FScmiPerfAttributesGet(instance_p, pinfo);
    if (ret != FT_SUCCESS)
    {
        FSCMI_PERF_ERROR("Can't get version,please check mem_address or chan_id.Error "
                         "code:0x%x.",
                         ret);
        return FSCMI_ERROR_REQUEST;
    }

    FSCMI_PERF_INFO("SCMI Perf "
                    "num_domains:%d,power_scale_mw:%d,stats_addr:0x%x,stats_size:0x%x."
                    "\n",
                    instance_p->perf.num_domains, instance_p->perf.power_scale_mw,
                    instance_p->perf.stats_addr, instance_p->perf.stats_size);
    for (u32 i = 0; i < pinfo->num_domains; i++)
    {
        struct FPerfDomInfo *dom = pinfo->dom_info + i;

        ret = FScmiPerfDomainAttributesGet(instance_p, i, dom);
        if (ret != FT_SUCCESS)
        {
            FSCMI_PERF_ERROR("Can't get version,please check mem_address or "
                             "chan_id.Error code:0x%x.",
                             ret);
            return FSCMI_ERROR_REQUEST;
        }

        ret = FScmiPerfDescribeLevelsGet(instance_p, i, dom);
        if (ret != FT_SUCCESS)
        {
            FSCMI_PERF_ERROR("Can't get version,please check mem_address or "
                             "chan_id.Error code:0x%x.",
                             ret);
            return FSCMI_ERROR_REQUEST;
        }
        if (dom->mult_factor == KHZ_TO_HZ)
        {
            FSCMI_PERF_INFO("SCMI Perf opp_count:%d,sustained_freq:%u "
                            "KHz,sustained_perf_level:%u KHz,mult_factor:%d "
                            "Hz,name:%s.\n",
                            dom->opp_count, dom->sustained_freq,
                            dom->sustained_perf_level, dom->mult_factor, dom->name);
        }
        else
        {
            FSCMI_PERF_INFO("SCMI Perf opp_count:%d,sustained_freq:%u "
                            "Hz,sustained_perf_level:%u Hz,mult_factor:%d "
                            "Hz,name:%s.\n",
                            dom->opp_count, dom->sustained_freq,
                            dom->sustained_perf_level, dom->mult_factor, dom->name);
        }
    }

    return FT_SUCCESS;
}
