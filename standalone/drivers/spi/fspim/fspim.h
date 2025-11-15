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
 * FilePath: fspim.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:08:38
 * Description:  This file is for providing spim basic api func and predefined variables.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/3   init commit
 * 1.1   zhugengyu  2022/4/15   support test mode
 * 1.2   zhugengyu  2022/5/13   support spi dma
 * 1.3 liqiaozhong 2022/12/30 add check func and spim option func
 * 1.4   huangjin   2023/12/26  adaptive pd2308
 */


#ifndef DRIVERS_FSPI_M_H
#define DRIVERS_FSPI_M_H

#ifdef __cplusplus
extern "C"
{
#endif

/***************************** Include Files *********************************/
#include "fparameters.h"
#include "ftypes.h"
#include "ferror_code.h"
#include "fassert.h"

/************************** Constant Definitions *****************************/

#define FSPIM_SUCCESS         FT_SUCCESS
#define FSPIM_ERR_INVAL_STATE FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 0)
#define FSPIM_ERR_NOT_READY   FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 1)
#define FSPIM_ERR_INVAL_PARAM FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 2)
#define FSPIM_ERR_BUS_BUSY    FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 3)
#define FSPIM_ERR_NOT_SUPPORT FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 4)
#define FSPIM_ERR_TIMEOUT     FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 5)
#define FSPIM_ERR_TRANS_FAIL  FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 6)
#define FSPIM_ERR_DMA_INIT    FT_MAKE_ERRCODE(ErrModBsp, ErrBspSpi, 6)

#define FSPIM_NUM_OF_ERR_CODE 8

typedef enum
{
    FSPIM_DEV_MASTER_MODE = 0 /* only support master mode */
} FSpimWorkMode;

typedef enum
{
    FSPIM_SLAVE_DEV_0 = 0,
    FSPIM_SLAVE_DEV_1,
    FSPIM_SLAVE_DEV_2,
    FSPIM_SLAVE_DEV_3,

    FSPIM_NUM_OF_SLAVE_DEV
} FSpimSlaveDevice;

typedef enum
{
    FSPIM_1_BYTE = 1,
    FSPIM_2_BYTE = 2,

    FSPIM_MAX_BYTES_NUM
} FSpimTransByte;

typedef enum
{
    FSPIM_TRANS_MODE_RX_TX = 0x0,
    FSPIM_TRANS_MODE_TX_ONLY = 0x1,
    FSPIM_TRANS_MODE_RX_ONLY = 0x2,
    FSPIM_TRANS_MODE_READ_EEPROM = 0x3,

    FSPIM_TRANS_MODE_MAX
} FSpimTransMode;

/*
    CPOL = 0, CPHA = 0, sample at the first rising edge
    CPOL = 1, CPHA = 1, sample at the second rising edge
    CPOL = 1, CPHA = 0, sample at the second falling edge
    CPOL = 0, CPHA = 1, sample at the first falling edge
*/

typedef enum
{
    FSPIM_CPOL_LOW = 0, /* pharse 0 CPOL=0 */
    FSPIM_CPOL_HIGH     /* pharse 1 CPOL=1 */
} FSpimCpolType;

typedef enum
{
    FSPIM_CPHA_1_EDGE = 0, /* sample at the 1st edge, CPHA=0 */
    FSPIM_CPHA_2_EDGE      /* sample at the 2nd edge, CPHA=1 */
} FSpimCphaType;

typedef enum
{
    FSPIM_INTR_EVT_RX_DONE = 0,  /*  receive complete event */
    FSPIM_INTR_EVT_TX_OVERFLOW,  /* send FIFO overflow event */
    FSPIM_INTR_EVT_RX_UNDERFLOW, /* receive FIFO underflow event */
    FSPIM_INTR_EVT_RX_OVERFLOW,  /* receive FIFO overflow event */

    FSPIM_INTR_EVT_NUM
} FSpimIntrEvtType;


typedef enum
{
    FSPIM_CPOLTYPE_OPTION = 0,
    FSPIM_CPHATYPE_OPTION,
    FSPIM_FREQUENCY_OPTION,
    FSPIM_SLAVE_ID_OPTION, /* 修改config中的slave id，并激活控制器相应片选 */
} FSpimOption;


typedef enum
{
    TRANS_WAY_POLL = 0,
    TRANS_WAY_INTERRUPT,
    TRANS_WAY_DDMA,
    TRANS_WAY_END
} FSpimTransWay;
/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
    u32 instance_id;               /* Device instance id */
    uintptr base_addr;             /* Device base address */
    u32 irq_num;                   /* Device interrupt id */
    u32 irq_prority;               /* Device interrupt priority */
    FSpimWorkMode work_mode;       /* Device work mode */
    FSpimSlaveDevice slave_dev_id; /* Slave device id */
    u32 sclk_hz;                   /* Spi transfer clock frequency in Hz */
    FSpimTransByte n_bytes;        /* Bytes in transfer */
    FSpimCpolType cpol;            /* Polarity of the clock */
    FSpimCphaType cpha;            /* Phase of the clock */
    boolean en_test;               /* Enable test mode */
    boolean en_dma;                /* Enable DMA */
    u32 caps;
    FSpimTransWay trans_way; /*transfer way: poll, interrupt, ddma*/
    u32 tx_fifo_threshold;
    u32 rx_fifo_threshold;
    u32 rx_dma_level;
    u32 tx_dma_level;
} FSpimConfig;

typedef void (*FSpimEvtHandler)(void *instance_p, void *param);

/**
 * This typedef contains driver instance data. The user is required to allocate a
 * variable of this type for every device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct
{
    FSpimConfig config;  /* Current active configs */
    u32 is_ready;        /* Device is initialized and ready */
    u32 length;          /* Data length in transfer */
    const void *tx_buff; /* Tx buffer beg */
    void *rx_buff;       /* Rx buffer beg */
    u32 tx_fifo_len;     /* Depth of tx fifo */
    u32 rx_fifo_len;     /* Depth of rx fifo */
    FSpimEvtHandler evt_handler[FSPIM_INTR_EVT_NUM]; /* event handler for interrupt */
    void *evt_param[FSPIM_INTR_EVT_NUM]; /* parameters ptr of event handler */
    u32 tx_count;
    u32 rx_count;
} FSpim;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* fspim_sinit.c */
/* 获取FSPIM驱动的默认配置参数 */
const FSpimConfig *FSpimLookupConfig(u32 instance_id);

/* fspim.c */
/* 完成FSPIM驱动实例的初始化，使之可以使用*/
FError FSpimCfgInitialize(FSpim *instance_p, const FSpimConfig *cofig_p);

/* 完成FSPIM驱动实例去使能，清零实例数据 */
void FSpimDeInitialize(FSpim *instance_p);

/* FSPIM临时修改参数操作 */
FError FSpimSetOption(FSpim *instance_p, FSpimOption option, u32 value);

/* FSPIM获取某些参数 */
u32 FSpimGetOption(FSpim *instance_p, FSpimOption option);

/* 先发送后接收数据 (阻塞处理)，利用Fifo进行处理 */
FError FSpimTransferPollFifo(FSpim *instance_p, const void *tx_buf, void *rx_buf, fsize_t len);

/* 启动SPIM DMA数据传输 */
FError FSpimTransferDMA(FSpim *instance_p);

/* 控制片选信号 */
void FSpimSetChipSelection(FSpim *instance_p, boolean on);

/* fspim_intr.c */
/* 先发送后接收数据 (中断处理)，利用Fifo进行处理  */
FError FSpimTransferByInterrupt(FSpim *instance_p, const void *tx_buf, void *rx_buf, fsize_t len);

/* SPIM中断处理函数 */
void FSpimInterruptHandler(s32 vector, void *param);

/* 注册FSPIM中断事件处理函数 */
void FSpimRegisterInterruptHandler(FSpim *instance_p, FSpimIntrEvtType evt,
                                   FSpimEvtHandler handler, void *param);

/* 打印SPIM控制寄存器信息 */
void FSpimDumpRegister(uintptr base_addr);

#ifdef __cplusplus
}
#endif

#endif