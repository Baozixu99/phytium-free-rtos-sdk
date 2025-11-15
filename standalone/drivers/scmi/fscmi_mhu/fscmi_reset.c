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
 * FilePath: fscmi_reset.c
 * Created Date: 2025-02-14 14:05:17
 * Last Modified: 2025-02-24 20:24:05
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0    Liusm       2025-02-14 14:05:17  first version
 */

#include "fscmi_reset.h"
#include <string.h>
#include "fscmi.h"
#include "fscmi_base.h"
#include "fio.h"
#include "fdrivers_port.h"
#include "ftypes.h"
#include "fswap.h"
#include "fassert.h"

#define FSCMI_REST_DEBUG_TAG "FSCMI_REST"
#define FSCMI_REST_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSCMI_REST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_REST_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FSCMI_REST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_REST_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSCMI_REST_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSCMI_REST_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSCMI_REST_DEBUG_TAG, format, ##__VA_ARGS__)

static struct FScmiTransferInfo *info;
/**
 * name: FScmiResetGetVersion
 * description: get reset protocol version
 * param[in] instance_p: FScmi instance pointer
 * param[out] rev_info: reset protocol version
*/
static FError FScmiResetGetVersion(FScmi *instance_p, u32 *rev_info)
{
    FError ret;

    if (info == NULL)
    {
        FSCMI_REST_ERROR("Info data structure not found,please check "
                         "initialization,protocol id is %d.\n",
                         SCMI_PROTOCOL_RESET);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, RESET_VERSION_GET, SCMI_PROTOCOL_RESET, 0,
                           sizeof(*rev_info), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_RESET, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    *rev_info = *(u32 *)info->rx.buf;

    return FT_SUCCESS;
}

static FError FScmiResetAttributesGet(FScmi *instance_p, struct FScmiResetInfo *pinfo)
{
    FError ret;

    if (info == NULL)
    {
        FSCMI_REST_ERROR("Info data structure not found,please check "
                         "initialization,protocol id is %d.\n",
                         SCMI_PROTOCOL_RESET);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, RESET_VERSION_GET, SCMI_PROTOCOL_RESET, 0,
                           sizeof(u32), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_RESET, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }
    pinfo->num_domains = (*(u32 *)info->rx.buf) & NUM_RESET_DOMAIN_MASK;

    return FT_SUCCESS;
}

static FError FScmiResetDomainAttributesGet(FScmi *instance_p, u32 domain,
                                            struct FResetDomainInfo *pdom_info)
{
    FError ret;
    struct FScmiMsgRespResetDomainAttributes *attr;

    if (info == NULL)
    {
        FSCMI_REST_ERROR("Info data structure not found,please check "
                         "initialization,protocol id is %d.\n",
                         SCMI_PROTOCOL_RESET);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, RESET_DOMAIN_ATTRIBUTES, SCMI_PROTOCOL_RESET,
                           sizeof(domain), sizeof(*attr), info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    *(u32 *)info->tx.buf = domain;
    attr = (struct FScmiMsgRespResetDomainAttributes *)(info->rx.buf);

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_RESET, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    u32 attribute = attr->attributes;

    pdom_info->async_reset = SUPPORTS_ASYNC_RESET(attribute);
    pdom_info->reset_notify = SUPPORTS_NOTIFY_RESET(attribute);
    pdom_info->latency_us = attr->latency;
    if (pdom_info->latency_us == BIT_MASK(32))
    {
        pdom_info->latency_us = 0;
    }
    strlcpy(pdom_info->name, attr->name, FSCMI_MAX_STR_SIZE);

    return FT_SUCCESS;
}

inline u32 FScmiResetGetNumDomainsGet(FScmi *instance_p)
{
    return instance_p->reset.num_domains;
}

char *FScmiResetGetDomainName(FScmi *instance_p, u32 domain)
{
    if (domain >= instance_p->reset.num_domains)
    {
        return NULL;
    }
    return instance_p->reset.dom_info[domain].name;
}

u32 FScmiResetGetDomainLatency(FScmi *instance_p, u32 domain)
{
    if (domain >= instance_p->reset.num_domains)
    {
        return 0;
    }
    return instance_p->reset.dom_info[domain].latency_us;
}

static FError FScmiResetDomainResetPort(FScmi *instance_p, u32 domain, u32 flag, u32 state)
{
    FError ret;
    struct FScmiMsgResetDomainReset *dom;
    struct FScmiResetInfo *pinfo = &instance_p->reset;
    struct FResetDomainInfo *pdom_info = pinfo->dom_info + domain;

    if (pdom_info->async_reset)
    {
        flag |= ASYNCHRONOUS_RESET;
    }
    if (info == NULL)
    {
        FSCMI_REST_ERROR("Info data structure not found,please check "
                         "initialization,protocol id is %d.\n",
                         SCMI_PROTOCOL_RESET);
        return FSCMI_ERROR_NULL_POINTER;
    }

    ret = FScmiMessageInit(instance_p, RESET, SCMI_PROTOCOL_RESET, sizeof(*dom), 0,
                           info->tx.buf);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Prepare hdr packaging is error ,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    dom = (struct FScmiMsgResetDomainReset *)(info->tx.buf);
    dom->domain_id = domain;
    dom->flags = flag;
    dom->reset_state = state;

    ret = FScmiDoTransport(instance_p, info, SCMI_PROTOCOL_RESET, FALSE);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("Transport package error,protocol id is %d\n", SCMI_PROTOCOL_RESET);
        return ret;
    }

    return FT_SUCCESS;
}

FError FScmiResetReset(FScmi *instance_p, u32 domain)
{
    return FScmiResetDomainResetPort(instance_p, domain, AUTONOMOUS_RESET, ARCH_COLD_RESET);
}

FError FScmiResetAssert(FScmi *instance_p, u32 domain)
{
    return FScmiResetDomainResetPort(instance_p, domain, EXPLICIT_RESET_ASSERT, ARCH_COLD_RESET);
}

FError FScmiResetDeassert(FScmi *instance_p, u32 domain)
{
    return FScmiResetDomainResetPort(instance_p, domain, 0, ARCH_COLD_RESET);
}

/**
 * @name: FScmiResetInit
 * @msg: Reset protocol initialization
 * @return {FError}             返回值
 * @param {FScmi} *instance_p   scmi协议实例
 * @param {boolean} poll_completion  是否轮询完成
 */
FError FScmiResetInit(FScmi *instance_p, boolean poll_completion)
{
    FError ret;
    u32 version;
    struct FScmiResetInfo *pinfo;
    pinfo = &instance_p->reset;

    info = FScmiGetInfo(instance_p, SCMI_PROTOCOL_RESET);
    if (info == NULL)
    {
        FSCMI_REST_ERROR("Info data structure not found, protocol id is %d.\n", SCMI_PROTOCOL_RESET);
        return FSCMI_ERROR_NULL_POINTER;
    }
    info->poll_completion = poll_completion;

    ret = FScmiResetGetVersion(instance_p, &version);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("FScmiResetGetVersion failed\n");
        return ret;
    }
    FSCMI_REST_INFO("Reset protocol version 0x%x", version);

    ret = FScmiResetAttributesGet(instance_p, pinfo);
    if (ret != FT_SUCCESS)
    {
        FSCMI_REST_ERROR("FScmiResetAttributesGet failed\n");
        return ret;
    }

    FSCMI_REST_INFO("Reset protocol attributes num_domains %d", pinfo->num_domains);
    for (u32 domain = 0; domain < pinfo->num_domains; domain++)
    {
        struct FResetDomainInfo *dom = pinfo->dom_info + domain;

        ret = FScmiResetDomainAttributesGet(instance_p, domain, dom);
        if (ret != FT_SUCCESS)
        {
            FSCMI_REST_ERROR("FScmiResetDomainAttributesGet failed\n");
            return ret;
        }
        FSCMI_REST_INFO("Reset domain %d: name %s async_reset %d reset_notify %d "
                        "latency_us %d",
                        domain, dom->name, dom->async_reset, dom->reset_notify, dom->latency_us);
    }

    pinfo->version = version;
    instance_p->reset = *pinfo;

    return FT_SUCCESS;
}
