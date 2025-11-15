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
 * FilePath: fdcdp_edid.c
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

#include <string.h>
#include "fio.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fassert.h"

#include "fdp.h"
#include "fdp_hw.h"
#include "fdcdp_aux.h"
#include "fdcdp_reg.h"
#include "fdcdp_edid.h"
/***************************** Macro Definitions **********/
#define FDCDP_EDID_DEBUG_TAG "FDCDP_EDID"
#define FDCDP_EDID_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FDCDP_EDID_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_EDID_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FDCDP_EDID_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDCDP_EDID_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FDCDP_EDID_DEBUG_TAG, format, ##__VA_ARGS__)

/***************************** Function Prototypes **********/

/**
 * @name: FDpCalculateSum8
 * @msg:  Perform the byte sum for buffer
 * @param {u8} *buffer is the Edid buffer
 * @param {u8} size is the buffer size.
 * @return sum
 */
static u8 FDpCalculateSum8(u8 *buffer, u8 size)
{
    u8 index;
    u8 sum = 0;
    /* Perform the byte sum for buffer*/
    for (index = 0; index < size; index++)
    {
        sum = (u8)(sum + buffer[index]);
    }
    return sum;
}

/**
 * @name: FDpParseEdidDtdTable
 * @msg:  read the aux information from the buffer
 * @param {u8} *buffer is the Edid buffer
 * @param {Auxtable} *list is the aux struct.
 * @return sum
 */
FError FDpParseEdidDtdTable(u8 *buffer, Auxtable *list)
{
    memset(list, 0, sizeof(Auxtable));
    /*
    pixel clock
    */
    list->pixel_clock = (((u32)buffer[1] << 8) + buffer[0]) * 10;
    /*
    hor pixel and ver pixel
    */
    list->hor_pixel = ((u32)((buffer[4] >> 4) & 0xF) << 8) + buffer[2];
    list->ver_pixel = ((u32)((buffer[7] >> 4) & 0xF) << 8) + buffer[5];

    /*
    hor blanking and ver blanking
    */
    list->hor_blanking = ((u32)(buffer[4] & 0xF) << 8) + buffer[3];
    list->ver_blanking = ((u32)(buffer[7] & 0xF) << 8) + buffer[6];
    /*
      hor sync front and ver sync front
    */
    list->hor_sync_front = ((u32)((buffer[11] >> 6) & 0x3) << 8) + buffer[8];
    list->ver_sync_front = ((u32)((buffer[11] >> 2) & 0x3) << 4) + ((buffer[10] >> 4) & 0xF);
    /*
    hor sync width and ver sync width
    */
    list->hor_sync_width = ((u32)((buffer[11] >> 4) & 0x3) << 8) + buffer[9];
    list->ver_sync_width = ((u32)((buffer[11] >> 0) & 0x3) << 4) + (buffer[10] & 0xF);
    /*
    hor sync polarity and ver sync polarity
    */
    list->hor_polarity = ((buffer[17] >> 1) & 0x1);
    list->ver_polarity = ((buffer[17] >> 2) & 0x1);

    return FDP_SUCCESS;
}

/**
 * @name: FDpParseDpEdidDtdList
 * @msg:  translate the edid information to the struct
 * @param {u8} *buffer is the aux buffer
 * @param {Auxtable} *list is the aux struct.
 * @return NUll
 */
FError FDpParseDpEdidDtdList(u8 *buffer, Auxtable *list)
{
    FASSERT(buffer != NULL);
    FError status = FDP_SUCCESS;
    u32 index;
    u8 check_sum;
    /*
     clear dtd list
    */
    memset(list, 0, sizeof(Auxtable));
    /*
      check sum
    */
    check_sum = FDpCalculateSum8(buffer, 128);
    if (check_sum != 0)
    {
        FDCDP_EDID_DEBUG("Edid checksum error, result : %x", check_sum);
        status = FDP_SUCCESS;
        goto Exit;
    }
    /*
    judge edid header
    */
    if ((buffer[0] != 0) || (buffer[7] != 0))
    {
        FDCDP_EDID_DEBUG("Edid header check failed!");
        status = FDP_SUCCESS;
        goto Exit;
    }
    for (index = 1; index < 7; index++)
    {
        if (buffer[index] != 0xFF)
        {
            FDCDP_EDID_DEBUG("Edid header check failed!");
            status = FDP_SUCCESS;
            goto Exit;
        }
    }
    /*
    parse dtd
    */
    index = 54;
    FDpParseEdidDtdTable(&buffer[index], list);

Exit:
    return status;
}

/**
 * @name: FDpGetEdid
 * @msg:  Get edid information form sink
 * @param {FDcCtrl} *instance_p is the all parameter of display
 * @param {u8} *buffer is the Edid information.
 * @return Null
 */
FError FDpGetEdid(FDpCtrl *instance_p, u8 *buffer)
{
    FASSERT(instance_p != NULL);
    FASSERT(buffer != NULL);
    FError status = FDP_SUCCESS;
    u32 data = 0;
    u32 write_data = 0;
    u8 loop_i;
    u8 loop_j;
    uintptr address = instance_p->config.dp_channe_base_addr;
    u32 timeout = 1000;

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_WRITE_FIFO, write_data);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1400);
    status = FDpWaitAuxReply(instance_p);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_WRITE_FIFO, write_data);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x400);
    status = FDpWaitAuxReply(instance_p);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1500);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x500);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA);
    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1100);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA);
    /*edid 0 128B*/
    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_WRITE_FIFO, write_data);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1400);
    status = FDpWaitAuxReply(instance_p);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_WRITE_FIFO, write_data);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x400);
    status = FDpWaitAuxReply(instance_p);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1500);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
    for (loop_i = 0; loop_i < 8; loop_i++)
    {
        while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
        {
            ;
        }
        FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
        FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x50f);
        status = FDpWaitAuxReply(instance_p);
        if (status != FDP_SUCCESS)
        {
            return FDP_SUCCESS;
        }
        data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
        data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
        for (loop_j = 0; loop_j < 16; loop_j++)
        {
            data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA);
            *(buffer + loop_i * 16 + loop_j) = data;
        }
    }

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1100);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA);
    /*edid 1 128B*/
    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_WRITE_FIFO, write_data);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1400);
    status = FDpWaitAuxReply(instance_p);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_WRITE_FIFO, write_data);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x400);
    status = FDpWaitAuxReply(instance_p);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1500);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);

    for (loop_i = 0; loop_i < 8; loop_i++)
    {
        while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
        {
            ;
        }
        FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
        FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x50f);
        status = FDpWaitAuxReply(instance_p);
        if (status != FDP_SUCCESS)
        {
            return FDP_FAILED;
        }
        data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
        data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
        for (loop_j = 0; loop_j < 16; loop_j++)
        {
            data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA);
            *(buffer + 128 + loop_i * 16 + loop_j) = data;
        }
    }

    while (FDpChannelRegRead(address, PHYTIUM_DP_INTERRUPT_RAW_STATUS) & 0x02 & timeout--)
    {
        ;
    }
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_ADDRESS, 0x50);
    FDpChannelRegWrite(address, PHYTIUM_DP_AUX_COMMAND, 0x1100);
    status = FDpWaitAuxReply(instance_p);
    if (status != FDP_SUCCESS)
    {
        return FDP_FAILED;
    }
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_CODE);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA_COUNT);
    data = FDpChannelRegRead(address, PHYTIUM_DP_AUX_REPLY_DATA);

    return FDP_SUCCESS;
}
