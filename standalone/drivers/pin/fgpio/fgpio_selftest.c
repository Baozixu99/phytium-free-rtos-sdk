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
 * FilePath: fgpio_selftest.c
 * Date: 2022-06-17 14:32:12
 * LastEditTime: 2022-06-17 14:32:12
 * Description:  This files is for dumping gpio register info 
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/3/1     init commit
 * 2.0   zhugengyu  2022/7/1     support e2000
 * 3.0   zhugengyu  2024/5/7     modify interface to use gpio by pin
 */
/***************************** Include Files *********************************/
#include "fdrivers_port.h"
#include "fassert.h"

#include "fgpio_hw.h"
#include "fgpio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FGPIO_DEBUG_TAG "FGPIO-TEST"
#define FGPIO_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_WARN(format, ...) FT_DEBUG_PRINT_W(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_INFO(format, ...) FT_DEBUG_PRINT_I(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)
#define FGPIO_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FGPIO_DEBUG_TAG, format, ##__VA_ARGS__)

#define FGPIO_DUMPER(base_addr, reg_off, reg_name)             \
    FGPIO_DEBUG("\t\t[%s]@0x%x\t=\t0x%x", reg_name, (reg_off), \
                FGpioReadReg32((base_addr), (reg_off)))
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/**
 * @name: FGpioDumpRegisters
 * @msg: 打印GPIO控制寄存器信息
 * @return {NONE}
 * @param {uintptr} base_addr, GPIO控制器基地址
 */
void FGpioDumpRegisters(uintptr base_addr)
{
    FASSERT(0 != base_addr);

    FGPIO_DEBUG("Dump register info @0x%x", base_addr);
    FGPIO_DUMPER(base_addr, FGPIO_SWPORTA_DR_OFFSET, "dr");
    FGPIO_DUMPER(base_addr, FGPIO_SWPORTA_DDR_OFFSET, "ddr");
    FGPIO_DUMPER(base_addr, FGPIO_EXT_PORTA_OFFSET, "ext_porta");
#if defined(FGPIO_PORT_B)
    FGPIO_DUMPER(base_addr, FGPIO_SWPORTB_DR_OFFSET, "portb_dr");
    FGPIO_DUMPER(base_addr, FGPIO_SWPORTB_DDR_OFFSET, "portb_ddr");
    FGPIO_DUMPER(base_addr, FGPIO_EXT_PORTB_OFFSET, "ext_portb");
#endif
    FGPIO_DUMPER(base_addr, FGPIO_INTEN_OFFSET, "inten");
    FGPIO_DUMPER(base_addr, FGPIO_INTMASK_OFFSET, "intmask");
    FGPIO_DUMPER(base_addr, FGPIO_INTTYPE_LEVEL_OFFSET, "intr_level");
    FGPIO_DUMPER(base_addr, FGPIO_INT_POLARITY_OFFSET, "intr_polarity");
    FGPIO_DUMPER(base_addr, FGPIO_INTSTATUS_OFFSET, "intr_status");
    FGPIO_DUMPER(base_addr, FGPIO_RAW_INTSTATUS_OFFSET, "raw_int_status");
    FGPIO_DUMPER(base_addr, FGPIO_LS_SYNC_OFFSET, "ls_sync");
    FGPIO_DUMPER(base_addr, FGPIO_DEBOUNCE_OFFSET, "debounce");
    FGPIO_DUMPER(base_addr, FGPIO_PORTA_EOI_OFFSET, "porta_eoi");

    return;
}
