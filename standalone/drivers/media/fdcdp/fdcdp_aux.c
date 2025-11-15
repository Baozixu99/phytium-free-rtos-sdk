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
 * FilePath: fdcdp_aux.c
 * Date: 2024-09-17 22:53:24
 * LastEditTime: 2024/09/19  12:5:25
 * Description:  This file is for This file is for Handling the hardware register and
 *               providing some function interface
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2024/09/19  Modify the format and establish the version
 */


/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fdrivers_port.h"
#include "fdc_hw.h"
#include "fdp.h"
#include "fdp_hw.h"
#include "fdcdp_reg.h"

/***************************** Macro Definitions *****************************/
#define FDCDP_AUX_DEBUG_TAG "FDCDP_AUX"
#define FDCDP_AUX_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FDCDP_AUX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_AUX_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FDCDP_AUX_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_AUX_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FDCDP_AUX_DEBUG_TAG, format, ##__VA_ARGS__)

/***************************** Function Prototypes ***************************/
/**
 * @name: FDpWaitAuxReply
 * @msg:  Wait util that an aux reply has been recieved
 * @param {FDpCtrl} *instance is the all parameter of display
 * @return status,0-reply has been received,1-failed to reply
 */
FError FDpWaitAuxReply(FDpCtrl *instance)
{
    FError status = FDP_FAILED;
    u32 reply_times = 5;
    u8 aux_status;
    u8 init_status;

    uintptr addr = instance->config.dp_channe_base_addr;
    while (reply_times > 0)
    {
        FDriverMdelay(10);
        aux_status = FDpChannelRegRead(addr, PHYTIUM_DP_AUX_STATUS);
        FDpChannelRegRead(addr, PHYTIUM_DP_AUX_REPLY_CODE);
        init_status = FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_RAW_STATUS);
        if ((init_status & 0x08) != 0)
        {
            status = FDP_FAILED;
            break;
        }
        else if ((init_status & 0x04) != 0)
        {
            if ((aux_status & 0x1) != 0)
            {
                status = FDP_SUCCESS;
                break;
            }
        }
        reply_times--;
    }
    return status;
}

/**
 * @name: FDpSinkDpcdWrite
 * @msg:  write phy register through aux channel.
 * @param {FDpCtrl} *instance is the all parameter of display
 * @param {uintptr} addr is the dpcd regiter addr.Twenty-bit addr for the start of the AUX Channel burst
 * @param {u8} data is the data that you want to write to the sink regiter through AUX channel.
 * @return Null
 */
FError FDpSinkDpcdWrite(FDpCtrl *instance, uintptr address, u8 data)
{
    FError status = FDP_SUCCESS;

    uintptr addr = instance->config.dp_channe_base_addr;
    FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_STATUS);
    FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_RAW_STATUS);
    FDpChannelRegWrite(addr, PHYTIUM_DP_AUX_ADDRESS, address);
    FDpChannelRegWrite(addr, PHYTIUM_DP_AUX_WRITE_FIFO, data);
    FDpChannelRegWrite(addr, PHYTIUM_DP_AUX_COMMAND, 0x800);
    status = FDpWaitAuxReply(instance);
    if (status == FDP_FAILED)
    {
        FDCDP_AUX_ERROR("Dpcd Write addr : 0x%x data : 0x%x , FMEDIA_DP_FAILED", addr, data);
        return FDP_FAILED;
    }
    return status;
}

/**
 * @name: FDpSinkDpcdRead
 * @msg:  Read phy register through aux channel.
 * @param {FDpCtrl} *instance is the all parameter of display
 * @param {uintptr} addr is the dpcd regiter addr.Twenty-bit addr for the start of the AUX Channel burst
 * @param {u8} data is the data that you want to write to the sink regiter through AUX channel.
 * @return Null
 */
FError FDpSinkDpcdRead(FDpCtrl *instance, uintptr address, u8 *data)
{
    FError status;

    uintptr addr = instance->config.dp_channe_base_addr;
    FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_STATUS);
    FDpChannelRegRead(addr, PHYTIUM_DP_INTERRUPT_RAW_STATUS);
    FDpChannelRegWrite(addr, PHYTIUM_DP_AUX_ADDRESS, address);
    FDpChannelRegWrite(addr, PHYTIUM_DP_AUX_COMMAND, 0x900);
    status = FDpWaitAuxReply(instance);
    if (status == FDP_FAILED)
    {
        FDCDP_AUX_ERROR("Dpcd Read addr : 0x%x data : 0x%x , FMEDIA_DP_FAILED", addr, data);
        return status;
    }
    FDpChannelRegRead(addr, PHYTIUM_DP_AUX_REPLY_CODE);
    *data = FDpChannelRegRead(addr, PHYTIUM_DP_AUX_REPLY_DATA);
    return FDP_SUCCESS;
}
