/*
 * Copyright (C) 2024, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: load_fw.h
 * Created Date: 2024-05-06 14:48:29
 * Last Modified: 2024-06-14 15:27:31
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 */
#ifndef LOAD_FW_H_
#define LOAD_FW_H_

/***************************** Include Files *********************************/
#include "platform_info.h"
#include "rsc_table.h"
/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/

#if defined __cplusplus
extern "C"
{
#endif

struct mem_file
{
    const void *base;
};


int load_exectuable_block(struct remoteproc *rproc, const struct image_store_ops *store_ops,
                          void *store, const char *img_path);
void print_resource_table(const struct remote_resource_table *table);

#if defined __cplusplus
}
#endif


#endif
