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
 * FilePath: fnand_common_cmd.h
 * @Date: 2022-07-05 19:01:01
 * @LastEditTime: 2022-07-05 19:01:02
 * @Description:  This file is for nand generic command documentation
 *
 * @Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */

#ifndef DRIVERS_NAND_COMMON_CMD_H
#define DRIVERS_NAND_COMMON_CMD_H

#include "ftypes.h"
#include "fnand.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*
 * Mandatory commands
 */

#define NAND_CMD_READ1               0x00
#define NAND_CMD_READ2               0x30 /* READ PAGE */

/* NAND Random data Read Column command (1st cycle) */
#define NAND_CMD_CHANGE_READ_COLUMN1 0x05

/* NAND Random data Read Column command (2nd cycle) */
#define NAND_CMD_CHANGE_READ_COLUMN2 0xE0
/* NAND Block Erase (1st cycle) */
#define NAND_CMD_BLOCK_ERASE1        0x60
/* NAND Block Erase (2nd cycle) */
#define NAND_CMD_BLOCK_ERASE2        0xD0
/* NAND Page Program command (1st cycle) */
#define NAND_CMD_PAGE_PROG1          0x80
/* NAND Page Program  command (2nd cycle)  */
#define NAND_CMD_PAGE_PROG2          0x10
/* NAND Change Write Column command */
#define NAND_CMD_CHANGE_WRITE_COLUMN 0x85
/* NAND Read ID command */
#define NAND_CMD_READ_ID             0x90
/* NAND Read Parameter Page command */
#define NAND_CMD_READ_PARAM_PAGE     0xEC
/* NAND Reset command */
#define NAND_CMD_RESET               0xFF

#define NAND_END_CMD_NONE            0xfff /* No End command */

#define NAND_CMD_READ_STATUS         0x70 /* Read status */

FError FNandFlashReset(FNand *instance_p, u32 chip_addr);
FError FNandFlashReadId(FNand *instance_p, u8 address, u8 *id_buffer, u32 buffer_length, u32 chip_addr);
void FNandFlashFuncRegister(FNand *instance_p);

#ifdef __cplusplus
}
#endif

#endif
