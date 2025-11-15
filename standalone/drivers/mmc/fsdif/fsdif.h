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
 * FilePath: fsdif.h
 * Date: 2022-05-26 16:20:52
 * LastEditTime: 2022-05-26 16:20:53
 * Description:  This file is for sdif user interface definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 * 1.1   zhugengyu  2022/6/6     modify according to tech manual.
 * 1.2   zhugengyu  2022/7/15    adopt to e2000
 * 1.3   zhugengyu  2022/11/23   fix multi-block rw issues
 * 2.0   zhugengyu  2023/9/16    rename as sdif, support SD 3.0 and rework clock timing
 * 2.1   zhugengyu  2023/10/23   add sdio interrupt handler
 * 3.0   zhugengyu  2024/12/17   adopt to bsd drivers
 */

#ifndef FSDIF_H
#define FSDIF_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "ferror_code.h"
#include "fdrivers_port.h"

#ifdef __cplusplus
extern "C"
{
#endif
/************************** Constant Definitions *****************************/

/* SDIF driver error code */
#define FSDIF_SUCCESS             FT_SUCCESS
#define FSDIF_ERR_TIMEOUT         FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 1)
#define FSDIF_ERR_NOT_INIT        FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 2)
#define FSDIF_ERR_SHORT_BUF       FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 3)
#define FSDIF_ERR_NOT_SUPPORT     FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 4)
#define FSDIF_ERR_INVALID_STATE   FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 5)
#define FSDIF_ERR_TRANS_TIMEOUT   FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 6)
#define FSDIF_ERR_CMD_TIMEOUT     FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 7)
#define FSDIF_ERR_NO_CARD         FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 8)
#define FSDIF_ERR_BUSY            FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 9)
#define FSDIF_ERR_DMA_BUF_UNALIGN FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 10)
#define FSDIF_ERR_INVALID_TIMING  FT_MAKE_ERRCODE(ErrModBsp, ErrBspMmc, 11)

typedef enum
{
    FSDIF_IDMA_TRANS_MODE, /* DMA trans mode */
    FSDIF_PIO_TRANS_MODE,  /* NO-DMA trans by read/write Fifo */
} FSdifTransMode;          /* SDIF trans mode */

typedef enum
{
    FSDIF_GENERAL_INTR, /* interrupt status belongs to controller */
    FSDIF_IDMA_INTR,    /* interrupt status belongs to DMA */
} FSdifIntrType;        /* SDIF interrupt status type */

typedef enum
{
    FSDIF_EVT_CARD_DETECTED = 0, /* card detected event */
    FSDIF_EVT_CMD_DONE,          /* cmd transfer finish event */
    FSDIF_EVT_DATA_DONE,         /* cmd with data transfer finish event */
    FSDIF_EVT_SDIO_IRQ,          /* sdio card customized event */
    FSDIF_EVT_ERR_OCCURE,        /* error occurred in transfer */

    FSDIF_NUM_OF_EVT
} FSdifEvtType; /* SDIF event type */

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
typedef struct _FSdif FSdif;
typedef struct _FSdifTiming FSdifTiming;

typedef void (*FSdifRelaxHandler)(void);
typedef void (*FSdifEvtHandler)(FSdif *const instance_p, void *args, u32 status, u32 dmac_status);

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
#define FSDIF_IDMAC_BUF_SIZE_MAX 0x1fffU
    u32 non2;    /* des3 --> unused */
    u32 addr_lo; /* des4 Lower 32-bits of Buffer Address Pointer 1 --> buffer 1 */
    u32 addr_hi; /* des5 Upper 32-bits of Buffer Address Pointer 1 */
/* Each descriptor can transfer up to 4KB of data in chained mode */
#define FSDIF_IDMAC_MAX_BUF_SIZE 0x1000U
    u32 desc_lo; /* des6 Lower 32-bits of Next Descriptor Address --> buffer 2 */
    u32 desc_hi; /* des7 Upper 32-bits of Next Descriptor Address */
} __attribute__((packed)) __attribute((aligned(4))) FSdifIDmaDesc; /* SDIF DMA descriptr */

typedef struct
{
    FSdifIDmaDesc *first_desc; /* virtual address of first descriptor */
    uintptr first_desc_dma;    /* physical address of first descriptor */
#define FSDIF_DMA_DESC_ALIGMENT (4U)
    u32 desc_num;      /* num of descriptors in the list */
    u32 desc_trans_sz; /* num of bytes transfer by one descriptor */
} FSdifIDmaDescList;   /* SDIF DMA descriptors list */

typedef struct
{
    /* buffer address shall aligned with transfer block size */
    void *buf; /* virtual address of buffer for memory access */
#define FSDIF_DMA_BUF_ALIGMENT (4U)
#ifndef FIO_SCATTERED_DMA
    uintptr buf_dma; /* physical address for DMA */
#endif
    u32 blksz; /* card block size */
#define FSDIF_BLOCK_SIZE 512U
    u32 blkcnt;  /* num of block in trans */
    u32 datalen; /* bytes in trans */
} FSdifData;     /* SDIF trans data */

typedef struct
{
    u32 cmdidx; /* command index */
#define FSDIF_SWITCH_VOLTAGE 11U
#define FSDIF_EXT_APP_CMD    55U
    u32 cmdarg;        /* command argument */
    u32 rawcmd;        /* rawcommand writes to register */
    u32 response[4];   /* command response buffer */
    FSdifData *data_p; /* SDIF trans data */
} FSdifCmdData;        /* SDIF trans command and data */

typedef struct _FSdifTiming
{
    boolean is_valid;
    boolean use_hold; /* 1: CMD[29] = 1, equal to add one sampling time delay */
    u32 clk_div;      /* for reg 0x8 */
    u32 clk_src;      /* for reg 0x108 */
    u32 shift;        /* for reg 0x110 */
    u32 clk_divider;  /* for reg 0x114 */
    void (*pad_delay)(u32 id); /* method to adjust pad delay */
} FSdifTiming;                 /* SDIF timing configuration */

typedef struct
{
    u32 instance_id;           /* Device instance id */
    uintptr base_addr;         /* Device base address */
    u32 irq_num;               /* Interrupt num */
    FSdifTransMode trans_mode; /* Trans mode, PIO/DMA */
    boolean non_removable;     /* No removeable media, e.g eMMC */
    u32 src_clk_rate;          /* rate of source clock */
} FSdifConfig;                 /* SDIF intance configuration */

typedef struct _FSdif
{
    FSdifConfig config;          /* Current active configs */
    u32 is_ready;                /* Device is initialized and ready */
    FSdifIDmaDescList desc_list; /* DMA descriptor list, valid in DMA trans mode */
    FSdifEvtHandler evt_handlers[FSDIF_NUM_OF_EVT]; /* call-backs for interrupt event */
    void *evt_args[FSDIF_NUM_OF_EVT];               /* arguments for event call-backs */
    FSdifRelaxHandler relax_handler;
    boolean card_need_init;
    u32 cur_clock;
    u32 prev_cmd; /* record previous command code */
    FSdifCmdData *cur_cmd;
} FSdif; /* SDIF intance */
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Get the device instance default configure  */
const FSdifConfig *FSdifLookupConfig(u32 instance_id);

/* initialization SDIF controller instance */
FError FSdifCfgInitialize(FSdif *const instance_p, const FSdifConfig *cofig_p);

/* deinitialization SDIF controller instance */
void FSdifDeInitialize(FSdif *const instance_p);

/* Setup DMA descriptor for SDIF controller instance */
FError FSdifSetIDMAList(FSdif *const instance_p, FSdifIDmaDesc *desc, uintptr desc_dma, u32 desc_num);

/* Set the Card clock freq with fixed timing */
FError FSdifSetClkFreqByDict(FSdif *const instance_p, boolean is_ddr,
                             const FSdifTiming *target_timing, u32 input_clk_hz);

/* Set the Card clock freq with caclcuated timing */
FError FSdifSetClkFreqByCalc(FSdif *const instance_p, boolean is_ddr, u32 input_clk_hz);

/* Check if card is in present */
boolean FSdifCheckCardExists(FSdif *const instance_p);

/* Check if card is in busy */
boolean FSdifCheckCardBusy(FSdif *const instance_p);

/* Power on/off card */
void FSdifSetCardPower(FSdif *const instance_p, boolean power_on);

/* Set card bus width */
void FSdifSetCardBusWidth(FSdif *const instance_p, uint32_t bus_width);

/* Set card DDR/SDR mode */
void FSdifSetCardDDRMode(FSdif *const instance_p, boolean enable);

#ifndef FIO_SCATTERED_DMA
/* Fill transfer descriptors with transfer buffer */
FError FSdifSetupDMADescriptor(FSdif *const instance_p, FSdifData *data_p);
#endif

/* Start command and data transfer in DMA mode */
FError FSdifDMATransfer(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);

/* Wait transfer finished by poll */
FError FSdifPollWaitTransferEnd(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);

/* Start command and data transfer in PIO mode */
FError FSdifPIOTransfer(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);

/* Get cmd response and received data after wait poll status or interrupt signal */
FError FSdifGetCmdResponse(FSdif *const instance_p, FSdifCmdData *const cmd_data_p);

/* Get SDIF controller interrupt mask */
u32 FSdifGetInterruptMask(FSdif *const instance_p, FSdifIntrType intr_type);

/* Enable/Disable SDIF controller interrupt */
void FSdifSetInterruptMask(FSdif *const instance_p, FSdifIntrType type, u32 set_mask, boolean enable);

/* Interrupt handler for SDIF instance */
void FSdifInterruptHandler(s32 vector, void *param);

/* Reset controller from error state */
FError FSdifRestart(FSdif *const instance_p);

/* Register event call-back function as handler for interrupt events */
void FSdifRegisterEvtHandler(FSdif *const instance_p, FSdifEvtType evt,
                             FSdifEvtHandler handler, void *handler_arg);

/* Register sleep call-back function */
void FSdifRegisterRelaxHandler(FSdif *const instance_p, FSdifRelaxHandler relax_handler);

/* Dump all register value of SDIF instance */
void FSdifDumpRegister(uintptr base_addr);

/* Dump command and data info */
void FSdifDumpCmdInfo(FSdifCmdData *const cmd_data);

#ifdef __cplusplus
}
#endif


#endif
