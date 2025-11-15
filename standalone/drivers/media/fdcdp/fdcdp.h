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
 * FilePath: fdcdp.h
 * Date: 2024-12-07 09:23:30
 * LastEditTime: 2024/12/07 10:10:31
 * Description:  This file is for connecting the dc and dp
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------  -------- --------------------------------------
 *  1.0  Wangzq     2024/12/07  Modify the format and establish the version
 */

#ifndef FDCDP_H
#define FDCDP_H

#include "fdc.h"
#include "fdp.h"
#include "ftypes.h"
#include "fparameters.h"

#define FDC_GOP_MAX_MODENUM 11

/*set the dp static params*/
const FDpConfig *FDpLookupConfig(u32 instance_id);

/*set the dc static params*/
const FDcConfig *FDcLookupConfig(u32 instance_id);

typedef enum
{
    FDCDP_CONNECT_TO_DISCONNCET = 0,
    FDCDP_DISCONNCET_TO_CONNECT
} FDcDpConnectStatus;

typedef enum
{
    FDCDP_NOT_INITIALIZED = 0,
    FDCDP_IS_INITIALIZED,
} FDcDpInitStatus;

typedef enum
{
    FDCDP_NOT_TRAIN = 0,
    FDCDP_IS_TRAIN,
} FDcDpTrainStatus;

typedef enum
{
    FDCDP_HPD_IRQ_CONNECTED = 0, /* dp disconnected */
    FDCDP_HPD_IRQ_DISCONNECTED,  /*the dp disconnected*/
    FDCDP_AUX_REPLY_TIMEOUT,     /*receive aux reply timeout*/
    FDCDP_AUX_REPLY_ERROR,       /*the aux reply is invalid*/

    FDCDP_INTR_MAX_NUM
} FDcDpIntrEventType;

typedef void (*FDCDPIntrHandler)(void *param, u32 index);

typedef struct
{
    FDcDpIntrEventType type; /* the intr type */
    FDCDPIntrHandler handler;
    void *param;
} FDcDpIntrConfig;

typedef struct
{
    /* fdc instace object */
    FDcCtrl dc_instance_p[FDC_INSTANCE_NUM];
    /* fdp instace object */
    FDpCtrl dp_instance_p[FDP_INSTANCE_NUM];
    /*the intr config of dcdp*/
    FDcDpIntrConfig intr_event[FDCDP_INTR_MAX_NUM];
    u32 is_ready[FDP_INSTANCE_NUM];
    u32 connect_flg[FDP_INSTANCE_NUM];
    u32 is_initialized[FDP_INSTANCE_NUM];
    u32 has_train_dp[FDP_INSTANCE_NUM];
    /*clone, horizontal and vertical display*/
    u32 multi_mode;
} FDcDp;

void FDcDpGeneralCfgInitial(FDcDp *instance, u8 channel);

FError FDcDpInitial(FDcDp *instance, u8 channel, u32 width, u32 height);

/*register the interrupt*/
void FDcDpRegisterHandler(FDcDp *instance_p, FDcDpIntrEventType type,
                          FDCDPIntrHandler handler, void *param);

/*the interrupt handler*/
void FDcDpInterruptHandler(s32 vector, void *args);

/*enable the interrupt*/
void FDcDpIrqEnable(FDcDp *instance_p, u32 index, FDcDpIntrEventType intr_event_p);

FError FDcDpDeInitialize(FDcDp *instance, u32 channel);

FError FDcDpCfgInitialize(FDcDp *instance, u32 index);

void FDcDpHotPlug(FDcDp *instance_p, u32 index, FDcDpConnectStatus connect_status);
#endif /* FDCDP_H */