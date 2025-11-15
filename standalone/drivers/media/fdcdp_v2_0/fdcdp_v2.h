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
 * FilePath: fdcdp_v2.h
 * Created Date: 2025-01-22 14:53:06
 * Last Modified: 2025-07-08 17:46:25
 * Description:  This file is for
 *
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0      wzq         2025/07/08         first version
 */
#ifndef FDCDP_V2_H
#define FDCDP_V2_H
/***************************** Include Files *********************************/
#include "ftypes.h"
#include "fparameters.h"
#include "ferror_code.h"
#include "fdp_v2.h"
#include "fdc_v2.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#define FDCDP_SUCCESS         0
#define FDCDP_NOT_INITIALIZED 0
#define FDCDP_IS_INITIALIZED  1

#define FDCDP_NOT_INIT        FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIAMsg, 3)
#define FDCDP_PARA_ERR        FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIAMsg, 4)
#define FDCDP_TRAIN_ERR       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMEDIAMsg, 5)
typedef enum
{
    FDCDP_CONNECT_TO_DISCONNCET = 0,
    FDCDP_DISCONNCET_TO_CONNECT
} FDcDpConnectStatus;

typedef enum
{
    FDCDP_NOT_TRAIN = 0,
    FDCDP_IS_TRAIN,
} FDcDpTrainStatus;
typedef enum
{
    FDCDP_HPD_IRQ_CONNECTED = 0, /* port connected */
    FDCDP_HPD_IRQ_DISCONNECTED,  /*the port disconnected*/
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

#define CPPC_ACSE    0x26FCC004
#define CPPC_SCCL    0x26FCC014
#define SHARMEM_BASE 0x26FC5940

typedef struct
{
    u32 res_a;
    volatile u32 status;
    uintptr res_b;
    u32 flags;
    volatile u32 len;
    u32 msg_header;
    u32 payload[4];
} FDcDpLowPowerSet;

typedef struct
{
    /* fdc instace object */
    FDcCtrl dc_instance_p[FDC_INSTANCE_NUM];
    /* fdp instace object */
    FDpCtrl dp_instance_p[FDP_INSTANCE_NUM];
    /*the intr config of dcdp*/
    FDcDpIntrConfig intr_event[FDCDP_INTR_MAX_NUM];
    /*the  info of port*/
    u32 connect_flg[FDP_INSTANCE_NUM];
    u32 is_initialized[FDP_INSTANCE_NUM];
    /*the port index of dcdp*/
    u32 port;
    /*the port config status of dcdp*/
    u32 is_ready[FDP_INSTANCE_NUM];
    u32 has_train_dp[FDP_INSTANCE_NUM];
    /*clone, horizontal and vertical display*/
    u32 multi_mode;
} FDcDp;

void FDcDpV2SetTiming(FDcDp *instance, u32 channel);

FError FDcDpV2Initial(FDcDp *instance, u32 channel, u32 width, u32 height);

FError FDcDpV2CfgInitialize(FDcDp *instance, u32 channel);

FError FDcDpV2GeneralCfgInitial(FDcDp *instance, u8 channel);

/*register the interrupt*/
void FDcDpV2RegisterHandler(FDcDp *instance_p, FDcDpIntrEventType type,
                            FDCDPIntrHandler handler, void *param);

/*the interrupt handler*/
void FDcDpV2InterruptHandler(s32 vector, void *args);

/*enable the interrupt*/
void FDcDpV2IrqEnable(FDcDp *instance_p, u32 index, FDcDpIntrEventType intr_event_p);

const FDcConfig *FDcV2LookupConfig(u32 instance_id);

const FDpConfig *FDpV2LookupConfig(u32 instance_id);

void FDcDpV2HotPlug(FDcDp *instance_p, u32 index, FDcDpConnectStatus connect_status);

FError FDcDpV2DeInitialize(FDcDp *instance, u32 index);

u32 FDcDpV2PwrOn(FDcDp *instance, u32 channel, boolean state);
#ifdef __cplusplus
}
#endif

#endif // FDCDP_H
