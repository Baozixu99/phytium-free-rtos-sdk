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
 * FilePath: fmhu.h
 * Date: 2022-12-29 18:07:43
 * LastEditTime: 2022-12-29 18:07:43
 * Description:  This file is for mhu function support and data struct
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/01/04 init
 */
#ifndef FMHU_H
#define FMHU_H

#include "ftypes.h"
#include "ferror_code.h"


#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FMHU_SUCCESS            FT_SUCCESS
#define FMHU_ERR_INVAL_PARM     FT_MAKE_ERRCODE(ErrModBsp, ErrBspMhu, 1)
#define FMHU_ERR_NOT_READY      FT_MAKE_ERRCODE(ErrModBsp, ErrBspMhu, 2)
#define FMHU_ERR_TIMEOUT        FT_MAKE_ERRCODE(ErrModBsp, ErrBspMhu, 3)
#define FMHU_ERR_NOT_SUPPORT    FT_MAKE_ERRCODE(ErrModBsp, ErrBspMhu, 4)
#define FMHU_ERR_INVAL_STATE    FT_MAKE_ERRCODE(ErrModBsp, ErrBspMhu, 5)

#define FSCMI_MBOX_TX_QUEUE_LEN 20

typedef void (*FMhuEvtHandler)(void *instance_p, void *param);

typedef struct
{
    u32 mhu_chan_id;   /* Mhu chan id */
    uintptr base_addr; /* Device base address */
    uintptr irq_addr;  /* Device interrupt address */
    u32 irq_num;       /* Device interrupt id */
    u32 irq_prority;   /* Device interrupt priority */
    u32 work_mode;     /* Device work mode intr or poll */
} FMhuConfig;

typedef struct
{
    FMhuConfig config;            /* Current active configs */
    u32 is_ready;                 /* Device is initialized and ready */
    volatile u32 status;          /* Mhu working or idle */
    FMhuEvtHandler slot_msg_recv; /* for interrupt handle */
} FMhu;

typedef struct
{
    FMhu mhu; /* mbox object instance */
    boolean tx_complete;
    /* send message */
    u32 msg_count, msg_free;
    void *msg_data[FSCMI_MBOX_TX_QUEUE_LEN];
    void *payload; /* share memory */
    void (*send_data)(uintptr addr, void *data);
    void (*startup)(uintptr addr);
    void (*shutdown)(uintptr addr);
    u32 (*last_tx_done)(uintptr addr, boolean secu_flag);
} FScmiMhu;

void FMhuGetDefConfig(FMhuConfig *config_p, u32 chan_id);
void FMhuChanProbe(FScmiMhu *instance_p);
void FMhuChanRemove(FScmiMhu *instance_p);

void FMhuIntrHandler(FScmiMhu *instance_p);
void FMhuSetChanIntrMask(FScmiMhu *instance_p, u32 status);
u32 FMhuGetChanIntrMask(FScmiMhu *instance_p);

#ifdef __cplusplus
}
#endif

#endif
