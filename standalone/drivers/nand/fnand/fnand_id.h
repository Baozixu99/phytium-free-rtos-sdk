/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fnand_id.h
 * Date: 2022-07-06 14:19:15
 * LastEditTime: 2022-07-06 14:19:15
 * Description:  This file is for functions in this file are the read id required functions
 * for this driver. 
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */


#ifndef FNAND_ID_H
#define FNAND_ID_H


#include "ftypes.h"
#include "fnand.h"


#ifdef __cplusplus
extern "C"
{
#endif


struct FNandManuFacturerOps
{
    FError (*detect)(FNand *instance_p, FNandId *id_p, u32 chip_addr); /* detect chip */
    int (*init)(FNand *instance_p, u32 chip_addr);
    void (*cleanup)(FNand *instance_p, u32 chip_addr);
};


typedef struct
{
    int id;
    char *name;
    const struct FNandManuFacturerOps *ops;
} FNandManuFacturer;

FError FNandDetect(FNand *instance_p);

#ifdef __cplusplus
}
#endif

#endif
