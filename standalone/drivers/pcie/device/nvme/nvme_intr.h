/*
 * 
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
 * FilePath: nvme_intr.h
 * Created Date: 2025-07-21 14:22:18
 * Last Modified: 2025-07-22 15:05:09
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef NVME_INTR_H
#define NVME_INTR_H

#include "fdrivers_port.h"
#include "nvme_cmd.h"

struct nvme_completion_poll_status
{
    struct nvme_controller *ctrlr;
    int status;
    struct nvme_completion cpl;
#define NVME_REQUEST_IS_FINSH     1
#define NVME_REQUEST_IS_NOT_FINSH 0
    volatile uint32_t finish_flg; /* 0 is not finish, 1 is finish */
    uint64_t timeout_tick;        /* 等待延时的时间 */
};

// void nvme_completion_poll_cb(void *arg, const struct nvme_completion *cpl);
typedef void (*nvme_poll_init)(struct nvme_completion_poll_status *status);
typedef void (*nvme_poll_wait)(struct nvme_completion_poll_status *status);

typedef void (*nvme_timeout_cb)(void *args);


#define NVME_CPL_STATUS_POLL_INIT(cpl_status, nvme_ctrlr) \
    {                                                     \
        .status = 0, .ctrlr = nvme_ctrlr                  \
    }


void nvme_cpl_status_poll_init(struct nvme_controller *ctrlr,
                               struct nvme_completion_poll_status *status);
void nvme_completion_poll(struct nvme_completion_poll_status *status);
void nvme_completion_poll_cb(void *arg, const struct nvme_completion *cpl);
bool nvme_cpl_status_is_error(struct nvme_completion_poll_status *status);
void nvme_cmd_qpair_msi_handler(s32 vector, void *arg);
#endif // !NVME_INTR_H
