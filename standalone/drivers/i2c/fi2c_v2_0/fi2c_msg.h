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
 * FilePath: fi2c_v2.0.h
 * Date: 2024-10-1 14:53:42
 * LastEditTime: 2024-10-10 08:36:14
 * Description:  This file is for  i2c driver header file.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangzq      2024/10/08  first commit
 */
#ifndef FI2C_MSG_H
#define FI2C_MSG_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fmsg_common.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
#define FI2C_MSG_DATA_LEN         56
#define FI2C_MSG_NOT_INIT         FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2cMsg, 1)

#define FI2C_MSG_PARA_ERR         FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2cMsg, 2)
#define FI2C_MSG_BUF_ERR          FT_MAKE_ERRCODE(ErrModBsp, ErrBspI2cMsg, 3)
/*operation mode*/
#define FI2C_MSG_MASTER           0
#define FI2C_MSG_SLAVE            1

#define FI2C_MSG_RD               0x0001
#define FI2C_MSG_WD               0x0000

#define FI2C_CON_MASTER           0x1
#define FI2C_CON_SLAVE_DISABLE    0x40
#define FI2C_CON_RESTART_EN       0x20

#define FI2C_MSG_MAX_RING         8

#define FI2C_MSG_SMBUS_BLOCK_MAX  32
#define FI2C_MSG_UNIT_SIZE        10

#define FI2C_MSG_SIGLE_BUF_LEN    51
#define FI2C_MSG_SINGLE_FRAME_CNT 32

#define FI2C_MSG_CON_SPEED_STD    0x2
#define FI2C_MSG_CON_SPEED_FAST   0x4
#define FI2C_MSG_CON_SPEED_HIGH   0x6

enum
{
    FI2C_MSG_SUCCESS = 0,
    FI2C_MSG_TIMEOUT,
    FI2C_MSG_CNT_ERR,
    FI2C_MSG_TX_ABRT,
    FI2C_MSG_INT_ERR,
    FI2C_MSG_BLOCK_SIZE,
    FI2C_MSG_INVALID_ADDR,
    FI2C_MSG_TRANS_PACKET_FAIL,
    FI2C_MSG_RUNNING,
    FI2C_MSG_CHECK_STATUS_ERR,
};

enum
{
    FI2C_STANDARD_SPEED = 0x1,
    FI2C_FAST_SPEED = 0x2,
    FI2C_HIGH_SPEED = 0x3,
    FI2C_SPEED_MODE_MAX
};

enum
{
    FI2C_SPEED_STANDARD_RATE = 100000, /* 100kb/s */
    FI2C_SPEED_FAST_RATE = 400000,     /* 400kb/s */
    FI2C_SPEED_HIGH_RATE = 1000000     /* 1Mkb/s */
};
typedef enum
{
    FI2C_MSG_CMD_DATA_XFER,
    FI2C_MSG_CMD_DATA_SLAVE,
} FI2cDataSubid;

typedef enum
{
    FI2C_MSG_CMD_DEFAULT_RESV = 0,
    FI2C_MSG_CMD_DEFAULT_RESUME,
    FI2C_MSG_CMD_DEFAULT_SSLV,
} FI2cReportDefaultSubid;

typedef enum
{
    FI2C_MSG_CMD_SMBCLK_EXT_LOW_TIMEOUT = 0,
    FI2C_MSG_CMD_SMBCLK_TMO_LOW_TIMEOUT,
    FI2C_MSG_CMD_SMDATA_LOW_TIMEOUT,
    FI2C_MSG_CMD_SMBALERT_IN_N,
    FI2C_MSG_CMD_SLAVE_EVENT,
} FI2cReportSubid;
typedef enum
{
    FI2C_MSG_CMD_SET_MODULE_EN = 0,
    FI2C_MSG_CMD_SET_MODE,
    FI2C_MSG_CMD_SET_RESTAR,
    FI2C_MSG_CMD_SET_ADDR_MODE,
    FI2C_MSG_CMD_SET_SPEED,
    FI2C_MSG_CMD_SET_INT_TL,
    FI2C_MSG_CMD_SET_SDA_HOLD,
    FI2C_MSG_CMD_SET_INTERRUPT,
    FI2C_MSG_CMD_SET_RX_FIFO_FULL,
    FI2C_MSG_CMD_SET_STOP_DET_IF_ADDRESSED,
    FI2C_MSG_CMD_SET_WRITE_PROTECT,
    FI2C_MSG_CMD_SET_SMBCLK_LOW_MEXT,
    FI2C_MSG_CMD_SET_SMBCLK_LOW_TIMEOUT,
    FI2C_MSG_CMD_SET_SMBDAT_STUCK_TIMEOUT,
    FI2C_MSG_CMD_SET_ADDR,
    FI2C_MSG_CMD_SET_SUSPEND
} FI2cSetSubid;

typedef struct
{
    u32 msg_total_num;
    u32 shmem_data_addr;
    int result;
} FI2cMsgTransMsgInfo;

typedef struct
{
    u32 min_scl_hightime_ns;
    u32 min_scl_lowtime_ns;
    u32 def_risetime_ns;
    u32 def_falltime_ns;
} FI2cMsgSpeedModeInfo; /* speed calculation related configs */

typedef struct
{
    u8 speed_mode;
    u8 calc_en;
    u16 scl_hcnt;
    u16 scl_lcnt;
    u16 sda_hold;
} FI2cMsgBusSpeedInfo;

typedef struct
{
    u8 rx_fifo_threshold;
    u8 tx_fifo_threshold;
} FI2cMsgFifoThreshold;

typedef struct
{
    u32 ss_hcnt;
    u32 ss_lcnt;
    u32 fs_hcnt;
    u32 fs_lcnt;
    u32 hs_hcnt;
    u32 hs_lcnt;
    u32 sda_hold;
    u32 tx_fifo_thr;
    u32 rx_fifo_thr;
    u32 smbclk_mext;
    u32 smbclk_timeout;
    u32 smbdat_timeout;
    u32 cfg;
    u32 intr_mask;
} FI2cMsgDefaultCfg;

typedef struct
{
    u32 instance_id;
    u32 irq_num;
    u32 irq_prority;
    FDevMsg msg;
} FI2cMsgConfig;

typedef struct
{
    u32 tx_cmd_cnt;
    u32 cur_cmd_cnt;
    u32 cur_index;
    u32 opt_finish_len;
    u32 tx_ring_cnt;
    boolean is_need_check;
    boolean is_last_frame;
} FI2cMsgTransfer;

typedef struct
{
    u16 addr;
    u16 flags;
    u8 type;
} __attribute__((packed)) FI2cMsgTransInfo;

struct FI2cMsg
{
    u16 addr;
    u16 flags;
#define FI2C_MSG_M_WR         0x0000
#define FI2C_MSG_M_RD         0x0001
#define FI2C_MSG_M_TEN        0x0010
#define FI2C_MSG_M_DMA_SAFE   0x0200
#define FI2C_MSG_M_RECV_LEN   0x0400
#define FI2C_MSG_M_NO_RD_ACK  0x0800
#define FI2C_MSG_M_IGNORE_NAK 0x1000
#define FI2C_MSG_M_DIR_ADDR   0x2000
#define FI2C_MSG_M_NOSTART    0x4000
#define FI2C_MSG_M_STOP       0x8000
    u16 len;
    u8 *buf;
};

enum
{
    FI2C_MSG_EVT_READ_REQUESTED = 0, /*master模式传输出错回调函数事件值*/
    FI2C_MSG_EVT_WRITE_REQUESTED,    /*master模式接收完成回调函数事件值*/
    FI2C_MSG_EVT_READ_PROCESSED,     /*master模式发送完成回调函数事件值*/
    FI2C_MSG_EVT_WRITE_RECEIVED,     /*master模式发送完成回调函数事件值*/
    FI2C_MSG_EVT_STOP,
    FI2C_MSG_INTR_EVT_NUM
}; /* master mode evt */
typedef void (*FI2cMsgEvtHandler)(void *instance_p, void *param);

typedef struct
{
    u8 reserved; /*取消模块id，作为保留字段，必须为0值*/
    u8 seq;      /*由发起方使用或维护，对端不改变该值，默认为0*/
    u8 cmd_type; /*功能性命令字段，可以用极少位表示，当前保留为8bit*/
    u8 cmd_subid; /*子命令id，与命令实现有关，最多为256个，搭配cmd_type补充*/
    u16 len;    /*在交互数据时，表示payload的长度，交互命令时为0*/
    u8 status1; /*保留字段，各模块根据需要定义，可返回某些函数执行后的返回值*/
    u8 status0; /*协议命令状态，表示命令执行相应*/
    u8 para[FI2C_MSG_DATA_LEN];
} FI2cMsgInfo;

typedef struct
{
    u32 clock;
    u32 flag;
    FI2cMsgInfo *i2c_msg;
    FI2cMsgTransfer mng;
    u8 real_index[FI2C_MSG_SINGLE_FRAME_CNT];
    void *tx_shmem_addr;
    void *rx_shmem_addr;
    u32 mode;
    FI2cMsgConfig config;
    u32 total_shmem_len;
    u32 total_cnt;
    u32 rx_buf_len;
    u32 cmd_err;
    u32 msg_num;
    u32 abort_source;
    struct FI2cMsg *msgs;
    u32 intr_mask;

    boolean complete_flag;
    u32 speed_mode;
    u32 clk_clock_frequency;

    u32 master_cfg;
    u32 slave_cfg;
    u32 sda_hold_time;
    u32 timeout_ms;
    u32 is_ready;
    FI2cMsgEvtHandler slave_evt_handlers[FI2C_MSG_INTR_EVT_NUM];
} FI2cMsgCtrl;

FError FI2cMsgCommonRegfileIntrDisable(FI2cMsgCtrl *const instance);

FError FI2cMsgCommonRegfileIntrEnable(FI2cMsgCtrl *const instance);

void FI2cMsgCommonRegfileS2MIntrClear(FI2cMsgCtrl *const instance, u32 state);

void FI2cMsgSetInterrupt(FI2cMsgCtrl *const instance, u32 is_enable, u32 intr_mask);

void FI2cMsgTrigSlaveIntr(FI2cMsgCtrl *const instance);

void FI2cMsgSlaveIsrHandler(FI2cMsgCtrl *const instance);

void FI2cMsgMasterRegfileIsr(int irq, void *dev_id);

void FI2cMsgMasterIsrHandler(FI2cMsgCtrl *const instance);

int FI2cMsgSlaveRegfileIsr(int irq, void *dev_id);

FError FI2cMsgCfgInitialize(FI2cMsgCtrl *instance_p, const FI2cMsgConfig *input_config_p);

void FI2cMsgDeInitialize(FI2cMsgCtrl *instance_p);

const FI2cMsgConfig *FI2cMsgLookupConfig(u32 instance_id);

void FI2cMsgSetBusSpeed(FI2cMsgCtrl *instance_p, u32 speed_mode, boolean auto_calc);
#ifdef __cplusplus
}
#endif

#endif
