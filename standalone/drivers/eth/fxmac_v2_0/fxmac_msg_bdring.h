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
 * FilePath: fxmac_msg_bdring.h
 * Date: 2024-10-28 14:46:52
 * LastEditTime: 2024-10-28 14:46:52
 * Description:  This file ontains DMA channel related structure and constant definition
 * as well as function prototypes.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huangjin   2024/10/28  first release
 */

#ifndef FXMAC_MSG_BDRING_H
#define FXMAC_MSG_BDRING_H

#include "ftypes.h"
#include "ferror_code.h"
#include "fxmac_msg_bd.h"
#include "fxmac_msg.h"
#ifdef __cplusplus
extern "C"
{
#endif

/**************************** Type Definitions *******************************/
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define lower_32_bits(n) ((u32)((n)&0xffffffff))

/************************** Function Prototypes ******************************/

/*
 * Scatter gather DMA related functions in fxmac_msg_bdring.c
 */

/**************************** Type Definitions *******************************/

/** This is an internal structure used to maintain the DMA list */
typedef struct
{
    uintptr phys_base_addr; /* Physical address of 1st BD in list */
    uintptr base_bd_addr;   /* Virtual address of 1st BD in list */
    uintptr high_bd_addr;   /* Virtual address of last BD in the list */
    u32 length;             /* Total size of ring in bytes */
    u32 run_state;          /* Flag to indicate DMA is started */
    u32 separation;         /* Number of bytes between the starting address
                                  of adjacent BDs */
    FXmacMsgBd *free_head;
    /* First BD in the free group */
    FXmacMsgBd *pre_head; /* First BD in the pre-work group */
    FXmacMsgBd *hw_head;  /* First BD in the work group */
    FXmacMsgBd *hw_tail;  /* Last BD in the work group */
    FXmacMsgBd *post_head;
    /* First BD in the post-work group */
    FXmacMsgBd *bda_restart;
    /* BDA to load when channel is started */

    volatile u32 hw_cnt; /* Number of BDs in work group */
    u32 pre_cnt;         /* Number of BDs in pre-work group */
    u32 free_cnt;        /* Number of allocatable BDs in the free group */
    u32 post_cnt;        /* Number of BDs in post-work group */
    u32 all_cnt;         /* Total Number of BDs for channel */
} FXmacMsgBdRing;

typedef struct
{
    u32 queue_id;
    FXmacMsgBdRing bdring;
} FXmacMsgQueueV1;

/**
 * @name: FXMAC_MSG_BD_RING_NEXT
 * @msg:  Return the next BD from bd_ptr in a list.
 *
 * @param  ring_ptr is the DMA channel to operate on.
 * @param  bd_ptr is the BD to operate on.
 * @return The next BD in the list relative to the bd_ptr parameter.
 */
#define FXMAC_MSG_BD_RING_NEXT(ring_ptr, bd_ptr)               \
    (((uintptr)((void *)(bd_ptr)) >= (ring_ptr)->high_bd_addr) \
         ? (FXmacMsgBd *)((void *)(ring_ptr)->base_bd_addr)    \
         : (FXmacMsgBd *)((uintptr)((void *)(bd_ptr)) + (ring_ptr)->separation))

/**
 * @name: FXMAC_MSG_BD_RING_CNT_CALC
 * @msg:  Use this macro at initialization time to determine how many BDs will fit
 * in a BD list within the given memory constraints.
 *
 * @param alignment specifies what byte alignment the BDs must fall on and
 *        must be a power of 2 to get an accurate calculation (32, 64, 128,...)
 * @param Bytes is the number of bytes to be used to store BDs.
 * @return Number of BDs that can fit in the given memory area
 */
#define FXMAC_MSG_BD_RING_CNT_CALC(alignment, Bytes) \
    (u32)((Bytes) / (sizeof(FXmacMsgBd)))

/**
 * @name: FXMAC_MSG_BD_RING_MEM_CALC
 * @msg:  Use this macro at initialization time to determine how many bytes of memory
 * is required to contain a given number of BDs at a given alignment.
 * @param alignment specifies what byte alignment the BDs must fall on. This
 *        parameter must be a power of 2 to get an accurate calculation (32, 64,
 *        128,...)
 * @param num_bd is the number of BDs to calculate memory size requirements for
 * @return The number of bytes of memory required to create a BD list with the
 *         given memory constraints.
 */
#define FXMAC_MSG_BD_RING_MEM_CALC(alignment, num_bd) \
    (u32)(sizeof(FXmacMsgBd) * (num_bd))

/**
 * @name: FXMAC_MSG_BD_RING_GET_CNT
 * @msg:  Return the total number of BDs allocated by this channel with
 * FXmacMsgBdRingCreate().
 * @param  ring_ptr is the DMA channel to operate on.
 * @return The total number of BDs allocated for this channel.
 */
#define FXMAC_MSG_BD_RING_GET_CNT(ring_ptr)      ((ring_ptr)->all_cnt)

/**
 * @name: FXMAC_MSG_BD_RING_GET_FREE_CNT
 * @msg:  Return the number of BDs allocatable with FXmacMsgBdRingAlloc() for pre-
 * processing.
 *
 * @param  ring_ptr is the DMA channel to operate on.
 * @return The number of BDs currently allocatable.
 */
#define FXMAC_MSG_BD_RING_GET_FREE_CNT(ring_ptr) ((ring_ptr)->free_cnt)

/**
 * @name: FXMAC_MSG_BD_RING_PREV
 * @msg:  Return the previous BD from bd_ptr in the list.
 * @param  ring_ptr is the DMA channel to operate on.
 * @param  bd_ptr is the BD to operate on
 * @return The previous BD in the list relative to the bd_ptr parameter.
 */
#define FXMAC_MSG_BD_RING_PREV(ring_ptr, bd_ptr)     \
    (((uintptr)(bd_ptr) <= (ring_ptr)->base_bd_addr) \
         ? (FXmacMsgBd *)(ring_ptr)->high_bd_addr    \
         : (FXmacMsgBd *)((uintptr)(bd_ptr) - (ring_ptr)->separation))

/************************** Function Prototypes ******************************/

/*
 * Scatter gather DMA related functions in FXmacMsgBdring.c
 */
FError FXmacMsgBdRingCreate(FXmacMsgBdRing *ring_ptr, uintptr phys_addr,
                            uintptr virt_addr, u32 alignment, u32 bd_count);
FError FXmacMsgBdRingClone(FXmacMsgBdRing *ring_ptr, FXmacMsgBd *src_bd_ptr, u8 direction);
FError FXmacMsgBdRingAlloc(FXmacMsgBdRing *ring_ptr, u32 num_bd, FXmacMsgBd **bd_set_ptr);
FError FXmacMsgBdRingUnAlloc(FXmacMsgBdRing *ring_ptr, u32 num_bd, FXmacMsgBd *bd_set_ptr);
FError FXmacMsgBdRingToHw(FXmacMsgBdRing *ring_ptr, u32 num_bd, FXmacMsgBd *bd_set_ptr);
FError FXmacMsgBdRingFree(FXmacMsgBdRing *ring_ptr, u32 num_bd, FXmacMsgBd *bd_set_ptr);
u32 FXmacMsgBdRingFromHwTx(FXmacMsgBdRing *ring_ptr, u32 bd_limit, FXmacMsgBd **bd_set_ptr);
u32 FXmacMsgBdRingFromHwRx(FXmacMsgBdRing *ring_ptr, u32 bd_limit, FXmacMsgBd **bd_set_ptr);
FError FXmacMsgBdRingCheck(FXmacMsgBdRing *ring_ptr, u8 direction);

void FXmacMsgBdringPtrReset(FXmacMsgBdRing *ring_ptr, void *virt_addrloc);

#ifdef __cplusplus
}
#endif

#endif