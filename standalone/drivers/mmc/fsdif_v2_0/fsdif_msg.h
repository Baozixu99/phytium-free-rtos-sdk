/*
* Copyright : (C) 2025 Phytium Information Technology, Inc.
* All Rights Reserved.
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
* FilePath: fsdif_msg.h
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for sdif user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/4/10    init commit
*/

#ifndef FSDIF_MSG_H
#define FSDIF_MSG_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/

#include "ferror_code.h"
#include "fdrivers_port.h"
#include "fsdif_msg_common.h"

/************************** Constant Definitions *****************************/

/* SDIF driver error code */
#define FSDIF_SUCCESS             FT_SUCCESS
#define FSDIF_ERR_RING_FULL       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 1)
#define FSDIF_ERR_AP_MSG_FAIL     FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 2)
#define FSDIF_ERR_NO_MEDIUM       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 3)
#define FSDIF_ERR_TIMEOUT         FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 4)
#define FSDIF_ERR_NOT_INIT        FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 5)
#define FSDIF_ERR_SHORT_BUF       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 6)
#define FSDIF_ERR_DMA_BUF_UNALIGN FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 7)
#define FSDIF_ERR_NO_CARD         FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 8)
#define FSDIF_ERR_CARD_BUSY       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 9)
#define FSDIF_ERR_ILLEGAL_BYTE    FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 10)

typedef enum
{
    FSDIF_EVT_CARD_DETECTED = 0, /* card detected event */
    FSDIF_EVT_CMD_DONE,          /* cmd transfer finish event */
    FSDIF_EVT_DATA_DONE,         /* cmd with data transfer finish event */
    FSDIF_EVT_SDIO_IRQ,          /* sdio card customized event */
    FSDIF_EVT_ERR_OCCURE,        /* error occurred in transfer */

    FSDIF_NUM_OF_EVT
} FSdifMsgEvtType; /* SDIF event type */

typedef enum
{
    FSDIF_CLK_SPEED_400KHZ = 400000U,
    FSDIF_CLK_SPEED_25_MHZ = 25000000U,
    FSDIF_CLK_SPEED_26_MHZ = 26000000U, /* mmc */
    FSDIF_CLK_SPEED_50_MHZ = 50000000U,
    FSDIF_CLK_SPEED_52_MHZ = 52000000U, /* mmc */
    FSDIF_CLK_SPEED_66_MHZ = 66000000U, /* mmc */
    FSDIF_CLK_SPEED_100_MHZ = 100000000U,
} FSdifClkSpeed;
/**************************** Type Definitions *******************************/
typedef struct FSdifMsgCtrl_ FSdifMsgCtrl;

typedef struct
{
    u32 attribute;                   /* des0 */
#define FSDIF_IDMAC_DES0_DIC BIT(1)  /* 内部描述表不触发TI/RI中断 */
#define FSDIF_IDMAC_DES0_LD  BIT(2)  /* 数据的最后一个描述符 */
#define FSDIF_IDMAC_DES0_FD  BIT(3)  /* 数据的第一个描述符 */
#define FSDIF_IDMAC_DES0_CH  BIT(4)  /* 链接下一个描述符地址 */
#define FSDIF_IDMAC_DES0_ER  BIT(5)  /* 链表已经到达最后一个链表 */
#define FSDIF_IDMAC_DES0_CES BIT(30) /* RINTSTS寄存器错误汇总 */
#define FSDIF_IDMAC_DES0_OWN BIT(31) /* 描述符关联DMA，完成传输后该位置置0 */
    u32 non1;                        /* des1 --> unused */
    u32 len;                         /* des2 buffer size*/
    u32 non2;                        /* des3 --> unused */
    u32 addr_lo; /* des4 Lower 32-bits of Buffer Address Pointer 1 --> buffer 1 */
    u32 addr_hi; /* des5 Upper 32-bits of Buffer Address Pointer 1 */
/* Each descriptor can transfer up to 4KB of data in chained mode */
#define FSDIF_IDMAC_MAX_BUF_SIZE 0x1000U
    u32 desc_lo; /* des6 Lower 32-bits of Next Descriptor Address --> buffer 2 */
    u32 desc_hi; /* des7 Upper 32-bits of Next Descriptor Address */
} __attribute__((packed)) __attribute((aligned(4))) FSdifMsgIDmaDesc; /* SDIF DMA descriptr */

typedef struct
{
    FSdifMsgIDmaDesc *first_desc; /* virtual address of first descriptor */
    uintptr first_desc_dma;       /* physical address of first descriptor */
    u32 desc_num;                 /* num of descriptors in the list */
    u32 desc_trans_sz;            /* num of bytes transfer by one descriptor */
} FSdifMsgIDmaDescList;           /* SDIF DMA descriptors list */

typedef struct
{
    FSdifMsgDataStartCmd cmdinfo;
    u32 response[4]; /* command response buffer */
    FError error;
} FSdifMsgCommand;

typedef struct
{
    FSdifMsgDataStartData datainfo;
#define FSDIF_DMA_BUF_ALIGMENT (4U)
    void *buf;       /* virtual address of buffer for memory access */
    uintptr buf_dma; /* physical address for DMA */
    FError error;
} FSdifMsgData;

typedef struct
{
    FSdifMsgCommand *command;
    FSdifMsgData *data;
} FSdifMsgRequest;

typedef struct
{
    u32 instance_id; /* Device instance id */
    FSdifMsgDev dev_msg;
    u32 irq_num;
    boolean non_removable; /* No removeable media, e.g eMMC */
    FSdifMsgDataInit init;
} FSdifMsgConfig; /* SDIF intance configuration */

typedef void (*FSdifMsgEvtHandler)(FSdifMsgCtrl *const instance, void *args, void *data);

typedef struct FSdifMsgCtrl_
{
    FSdifMsgConfig config;          /* Current active configs */
    u32 is_ready;                   /* Device is initialized and ready */
    FSdifMsgIDmaDescList desc_list; /* DMA descriptor list, valid in DMA trans mode */

    FSdifMsgInfo sdif_msg;

    FSdifMsgCommand *cur_cmd;
    FSdifMsgData *cur_data;

    FSdifMsgEvtHandler evt_handlers[FSDIF_NUM_OF_EVT]; /* call-backs for interrupt event */
    void *evt_args[FSDIF_NUM_OF_EVT]; /* arguments for event call-backs */

    FSdifMsgDataSetIos cur_ios;
    FSdifMsgDataSwitchVolt cur_volt;

    boolean debug_enabled;
    boolean debug_alive_enabled;
    boolean host_need_init;
} FSdifMsgCtrl; /* SDIF intance */

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Get the device instance default configure  */
const FSdifMsgConfig *FSdifMsgLookupConfig(u32 instance_id);

/* initialization SDIF controller instance */
FError FSdifMsgCfgInitialize(FSdifMsgCtrl *const instance, const FSdifMsgConfig *cofig_p);

/* deinitialization SDIF controller instance */
void FSdifMsgDeInitialize(FSdifMsgCtrl *const instance);

/* Setup DMA descriptor for SDIF controller instance */
FError FSdifMsgSetIDMAList(FSdifMsgCtrl *const instance, FSdifMsgIDmaDesc *desc,
                           uintptr desc_dma, u32 desc_num);

#ifndef FIO_SCATTERED_DMA
/* Setup DMA descriptor for read/write data */
FError FSdifMsgSetupDMADescriptor(FSdifMsgCtrl *const instance, FSdifMsgData *data);
#endif

/* Start command and data transfer in DMA mode */
FError FSdifMsgDMATransfer(FSdifMsgCtrl *const instance, FSdifMsgRequest *const mrq);

/* Check if card is in present */
boolean FSdifMsgCheckifCardExists(FSdifMsgCtrl *const instance);

/* Check if card is in busy */
boolean FSdifMsgCheckifCardBusy(FSdifMsgCtrl *const instance);

/* Check if card is read-only */
boolean FSdifMsgCheckifCardReadonly(FSdifMsgCtrl *const instance);

/* Set card buswidth/clock/timing */
FError FSdifMsgSetIos(FSdifMsgCtrl *const instance, FSdifMsgDataSetIos *ios);

/* Set card voltage */
FError FSdifMsgSetVoltage(FSdifMsgCtrl *const instance, FSdifMsgDataSwitchVolt *volt);

/* Enable/disable sdio irq */
FError FSdifMsgEnableSdioIrq(FSdifMsgCtrl *const instance, boolean enable);

/* Interrupt handler for SDIF instance */
void FSdifMsgInterruptHandler(s32 vector, void *param);

/* Register event call-back function as handler for interrupt events */
void FSdifMsgRegisterEvtHandler(FSdifMsgCtrl *const instance, FSdifMsgEvtType evt,
                                FSdifMsgEvtHandler handler, void *handler_arg);


#ifdef __cplusplus
}
#endif

#endif /* FSDIF_V2_H */