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
 * FilePath: fi3c_master.c
 * Date: 2021-11-01 14:53:42
 * LastEditTime: 2022-02-18 08:36:46
 * Description:  This file is for i3c master drivers
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhangyan    2023/9/11   first commit
 */

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>
#include "fio.h"
#include "fsleep.h"
#include "fdebug.h"
#include "fi3c_hw.h"
#include "fi3c.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FI3C_DEBUG_TAG          "I3C_MASTER"
#define FI3C_ERROR(format, ...) FT_DEBUG_PRINT_E(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_INFO(format, ...)  FT_DEBUG_PRINT_I(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)
#define FI3C_DEBUG(format, ...) FT_DEBUG_PRINT_D(FI3C_DEBUG_TAG, format, ##__VA_ARGS__)

/**
 * @name: FI3cAddressParityCheck
 * @msg: 填充Device Retaining 地址前进行校验
 * @return {u32 } check_num 预校验数
 * @param {u32} value 校验结果
 */
static u32 FI3cAddressParityCheck(u32 check_num)
{
    u32 parity_check = 0;
    u32 value = 0;

    parity_check = check_num - ((check_num >> 1) & 0x55);
    parity_check = (parity_check & 0x33) + ((parity_check >> 2) & 0x33);
    value = (parity_check + (parity_check >> 4)) & 0x0f;

    return value;
}

/**
 * @name: FI3cPrepareRR0DevAddress
 * @msg: 准备Device Retaining寄存器地址部分填充内容
 * @return {u32 } rr0寄存器值
 * @param {uintptr} slave_address 从机地址
 */
static u32 FI3cPrepareRR0DevAddress(uintptr slave_address)
{
    u32 dev_id_rr0;
    dev_id_rr0 = (slave_address << 1) & 0xff;
    /*RR0[7:1] = addr[6:0] */
    dev_id_rr0 |= (slave_address & GENMASK(6, 0)) << 1;
    /*RR0[15:13] = addr[9:7] */
    dev_id_rr0 |= (slave_address & GENMASK(9, 7)) << 13;
    if ((FI3cAddressParityCheck(slave_address & 0x7f) & 1) == 0)
    {
        dev_id_rr0 |= 1;
    }

    return dev_id_rr0;
}

/**
 * @name: FI3cDevPrepare
 * @msg: 准备Device Retaining寄存器地址部分填充内容
 * @return {FError } 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u32} dev_id 预填写的设备号
 * @param {uintptr} slave_address 从机地址
 */
static FError FI3cDevPrepare(FI3c *instance_p, u32 dev_id, uintptr slave_address)
{
    u32 dev_id_rr0;
    uintptr base_addr = instance_p->config.base_addr;
    dev_id_rr0 = FI3cPrepareRR0DevAddress(slave_address);

    if (instance_p->config.trans_mode == FI3C_MODE)
    {
        dev_id_rr0 = dev_id_rr0 | FI3C_DEV_IS_I3C;
        FI3C_DEBUG("trans_mode == i3c mode ,dev_id_rr0 == %x", dev_id_rr0);
    }
    else
    {
        FI3C_DEBUG("trans_mode == i2c mode ,dev_id_rr0 == %x", dev_id_rr0);
    }

    FI3C_WRITE_REG32(base_addr, FI3C_DEV_ID_RR0(dev_id), dev_id_rr0);

    return FT_SUCCESS;
}

/**
 * @name: FI3cMasterManualDevInit
 * @msg:  手动添加设备信息
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u32} dev_id 预添加的从机id号
 * @param {uintptr} slave_address 从机地址
 */
FError FI3cMasterManualDevInit(FI3c *instance_p, u32 dev_id, uintptr slave_address)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 dev_ctrl_reg;

    dev_ctrl_reg = FI3C_READ_REG32(base_addr, FI3C_DEVS_CTRL_OFFSET) | FI3C_DEV_ACTIVE(dev_id);
    FI3C_WRITE_REG32(base_addr, FI3C_DEVS_CTRL_OFFSET, dev_ctrl_reg);

    ret = FI3cDevPrepare(instance_p, dev_id, slave_address);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("Device prepare failed.");
    }

    return ret;
}

/**
 * @name: FI3cMasterAllocXfer
 * @msg: 为待发送Xfer分配内存空间
 * @return {FI3cXferConfig *} 返回FI3cXferConfig指针
 * @param {u32} *ncmd 本次需要发送的cmd数量
 */
static FI3cXferConfig *FI3cMasterAllocXfer(u32 ncmd)
{
    FI3cXferConfig *xfer;
    u32 size = sizeof(u32) + sizeof(FI3cCmdConfig) * ncmd;
    xfer = (FI3cXferConfig *)malloc(size);
    if (!xfer)
    {
        FI3C_ERROR("Alloc xfer memory failed!");
        return NULL;
    }
    memset(xfer, 0, size);

    return xfer;
}

/**
 * @name: FI3cMasterSendCmd
 * @msg: 填充待发送cmd
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {FI3cCmdConfig} 待发送的cmd
 */
static FError FI3cMasterSendCmd(FI3c *instance_p, FI3cCmdConfig cmd)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 cmd0_reg_val = 0;
    u32 cmd1_reg_val = 0;

    cmd1_reg_val |= FI3C_CMD1_FIFO_CMDID(cmd.cmd_id);
    if (cmd.is_ccc == FI3C_ENABLE_SEND_CCC)
    {
        cmd1_reg_val |= FI3C_CMD1_FIFO_CCC_CSRADDR(cmd.ccc_code);
        FI3C_DEBUG("send ccc code = 0x%x", cmd.ccc_code);
    }
    else
    {
        cmd1_reg_val |= FI3C_CMD1_FIFO_CCC_CSRADDR(cmd.sub_address);
        FI3C_DEBUG("send sub address = 0x%x", cmd.sub_address);
    }

    cmd.pl_len = cmd.rnw == FI3C_CMD0_FIFO_RNW_ENABLE ? cmd.rx_len : cmd.tx_len;
    cmd0_reg_val |= FI3C_CMD0_FIFO_RNW(cmd.rnw);
    cmd0_reg_val |= FI3C_CMD0_FIFO_DEV_ADDR(cmd.dev_addr);
    cmd0_reg_val |= FI3C_CMD0_FIFO_PL_LEN(cmd.pl_len);
    cmd0_reg_val |= FI3C_CMD0_FIFO_IS_10B(cmd.is_10bit);
    cmd0_reg_val |= FI3C_CMD0_FIFO_RSBC(cmd.rsbc);
    cmd0_reg_val |= FI3C_CMD0_FIFO_SBCA(cmd.sbca);
    cmd0_reg_val |= FI3C_CMD0_FIFO_PRIV_XMIT_MODE(cmd.xmit_mode);
    cmd0_reg_val |= FI3C_CMD0_FIFO_BCH(cmd.bch);
    cmd0_reg_val |= FI3C_CMD0_FIFO_IS_CCC(cmd.is_ccc);
    cmd0_reg_val |= FI3C_CMD0_FIFO_IS_DDR(cmd.is_addr);
    FI3C_DEBUG("cmd0_reg_val = 0x%x, cmd1_reg_val = 0x%x", cmd0_reg_val, cmd1_reg_val);
    FI3C_WRITE_REG32(base_addr, FI3C_CMD0_FIFO_OFFSET, cmd0_reg_val);
    FI3C_WRITE_REG32(base_addr, FI3C_CMD1_FIFO_OFFSET, cmd1_reg_val);

    return ret;
}

/**
 * @name: FI3cMasterSendCCC
 * @msg: 发送CCC命令
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u8} ccc_code CCC命令代码
 * @param {uintptr} slave_address 从机地址
 * @param {u8} length 发送字节数
 * @param {u8} mode CCC模式
 * @param {u8} *buffer 待发送缓存
 */
FError FI3cMasterSendCCC(FI3c *instance_p, u8 ccc_code, uintptr slave_address,
                         u8 length, u8 mode, u8 *buffer, boolean is_bch)
{
    FError ret = FI3C_SUCCESS;
    FI3cXferConfig *xfer;

    xfer = FI3cMasterAllocXfer(FI3C_SEND_CCC_NUM_CMD);
    if (xfer == NULL)
    {
        FI3C_ERROR("The send ccc code xfer allocate failed.");
        goto out;
    }
    xfer->ncmd = FI3C_SEND_CCC_NUM_CMD;
    if (is_bch == FI3C_ENABLE_SEND_BCH)
    {
        xfer->cmds[0].bch = FI3C_ENABLE_SEND_BCH;
    }
    xfer->cmds[0].tx_buf = buffer;
    xfer->cmds[0].tx_len = length;
    xfer->cmds[0].is_ccc = FI3C_ENABLE_SEND_CCC;
    xfer->cmds[0].ccc_code = ccc_code | mode;
    xfer->cmds[0].dev_addr = slave_address;
    xfer->cmds[0].pl_len = length;

    ret = FI3cMasterStartXfer(instance_p, xfer);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("FI3cMasterSendCCC FI3cMasterStartXfer failed.");
        goto out;
    }
    FI3cMasterGetCmdr(instance_p, &xfer->cmds[0]);
out:
    free(xfer);

    return ret;
}

/**
 * @name: FI3cMasterGetSlaveDevInfo
 * @msg: 获取当前所有设备信息
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 */
FError FI3cMasterGetSlaveDevInfo(FI3c *instance_p)
{
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    /*dev_0 is the master device*/
    for (int dev_num = 1; dev_num < FI3C_MAX_DEVICE_NUM; dev_num++)
    {
        FI3cDevice *dev;
        dev = &instance_p->dev[dev_num];

        u32 dev_rr0_reg = FI3C_READ_REG32(base_addr, FI3C_DEV_ID_RR0(dev_num));
        u32 dev_rr1_reg = FI3C_READ_REG32(base_addr, FI3C_DEV_ID_RR1(dev_num));
        u32 dev_rr2_reg = FI3C_READ_REG32(base_addr, FI3C_DEV_ID_RR2(dev_num));
        FI3C_DEBUG("dev_rr0_reg = 0x%x, dev_rr1_reg = 0x%x, dev_rr2_reg = 0x%x",
                   dev_rr0_reg, dev_rr1_reg, dev_rr2_reg);

        dev->dev_id = dev_num;
        dev->dev_cfg.is_i3c = ((dev_rr0_reg & FI3C_DEV_IS_I3C) >> 9);

        if (dev->dev_cfg.is_i3c)
        {
            dev->dev_cfg.slave_addr = ((dev_rr0_reg & FI3C_DEV_DEV_ADDR_MASK) >> 1);
        }
        else
        {
            dev->dev_cfg.slave_addr = ((dev_rr0_reg & FI3C_DEV_DEV_ADDR_MASK) >> 1);
            dev->dev_cfg.lvr_sa_msb = ((dev_rr0_reg & FI3C_DEV_IVR_SA_MSB_MASK) >> 6);
            dev->dev_cfg.slave_addr |= dev->dev_cfg.lvr_sa_msb;
        }

        dev->dev_cfg.pib_msb = (dev_rr1_reg & FI3C_DEV_PID_MSB_MASK);
        dev->dev_cfg.pid_lsb = ((dev_rr2_reg & FI3C_DEV_PID_LSB_MASK) >> 16);
        dev->dev_cfg.bcr = ((dev_rr2_reg & FI3C_DEV_BCR_MASK) >> 8);
        dev->dev_cfg.dcr_lvr = (dev_rr2_reg & FI3C_DCR_LVR_MASK);
    }

    return ret;
}

/**
 * @name: FI3cMasterStartXfer
 * @msg: 开始本次传输
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {FI3cXferConfig} *xfer 待发送的xfer
 */
FError FI3cMasterStartXfer(FI3c *instance_p, FI3cXferConfig *xfer)
{
    FASSERT(instance_p);
    FASSERT(xfer);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 ctrl_reg, i, j, write_count, read_count;
    u32 *rx_fifo;
    u32 *tx_fifo;
    /*Alloc the rx_fifo and tx_fifo memory*/
    rx_fifo = malloc(FI3C_XFER_SIZE);
    tx_fifo = malloc(FI3C_XFER_SIZE);
    memset(rx_fifo, 0, FI3C_XFER_SIZE);
    memset(tx_fifo, 0, FI3C_XFER_SIZE);
    /*Enable I3C, preparation for sending*/
    ctrl_reg = FI3C_READ_REG32(base_addr, FI3C_CTRL_OFFSET);
    ctrl_reg |= instance_p->config.bus_mode | FI3C_CTRL_MCS_EN | FI3C_CTRL_DEV_EN | FI3C_CTRL_MST_ACK;
    FI3C_WRITE_REG32(base_addr, FI3C_CTRL_OFFSET, ctrl_reg);
    for (i = 0; i < xfer->ncmd; i++)
    {
        memcpy(tx_fifo, xfer->cmds[i].tx_buf, xfer->cmds[i].tx_len);
        if (xfer->cmds[i].tx_len % FI3C_ALIGNED_BYTE)
        {
            write_count = (xfer->cmds[i].tx_len / FI3C_ALIGNED_BYTE) + 1;
        }
        else
        {
            write_count = (xfer->cmds[i].tx_len / FI3C_ALIGNED_BYTE);
        }
        for (j = 0; j < write_count; j++)
        {
            FI3C_DEBUG("start write the tx_fifo[%d] = 0x%x", j, tx_fifo[j]);
            FI3C_WRITE_TX_FIFO(base_addr, tx_fifo[j]);
        }
        ret = FI3cMasterSendCmd(instance_p, xfer->cmds[i]);
        if (ret != FI3C_SUCCESS)
        {
            FI3C_ERROR("Send cmd failed !");
            goto out;
        }
        /*begin sending*/
        ctrl_reg |= FI3C_CTRL_MCS;
        FI3C_WRITE_REG32(base_addr, FI3C_CTRL_OFFSET, ctrl_reg);
        /*begin recving data*/
        if (xfer->cmds[i].rnw == FI3C_CMD0_FIFO_RNW_ENABLE)
        {
            if (xfer->cmds[i].rx_len % FI3C_ALIGNED_BYTE)
            {

                read_count = (xfer->cmds[i].rx_len / FI3C_ALIGNED_BYTE) + 1;
            }
            else
            {
                read_count = (xfer->cmds[i].rx_len / FI3C_ALIGNED_BYTE);
            }
            for (j = 0; j < read_count; j++)
            {
                fsleep_millisec(1);
                rx_fifo[j] = FI3C_READ_RX_FIFO(base_addr);
                FI3C_DEBUG("Read the rx_fifo[%d] = 0x%x", j, rx_fifo[j]);
            }
            memcpy(xfer->cmds[i].rx_buf, rx_fifo, xfer->cmds[i].rx_len);
        }
        FI3cMasterGetCmdr(instance_p, &xfer->cmds[i]);
    }
out:
    free(rx_fifo);
    free(tx_fifo);

    return ret;
}

/**
 * @name: FI3cMasterEndXfer
 * @msg: 结束本次传输
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {FI3cXferConfig} *xfer 待发送的xfer
 */
FError FI3cMasterEndXfer(FI3c *instance_p)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 status0, time_out;

    time_out = 0;
    for (status0 = FI3C_READ_REG32(base_addr, FI3C_MST_STATUS0_OFFSET);
         !(status0 & FI3C_MST_STATUS0_IDLE);
         status0 = FI3C_READ_REG32(base_addr, FI3C_MST_STATUS0_OFFSET))
    {
        fsleep_millisec(10);
        if (time_out++ == FI3C_TIME_OUT)
        {
            FI3C_ERROR("xfer timeout");
            return FI3C_ERR_TIMEOUT;
        }
    }
    /*End of xfer, clear fifo reg*/
    FI3cSetEnable(base_addr, FALSE);
    FI3C_WRITE_REG32(base_addr, FI3C_FLUSH_CTRL_OFFSET,
                     FI3C_FLUSH_RX_FIFO_MASK | FI3C_FLUSH_TX_FIFO_MASK |
                         FI3C_FLUSH_CMD_FIFO_MASK | FI3C_FLUSH_CMD_RESP_MASK);
    FI3cSetEnable(base_addr, TRUE);

    return ret;
}

/**
 * @name: FI3cMasterDataXfers
 * @msg: 轮询模式下向从机传输数据
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {uintptr} slave_addr 从机地址
 * @param {uintptr} sub_address 从机内部偏移地址
 * @param {FI3cPriveXfer} *prive_xfer 待传输数据内容
 * @param {u32} num_xfer 传输次数
 */
FError FI3cMasterDataXfers(FI3c *instance_p, uintptr slave_addr, uintptr sub_address,
                           FI3cPriveXfer *prive_xfer, u32 num_xfer)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    FI3cXferConfig *xfer;
    u32 index;
    xfer = FI3cMasterAllocXfer(num_xfer);
    if (!xfer)
    {
        FI3C_ERROR("The xfer allocate failed.");
        return FI3C_ERR_INVAL_PARM;
    }
    xfer->ncmd = num_xfer;
    for (index = 0; index < num_xfer; index++)
    {
        if (prive_xfer->rnw == FI3C_CMD0_FIFO_RNW_ENABLE)
        {
            xfer->cmds[index].rnw = FI3C_CMD0_FIFO_RNW_ENABLE;
            xfer->cmds[index].rx_buf = prive_xfer[index].date_in;
            xfer->cmds[index].rx_len = prive_xfer[index].length;
            xfer->cmds[index].dev_addr = slave_addr;
        }
        else
        {
            xfer->cmds[index].tx_buf = prive_xfer[index].date_out;
            xfer->cmds[index].tx_len = prive_xfer[index].length;
            xfer->cmds[index].dev_addr = slave_addr;
        }
        if (instance_p->config.sub_scba == SUB_ADDRESS_16BIT)
        {
            xfer->cmds[index].sbca = FI3C_ENABLE_SCBA;
        }
        xfer->cmds[index].xmit_mode = instance_p->config.xmit_mode;
        xfer->cmds[index].sub_address = sub_address;
    }

    ret = FI3cMasterStartXfer(instance_p, xfer);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("Start xfer failed.");
    }

    ret = FI3cMasterEndXfer(instance_p);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("End xfer failed.");
    }
    free(xfer);

    return ret;
}

/**
 * @name: FI3cMasterWriteData
 * @msg:  轮询模式下向从机写入数据
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {uintptr} slave_addr 从机地址
 * @param {uintptr} sub_address 从机内部偏移地址
 * @param {u8} *write_buf 数据缓存
 * @param {u32} length 传输字节长度
 */
FError FI3cMasterWriteData(FI3c *instance_p, uintptr slave_address, uintptr sub_address,
                           u8 *write_buf, u8 byte_len)
{
    FASSERT(instance_p);
    FASSERT(byte_len <= FI3C_XFER_MAX_BYTE_LEN);
    FError ret = FI3C_SUCCESS;
    FI3cPriveXfer *prive_xfer;
    u32 num_xfer = FI3C_CMD_SEND_TIMES;
    u32 i;

    prive_xfer = malloc(sizeof(FI3cPriveXfer));
    memset(prive_xfer, 0, sizeof(FI3cPriveXfer));
    FI3C_DEBUG("Write data byte len = %d.", byte_len);
    prive_xfer[0].length = byte_len;

    /*i3c mode requires sending the address once first*/
    if (instance_p->config.trans_mode == FI3C_MODE)
    {
        if (instance_p->config.sub_scba == SUB_ADDRESS_16BIT)
        {
            for (i = byte_len; i > 1; i--)
            {
                write_buf[i] = write_buf[i - 2];
            }
            write_buf[0] = sub_address;
            write_buf[1] = sub_address >> 8;
        }
        else
        {
            for (i = byte_len; i > 0; i--)
            {
                write_buf[i] = write_buf[i - 1];
            }
            write_buf[0] = sub_address;
        }
    }
    prive_xfer[0].date_out = write_buf;
    /*start write data*/
    ret = FI3cMasterDataXfers(instance_p, slave_address, sub_address, prive_xfer, num_xfer);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("Write failed.");
    }
    free(prive_xfer);

    return ret;
}

/**
 * @name: FI3cMasterReadData
 * @msg:  轮询模式下从从机读出数据
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {uintptr} slave_addr 从机地址
 * @param {uintptr} sub_address 从机内部偏移地址
 * @param {u8} *read_buf 数据缓存
 * @param {u32} length 传输字节长度
 */
FError FI3cMasterReadData(FI3c *instance_p, uintptr slave_address, uintptr sub_address,
                          u8 *read_buf, u8 byte_len)
{
    FASSERT(instance_p);
    FASSERT(byte_len <= FI3C_XFER_MAX_BYTE_LEN);
    FError ret = FI3C_SUCCESS;
    FI3cPriveXfer *prive_xfer;
    u32 num_xfer = FI3C_CMD_SEND_TIMES;

    prive_xfer = malloc(sizeof(FI3cPriveXfer));
    memset(prive_xfer, 0, sizeof(FI3cPriveXfer));
    FI3C_DEBUG("Read data byte len = %d.", byte_len);
    prive_xfer[0].rnw = FI3C_CMD0_FIFO_RNW_ENABLE;
    prive_xfer[0].length = byte_len;
    prive_xfer[0].date_in = read_buf;

    /*start read data*/
    ret = FI3cMasterDataXfers(instance_p, slave_address, sub_address, prive_xfer, num_xfer);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("Read failed.");
    }

    free(prive_xfer);
    return ret;
}

/**
 * @name: FI3cMasterGetCmdr
 * @msg:  读取已发送命令，从机返回内容
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {FI3cCmdConfig}  已发送命令
 */
FError FI3cMasterGetCmdr(FI3c *instance_p, FI3cCmdConfig *cmd)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 cmdr_reg, cmd_id, xfer_bytes, error;

    cmdr_reg = FI3C_READ_REG32(base_addr, FI3C_CMDR_OFFSET);
    cmd_id = cmdr_reg & FI3C_CMDR_CMDID_MASK;
    xfer_bytes = (cmdr_reg & FI3C_CMDR_XFER_BYTES_MASK) >> 8;
    error = (cmdr_reg & FI3C_CMDR_ERROR_MASK) >> 24;
    if (error != 0)
    {
        cmd->error = error;
        FI3C_DEBUG("Error occurred! cmd_id = %d, xfer_bytes = %d, error = %x", cmd_id,
                   xfer_bytes, error);
    }
    else
    {
        FI3C_DEBUG("No error occurred! cmd_id = %d, xfer_bytes = %d", cmd_id, xfer_bytes);
    }

    return ret;
}

/**
 * @name: FI3cMasterEnableIBI
 * @msg:  使能带内中断
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u32}  目标设备ID号
 */
FError FI3cMasterEnableIBI(FI3c *instance_p, u32 dev_id)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 sir_map_reg = 0, sir_config = 0;

    sir_map_reg = FI3C_READ_REG32(base_addr, FI3C_SIR_MAP_DEV_REG(dev_id));
    sir_map_reg &= ~FI3C_SIR_MAP_DEV_CONF_MASK(dev_id);
    sir_config = FI3C_SIR_MAP_DEV_ROLE(instance_p->dev[dev_id].dev_cfg.bcr >> 6);
    sir_config |= FI3C_SIR_MAP_DEV_DA(instance_p->dev[dev_id].dev_cfg.slave_addr);
    sir_config |= FI3C_SIR_MAP_DEV_PL(FI3C_XFER_MAX_BYTE_LEN);
    sir_config |= FI3C_SIR_MAP_DEV_ACK;

    if (instance_p->dev[dev_id].dev_cfg.bcr & FI3C_BCR_MAX_DATA_SPEED_LIM)
    {
        sir_config |= FI3C_SIR_MAP_DEV_SLOW;
    }
    sir_map_reg |= FI3C_SIR_MAP_DEV_CONF(dev_id, sir_config);

    FI3C_WRITE_REG32(base_addr, FI3C_SIR_MAP_DEV_REG(dev_id), sir_map_reg);

    ret = FI3cMasterSendCCC(instance_p, FI3C_ENEC_DC_CCC_CMD,
                            instance_p->dev[dev_id].dev_cfg.slave_addr, 0, 0, NULL, 0);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("send ccc ENEC failed!");
    }

    return ret;
}

/**
 * @name: FI3cMasterDisableIBI
 * @msg:  关闭带内中断
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u32}  目标设备ID号
 */
FError FI3cMasterDisableIBI(FI3c *instance_p, u32 dev_id)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 sir_map_reg = 0;

    FI3cMasterSendCCC(instance_p, FI3C_DISEC_DC_CCC_CMD,
                      instance_p->dev[dev_id].dev_cfg.slave_addr, 0, 0, NULL, 0);
    if (ret != FI3C_SUCCESS)
    {
        FI3C_ERROR("send ccc DISEC failed!");
    }

    sir_map_reg = FI3C_READ_REG32(base_addr, FI3C_SIR_MAP_DEV_REG(dev_id));
    sir_map_reg &= ~FI3C_SIR_MAP_DEV_CONF_MASK(dev_id);
    sir_map_reg |= FI3C_SIR_MAP_DEV_CONF(dev_id, FI3C_SIR_MAP_DEV_DA(0x7e));

    FI3C_WRITE_REG32(base_addr, FI3C_SIR_MAP_DEV_REG(dev_id), sir_map_reg);

    return ret;
}

/**
 * @name: FI3cMasterEnableHotJoin
 * @msg:  使能热加入处理
 * @return {FError} 返回错误码
 * @param {FI3c} *instance_p I3C驱动实例数据
 * @param {u32}  目标设备ID号
 */
FError FI3cMasterEnableHotJoin(FI3c *instance_p)
{
    FASSERT(instance_p);
    FError ret = FI3C_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;
    u32 ctrl_reg;

    ctrl_reg = FI3C_READ_REG32(base_addr, FI3C_CTRL_OFFSET);
    ctrl_reg |= FI3C_CTRL_JI_ACK | FI3C_CTRL_DEV_EN;
    FI3C_WRITE_REG32(base_addr, FI3C_CTRL_OFFSET, ctrl_reg);

    return ret;
}