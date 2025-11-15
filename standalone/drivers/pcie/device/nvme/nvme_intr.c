/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
 * 
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 *      https://opensource.org/licenses/BSD-3-Clause
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * 
 * FilePath: nvme_intr.c
 * Created Date: 2025-07-21 11:59:47
 * Last Modified: 2025-07-23 15:55:59
 * Description:  This file serves as the interrupt response file for different RTOS. Developers need to make appropriate adjustments based on the different operating systems they are porting to, in order to fully utilize the maximum performance of this operating system.
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * v1.0    huanghe		2025-07-22	First Release
 */


#include "nvme_intr.h"
#include "nvme.h"
#include "fdrivers_port.h"
#include "nvme_cmd.h"


#define NVME_INTR_DEBUG_TAG "NVME_INTR"
#define NVME_INTR_DEBUG_I(format, ...) \
    FT_DEBUG_PRINT_I(NVME_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_INTR_DEBUG_W(format, ...) \
    FT_DEBUG_PRINT_W(NVME_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_INTR_DEBUG_E(format, ...) \
    FT_DEBUG_PRINT_E(NVME_INTR_DEBUG_TAG, format, ##__VA_ARGS__)
#define NVME_INTR_DEBUG_D(format, ...) \
    FT_DEBUG_PRINT_D(NVME_INTR_DEBUG_TAG, format, ##__VA_ARGS__)


void nvme_cpl_status_poll_init(struct nvme_controller *ctrlr, struct nvme_completion_poll_status *status)
{
    status->ctrlr = ctrlr;
    status->status = 0;
    status->finish_flg = 0;
}


static u32 nvme_arch_protect(void)
{
    u32 cur;
    cur = MFCPSR();
    MTCPSR(cur | 0xC0);
    return cur;
}

static void nvme_arch_unprotect(u32 prev)
{
    MTCPSR(prev);
}


int request_timeout(struct nvme_controller *ctrlr)
{
    u32 current = TICKS_TO_SECONDS();
    struct nvme_request *request, *next;

    SYS_DLIST_FOR_EACH_CONTAINER_SAFE(&ctrlr->pending_request, request, next, node)
    {
        if (request == 0)
        {
            NVME_INTR_DEBUG_I("rq is null");
            break;
        }

        if ((int32_t)(request->req_start + CONFIG_NVME_REQUEST_TIMEOUT_SEC - current) > 0)
        {
            break;
        }

        NVME_INTR_DEBUG_W("Request %p CID %u timed-out", request, request->cmd.cdw0.cid);

        /* ToDo:
		 * - check CSTS for fatal fault
		 * - reset hw otherwise if it's the case
		 * - or check completion for missed interruption
		 */

        if (request->cb_fn)
        {
            request->cb_fn(request->cb_arg, NULL);
        }
        return -1;
    }
    return 0;
}

void nvme_completion_poll(struct nvme_completion_poll_status *status)
{
    u32 cur;
    struct nvme_controller *ctrlr = status->ctrlr;
    while (status->finish_flg == NVME_REQUEST_IS_NOT_FINSH)
    {
        cur = nvme_arch_protect();
        /* 检查当前注册的pending request 是否被超时 */
        if (request_timeout(ctrlr) == -1)
        {
            nvme_arch_unprotect(cur);
            break;
        }
        nvme_arch_unprotect(cur);
    }
    status->finish_flg = NVME_REQUEST_IS_NOT_FINSH;
}


void nvme_completion_poll_cb(void *arg, const struct nvme_completion *cpl)
{
    struct nvme_completion_poll_status *status = arg;
    if (cpl != NULL)
    {
        memcpy(&status->cpl, cpl, sizeof(*cpl));
    }
    else
    {
        status->status = -ETIMEDOUT;
    }
    status->finish_flg = 1;
}


bool nvme_cpl_status_is_error(struct nvme_completion_poll_status *status)
{
    return ((status->status != 0) || nvme_completion_is_error(&status->cpl));
}


void nvme_cmd_qpair_msi_handler(s32 vector, void *arg)
{
    const struct nvme_cmd_qpair *qpair = arg;
    nvme_cmd_qpair_process_completion((struct nvme_cmd_qpair *)qpair);
}
