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
 * FilePath: fmio.h
 * Date: 2022-06-21 15:40:06
 * LastEditTime: 2022-06-21 15:40:06
 * Description:  This file is for user external interface definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  liushengming  2022/06/21  first commit
 */

#ifndef FMIO_H
#define FMIO_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"
#include "fassert.h"

/************************** Constant Definitions *****************************/
#define FMIO_SUCCESS         FT_SUCCESS
#define FMIO_ERR_INVAL_PARM  FT_MAKE_ERRCODE(ErrModBsp, ErrBspMio, 1)
#define FMIO_ERR_NOT_READY   FT_MAKE_ERRCODE(ErrModBsp, ErrBspMio, 2)
#define FMIO_ERR_TIMEOUT     FT_MAKE_ERRCODE(ErrModBsp, ErrBspMio, 3)
#define FMIO_ERR_NOT_SUPPORT FT_MAKE_ERRCODE(ErrModBsp, ErrBspMio, 4)
#define FMIO_ERR_INVAL_STATE FT_MAKE_ERRCODE(ErrModBsp, ErrBspMio, 5)

/**************************** Type Definitions *******************************/

typedef struct
{
    u32 instance_id;        /*mio id*/
    uintptr func_base_addr; /*I2C or UART function address*/
    u32 irq_num;            /* Device interrupt id */
    uintptr mio_base_addr;  /*MIO control address*/
} FMioConfig;               /*mio configs*/

typedef struct
{
    FMioConfig config; /* mio config */
    u32 is_ready;      /* mio initialize the complete flag */
} FMioCtrl;

/************************** Function Prototypes ******************************/
/*获取MIO的配置信息*/
const FMioConfig *FMioLookupConfig(u32 instance_id);

/*初始化MIO的功能*/
FError FMioFuncInit(FMioCtrl *instance_p, u32 mio_type);

/*去初始化*/
FError FMioFuncDeinit(FMioCtrl *instance_p);

/*获取功能配置的基地址*/
uintptr FMioFuncGetAddress(FMioCtrl *instance_p, u32 mio_type);

/*获取功能的中断号*/
u32 FMioFuncGetIrqNum(FMioCtrl *instance_p, u32 mio_type);

#ifdef __cplusplus
}
#endif

#endif