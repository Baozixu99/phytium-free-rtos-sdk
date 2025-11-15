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
 * FilePath: fgic_its.c
 * Created Date: 2024-07-24 11:29:49
 * Last Modified: 2025-06-05 17:04:32
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0		huanghe	  2024/08/09  		first version
 */

#ifndef FGIC_ITS_H
#define FGIC_ITS_H


#include "ftypes.h"
#include "fgic_v3.h"
#include "fkernel.h"

#define MASK(__basename)            (__basename##_MASK << __basename##_SHIFT)
#define MASK_SET(__val, __basename) (((__val)&__basename##_MASK) << __basename##_SHIFT)
#define MASK_GET(__reg, __basename) \
    (((__reg) >> __basename##_SHIFT) & __basename##_MASK)

/* Cache and Share ability for ITS & Redistributor LPI state tables */
#define GIC_BASER_CACHE_NGNRNE              0x0ULL /* Device-nGnRnE */
#define GIC_BASER_CACHE_INNERLIKE           0x0ULL /* Same as Inner Cacheability. */
#define GIC_BASER_CACHE_NCACHEABLE          0x1ULL /* Non-cacheable */
#define GIC_BASER_CACHE_RAWT                0x2ULL /* Cacheable R-allocate, W-through */
#define GIC_BASER_CACHE_RAWB                0x3ULL /* Cacheable R-allocate, W-back */
#define GIC_BASER_CACHE_WAWT                0x4ULL /* Cacheable W-allocate, W-through */
#define GIC_BASER_CACHE_WAWB                0x5ULL /* Cacheable W-allocate, W-back */
#define GIC_BASER_CACHE_RAWAWT              0x6ULL /* Cacheable R-allocate, W-allocate, W-through */
#define GIC_BASER_CACHE_RAWAWB              0x7ULL /* Cacheable R-allocate, W-allocate, W-back */
#define GIC_BASER_SHARE_NO                  0x0ULL /* Non-shareable */
#define GIC_BASER_SHARE_INNER               0x1ULL /* Inner Shareable */
#define GIC_BASER_SHARE_OUTER               0x2ULL /* Outer Shareable */

/*
 * GIC Register Interface Base Addresses
 */

#define GIC_RDIST_BASE                      DT_REG_ADDR_BY_IDX(DT_INST(0, arm_gic), 1)
#define GIC_RDIST_SIZE                      DT_REG_SIZE_BY_IDX(DT_INST(0, arm_gic), 1)

/* SGI base is at 64K offset from Redistributor */
#define GICR_SGI_BASE_OFF                   0x10000

/* GICR registers offset from RD_base(n) */
#define GICR_CTLR                           0x0000
#define GICR_IIDR                           0x0004
#define GICR_TYPER                          0x0008
#define GICR_STATUSR                        0x0010
#define GICR_WAKER                          0x0014
#define GICR_PWRR                           0x0024
#define GICR_PROPBASER                      0x0070
#define GICR_PENDBASER                      0x0078

/* Register bit definitions */

/* GICD_CTLR Interrupt group definitions */
#define GICD_CTLR_ENABLE_G0                 0
#define GICD_CTLR_ENABLE_G1NS               1
#define GICD_CTLR_ENABLE_G1S                2
#define GICD_CTRL_ARE_S                     4
#define GICD_CTRL_ARE_NS                    5
#define GICD_CTRL_NS                        6
#define GICD_CGRL_E1NWF                     7

/* GICD_CTLR Register write progress bit */
#define GICD_CTLR_RWP                       31

/* GICR_CTLR */
#define GICR_CTLR_ENABLE_LPIS               BIT(0)
#define GICR_CTLR_RWP                       3

/* GICR_IIDR */
#define GICR_IIDR_PRODUCT_ID_SHIFT          24
#define GICR_IIDR_PRODUCT_ID_MASK           0xFFUL
#define GICR_IIDR_PRODUCT_ID_GET(_val)      MASK_GET(_val, GICR_IIDR_PRODUCT_ID)

/* GICR_TYPER */
#define GICR_TYPER_AFFINITY_VALUE_SHIFT     32
#define GICR_TYPER_AFFINITY_VALUE_MASK      0xFFFFFFFFULL
#define GICR_TYPER_AFFINITY_VALUE_GET(_val) MASK_GET(_val, GICR_TYPER_AFFINITY_VALUE)
#define GICR_TYPER_LAST_SHIFT               4
#define GICR_TYPER_LAST_MASK                0x1UL
#define GICR_TYPER_LAST_GET(_val)           MASK_GET(_val, GICR_TYPER_LAST)
#define GICR_TYPER_PROCESSOR_NUMBER_SHIFT   8
#define GICR_TYPER_PROCESSOR_NUMBER_MASK    0xFFFFU
#define GICR_TYPER_PROCESSOR_NUMBER_GET(_val) \
    MASK_GET(_val, GICR_TYPER_PROCESSOR_NUMBER)

/* GICR_WAKER */
#define GICR_WAKER_PS                     1
#define GICR_WAKER_CA                     2

/* GICR_PWRR */
#define GICR_PWRR_RDPD                    0
#define GICR_PWRR_RDAG                    1
#define GICR_PWRR_RDGPO                   3

/* GICR_PROPBASER */
#define GITR_PROPBASER_ID_BITS_MASK       0x1fUL
#define GITR_PROPBASER_INNER_CACHE_SHIFT  7
#define GITR_PROPBASER_INNER_CACHE_MASK   0x7UL
#define GITR_PROPBASER_SHAREABILITY_SHIFT 10
#define GITR_PROPBASER_SHAREABILITY_MASK  0x3UL
#define GITR_PROPBASER_ADDR_SHIFT         12
#define GITR_PROPBASER_ADDR_MASK          0xFFFFFFFFFFUL
#define GITR_PROPBASER_OUTER_CACHE_SHIFT  56
#define GITR_PROPBASER_OUTER_CACHE_MASK   0x7UL
#define BIT64(_n)                         (1ULL << (_n))
/* GICR_PENDBASER */
#define GITR_PENDBASER_INNER_CACHE_SHIFT  7
#define GITR_PENDBASER_INNER_CACHE_MASK   0x7UL
#define GITR_PENDBASER_SHAREABILITY_SHIFT 10
#define GITR_PENDBASER_SHAREABILITY_MASK  0x3UL
#define GITR_PENDBASER_ADDR_SHIFT         16
#define GITR_PENDBASER_ADDR_MASK          0xFFFFFFFFFUL
#define GITR_PENDBASER_OUTER_CACHE_SHIFT  56
#define GITR_PENDBASER_OUTER_CACHE_MASK   0x7UL
#define GITR_PENDBASER_PTZ                BIT64(62)

/* GITCD_IROUTER */
#define GIC_DIST_IROUTER                  0x6000
#define IROUTER(base, n)                  (base + GIC_DIST_IROUTER + (n)*8)

/*
 * ITS registers, offsets from ITS_base
 */
#define GITS_CTLR                         0x0000
#define GITS_IIDR                         0x0004
#define GITS_TYPER                        0x0008
#define GITS_STATUSR                      0x0040
#define GITS_UMSIR                        0x0048
#define GITS_CBASER                       0x0080
#define GITS_CWRITER                      0x0088
#define GITS_CREADR                       0x0090
#define GITS_BASER(n)                     (0x0100 + ((n)*8))

#define GITS_TRANSLATER \
    0x10040 /* The interrupt translation space, which is located at ITS translation register (ITS_base + 0x010000) */

/* ITS CTLR register */
#define GITS_CTLR_ENABLED_SHIFT             0U
#define GITS_CTLR_ENABLED_MASK              0x1U
#define GITS_CTLR_ITS_NUMBER_SHIFT          4
#define GITS_CTLR_ITS_NUMBER_MASK           0xfUL
#define GITS_CTLR_QUIESCENT_SHIFT           31
#define GITS_CTLR_QUIESCENT_MASK            0x1UL

#define GITS_CTLR_ENABLED_GET(_val)         MASK_GET(_val, GITS_CTLR_ENABLED)
#define GITS_CTLR_QUIESCENT_GET(_val)       MASK_GET(_val, GITS_CTLR_QUIESCENT)

/* ITS TYPER register */
#define GITS_TYPER_PHY_SHIFT                0
#define GITS_TYPER_PHY_MASK                 0x1UL
#define GITS_TYPER_VIRT_SHIFT               1
#define GITS_TYPER_VIRT_MASK                0x1UL
#define GITS_TYPER_ITT_ENTRY_SIZE_SHIFT     4
#define GITS_TYPER_ITT_ENTRY_SIZE_MASK      0xfUL
#define GITS_TYPER_IDBITS_SHIFT             8
#define GITS_TYPER_IDBITS_MASK              0x1fUL
#define GITS_TYPER_DEVBITS_SHIFT            13
#define GITS_TYPER_DEVBITS_MASK             0x1fUL
#define GITS_TYPER_SEIS_SHIFT               18
#define GITS_TYPER_SEIS_MASK                0x1UL
#define GITS_TYPER_PTA_SHIFT                19
#define GITS_TYPER_PTA_MASK                 0x1UL
#define GITS_TYPER_HCC_SHIFT                24
#define GITS_TYPER_HCC_MASK                 0xffUL
#define GITS_TYPER_CIDBITS_SHIFT            32
#define GITS_TYPER_CIDBITS_MASK             0xfUL
#define GITS_TYPER_CIL_SHIFT                36
#define GITS_TYPER_CIL_MASK                 0x1UL

#define GITS_TYPER_ITT_ENTRY_SIZE_GET(_val) MASK_GET(_val, GITS_TYPER_ITT_ENTRY_SIZE)
#define GITS_TYPER_PTA_GET(_val)            MASK_GET(_val, GITS_TYPER_PTA)
#define GITS_TYPER_HCC_GET(_val)            MASK_GET(_val, GITS_TYPER_HCC)
#define GITS_TYPER_DEVBITS_GET(_val)        MASK_GET(_val, GITS_TYPER_DEVBITS)
#define GITS_TYPER_IDBITS_GET(_val)         MASK_GET(_val, GITS_TYPER_IDBITS)

/* ITS COMMON BASER / CBASER register */

/* ITS CBASER register */
#define GITS_CBASER_SIZE_SHIFT              0
#define GITS_CBASER_SIZE_MASK               0xffUL
#define GITS_CBASER_SHAREABILITY_SHIFT      10
#define GITS_CBASER_SHAREABILITY_MASK       0x3UL
#define GITS_CBASER_ADDR_SHIFT              12
#define GITS_CBASER_ADDR_MASK               0xfffffffffUL
#define GITS_CBASER_OUTER_CACHE_SHIFT       53
#define GITS_CBASER_OUTER_CACHE_MASK        0x7UL
#define GITS_CBASER_INNER_CACHE_SHIFT       59
#define GITS_CBASER_INNER_CACHE_MASK        0x7UL
#define GITS_CBASER_VALID_SHIFT             63
#define GITS_CBASER_VALID_MASK              0x1UL

/* ITS BASER<n> register */
#define GITS_BASER_SIZE_SHIFT               0
#define GITS_BASER_SIZE_MASK                0xffULL
#define GITS_BASER_PAGE_SIZE_SHIFT          8
#define GITS_BASER_PAGE_SIZE_MASK           0x3ULL
#define GITS_BASER_PAGE_SIZE_4K             0
#define GITS_BASER_PAGE_SIZE_16K            1
#define GITS_BASER_PAGE_SIZE_64K            2
#define GITS_BASER_SHAREABILITY_SHIFT       10
#define GITS_BASER_SHAREABILITY_MASK        0x3ULL
#define GITS_BASER_ADDR_SHIFT               12
#define GITS_BASER_ADDR_MASK                0xfffffffff
#define GITS_BASER_ENTRY_SIZE_SHIFT         48
#define GITS_BASER_ENTRY_SIZE_MASK          0x1fULL
#define GITS_BASER_OUTER_CACHE_SHIFT        53
#define GITS_BASER_OUTER_CACHE_MASK         0x7ULL
#define GITS_BASER_TYPE_SHIFT               56
#define GITS_BASER_TYPE_MASK                0x7ULL
#define GITS_BASER_INNER_CACHE_SHIFT        59
#define GITS_BASER_INNER_CACHE_MASK         0x7ULL
#define GITS_BASER_INDIRECT_SHIFT           62
#define GITS_BASER_INDIRECT_MASK            0x1ULL
#define GITS_BASER_VALID_SHIFT              63
#define GITS_BASER_VALID_MASK               0x1ULL

#define GITS_BASER_TYPE_NONE                0
#define GITS_BASER_TYPE_DEVICE              1
#define GITS_BASER_TYPE_COLLECTION          4

#define GITS_BASER_TYPE_GET(_val)           MASK_GET(_val, GITS_BASER_TYPE)
#define GITS_BASER_PAGE_SIZE_GET(_val)      MASK_GET(_val, GITS_BASER_PAGE_SIZE)
#define GITS_BASER_ENTRY_SIZE_GET(_val)     MASK_GET(_val, GITS_BASER_ENTRY_SIZE)
#define GITS_BASER_INDIRECT_GET(_val)       MASK_GET(_val, GITS_BASER_INDIRECT)

#define GITS_BASER_NR_REGS                  8

/* ITS Commands */

#define GITS_CMD_ID_MOVI                    0x01
#define GITS_CMD_ID_INT                     0x03
#define GITS_CMD_ID_CLEAR                   0x04
#define GITS_CMD_ID_SYNC                    0x05
#define GITS_CMD_ID_MAPD                    0x08
#define GITS_CMD_ID_MAPC                    0x09
#define GITS_CMD_ID_MAPTI                   0x0a
#define GITS_CMD_ID_MAPI                    0x0b
#define GITS_CMD_ID_INV                     0x0c
#define GITS_CMD_ID_INVALL                  0x0d
#define GITS_CMD_ID_MOVALL                  0x0e
#define GITS_CMD_ID_DISCARD                 0x0f

#define GITS_CMD_ID_OFFSET                  0
#define GITS_CMD_ID_SHIFT                   0
#define GITS_CMD_ID_MASK                    0xffUL

#define GITS_CMD_DEVICEID_OFFSET            0
#define GITS_CMD_DEVICEID_SHIFT             32
#define GITS_CMD_DEVICEID_MASK              0xffffffffUL

#define GITS_CMD_SIZE_OFFSET                1
#define GITS_CMD_SIZE_SHIFT                 0
#define GITS_CMD_SIZE_MASK                  0x1fUL

#define GITS_CMD_EVENTID_OFFSET             1
#define GITS_CMD_EVENTID_SHIFT              0
#define GITS_CMD_EVENTID_MASK               0xffffffffUL

#define GITS_CMD_PINTID_OFFSET              1
#define GITS_CMD_PINTID_SHIFT               32
#define GITS_CMD_PINTID_MASK                0xffffffffUL

#define GITS_CMD_ICID_OFFSET                2
#define GITS_CMD_ICID_SHIFT                 0
#define GITS_CMD_ICID_MASK                  0xffffUL

#define GITS_CMD_ITTADDR_OFFSET             2
#define GITS_CMD_ITTADDR_SHIFT              8
#define GITS_CMD_ITTADDR_MASK               0xffffffffffUL
#define GITS_CMD_ITTADDR_ALIGN              GITS_CMD_ITTADDR_SHIFT
#define GITS_CMD_ITTADDR_ALIGN_SZ           (BIT(0) << GITS_CMD_ITTADDR_ALIGN)

#define GITS_CMD_RDBASE_OFFSET              2
#define GITS_CMD_RDBASE_SHIFT               16
#define GITS_CMD_RDBASE_MASK                0xffffffffUL
#define GITS_CMD_RDBASE_ALIGN               GITS_CMD_RDBASE_SHIFT

#define GITS_CMD_VALID_OFFSET               2
#define GITS_CMD_VALID_SHIFT                63
#define GITS_CMD_VALID_MASK                 0x1UL

FError FGicItsInit(FGic *instanse_p);
void FGicLpisSetup(FGic *instanse_p);
FError FGicItsDeviceIdInit(const FGic *instanse_p, u32 device_id, u32 nites);
FError FGicItsDeviceIdDeinit(const FGic *instanse_p, u32 device_id);

FError FGicItsMapIntId(const FGic *instanse_p, u32 device_id, u32 event_id, u32 intid);
void FGicItsLpiSetup(const FGic *instanse_p, u32 intid, u8 enable);
void FGicItsLpiSetPriority(const FGic *instanse_p, u32 intid, u32 prio);
FError FGicItsSendIntCmd(FGicItsData *data, u32 device_id, u32 event_id);
u32 FGicItsGetTransAddr(const FGic *instanse_p);

u32 FGicItsLpiIdAlloc(FGic *instanse_p);
void FGicItsLpiIdFree(FGic *instanse_p, u32 intid);

u8 FGicItsLpiGetPriority(const FGic *instanse_p, u32 intid);

#endif
