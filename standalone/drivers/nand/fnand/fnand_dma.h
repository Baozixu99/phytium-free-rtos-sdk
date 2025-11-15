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
 * FilePath: fnand_dma.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:27
 * Description:  This file is dma descriptor management API. 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */

#ifndef FNAND_DMA_H
#define FNAND_DMA_H

#include "ftypes.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define FNAND_CMDCTRL_TYPE_RESET    0x00 /* reset */
#define FNAND_CMDCTRL_TYPE_SET_FTR  0x01 /* Set features */
#define FNAND_CMDCTRL_TYPE_GET_FTR  0x02 /* Get features */
#define FNAND_CMDCTRL_TYPE_READ_ID  0x03 /* Read ID */
#define FNAND_CMDCTRL_TYPE_READ_COL 0x03 /* Read Column */
#define FNAND_CMDCTRL_TYPE_PAGE_PRO 0x04 /* Page program */
#define FNAND_CMDCTRL_TYPE_ERASE    0x05 /* Block Erase */
#define FNAND_CMDCTRL_TYPE_READ     0x06 /* Read */
#define FNAND_CMDCTRL_TYPE_TOGGLE   0x07 /* Toggle Two_plane */

#define FNAND_CMDCTRL_READ_PARAM    0x02
#define FNAND_CMDCTRL_READ_STATUS   0x03

#define FNAND_CMDCTRL_CH_READ_COL   0x03
#define FNAND_CMDCTRL_CH_ROW_ADDR   0x01
#define FNAND_CMDCTRL_CH_WR_COL     0x01

#define FNAND_NFC_ADDR_MAX_LEN      0x5

#define FNAND_DESCRIPTORS_SIZE      16

struct CmdCtrl
{
    u16 csel : 4; /* 每一位表示选择NAND FLASH 设备  */
    u16 dbc : 1; /* 表示是否有2级命令，1表示有，只有此位为1时，描述符表的CMD1才有效 */
    u16 addr_cyc : 3; /* 表示指令有几个周期，‘b000’:表示没有周期 ‘b001’:表示1一个地址周期，一次类推 */
    u16 nc : 1; /* 表示是否有连续的下一个指令，一般多页操作需要连续发送多个指令 */
    u16 cmd_type : 4; /* 表示命令类型 */
    u16 dc : 1; /* 表示命令发送是否包含有数据周期，有数据此位为1 */
    u16 auto_rs : 1; /* 表示命令发送完成后是否检测闪存状态 */
    u16 ecc_en : 1; /* ECC 数据发送和读取使能位，位1 表示该命令仅发送或者读取ECC数据，当读命令该位使能位1后，控制器会对上一次数据进行ECC 校验，并返回结果 */
};

struct FNandDmaDescriptor
{
    u8 cmd0; /* NAND FLASH 第一个命令编码 */
    u8 cmd1; /* NAND FLASH 第二个命令编码 */
    union
    {
        u16 ctrl;
        struct CmdCtrl nfc_ctrl;
    } cmd_ctrl; /* 16位命令字 */
    u8 addr[FNAND_NFC_ADDR_MAX_LEN];
    u16 page_cnt;
    u8 mem_addr_first[FNAND_NFC_ADDR_MAX_LEN];

} __attribute__((packed)) __attribute__((aligned(128)));


typedef struct
{
    u8 *addr_p; /* Address  */
    u32 addr_length;
    uintptr phy_address;
    u32 phy_bytes_length;
    u32 chip_addr;
    u8 contiune_dma; /*  */
} FNandDmaPackData;


/* DMA format */
typedef struct
{
    s32 start_cmd;  /* Start command */
    s32 end_cmd;    /* End command */
    u8 addr_cycles; /* Number of address cycles */
    u8 cmd_type;    /* Presentation command type ,followed by FNAND_CMDCTRL_XXXX */
    u8 ecc_en;      /* Hardware ecc open */
    u8 auto_rs;     /* 表示命令发送完成后是否检测闪存状态 */
} FNandCmdFormat;


#ifdef __cplusplus
}
#endif

#endif // !
