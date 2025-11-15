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
 * FilePath: fnand_onfi.h
 * Date: 2022-07-05 19:10:47
 * LastEditTime: 2022-07-05 19:10:47
 * Description:  This file is for onfi type nand
 *
 * Modify History:
 *  Ver   Who  Date   Changes
 * ----- ------  -------- --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */
#ifndef DRIVERS_NAND_FNAND_ONFI_H
#define DRIVERS_NAND_FNAND_ONFI_H

#include "ftypes.h"
#include "fnand.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ONFI_CMD_READ_ID \
    0x90 /* ONFI Read ID \
command */
#define ONFI_CMD_READ_PARAM_PAGE \
    0xEC /* ONFI Read \
                          Parameter Page                   \
command */


#define ONFI_END_CMD_NONE 0xfff /* No End command */

struct OnfiNandGeometry
{
    /* rev info and features block */
    /* 'O' 'N' 'F' 'I'  */
    u8 sig[4];
    u16 revision;
    u16 features;
    u16 opt_cmd;
    u8 reserved0[2];
    u16 ext_param_page_length; /* since ONFI 2.1 */
    u8 num_of_param_pages;     /* since ONFI 2.1 */
    u8 reserved1[17];

    /* manufacturer information block */
    char manufacturer[12];
    char model[20];
    u8 jedec_id;
    u16 date_code;
    u8 reserved2[13];

    /* memory organization block */
    u32 byte_per_page;
    u16 spare_bytes_per_page;
    u32 data_bytes_per_ppage;
    u16 spare_bytes_per_ppage;
    u32 pages_per_block;
    u32 blocks_per_lun;
    u8 lun_count;
    u8 addr_cycles;
    u8 bits_per_cell;
    u16 bb_per_lun;
    u16 block_endurance;
    u8 guaranteed_good_blocks;
    u16 guaranteed_block_endurance;
    u8 programs_per_page;
    u8 ppage_attr;
    u8 ecc_bits;
    u8 interleaved_bits;
    u8 interleaved_ops;
    u8 reserved3[13];

    /* electrical parameter block */
    u8 io_pin_capacitance_max;
    u16 async_timing_mode;
    u16 program_cache_timing_mode;
    u16 t_prog;
    u16 t_bers;
    u16 t_r;
    u16 t_ccs;
    u16 src_sync_timing_mode;
    u8 src_ssync_features;
    u16 clk_pin_capacitance_typ;
    u16 io_pin_capacitance_typ;
    u16 input_pin_capacitance_typ;
    u8 input_pin_capacitance_max;
    u8 driver_strength_support;
    u16 t_int_r;
    u16 t_adl;
    u8 reserved4[8];

    /* vendor */
    u16 vendor_revision;
    u8 vendor[88];

    u16 crc;
} __attribute__((__packed__));

#ifdef __cplusplus
}
#endif

#endif // !1