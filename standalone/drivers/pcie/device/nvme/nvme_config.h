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
 * FilePath: nvme_config.h
 * Created Date: 2025-07-10 16:43:00
 * Last Modified: 2025-07-22 14:14:15
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */

#ifndef NVME_CONFIG_H
#define NVME_CONFIG_H

#include "fdebug.h"
#include <errno.h>

#define CONFIG_NVME_ADMIN_ENTRIES       256
#define CONFIG_NVME_IO_QUEUES           1
#define CONFIG_NVME_IO_ENTRIES          16
#define CONFIG_NVME_RETRY_COUNT         2
#define CONFIG_NVME_REQUEST_TIMEOUT_SEC 5
#define CONFIG_NVME_PRP_LIST_AMOUNT     2
#define CONFIG_NVME_MAX_NAMESPACES      1
#define CONFIG_NVME_INT_PRIORITY        2
#define CONFIG_NVME_LOG_LEVEL_DBG

#define ARG_UNUSED(x)        ((void)(x))

#define sys_le16_to_cpu(val) (val)
#define sys_cpu_to_le16(val) (val)
#define sys_le24_to_cpu(val) (val)
#define sys_cpu_to_le24(val) (val)
#define sys_le32_to_cpu(val) (val)
#define sys_cpu_to_le32(val) (val)
#define sys_le40_to_cpu(val) (val)
#define sys_cpu_to_le40(val) (val)
#define sys_le48_to_cpu(val) (val)
#define sys_cpu_to_le48(val) (val)
#define sys_le64_to_cpu(val) (val)
#define sys_cpu_to_le64(val) (val)


#endif // !
