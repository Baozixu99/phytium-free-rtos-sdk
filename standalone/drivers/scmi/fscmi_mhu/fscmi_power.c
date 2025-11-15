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
 * FilePath: fscmi_power.c
 * Created Date: 2025-02-11 14:48:58
 * Last Modified: 2025-02-26 15:01:17
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0    Liusm      2025-02-11 14:48:58  Create file
 */

#include <string.h>
#include "fio.h"
#include "fdrivers_port.h"
#include "ftypes.h"
#include "fswap.h"
#include "fassert.h"
#include "fparameters.h"
#include "fscmi.h"
#include "fscmi_base.h"
#include "fscmi_power.h"

#define FSCMI_POWE_DEBUG_TAG "FSCMI_POWE"
#define FSCMI_POWE_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSCMI_POWE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_POWE_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FSCMI_POWE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_POWE_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSCMI_POWE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_POWE_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSCMI_POWE_DEBUG_TAG, format, ##__VA_ARGS__)

#define DOMAIN_COUNT            (sizeof(domain_names) / sizeof(domain_names[0]))
#define INVALID_POWER_DOMAIN_ID (-1)

static struct FScmiTransferInfo *info;
static const char *domain_names[] = {
    "core0",     "core1",      "core2",      "core3",      "core4",      "core5",
    "core6",     "core7",      "dmu0",       "dmu1",       "smmu",       "sce",
    "peu_x8_0",  "peu_x4_0",   "peu_x4_1",   "peu_x1_0",   "peu_x1_1",   "peu_x1_2",
    "peu_x16",   "peu_x8_1",   "peu_rcep_0", "peu_rcep_1", "peu_rcep_2", "peu_rcep_3",
    "peu_sata",  "peu_all",    "tdbg",       "noc",        "mix",        "peu_wp",
    "gsu",       "gsu_sata",   "gsu_mac0",   "mac1",       "fdbg",       "lsd",
    "lsd_i2s",   "lsd_wdt0",   "lsd_wdt1",   "dmac",       "int",        "ocm",
    "dbg_intf",  "ras_wp",     "its",        "nest0",      "nest1",      "pgu_c0",
    "pgu_c1",    "pgu_c2",     "psd_c3",     "psd_c4",     "psd_c5",     "pgu_gmac",
    "pgu_usb",   "usb3_0",     "usb3_1",     "usb3_2",     "usb3_3",     "usb2_0",
    "usb2_1",    "npu",        "gpu_3d",     "gpu_3d_c0",  "gpu_3d_c1",  "gpu_3d_c2",
    "gpu_3d_c3", "psd_sata",   "dc_dp0",     "dc_dp1",     "dc_dp2",     "sce_virt1",
    "sce_virt2", "sce_ckstop", "vpu_en",     "vpu_de",     "clust0",     "clust1",
    "clust2",    "gsd",        "peu",        "psu",        "vpu",        "dmu",
    "ocn"};

typedef struct
{
    boolean state_asyn;            /* Power state asynchronous support. */
    char name[FSCMI_MAX_STR_SIZE]; /* 电源域名称*/
    s32 id;                        /* 电源域ID*/
} FPowerDomainEntry;

/* 查找函数（线性搜索）*/
static int FindDomain(const char *name)
{
    if (name == NULL)
    {
        return -1;
    }

    for (size_t i = 0; i < DOMAIN_COUNT; i++)
    {
        if (strcmp(name, domain_names[i]) == 0)
        {
            return 0;
        }
    }

    return -1; // Not found
}


static FPowerDomainEntry power_domain_info[FSCMI_MAX_POWER_DOMAINS] = {0}; /*记录扫描出来的电源域信息*/

static FError FScmiPowerGetVersion(FScmi *instance_p, u32 *rev_info)
{
    FError ret;

    if (info == NULL)
    {
        FSCMI_POWE_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d\n",
                         SCMI_PROTOCOL_POWER);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, POWER_VERSION_GET, SCMI_PROTOCOL_POWER, 0,
                           sizeof(*rev_info), info->tx.buf);

    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_POWER, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    /* Fill in the obtained parameters */
    *rev_info = *(u32 *)info->rx.buf;

    return FT_SUCCESS;
}

static FError FScmiPowerAttributesGet(FScmi *instance_p, struct FScmiPowerInfo *pinfo)
{
    FError ret;
    struct FScmiMsgRespPowerAttributes *attr;

    if (info == NULL)
    {
        FSCMI_POWE_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d\n",
                         SCMI_PROTOCOL_POWER);
        return FSCMI_ERROR_NULL_POINTER;
    }
    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, POWER_ATTRIBUTES_GET, SCMI_PROTOCOL_POWER, 0,
                           sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    attr = (struct FScmiMsgRespPowerAttributes *)info->rx.buf;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_POWER, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    pinfo->num_domains = attr->num_domains;
    pinfo->stats_addr = (u32)attr->stats_addr_low | ((u64)(attr->stats_addr_high) << 32);
    pinfo->stats_size = attr->stats_size;

    return FT_SUCCESS;
}

static FError FScmiPowerDomainAttributesGet(FScmi *instance_p, u32 domain, struct FPowerDomInfo *dom_info)
{
    FError ret;
    struct FScmiMsgRespPowerDomainAttributes *attr;

    if (info == NULL)
    {
        FSCMI_POWE_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d\n",
                         SCMI_PROTOCOL_POWER);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, POWER_DOMAIN_ATTRIBUTES_GET, SCMI_PROTOCOL_POWER,
                           sizeof(domain), sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    *(u32 *)info->tx.buf = domain;
    attr = (struct FScmiMsgRespPowerDomainAttributes *)info->rx.buf;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_POWER, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    u32 flags = attr->flags;

    if (SUPPORTS_STATE_CHANGE_NOTIFY(flags))
    {
        dom_info->state_change_notify = TRUE;
    }
    else
    {
        dom_info->state_change_notify = FALSE;
    }

    if (SUPPORTS_STATE_ASYN(flags))
    {
        dom_info->state_asyn = TRUE;
    }
    else
    {
        dom_info->state_asyn = FALSE;
    }
    if (SUPPORTS_STATE_SYN(flags))
    {
        dom_info->state_syn = TRUE;
    }
    else
    {
        dom_info->state_syn = FALSE;
    }
    if (SUPPORTS_STATE_CHANGE_REQ_NOTIFY(flags))
    {
        dom_info->state_change_req_notify = TRUE;
    }
    else
    {
        dom_info->state_change_req_notify = FALSE;
    }
    FSCMI_POWE_INFO("[n,a,s,c]===> [%d, %d, %d, %d] \n", dom_info->state_change_notify,
                    dom_info->state_asyn, dom_info->state_syn, dom_info->state_change_req_notify);

    strlcpy(dom_info->name, attr->name, FSCMI_MAX_STR_SIZE);

    return FT_SUCCESS;
}


FError FScmiPowerStateSet(FScmi *instance_p, u32 asyn, u32 domain, u32 state)
{
    FError ret;
    struct FScmiPowerSetState *setState;

    if (info == NULL)
    {
        FSCMI_POWE_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d\n",
                         SCMI_PROTOCOL_POWER);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, POWER_STATE_SET, SCMI_PROTOCOL_POWER,
                           sizeof(*setState), 0, info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    setState = (struct FScmiPowerSetState *)info->tx.buf;

    setState->flag = POWER_FLAG_ASYNC(asyn);
    setState->domain = domain;
    if (state == POWER_ON)
    {
        setState->state = BIT(30) | POWER_STATE(POWER_STATE_ON);
    }
    else
    {
        setState->state = BIT(30) | POWER_STATE(POWER_STATE_OFF);
    }

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_POWER, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    FSCMI_POWE_INFO("FScmiPowerStateSet flag:%d, domain:%d power state:%x .\n",
                    setState->flag, setState->domain, setState->state);
    return FT_SUCCESS;
}

FError FScmiPowerStateGet(FScmi *instance_p, u32 domain, u32 *state)
{
    FError ret;
    u32 powerState;

    if (info == NULL)
    {
        FSCMI_POWE_ERROR("Info data structure not found,please check initialization, "
                         "protocol id is %d\n",
                         SCMI_PROTOCOL_POWER);
        return FSCMI_ERROR_NULL_POINTER;
    }

    /* Prepare hdr packaging */
    ret = FScmiMessageInit(instance_p, POWER_STATE_GET, SCMI_PROTOCOL_POWER,
                           sizeof(u32), sizeof(u32), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    *(u32 *)info->tx.buf = domain;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_POWER, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return ret;
    }

    powerState = *(u32 *)info->rx.buf;

    // if(POWER_STATE(powerState) == POWER_STATE_ON)
    // 	*state = POWER_ON;
    // else
    // 	*state = POWER_OFF;
    *state = POWER_STATE(powerState);
    FSCMI_POWE_INFO("FScmiPowerStateGet domain:%d, powerState:0x%x.\n", domain, powerState);
    return FT_SUCCESS;
}

/**
 * @name: FScmiPowerInit
 * @msg: Power protocol initialization
 * @return {FError}             返回值
 * @param {FScmi} *instance_p   scmi协议实例
 * @param {boolean} poll_completion  是否轮询完成
 */
FError FScmiPowerInit(FScmi *instance_p, boolean poll_completion)
{
    FError ret;
    struct FScmiPowerInfo *pinfo;
    pinfo = &instance_p->power;
    u32 i = 0;

    info = FScmiGetInfo(instance_p, SCMI_PROTOCOL_POWER);
    if (info == NULL)
    {
        FSCMI_POWE_ERROR("Info data structure not found ,protocol id is %d\n", SCMI_PROTOCOL_POWER);
        return FSCMI_ERROR_NULL_POINTER;
    }
    info->poll_completion = poll_completion;

    ret = FScmiPowerGetVersion(instance_p, &instance_p->power.version);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Can't get version,please check mem_address or chan_id.Error "
                         "code:0x%x.\n",
                         ret);
        return FSCMI_ERROR_REQUEST;
    }
    instance_p->power.major_ver = (instance_p->power.version >> 16);
    instance_p->power.minor_ver = ((instance_p->power.version & 0xffff) >> 12);
    FSCMI_POWE_INFO("Power Version %d.%d\n", instance_p->power.major_ver,
                    instance_p->power.minor_ver);

    ret = FScmiPowerAttributesGet(instance_p, pinfo);
    if (ret != FT_SUCCESS)
    {
        FSCMI_POWE_ERROR("Can't get version,please check mem_address or chan_id.Error "
                         "code:0x%x.\n",
                         ret);
        return FSCMI_ERROR_REQUEST;
    }

    FSCMI_POWE_INFO("Number of power domains :%d.\n", instance_p->power.num_domains);
    for (i = 0; i < pinfo->num_domains; i++)
    {
        struct FPowerDomInfo *dom = pinfo->dom_info + i;

        ret = FScmiPowerDomainAttributesGet(instance_p, i, dom);
        if (ret != FT_SUCCESS)
        {
            FSCMI_POWE_ERROR("Can't get version,please check mem_address or "
                             "chan_id.Error code:0x%x.\n",
                             ret);
            return FSCMI_ERROR_REQUEST;
        }

        FSCMI_POWE_INFO("SCMI power domain %d    name:%s\n", i, dom->name);
    }

    return FT_SUCCESS;
}

/**
 * @brief Power domain initialization for standalone usage
 * @param instance_p Protocol instance
 * @return FT_SUCCESS: success, FT_FAILURE: failure
*/
FError FPowerInit(FScmi *instance_p)
{
    FError ret;
    u32 i;

    instance_p->config.mbox_type = FSCMI_MBOX_MHU_TYPE;
    instance_p->config.share_mem = FSCMI_SHR_MEM_ADDR;

    FMhuGetDefConfig(&instance_p->scmi_mhu.mhu.config, 0U);

    FMhuChanProbe(&instance_p->scmi_mhu);

    ret = FScmiPowerInit(instance_p, TRUE);
    if (ret != FT_SUCCESS)
    {
        printf("FScmiPowerInit init error.Error code:0x%x.", ret);
        return FSCMI_ERROR_INIT;
    }

    /* Finally confirm the completion status */
    instance_p->is_ready = FT_COMPONENT_IS_READY;

    printf("Power Version:%d.%d\r\n", instance_p->power.major_ver, instance_p->power.minor_ver);
    printf("Power domains_num:%d.\r\n", instance_p->power.num_domains);

    for (i = 0; i < instance_p->power.num_domains; i++)
    {
        if (FindDomain(instance_p->power.dom_info[i].name) == 0)
        {
            power_domain_info[i].state_asyn = instance_p->power.dom_info[i].state_asyn;
            strncpy(power_domain_info[i].name, instance_p->power.dom_info[i].name, FSCMI_MAX_STR_SIZE);
            power_domain_info[i].id = i;
        }
        else
        {
            power_domain_info[i].id = INVALID_POWER_DOMAIN_ID; /*标记无法配置项目*/
            *power_domain_info[i].name = '\0';
        }
    }
    printf("Power domains you can set:\r\n");
    for (i = 0; i < instance_p->power.num_domains; i++)
    {
        if (power_domain_info[i].id != INVALID_POWER_DOMAIN_ID)
        {
            printf("Power domain_id:%d,name:%s,state_asyn:%d.\r\n", i,
                   power_domain_info[i].name, power_domain_info[i].state_asyn);
        }
    }

    return FT_SUCCESS;
}

/**
 * @brief 设置电源域状态通过名称
 * @param name 电源域名称
 * @return 0：成功，-1：失败
 */
FError FPowerSetByName(FScmi *instance_p, const char *name, u32 state)
{
    FError ret;
    u32 power_state;

    for (u32 i = 0; i < FSCMI_MAX_POWER_DOMAINS; i++)
    {
        if (strcmp(name, power_domain_info[i].name) == 0)
        {
            ret = FScmiPowerStateSet(instance_p, power_domain_info[i].state_asyn,
                                     power_domain_info[i].id, state);
            if (ret != FT_SUCCESS)
            {
                return FSCMI_ERROR_REQUEST;
            }
            ret = FScmiPowerStateGet(instance_p, i, &power_state);
            if (ret != FT_SUCCESS)
            {
                return FSCMI_ERROR_REQUEST;
            }
            printf("SCMI Power domain_id:%d,name:%s.\n", i,
                   instance_p->power.dom_info[i].name);
            printf("PowerStateGet power_state:0x%x.\r\n", power_state);
        }
    }
    return FT_SUCCESS;
}
