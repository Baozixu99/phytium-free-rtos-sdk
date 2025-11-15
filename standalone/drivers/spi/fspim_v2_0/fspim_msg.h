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
 * FilePath: fspim_msg.h
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi iop api interface and predefined macro.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */


#ifndef FSPIM_MSG_H
#define FSPIM_MSG_H

#ifdef __cplusplus
extern "C"
{
#endif


/***************************** Include Files *********************************/
#include "fparameters.h"
#include "ferror_code.h"
#include "fassert.h"
#include "fmsg_common.h"
#include "fdrivers_port.h"


#define FMSG_MAX_DATA_LENGTH               56


/***************************** Spi Flash Cmd **********/
#define FSPI_FLASH_CMD_WRITE_ENABLE        0x06
#define FSPI_FLASH_CMD_ERASE_4K            0x20
#define FSPI_FLASH_CMD_ERASE_CHIP          0xc7
#define FSPI_FLASH_CMD_READ_DATA           0x03
#define FSPI_FLASH_CMD_READ_FAST           0x0b
#define FSPI_FLASH_CMD_READ_4B             0x13
#define FSPI_FLASH_CMD_FAST_READ_4B        0x0c
#define FSPI_FLASH_CMD_PAGE_PROGRAM        0x02
#define FSPI_FLASH_CMD_PAGE_PROGRAM_4B     0x12
#define FSPI_FLASH_CMD_READ_STATUS_REG     0x05
#define FSPI_FLASH_CMD_WRITE_DISABLE       0x04

#define FSPI_FLASH_STATUS_REG_WEL_VALUE    0x02

#define CMD_MSG_NOT_COMPLETION             0
#define CMD_MSG_COMPLETION                 1

#define FLASH_PAGE_SIZE                    256
#define SPI_TRANS_DATA_SIZE                1024

/* status0 */
/* 暂未响应或命令超时 */
#define FSPI_MSG_STATUS0_NOT_READY         0x00
/* 命令执行成功 */
#define FSPI_MSG_STATUS0_SUCCESS           0x01
/* 命令解析成功，正在执行过程中*/
#define FSPI_MSG_STATUS0_GOING             0x02
/* 未知错误，未定义的错误*/
#define FSPI_MSG_STATUS0_GENERIC_ERROR     0x10
/* 对端不支持该条类型*/
#define FSPI_MSG_STATUS0_TYPE_NOT_SUPPORT  0x11
/* 对端不支持该条命令 */
#define FSPI_MSG_STATUS0_CMD_NOT_SUPPORT   0x12
/* 一个或多个参数解析不正确 */
#define FSPI_MSG_STATUS0_INVALID_PARAMETER 0x13


typedef struct
{
    u8 reserved; /*取消模块id，作为保留字段，必须为0值*/
    u8 seq;      /*由发起方使用或维护，对端不改变该值，默认为0*/
    u8 cmd_type; /*功能性命令字段，可以用极少位表示，当前保留为8bit*/
    u8 cmd_subid; /*子命令id，与命令实现有关，最多为256个，搭配cmd_type补充*/
    u16 len;    /*在交互数据时，表示payload的长度，交互命令时为0*/
    u8 status1; /*保留字段，各模块根据需要定义，可返回某些函数执行后的返回值*/
    u8 status0; /*协议命令状态，表示命令执行相应*/
    u8 para[FMSG_MAX_DATA_LENGTH]; /*传输数据*/
} FSpiMsgInfo;


/* status1 */
typedef enum
{
    ERR_SPI_SUCEESS = 0,
    ERR_SPI_BUSY,                /* spi 总线繁忙 */
    ERR_SPI_DMA_QUEUE,           /* DMA队列传输异常 */
    ERR_SPI_DMA_TIMEOUT,         /* DMA传输超时 */
    ERR_SPI_DEVT_TIMEOUT,        /* flash设备操作超时 */
    ERR_SPI_DMA_TXCHAN,          /* spi tx通道DMA初始化失败*/
    ERR_SPI_DMA_RXCHAN,          /* spi rx通道DMA初始化失败*/
    ERR_SPI_DMA_QUEUE_INIT_FAIL, /* DMA队列初始化失败 */
} SpiStatus1ErrorCode;

typedef enum
{
    FSPI_MSG_INTR_EVT_NUM,
} FSpiMsgEvtType;

typedef enum
{
    FSPI_MSG_SLAVE_DEV_0 = 0,
    FSPI_MSG_SLAVE_DEV_1,
    FSPI_MSG_SLAVE_DEV_2,
    FSPI_MSG_SLAVE_DEV_3,
    FSPIM_NUM_OF_SLAVE_DEV
} FSpiMsgSlaveDevice;


typedef enum
{
    FSPI_MSG_SUBCMD_SET_MODULE_EN = 0,
    FSPI_MSG_SUBCMD_SET_DATA_WIDTH, /* data width*/
    FSPI_MSG_SUBCMD_SET_MODE,       /*cpol, cpha*/
    FSPI_MSG_SUBCMD_SET_TMOD,       /*transfer mode*/
    FSPI_MSG_SUBCMD_SET_BAUDR,      /*set clk div*/
    FSPI_MSG_SUBCMD_SET_INI_TI,
    FSPI_MSG_SUBCMD_SET_NDF,
    FSPI_MSG_SUBCMD_SET_CS,
    FSPI_MSG_SUBCMD_SET_INTERRUPT, /*interrupt enable/disable*/
} FSpiMsgSetSubCmd;

typedef enum
{
    FSPI_MSG_SUBCMD_DATA_TX = 0,
    FSPI_MSG_SUBCMD_DATA_RX,
    FSPI_MSG_SUBCMD_DATA_FLASH_TX,
    FSPI_MSG_SUBCMD_DATA_FLASH_ERASE,
    FSPI_MSG_SUBCMD_DATA_DMA_TX,
    FSPI_MSG_SUBCMD_DATA_DMA_RX,
    FSPI_MSG_SUBCMD_DATA_FLASH_DMA_TX,

} FSpiMsgDataSubCmd;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct
{
    u32 instance_id;                 /* Device instance id */
    u32 irq_num;                     /* Device interrupt id */
    u32 irq_priority;                /* Device interrupt priority */
    FSpiMsgSlaveDevice slave_dev_id; /* Slave device id */
    u32 clk_div;                     /* set clk div */
    u8 n_bytes;                      /* data width */
    u8 mode;                         /* bit[1]: cpol, bit[0]: cpha */
    u8 tmod;                         /* tx and rx, rx only, tx_only */
    FDevMsg spi_msg;
    boolean en_dma; /* enable dma */
} FSpiMsgConfig;

typedef void (*FSpiMsgEvtHandler)(void *spi_msg_ctrl, void *param);

typedef struct
{
    void *tx_shmem_addr;
    void *rx_shmem_addr;
    FSpiMsgConfig spi_msg_config;
    FSpiMsgInfo *msg;
    u32 trans_len;
    void *tx;
    void *tx_end;
    void *rx;
    void *rx_end;

    u32 cmd_completion;
    u32 is_ready;

    FSpiMsgEvtHandler evt_handler[FSPI_MSG_INTR_EVT_NUM]; /* event handler for interrupt */
    void *evt_param[FSPI_MSG_INTR_EVT_NUM]; /* parameters ptr of event handler */

    u8 flash_cmd;

    u8 addr_bytes;
} FSpiMsgCtrl;

/* 命令MSG传输完成中断处理函数*/
void FSpiMsgInterruptHandler(s32 vector, void *param);

/* 获取FSPIM_V2驱动的默认配置参数 */
const FSpiMsgConfig *FSpiMsgLookupConfig(u32 instance_id);

/* 完成FSPIM_V2驱动实例的初始化，使之可以使用*/
int FSpiMsgCfgInitialize(FSpiMsgCtrl *spi_msg_ctrl, const FSpiMsgConfig *spi_msg_config);

/* 完成FSPIM_V2驱动实例去使能，清零实例数据 */
int FSpiMsgDeInitialize(FSpiMsgCtrl *spi_msg_ctrl);

/* 使用FSPIM_V2驱动传输数据*/
int FSpiMsgTransfer(FSpiMsgCtrl *spi_msg_ctrl, const void *tx_buf, void *rx_buf, uint32_t len);

/* 展示当前发送的MSG命令*/
void FSpiMsgShowMsg(FSpiMsgInfo *msg);


#ifdef __cplusplus
}
#endif

#endif