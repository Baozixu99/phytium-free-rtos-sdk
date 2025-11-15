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
 * FilePath: fgpio_hw.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:25:35
 * Description:  This files is for GPIO register definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/3/1     init commit
 * 2.0   zhugengyu  2022/7/1     support e2000
 */


#ifndef FGPIO_HW_H
#define FGPIO_HW_H


/***************************** Include Files *********************************/
#include "fparameters.h"
#include "fio.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
/** @name Register Map
 *
 * Register offsets from the base address of an GPIO device.
 * @{
 */
#define FGPIO_SWPORTA_DR_OFFSET    0x00 /* WR Port A Output Data Register */
#define FGPIO_SWPORTA_DDR_OFFSET   0x04 /* WR Port A Data Direction Register */
#define FGPIO_EXT_PORTA_OFFSET     0x08 /* RO Port A Input Data Register */

#define FGPIO_SWPORTB_DR_OFFSET    0x0c /* WR Port B Output Data Register */
#define FGPIO_SWPORTB_DDR_OFFSET   0x10 /* WR Port B Data Direction Register */
#define FGPIO_EXT_PORTB_OFFSET     0x14 /* RO Port B Input Data Register */

#define FGPIO_INTEN_OFFSET         0x18 /* WR Port A Interrput Enable Register */
#define FGPIO_INTMASK_OFFSET       0x1c /* WR Port A Interrupt Mask Register */
#define FGPIO_INTTYPE_LEVEL_OFFSET 0x20 /* WR Port A Interrupt Level Register */
#define FGPIO_INT_POLARITY_OFFSET  0x24 /* WR Port A Interrupt Polarity Register */
#define FGPIO_INTSTATUS_OFFSET     0x28 /* RO Port A Interrupt Status Register */
#define FGPIO_RAW_INTSTATUS_OFFSET 0x2c /* RO Port A Raw Interrupt Status Register */
#define FGPIO_LS_SYNC_OFFSET \
    0x30 /* WR Level-sensitive Synchronization Enable Register */
#define FGPIO_DEBOUNCE_OFFSET          0x34 /* WR Debounce Enable Register */
#define FGPIO_PORTA_EOI_OFFSET         0x38 /* WO Port A Clear Interrupt Register */

/* Valid bits for each port registers, e.g 7, 15 */
#define FGPIO_PIN_MSB                  (FGPIO_PIN_NUM - 1)

/** @name FGPIO_SWPORTA_DR_OFFSET Register
 */
#define FGPIO_SWPORTA_DR_SET(dir)      SET_REG32_BITS((dir), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTA_DR_GET(reg_val)  GET_REG32_BITS((reg_val), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTA_DR_MASK          GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_SWPORTA_DDR_OFFSET Register
 */
#define FGPIO_SWPORTA_DDR_SET(dir)     SET_REG32_BITS((dir), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTA_DDR_GET(reg_val) GET_REG32_BITS((reg_val), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTA_DDR_MASK         GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_EXT_PORTA_OFFSET Register
 */
#define FGPIO_EXT_PORTA_SET(dir)       SET_REG32_BITS((dir), FGPIO_PIN_MSB, 0)
#define FGPIO_EXT_PORTA_GET(reg_val)   GET_REG32_BITS((reg_val), FGPIO_PIN_MSB, 0)
#define FGPIO_EXT_PORTA_MASK           GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_SWPORTB_DR_OFFSET Register
 */
#define FGPIO_SWPORTB_DR_SET(dir)      SET_REG32_BITS((dir), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTB_DR_GET(reg_val)  GET_REG32_BITS((reg_val), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTB_DR_MASK          GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_SWPORTB_DDR_OFFSET Register
 */
#define FGPIO_SWPORTB_DDR_SET(dir)     SET_REG32_BITS((dir), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTB_DDR_GET(reg_val) GET_REG32_BITS((reg_val), FGPIO_PIN_MSB, 0)
#define FGPIO_SWPORTB_DDR_MASK         GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_EXT_PORTB_OFFSET Register
 */
#define FGPIO_EXT_PORTB_SET(dir)       SET_REG32_BITS((dir), FGPIO_PIN_MSB, 0)
#define FGPIO_EXT_PORTB_GET(reg_val)   GET_REG32_BITS((reg_val), FGPIO_PIN_MSB, 0)
#define FGPIO_EXT_PORTB_MASK           GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_INTEN_OFFSET Register
 */
#define FGPIO_INTR_PORTA_EN(n)         BIT(n) /* 1: enable the intr of n-th port in group-a */

/** @name FGPIO_INTMASK_OFFSET Register
 */
#define FGPIO_INTR_PORTA_MASK(n) \
    BIT(n) /* 1: disable the intr of n-th port in group-a */
#define FGPIO_INTR_PORTA_MASKALL  GENMASK(FGPIO_PIN_MSB, 0)

/** @name FGPIO_INTTYPE_LEVEL_OFFSET Register
 */
#define FGPIO_INTR_PORTA_LEVEL(n) BIT(n) /* 1: intr by edge, 0: intr by level */

/** @name FGPIO_INT_POLARITY_OFFSET Register
 */
#define FGPIO_INTR_PORTA_POLARITY(n) \
    BIT(n) /* 1: intr by rising-edge/high-level, 0: intr by falling-edge/low-level */

/** @name FGPIO_INTSTATUS_OFFSET Register
 */
#define FGPIO_INTR_PORTA_STATUS(n)     BIT(n) /* intr status */

/** @name FGPIO_RAW_INTSTATUS_OFFSET Register
 */
#define FGPIO_INTR_PORTA_RAW_STATUS(n) BIT(n) /* intr status without masking */

/** @name FGPIO_LS_SYNC_OFFSET Register
 */
#define FGPIO_PCLK_INTR_SYNC(n)        BIT(n) /* 1: sync to pclk_intr */

/** @name FGPIO_PORTA_EOI_OFFSET Register
 */
#define FGPIO_CLR_INTR_PORTA(n)        BIT(n) /* 1: clear interrupt */


/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
static inline u32 FGpioReadReg32(uintptr base_addr, uintptr reg_off)
{
    return FtIn32(base_addr + reg_off);
}

static inline void FGpioWriteReg32(uintptr base_addr, uintptr reg_off, const u32 reg_val)
{
    FtOut32(base_addr + reg_off, reg_val);
}

static inline void FGpioSetBit32(uintptr base_addr, uintptr reg_off, u32 bit)
{
    if (0 == bit)
    {
        FtClearBit32(base_addr + reg_off, bit);
    }
    else if (1 == bit)
    {
        FtSetBit32(base_addr + reg_off, bit);
    }
}

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif
