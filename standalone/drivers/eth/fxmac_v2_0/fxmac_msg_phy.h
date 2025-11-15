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
 * FilePath: fxmac_msg_phy.h
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file is for phy configuration.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#ifndef FXMAC_MSG_PHY_H
#define FXMAC_MSG_PHY_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fxmac_msg.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FXMAC_MSG_PHY_RESET_ENABLE  1
#define FXMAC_MSG_PHY_RESET_DISABLE 0

enum
{
    FXMAC_MSG_PHY_AUTONEGOTIATION_DISABLE = 0,
    FXMAC_MSG_PHY_AUTONEGOTIATION_ENABLE
};

enum
{
    FXMAC_MSG_PHY_MODE_HALFDUPLEX = 0,
    FXMAC_MSG_PHY_MODE_FULLDUPLEX
};

typedef struct
{
    FXmacMsgPhyInterface interface;
    int speed;
    int duplex;
    int pause;
    int rate_matching;
    unsigned int link        : 1;
    unsigned int an_complete : 1;
} PhylinkLinkState;

enum
{
    MLO_AN_PHY = 0, /* Conventional PHY */
    MLO_AN_FIXED,   /* Fixed-link mode */
    MLO_AN_INBAND,  /* In-band protocol */
};

/* phy interface */
void FXmacMsgInterfaceConfig(FXmacMsgCtrl *pdata, unsigned int mode);
int FXmacMsgInterfaceLinkup(FXmacMsgCtrl *pdata, FXmacMsgPhyInterface interface,
                            int speed, int duplex);
int FXmacMsgInterfaceLinkdown(FXmacMsgCtrl *pdata);
int FXmacMsgPcsLinkup(FXmacMsgCtrl *pdata, FXmacMsgPhyInterface interface, int speed, int duplex);
int FXmacMsgPcsLinkdown(FXmacMsgCtrl *pdata);
unsigned int FXmacMsgPcsGetLink(FXmacMsgCtrl *pdata, FXmacMsgPhyInterface interface);
FError FXmacMsgPhyInit(FXmacMsgCtrl *instance_p, u32 speed, u32 duplex_mode,
                       u32 autonegotiation_en, u32 reset_flag);


#ifdef __cplusplus
}
#endif

#endif // !