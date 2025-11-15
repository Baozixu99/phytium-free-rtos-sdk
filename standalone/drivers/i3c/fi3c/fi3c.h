/*
 * Copyright (C) 2021, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fi3c.h
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:37:04
 * Description:  This file is for complete user external interface definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan    2022/9/12  first commit
 */
#ifndef FI3C_H
#define FI3C_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fassert.h"
#include "ferror_code.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FI3C_SUCCESS           FT_SUCCESS
#define FI3C_ERR_INVAL_PARM    FT_MAKE_ERRCODE(ErrModBsp, ErrBspI3c, 1)
#define FI3C_ERR_NOT_READY     FT_MAKE_ERRCODE(ErrModBsp, ErrBspI3c, 2)
#define FI3C_ERR_TIMEOUT       FT_MAKE_ERRCODE(ErrModBsp, ErrBspI3c, 3)
#define FI3C_ERR_NOT_SUPPORT   FT_MAKE_ERRCODE(ErrModBsp, ErrBspI3c, 4)
#define FI3C_ERR_INVAL_STATE   FT_MAKE_ERRCODE(ErrModBsp, ErrBspI3c, 5)
#define FI3C_ERR_FAILURE       FT_MAKE_ERRCODE(ErrModBsp, ErrBspI3c, 6)

#define FI3C_TIME_OUT          1000
#define FI3C_XFER_SIZE         1024
#define FI3C_ALIGNED_BYTE      4
#define FI3C_XFER_MAX_BYTE_LEN 128
#define FI3C_MAX_DEVICE_NUM    12
#define FI3C_SEND_CCC_NUM_CMD  1
#define FI3C_ENABLE_SCBA       1
#define FI3C_ENABLE_SEND_CCC   1
#define FI3C_ENABLE_SEND_BCH   1
#define FI3C_CMD_SEND_TIMES    1
#define FI3C_DISABLE_SEND_BCH  0

typedef void (*FI3cIntrEventHandler)(void *param);
/*INTR EVENT*/
typedef enum
{
    FI3C_INTR_EVENT_CMDR_OVF = 0, /* Handler type for interrupt */
    FI3C_INTR_EVENT_CMDR_UNF = 1,
    FI3C_INTR_EVENT_CMDR_THR = 2,
    FI3C_INTR_EVENT_CMDD_OVF = 3,
    FI3C_INTR_EVENT_CMDD_THR = 4,
    FI3C_INTR_EVENT_CMDD_EMP = 5,
    FI3C_INTR_EVENT_RX_UNF = 6,
    FI3C_INTR_EVENT_RX_THR = 7,
    FI3C_INTR_EVENT_IBIR_OVF = 8,
    FI3C_INTR_EVENT_IBIR_UNF = 9,
    FI3C_INTR_EVENT_IBIR_THR = 10,
    FI3C_INTR_EVENT_IBID_UNF = 11,
    FI3C_INTR_EVENT_IBID_THR = 12,
    FI3C_INTR_EVENT_TX_OVF = 13,
    FI3C_INTR_EVENT_TX_THR = 14,
    FI3C_INTR_EVENT_IMM_COMP = 15,
    FI3C_INTR_EVENT_MR_DONE = 16,
    FI3C_INTR_EVENT_HALTED = 17,

    FI3C_MASTER_INTR_EVENT_NUM

} FI3cIntrEventType;
/*XIMT MODE*/
enum
{
    FI3C_XMIT_BURST_STATIC_SUBADDR = 0,
    FI3C_XMIT_SINGLE_INC_SUBADDR = 1,
    FI3C_XMIT_SINGLE_STATIC_SUBADDR = 2,
    FI3C_XMIT_BURST_WITHOUT_SUBADDR = 3,
};
/*CCC CODE*/
enum
{
    FI3C_ENEC_BC_CCC_CMD = 0x00,
    FI3C_ENEC_DC_CCC_CMD = 0x80,
    FI3C_DISEC_BC_CCC_CMD = 0x01,
    FI3C_DISEC_DC_CCC_CMD = 0x81,
    FI3C_ENTASO_BC_CCC_CMD = 0x02,
    FI3C_ENTASO_DC_CCC_CMD = 0x82,
    FI3C_RSTDAA_BC_CCC_CMD = 0x06,
    FI3C_RSTDAA_DC_CCC_CMD = 0x86,
    FI3C_ENTDAA_DC_CCC_CMD = 0x07,
    FI3C_SETNML_BC_CCC_CMD = 0x09,
    FI3C_SETNML_DC_CCC_CMD = 0x89,
    FI3C_SETMRL_BC_CCC_CMD = 0x0A,
    FI3C_SETMRL_DC_CCC_CMD = 0x8A,
    FI3C_DEFSLVS_BC_CCC_CMD = 0x08,
    FI3C_ENTHDR_BC_CCC_CMD = 0x20,
    FI3C_SETDASA_DC_CCC_CMD = 0x87,
    FI3C_SETNEWDA_DC_CCC_CMD = 0x88,
    FI3C_GETNWL_DC_CCC_CMD = 0x8B,
    FI3C_GETMRL_DC_CCC_CMD = 0x8C,
    FI3C_GETPID_DC_CCC_CMD = 0x8D,
    FI3C_GETBCR_DC_CCC_CMD = 0x8E,
    FI3C_GETDCR_DC_CCC_CMD = 0x8F,
    FI3C_GETSTATUS_DC_CCC_CMD = 0x90,
    FI3C_GETHEDCAP_CCC_CMD = 0x96,
};
/*BUS MODE*/
enum
{
    PURE_BUS_MODE = 0,
    MIXED_FASH_BUS_MODE = 2,
    MIXED_LIMITED_BUS_MODE = 3,
};

enum
{
    FI3C_SDR_MODE = 0,
    FI3C_DDR_MODE,
};

enum
{
    FI3C_MODE = 0,
    FI2C_MODE,
};

enum
{
    SUB_ADDRESS_8BIT = 0,
    SUB_ADDRESS_16BIT = 1,
};

typedef struct
{
    u8 cmd_id;
    u8 ccc_code;
    uintptr sub_address; /*Slave internal offset address*/

    u8 rnw;      /*trans direction*/
    u8 dev_addr; /*slave dev addr*/
    u16 pl_len;  /*bytes length*/
    u8 is_10bit; /*10 bit I2C address*/
    u8 rsbc;     /*Sending sr between commands*/
    u8 sbca;     /*Whether to use 16 bit sub addr*/
    u8 xmit_mode;
    u8 bch;      /*Whether to send broadcast address 7E*/
    u32 is_ccc;  /*Whether to send CCC command*/
    u32 is_addr; /*Whether to use DDR mode*/

    u8 *tx_buf;
    u32 tx_len;

    u8 *rx_buf;
    u32 rx_len;

    u32 error; /*cmd error code*/
} FI3cCmdConfig;

typedef struct
{
    u32 ncmd; /*xfer cmd num*/
    FI3cCmdConfig cmds[];
} FI3cXferConfig;

typedef struct
{
    u32 rnw;
    u32 length;
    u8 *date_in;
    u8 *date_out;
} FI3cPriveXfer;

typedef struct
{
    u32 cmdfifodepth;
    u32 cmdrfifodepth;
    u32 txfifodepth;
    u32 rxfifodepth;
    u32 ibirfifodepth;
} FI3cMasterCaps;

typedef struct
{
    uintptr slave_addr;
    u8 lvr_sa_msb;   /*dev high 3 bit address*/
    u8 lvr_ext_addr; /*i3c dev mode 0:7bit 1: I2C 10bit*/
    u32 is_i3c;      /*is i3c dev*/
    u32 pid_lsb;     /*dev temporary id*/
    u32 pib_msb;     /*dev temporary id*/
    u8 bcr;          /*bcr code*/
    u8 dcr_lvr;      /*Characteristic register*/
} FI3cDevConfig;

typedef struct
{
    u32 dev_id;
    FI3cDevConfig dev_cfg;
} FI3cDevice;

typedef struct
{
    u32 instance_id;   /* Device instance id */
    uintptr base_addr; /* Device base address */
    u32 irq_num;       /* Device interrupt id */
    u32 irq_prority;   /* Device interrupt priority */
    u32 thd_del;
    u32 i3c_sclrate; /* I2C speed rate Hz*/
    u32 i2c_sclrate; /* I3C speed rate Hz*/
    u8 pp_low;
    u8 od_low;
    u32 sub_scba;  /* sub address bit 8 or 16*/
    u8 xmit_mode;  /* xmit mode */
    u8 bus_mode;   /* bus mode, I2C or I3C*/
    u8 trans_mode; /* trans mode */
    FI3cMasterCaps caps;
} FI3cConfig; /* Device configure setting */

typedef struct
{
    FI3cIntrEventType type;
    FI3cIntrEventHandler handler;
    void *param;
} FI3cIntrEventConfig;

typedef struct
{
    FI3cConfig config;                   /* Current active configs */
    u32 is_ready;                        /* Device is initialized and ready */
    FI3cDevice dev[FI3C_MAX_DEVICE_NUM]; /* I3C device Config*/
    FI3cIntrEventConfig intr_event[FI3C_MASTER_INTR_EVENT_NUM];
} FI3c; /* Device instance */

/*获取I3C驱动的默认配置参数*/
const FI3cConfig *FI3cLookupConfig(u32 instance_id);
/*主机模式下的I3C中断响应函数*/
void FI3cMasterIntrHandler(s32 vector, void *param);
/*注册I3C主机中断事件函数*/
void FI3cMasterRegisterIntrHandler(FI3c *instance_p, FI3cIntrEventConfig *intr_event_p);
/*完成I3C驱动实例的初始化，使之可以使用*/
FError FI3cCfgInitialize(FI3c *instance_p, const FI3cConfig *config);
/*完成I3C设备配置*/
FError FI3cReset(FI3c *instance_p);
/*完成I3C驱动实例去使能，清零实例数据*/
void FI3cDeInitialize(FI3c *instance_p);
/*使能I3C中断*/
FError FI3cInterruptEnable(FI3c *instance_p, FI3cIntrEventType event_type);
/*读取已发送命令,从机返回内容*/
FError FI3cMasterGetCmdr(FI3c *instance_p, FI3cCmdConfig *cmd);
/*发送CCC命令*/
FError FI3cMasterSendCCC(FI3c *instance_p, u8 ccc_code, uintptr slave_address,
                         u8 length, u8 mode, u8 *buffer, boolean is_bch);
/*开始本次传输*/
FError FI3cMasterStartXfer(FI3c *instance_p, FI3cXferConfig *xfer);
/*轮询模式下向从机写入数据*/
FError FI3cMasterWriteData(FI3c *instance_p, uintptr slave_address, uintptr sub_address,
                           u8 *write_buf, u8 byte_len);
/*轮询模式下从从机读出数据*/
FError FI3cMasterReadData(FI3c *instance_p, uintptr slave_address, uintptr sub_address,
                          u8 *read_buf, u8 byte_len);
/*手动添加设备信息*/
FError FI3cMasterManualDevInit(FI3c *instance_p, u32 dev_id, uintptr slave_address);
/*获取当前所有设备信息*/
FError FI3cMasterGetSlaveDevInfo(FI3c *instance_p);
/*使能热加入处理*/
FError FI3cMasterEnableHotJoin(FI3c *instance_p);
#ifdef __cplusplus
}
#endif

#endif