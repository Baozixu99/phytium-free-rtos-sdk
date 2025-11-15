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
 * FilePath: fspim.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 09:08:32
 * Description:  This file is for spim api implementation.
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

/***************************** Include Files *********************************/

#include <string.h>
#include "fio.h"
#include "ferror_code.h"
#include "ftypes.h"
#include "fdrivers_port.h"
#include "fspim_hw.h"
#include "fspim.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FSPIM_DEBUG_TAG "SPIM"
#define FSPIM_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_WARN(format, ...) FT_DEBUG_PRINT_W(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_INFO(format, ...) FT_DEBUG_PRINT_I(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSPIM_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
FError FSpimReset(FSpim *instance_p);


/*****************************************************************************/

/* 此文件主要为了完成用户对外接口，用户可以使用这些接口直接开始工作 */

/* - 包括用户API的定义和实现
   - 同时包含必要的OPTION方法，方便用户进行配置
   - 如果驱动可以直接进行I/O操作，在此源文件下可以将API 进行实现 */

/*
 * @name: FSpimCfgInitialize
 * @msg:  Initializes a specific instance such that it is ready to be used.
 * @param {FSpim} *instance_p FSPIM驱动控制数据
 * @param {FSpimConfig} *config_p FSPIM驱动配置数据
 * @return 驱动初始化的错误码信息，FSPIM_SUCCESS 表示初始化成功，其它返回值表示初始化失败
 */
FError FSpimCfgInitialize(FSpim *instance_p, const FSpimConfig *input_config_p)
{
    FASSERT(instance_p && input_config_p);

    FError ret = FSPIM_SUCCESS;
    /*
     * If the device is started, disallow the initialize and return a Status
     * indicating it is started.  This allows the user to de-initialize the device
     * and reinitialize, but prevents a user from inadvertently
     * initializing.
     */
    if (FT_COMPONENT_IS_READY == instance_p->is_ready)
    {
        FSPIM_WARN("The device has been initialized!!!");
    }

    /*
     * Set default values and configuration data, including setting the
     * callback handlers to stubs  so the system will not crash should the
     * application not assign its own callbacks.
     */
    FSpimDeInitialize(instance_p);
    instance_p->config = *input_config_p;

    /* Reset the device. */
    ret = FSpimReset(instance_p);
    if (FSPIM_SUCCESS == ret)
    {
        instance_p->is_ready = FT_COMPONENT_IS_READY;
    }

    return ret;
}

/**
 * @name: FSpimDeInitialize
 * @msg: DeInitialization function for the device instance
 * @return {无}
 * @param {FSpim} *instance_p FSPIM驱动控制数据
 */
void FSpimDeInitialize(FSpim *instance_p)
{
    FASSERT(instance_p);

    instance_p->is_ready = 0;
    memset(instance_p, 0, sizeof(*instance_p));

    return;
}

/**
 * @name: FSpimReset
 * @msg: Reset FSPIM controller
 * @return {FError} FSPIM_SUCCESS表示重置成功，其它返回值表示重置失败
 * @param {FSpim} *instance_p
 */
FError FSpimReset(FSpim *instance_p)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    FError ret = FSPIM_SUCCESS;
    u32 reg_val;
    u32 fifo;

    /* 禁用SPI控制器 */
    FSpimSetEnable(base_addr, FALSE);

    /* 选择数据长度和帧格式 */
    reg_val = FSPIM_CTRL_R0_DFS(FSPIM_DEFAULT_DFS) |
              FSPIM_CTRL_R0_FRF(FSPIM_DEFAULT_FRF) | FSPIM_CTRL_R0_CFS(FSPIM_DEFAULT_CFS);

    if (instance_p->config.en_test)
    {
        reg_val |= FSPIM_CTRL_R0_SLV_SRL(FSPIM_SRL_TEST); /* 设置测试模式，TX Fifo和RX Fifo内部短接 */
    }
    else
    {
        reg_val |= FSPIM_CTRL_R0_SLV_SRL(FSPIM_SRL_NORAML); /* 设置为正常模式 */
    }

    FSpimSetCtrlR0(base_addr, reg_val);

    /* 选择串行时钟极性和相位 */
    FSpimSetCpha(base_addr, instance_p->config.cpha);
    FSpimSetCpol(base_addr, instance_p->config.cpol);

    /* 设置传输模式 */
    FSpimSetTransMode(base_addr, FSPIM_TRANS_MODE_RX_TX);

    /* 禁用spi的slave模式输出 */
    FSpimSetSlaveEnable(base_addr, FALSE);

    /* 禁用SPI 中断，设置slave设备 */
    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);
    FSpimSelSlaveDev(base_addr, instance_p->config.slave_dev_id);

    /* 获取SPI RX/TX FIFO 深度 */
    if (0 == instance_p->tx_fifo_len)
    {
        fifo = FSpimGetTxFifoDepth(base_addr); /* 检测fifo深度 */
        instance_p->tx_fifo_len = fifo;
        FSPIM_INFO("The fifo depth is %d ,tx effective length bits %d", fifo, instance_p->tx_fifo_len);
    }

    if (0 == instance_p->rx_fifo_len)
    {
        fifo = FSpimGetRxFifoDepth(base_addr);
        instance_p->rx_fifo_len = fifo;
        FSPIM_INFO("The fifo depth is %d ,rx effective length bits %d", fifo, instance_p->rx_fifo_len);
    }

    FSPIM_WRITE_REG32(base_addr, FSPIM_DMA_CR_OFFSET, 0x0); /* disable ddma */

    if (instance_p->config.trans_way == TRANS_WAY_DDMA)
    {
        instance_p->config.en_dma = TRUE;
    }

    FSpimSetCtrlR1(base_addr, 0);
    FSpimSetRxFifoThreshold(base_addr, instance_p->config.rx_fifo_threshold);
    FSpimSetTxFifoThreshold(base_addr, instance_p->config.tx_fifo_threshold);
    FSpimSetRxDMALevel(base_addr, instance_p->config.rx_dma_level);
    FSpimSetTxDMALevel(base_addr, instance_p->config.tx_dma_level);

    ret = FSpimSetSpeed(base_addr, instance_p->config.sclk_hz);
    if (FSPIM_SUCCESS != ret)
    {
        return ret;
    }

    FSPIM_WRITE_REG32(base_addr, FSPIM_RX_SAMPLE_DLY_OFFSET, 0);

    return ret;
}

/*
 * @name: FSpimSetOption
 * @msg:  Give user a way to set speed and polarity etc.
 * @param {FSpim} *instance_p FSPIM驱动控制数据
 * @param {u32} option FSPIM操作标识数
 * @param {u32} value FSPIM用户自定参数
 * @return 驱动初始化的错误码信息，FSPIM_SUCCESS 表示设置成功，其它返回值表示设置失败
 */
FError FSpimSetOption(FSpim *instance_p, FSpimOption option, u32 value)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    FError ret = FSPIM_SUCCESS;
    boolean enabled = FSpimGetEnable(base_addr);

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSPIM_ERROR("The device is not initialized!!!");
        return FSPIM_ERR_NOT_READY;
    }

    FSpimSetEnable(base_addr, FALSE);

    if (option == FSPIM_CPOLTYPE_OPTION)
    {
        if (value == FSPIM_CPOL_HIGH || value == FSPIM_CPOL_LOW)
        {
            FSpimSetCpol(base_addr, value);
            instance_p->config.cpol = value;
            FSPIM_INFO("Set cpol to %d", value);
        }
        else
        {
            FSPIM_ERROR("Input error, CPOL value should be 0 or 1.");
            return FSPIM_ERR_INVAL_PARAM;
        }
    }

    if (option == FSPIM_CPHATYPE_OPTION)
    {
        if (value == FSPIM_CPHA_2_EDGE || value == FSPIM_CPHA_1_EDGE)
        {
            FSpimSetCpha(base_addr, value);
            instance_p->config.cpha = value;
            FSPIM_INFO("Set cpha to %d", value);
        }
        else
        {
            FSPIM_ERROR("Input error, CPHA value should be 0 or 1.");
            return FSPIM_ERR_INVAL_PARAM;
        }
    }

    if (option == FSPIM_FREQUENCY_OPTION)
    {
        if (value <= (FSPI_CLK_FREQ_HZ / FSPIM_BAUD_R_SCKDV_MIN))
        {
            ret = FSpimSetSpeed(base_addr, value);
            if (FSPIM_SUCCESS != ret)
            {
                return ret;
            }
            instance_p->config.sclk_hz = value;
            FSPIM_INFO("Set spim freqency to %d", value);
        }
        else
        {
            FSPIM_ERROR("Input error, spim freqency value should be no more than %ld.",
                        FSPI_CLK_FREQ_HZ / 2);
            return FSPIM_ERR_INVAL_PARAM;
        }
    }

    if (option == FSPIM_SLAVE_ID_OPTION) /* 切换当前使能的cs */
    {
        if (value >= FSPIM_SLAVE_DEV_0 && value < FSPIM_NUM_OF_SLAVE_DEV)
        {
            instance_p->config.slave_dev_id = value;
            FSpimSelSlaveDev(base_addr, value);
            FSPIM_INFO("Set spi slave device id to %d", value);
        }
        else
        {
            FSPIM_ERROR("Input error, spi slave device id can only be one of 0, 1, 2, "
                        "3");
            return FSPIM_ERR_INVAL_PARAM;
        }
    }

    if (enabled)
    {
        FSpimSetEnable(base_addr, TRUE);
    }

    return ret;
}

/*
 * @name: FSpimGetOption
 * @msg:  Give user a way to get speed and polarity etc.
 * @param {FSpim} *instance_p FSPIM驱动控制数据
 * @param {u32} option FSPIM操作标识数
 * @return {u32} 获取到的参数值
 */
u32 FSpimGetOption(FSpim *instance_p, FSpimOption option)
{
    FASSERT(instance_p);
    uintptr base_addr = instance_p->config.base_addr;
    u32 value = 0;

    if (option == FSPIM_CPOLTYPE_OPTION)
    {
        value = FSpimGetCpol(base_addr);
        FSPIM_INFO("Get cpol value: %d", value);
    }
    else if (option == FSPIM_CPHATYPE_OPTION)
    {
        value = FSpimGetCpha(base_addr);
        FSPIM_INFO("Get cpha value: %d", value);
    }
    else if (option == FSPIM_FREQUENCY_OPTION)
    {
        value = FSpimGetSpeed(base_addr);
        FSPIM_INFO("Get freq_clock value: 0x%x, %d", value, value);
    }
    else
    {
        FSPIM_ERROR("Option is invalid.");
    }

    return value;
}
/**
 * @name: FSpimGetTxRound
 * @msg: 计算当前FIFO支持的发送字节数
 * @return {fsize_t} 当前TX FIFO可以容纳的字节数
 * @param {FSpim} *instance_p
 */
static fsize_t FSpimGetTxRound(FSpim *instance_p)
{
    fsize_t data_width = instance_p->config.n_bytes;
    uintptr base_addr = instance_p->config.base_addr;
    fsize_t tx_left_round, tx_fifo_room, rx_tx_gap;

    tx_left_round = (fsize_t)(instance_p->length - instance_p->tx_count) / data_width;

    tx_fifo_room = instance_p->tx_fifo_len - FSpimGetTxFifoLevel(base_addr);

    rx_tx_gap = ((fsize_t)(instance_p->length - instance_p->rx_count) -
                 (fsize_t)(instance_p->length - instance_p->tx_count)) /
                data_width;

    FSPIM_DEBUG("tx_left_round: %d, tx_fifo_room: %d, gap: %d, instance_p->tx_count: "
                "%ld",
                tx_left_round, tx_fifo_room,
                ((fsize_t)(instance_p->tx_fifo_len) - rx_tx_gap), instance_p->tx_count);
    return min3(tx_left_round, tx_fifo_room, ((fsize_t)(instance_p->tx_fifo_len) - rx_tx_gap));
}

/**
 * @name: FSpimFifoTx
 * @msg: 利用Fifo进行发送
 * @return {无}
 * @param {FSpim} *instance_p
 */
void FSpimFifoTx(FSpim *instance_p)
{
    FASSERT(instance_p);
    fsize_t tx_round = FSpimGetTxRound(instance_p);
    FSPIM_DEBUG("tx round: %d", tx_round);
    uintptr base_addr = instance_p->config.base_addr;
    u32 data_width = instance_p->config.n_bytes;
    u16 data = 0;

    if (FSPIM_2_BYTE == data_width)
    {
        data = FSPIM_TWO_BYTE_DATA_MASK;
    }
    else
    {
        data = FSPIM_ONE_BYTE_DATA_MASK;
    }

    while (tx_round)
    {
        if (instance_p->tx_buff)
        {
            if (FSPIM_1_BYTE == data_width)
            {
                /*
                 * Data Transfer Width is Byte (8 bit).
                 */
                data = *(u8 *)(instance_p->tx_buff);
            }
            else if (FSPIM_2_BYTE == data_width)
            {
                /*
                 * Data Transfer Width is Half Word (16 bit).
                 */
                data = *(u16 *)(instance_p->tx_buff);
            }
            else
            {
                FASSERT(0);
            }
            instance_p->tx_buff += data_width;
        }
        instance_p->tx_count += data_width;
        FSpimWriteData(base_addr, data);
        FSPIM_DEBUG("  send 0x%x", data);
        tx_round--;
    }
}

/**
 * @name: FSpimGetRxRound
 * @msg: 获取当前Fifo支持的接收字节数
 * @return {fsize_t} 当前RX FIFO可以容纳的字节数
 * @param {FSpim} *instance_p
 */
static fsize_t FSpimGetRxRound(FSpim *instance_p)
{
    fsize_t data_width = instance_p->config.n_bytes;
    uintptr base_addr = instance_p->config.base_addr;

    fsize_t rx_left_round = (fsize_t)(instance_p->length - instance_p->rx_count) / data_width;

    FSPIM_DEBUG("left round %d, rx level %d,instance_p->rx_count %ld", rx_left_round,
                FSpimGetRxFifoLevel(base_addr), instance_p->rx_count);
    return min(rx_left_round, (fsize_t)FSpimGetRxFifoLevel(base_addr));
}

/**
 * @name: FSpimFifoRx
 * @msg: 利用Fifo进行接收
 * @return {无}
 * @param {FSpim} *instance_p
 */
void FSpimFifoRx(FSpim *instance_p)
{
    FASSERT(instance_p);
    fsize_t rx_round = FSpimGetRxRound(instance_p);
    FSPIM_DEBUG("rx round: %d", rx_round);
    uintptr base_addr = instance_p->config.base_addr;
    u32 data_width = instance_p->config.n_bytes;
    u16 data;
    while (rx_round)
    {
        data = FSpimReadData(base_addr);
        if (instance_p->rx_buff)
        {
            if (FSPIM_1_BYTE == data_width)
            {
                /*
                 * Data Transfer Width is Byte (8 bit).
                 */
                *(u8 *)(instance_p->rx_buff) = (u8)data;
                FSPIM_DEBUG("  recv 0x%x", *(u8 *)(instance_p->rx_buff));
            }
            else if (FSPIM_2_BYTE == data_width)
            {
                /*
                 * Data Transfer Width is Half Word (16 bit).
                 */
                *(u16 *)(instance_p->rx_buff) = (u16)data;
                FSPIM_DEBUG("  recv 0x%x", *(u16 *)(instance_p->rx_buff));
            }
            else
            {
                FASSERT(0);
            }
            instance_p->rx_buff += data_width;
        }
        instance_p->rx_count += data_width;
        rx_round--;
    }

    return;
}

/**
 * @name: FSpimTransferPollFifo
 * @msg: 先发送后接收数据 (阻塞处理)，利用Fifo进行处理
 * @return {FError} FSPIM_SUCCESS表示处理成功，其它返回值表示处理失败
 * @param {FSpim} *instance_p 驱动控制数据
 * @param {void} *tx_buf 写缓冲区，可以为空，为空时表示只关注读数据，此时驱动会发送0xff读数据
 * @param {void} *rx_buf 读缓冲区, 可以为空，为空时表示值关注写数据，此时SPI总线上返回的数据会被抛弃
 * @param {fsize_t} len 进行传输的长度，如果tx_buf或者rx_buf不为空，则两个buf的长度必须为len
    - 使用此函数前需要确保FSPIM驱动初始化成功
    - 从函数不会使用中断，会按照TX FIFO的深度进行传输，每次发送填满TX FIFO后触发发送/接收动作
 */
FError FSpimTransferPollFifo(FSpim *instance_p, const void *tx_buf, void *rx_buf, fsize_t len)
{
    FASSERT(instance_p);
    u32 reg_val;
    uintptr base_addr = instance_p->config.base_addr;
    u32 data_width = instance_p->config.n_bytes;

    FError ret = FSPIM_SUCCESS;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSPIM_ERROR("The device is not initialized!!!");
        return FSPIM_ERR_NOT_READY;
    }
    FSPIM_DEBUG("buff address rx= %x, tx=%x\r\n", rx_buf, tx_buf);
    FSpimSetEnable(base_addr, FALSE);

    reg_val = FSpimGetCtrlR0(base_addr);

    reg_val &= ~FSPIM_CTRL_R0_DFS_MASK;
    reg_val |= FSPIM_CTRL_R0_DFS((data_width << 3) - 1);

    reg_val &= ~FSPIM_CTRL_R0_TMOD_MASK;
    if (tx_buf && rx_buf)
    {
        reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_TX);
    }
    else if (rx_buf)
    {
        reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_ONLY);
    }
    else
    {
        reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_TX);
    }

    FSpimSetCtrlR0(base_addr, reg_val);

    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);
    instance_p->tx_count = 0;
    instance_p->rx_count = 0;
    instance_p->length = len;
    instance_p->tx_buff = tx_buf;
    instance_p->rx_buff = rx_buf;
    FSPIM_DEBUG("tx buff@%p-%d, rx buff@%p-%d", instance_p->tx_buff, len, instance_p->rx_buff, len);

    FSpimSetEnable(base_addr, TRUE);
    do
    {
        FSpimFifoTx(instance_p);
        FSpimFifoRx(instance_p);

    } while (instance_p->tx_count < len || instance_p->rx_count < len);
    return ret;
}

/**
 * @name: FSpimTransferByInterrupt
 * @msg: 配置并打开spim中断传输，利用Fifo进行处理
 * @return {FError} FSPIM_SUCCESS表示成功打开中断，其它返回值表示失败
 * @param {FSpim} *instance_p 驱动控制数据
 * @param {void} *tx_buf 写缓冲区
 * @param {void} *rx_buf 读缓冲区
 * @param {fsize_t} len 读写缓冲区长度 （必须相等）
 */
FError FSpimTransferByInterrupt(FSpim *instance_p, const void *tx_buf, void *rx_buf, fsize_t len)
{
    FASSERT(instance_p);
    u32 reg_val;
    uintptr base_addr = instance_p->config.base_addr;
    u32 data_width = instance_p->config.n_bytes;
    u32 tx_level;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSPIM_ERROR("The device is not initialized!!!");
        return FSPIM_ERR_NOT_READY;
    }

    FSpimSetEnable(base_addr, FALSE);

    reg_val = FSpimGetCtrlR0(base_addr);

    reg_val &= ~FSPIM_CTRL_R0_DFS_MASK;
    reg_val |= FSPIM_CTRL_R0_DFS((data_width << 3) - 1);

    reg_val &= ~FSPIM_CTRL_R0_TMOD_MASK;
    if (tx_buf && rx_buf)
    {
        reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_TX);
    }
    else if (rx_buf)
    {
        reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_ONLY);
    }
    else
    {
        reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_TX);
    }

    FSpimSetCtrlR0(base_addr, reg_val);

    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS);
    instance_p->tx_count = 0;
    instance_p->rx_count = 0;
    instance_p->length = len;
    instance_p->tx_buff = tx_buf;
    instance_p->rx_buff = rx_buf;
    /* 设置中断触发的时机，fifo填满一半，或者所有的数据填完 */
    tx_level = min(instance_p->tx_fifo_len / 2 + 1, instance_p->length / data_width);
    FSpimSetTxFifoThreshold(base_addr, tx_level);
    FSpimUmaskIrq(base_addr, FSPIM_IMR_TXEIS | FSPIM_IMR_TXOIS | FSPIM_IMR_RXUIS | FSPIM_IMR_RXOIS);

    FSpimSetEnable(base_addr, TRUE);

    return FSPIM_SUCCESS;
}

/**
 * @name: FSpimTransferDMA
 * @msg: 启动SPIM DMA数据传输
 * @return {FError} FSPIM_SUCCESS表示启动DMA传输成功，其它值表示失败
 * @param {FSpim} *instance_p, 驱动控制数据
 * @param {boolean} tx, TRUE: 启动发送DMA
 * @param {boolean} rx, TRUE: 启动接收DMA
 */
FError FSpimTransferDMA(FSpim *instance_p)
{
    FASSERT(instance_p);
    FASSERT(instance_p->config.caps & FSPI_DMA_CAPACITY);
    u32 reg_val;
    uintptr base_addr = instance_p->config.base_addr;
    u32 data_width = instance_p->config.n_bytes;

    /* disable ddma */
    FSpimDisenableDdmaChannel(base_addr);

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSPIM_ERROR("device is not yet initialized!!!");
        return FSPIM_ERR_NOT_READY;
    }

    FSpimSetEnable(base_addr, FALSE);

    /* set up spim transfer mode */
    reg_val = FSpimGetCtrlR0(base_addr);
    reg_val &= ~FSPIM_CTRL_R0_DFS_MASK;
    reg_val |= FSPIM_CTRL_R0_DFS((data_width << 3) - 1);

    reg_val &= ~FSPIM_CTRL_R0_TMOD_MASK;

    reg_val |= FSPIM_CTRL_R0_TMOD(FSPIM_TMOD_RX_TX);

    FSpimSetCtrlR0(base_addr, reg_val);

    FSpimMaskIrq(base_addr, FSPIM_IMR_ALL_BITS); /* mask all interrupts */

    FSpimSetEnable(base_addr, TRUE);

    /* enable DMA tx / rx */
    reg_val = FSPIM_READ_REG32(base_addr, FSPIM_DMA_CR_OFFSET);

    reg_val |= FSPIM_DMA_CR_TDMAE;
    reg_val |= FSPIM_DMA_CR_RDMAE;
    FSPIM_WRITE_REG32(base_addr, FSPIM_DMA_CR_OFFSET, reg_val);

    return FSPIM_SUCCESS;
}

/**
 * @name: FSpimSetChipSelection
 * @msg: 设置片选信号
 * @return {NONE}
 * @param {FSpim} *instance_p, 驱动控制数据
 * @param {boolean} on, TRUE: 片选打开, FALSE: 片选关闭
 */
void FSpimSetChipSelection(FSpim *instance_p, boolean on)
{
    FASSERT(instance_p);
    u32 reg_val;
    FSpimSlaveDevice cs_n = instance_p->config.slave_dev_id;
    uintptr base_addr = instance_p->config.base_addr;
    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSPIM_ERROR("device is not yet initialized!!!");
        return;
    }

    reg_val = FSPIM_READ_REG32(base_addr, FSPIM_CS_OFFSET);

    if (on)
    {
        reg_val |= FSPIM_CHIP_SEL_EN((u32)cs_n);
        reg_val |= FSPIM_CHIP_SEL((u32)cs_n);
    }
    else
    {
        reg_val &= ~FSPIM_CHIP_SEL_EN((u32)cs_n);
        reg_val &= ~FSPIM_CHIP_SEL((u32)cs_n);
    }

    FSPIM_WRITE_REG32(base_addr, FSPIM_CS_OFFSET, reg_val);

    return;
}
