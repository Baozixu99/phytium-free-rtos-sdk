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
 * FilePath: fspim_msg.c
 * Date: 2024-10-14 11:27:42
 * LastEditTime: 2024-10-24 17:46:03
 * Description:  This file is for providing spi msg api implementation.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun   2024/10/24   first release
 */
#include <string.h>
#include "fspim_msg.h"
#include "fmsg_common.h"
#include "fspim_msg_hw.h"
#include "fdrivers_port.h"
#include "fassert.h"

#define FSPIM_MSG_DEBUG_TAG "FSPIM-MSG"
#define FSPIM_MSG_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_MSG_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_MSG_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSPIM_MSG_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSPIM_MSG_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/*****************************************************************************/

void *MemcpyByte(void *dest, const void *src, u32 len)
{
    while (len >= 8)
    {
        *(u64 *)dest = *(u64 *)src;
        dest += 8;
        src += 8;
        len -= 8;
    }
    while (len)
    {
        *(u8 *)dest = *(u8 *)src;
        dest++;
        src++;
        len--;
    }
    return dest;
}


/*
 * @name: FSpiMsgConfigValidate
 * @msg:  使spi配置生效，即将spi配置命令下发到Slave核
 * @param {FSpiMsgCtrl} FSpiMsg驱动控制数据
 * @return 驱动初始化的错误码信息，0 表示配置成功，其它返回值表示配置失败
 */

int FSpiMsgConfigValidate(FSpiMsgCtrl *spi_msg_ctrl)
{
    int ret = 0;
    ret = FSpiMsgSetEnable(spi_msg_ctrl, FALSE);

    if (ret != 0)
    {
        FSPIM_MSG_ERROR("Disable chip failed");
        return ret;
    }
    u32 clk_div = spi_msg_ctrl->spi_msg_config.clk_div;
    ret = FSpiMsgSetClkDiv(spi_msg_ctrl, clk_div);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("Set clk div failed");
        return ret;
    }
    FSpimTransByte n_bytes = spi_msg_ctrl->spi_msg_config.n_bytes;
    ret = FSpiMsgSetDataWidth(spi_msg_ctrl, n_bytes);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("Set data width failed");
        return ret;
    }
    FSpimCphaCpolMode mode = spi_msg_ctrl->spi_msg_config.mode;
    ret = FSpiMsgSetMode(spi_msg_ctrl, mode);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("Set mode failed");
        return ret;
    }
    u8 tmod = spi_msg_ctrl->spi_msg_config.tmod;
    ret = FSpiMsgSetTransMode(spi_msg_ctrl, tmod);

    if (ret != 0)
    {
        FSPIM_MSG_ERROR("Set tmod failed");
        return ret;
    }

    ret = FSpiMsgSetEnable(spi_msg_ctrl, TRUE);
    if (ret != 0)
    {
        FSPIM_MSG_ERROR("Enable chip failed");
        return ret;
    }

    return ret;
}

/*
 * @name: FSpiMsgCfgInitialize
 * @msg:  Initializes a specific fspimsg instance such that it is ready to be used.
 * @param {FSpiMsgCtrl} FSpiMsg驱动控制数据
 * @param {FSpiMsgConfig} *spi_msg_config FSpiMsg驱动配置数据
 * @return 驱动初始化的错误码信息，ERR_SUCCESS 表示初始化成功，其它返回值表示初始化失败
 */

int FSpiMsgCfgInitialize(FSpiMsgCtrl *spi_msg_ctrl, const FSpiMsgConfig *spi_msg_config)
{
    int ret = 0;
    FASSERT(spi_msg_ctrl && spi_msg_config);

    if (FT_COMPONENT_IS_READY == spi_msg_ctrl->is_ready)
    {
        FSPIM_MSG_WARN("FSpiMsgCfgInitialize: spi_msg_ctrl is already initialized");
    }

    spi_msg_ctrl->spi_msg_config = *spi_msg_config;
    spi_msg_ctrl->tx_shmem_addr = (void *)spi_msg_config->spi_msg.shmem;
    spi_msg_ctrl->msg = (FSpiMsgInfo *)spi_msg_ctrl->tx_shmem_addr;

    FSpiMsgSetDebug(spi_msg_ctrl, FALSE);

    FSpiMsgSetAlive(spi_msg_ctrl, FALSE);

    ret = FspiMsgSetDefault(spi_msg_ctrl);
    if (ret != ERR_SUCCESS)
    {
        FSPIM_MSG_ERROR("Set default failed");
        return ret;
    }

    ret = FSpiMsgConfigValidate(spi_msg_ctrl);
    if (ret != ERR_SUCCESS)
    {
        FSPIM_MSG_ERROR("Config spi regs failed, %d", ret);
        return ret;
    }

    spi_msg_ctrl->is_ready = FT_COMPONENT_IS_READY;
    return ret;
}

/**
 * @name: FSpiMsgDeInitialize
 * @msg: DeInitialization function for the device instance
 * @return {无}
 * @param {FSpiMsgCtrl} *spi_msg_ctrl FSpiMsg驱动控制数据
 */

int FSpiMsgDeInitialize(FSpiMsgCtrl *spi_msg_ctrl)
{
    int ret = 0;
    FASSERT(spi_msg_ctrl);

    spi_msg_ctrl->is_ready = 0;
    memset(spi_msg_ctrl, 0, sizeof(FSpiMsgCtrl));
    return ret;
}


/**
 * @name: FSpiMsgRead
 * @msg: 通过MSG结构接收数据
 * @return {int} 0表示成功返回，其它返回值表示操作失败
 * @param {FSpiMsgCtrl} *spi_msg_ctrl FSpiMsg驱动控制数据
 */

int FSpiMsgRead(FSpiMsgCtrl *spi_msg_ctrl)
{
    u64 smem_rx;
    int ret = 0;
    u32 recv_len;
    u8 first = 1;

    do
    {
        recv_len = min((u32)(spi_msg_ctrl->rx_end - spi_msg_ctrl->rx), (u32)128);

        spi_msg_ctrl->msg->cmd_type = FMSG_CMD_DATA;
        spi_msg_ctrl->msg->cmd_subid = FSPI_MSG_SUBCMD_DATA_RX;

        smem_rx = (u64)spi_msg_ctrl->msg + sizeof(FSpiMsgInfo);

        *(u64 *)&spi_msg_ctrl->msg->para[0] = sizeof(FSpiMsgInfo);
        *(u32 *)&spi_msg_ctrl->msg->para[8] = recv_len;
        spi_msg_ctrl->msg->para[12] = spi_msg_ctrl->spi_msg_config.slave_dev_id;
        spi_msg_ctrl->msg->para[13] = spi_msg_ctrl->spi_msg_config.n_bytes << 3;
        spi_msg_ctrl->msg->para[14] = spi_msg_ctrl->spi_msg_config.mode;
        spi_msg_ctrl->msg->para[15] = spi_msg_ctrl->spi_msg_config.tmod;
        if (first == 1)
        {
            /*如果是第一次发送，则使能片选*/
            spi_msg_ctrl->msg->para[16] = 1;
        }
        else
        {
            /*如果不是第一次发送，则不操作片选*/
            spi_msg_ctrl->msg->para[16] = 0;
        }
        spi_msg_ctrl->msg->para[17] = first;

        ret = FSpiMsgSet(spi_msg_ctrl);

        if (ret)
        {
            FSPIM_MSG_ERROR("rx failed!");
            return ret;
        }
        MemcpyByte((void *)spi_msg_ctrl->rx, (void *)smem_rx, recv_len);
        spi_msg_ctrl->rx += recv_len;
        first = 0;
    } while (spi_msg_ctrl->rx_end > spi_msg_ctrl->rx);

    return ret;
}

void FSpiMsgPackGeneralFlashCmd(FSpiMsgCtrl *spi_msg_ctrl)
{
    u64 smem_tx;

    int len = spi_msg_ctrl->trans_len;
    smem_tx = (u64)spi_msg_ctrl->msg + sizeof(FSpiMsgInfo);

    MemcpyByte((void *)smem_tx, spi_msg_ctrl->tx, len);

    *(u32 *)&spi_msg_ctrl->msg->para[8] = len;

    *(u32 *)&spi_msg_ctrl->msg->para[0] = sizeof(FSpiMsgInfo);
    spi_msg_ctrl->tx += len;
    spi_msg_ctrl->msg->para[12] = spi_msg_ctrl->spi_msg_config.slave_dev_id;
    spi_msg_ctrl->msg->para[13] = spi_msg_ctrl->spi_msg_config.n_bytes << 3;
    spi_msg_ctrl->msg->para[14] = spi_msg_ctrl->spi_msg_config.mode;
    spi_msg_ctrl->msg->para[15] = spi_msg_ctrl->spi_msg_config.tmod;
    spi_msg_ctrl->msg->para[16] = 1;
    spi_msg_ctrl->msg->para[17] = 1;
}

int FSpiMsgFlashErase(FSpiMsgCtrl *spi_msg_ctrl)
{
    int ret = 0;
    memset(spi_msg_ctrl->msg, 0, sizeof(FSpiMsgInfo));

    spi_msg_ctrl->msg->cmd_type = FMSG_CMD_DATA;

    spi_msg_ctrl->msg->cmd_subid = FSPI_MSG_SUBCMD_DATA_FLASH_ERASE;

    FSpiMsgPackGeneralFlashCmd(spi_msg_ctrl);

    ret = FSpiMsgSet(spi_msg_ctrl);


    return ret;
}

int FSpiMsgWriteFlashCmd(FSpiMsgCtrl *spi_msg_ctrl)
{
    int ret = 0;
    memset(spi_msg_ctrl->msg, 0, sizeof(FSpiMsgInfo));

    spi_msg_ctrl->msg->cmd_type = FMSG_CMD_DATA;

    spi_msg_ctrl->msg->cmd_subid = FSPI_MSG_SUBCMD_DATA_TX;

    FSpiMsgPackGeneralFlashCmd(spi_msg_ctrl);

    ret = FSpiMsgSet(spi_msg_ctrl);

    return ret;
}

int FSpiMsgWriteFlashData(FSpiMsgCtrl *spi_msg_ctrl)
{
    u32 trans_data_len;
    int ret = 0;
    u64 smem_tx;
    u8 cmd_addr[8];

    /* first设置为1，表示是第一次发送该命令, 0：非第一次*/
    u8 first = 1;

    cmd_addr[0] = 1 + spi_msg_ctrl->addr_bytes; //命令和地址长度
    cmd_addr[1] = spi_msg_ctrl->flash_cmd;      //命令
    MemcpyByte((void *)&cmd_addr[2], spi_msg_ctrl->tx + 1, spi_msg_ctrl->addr_bytes); //地址

    /* 封装命令和地址*/
    spi_msg_ctrl->msg->para[18] = cmd_addr[0];
    spi_msg_ctrl->msg->para[19] = cmd_addr[1];
    spi_msg_ctrl->msg->para[20] = cmd_addr[2];
    spi_msg_ctrl->msg->para[21] = cmd_addr[3];
    spi_msg_ctrl->msg->para[22] = cmd_addr[4];
    spi_msg_ctrl->msg->para[23] = cmd_addr[5];

    /* 封装数据 */
    spi_msg_ctrl->tx += spi_msg_ctrl->addr_bytes + 1; //从数据所在地址开始

    do
    {
        trans_data_len = min((u32)(spi_msg_ctrl->tx_end - spi_msg_ctrl->tx), (u32)SPI_TRANS_DATA_SIZE);

        spi_msg_ctrl->msg->cmd_type = FMSG_CMD_DATA;

        spi_msg_ctrl->msg->cmd_subid = FSPI_MSG_SUBCMD_DATA_FLASH_TX;

        smem_tx = (u64)spi_msg_ctrl->msg + sizeof(FSpiMsgInfo);

        MemcpyByte((void *)smem_tx, (void *)spi_msg_ctrl->tx, trans_data_len);

        *(u64 *)&spi_msg_ctrl->msg->para[0] = sizeof(FSpiMsgInfo);

        *(u32 *)&spi_msg_ctrl->msg->para[8] = trans_data_len;

        spi_msg_ctrl->msg->para[12] = spi_msg_ctrl->spi_msg_config.slave_dev_id;
        spi_msg_ctrl->msg->para[13] = spi_msg_ctrl->spi_msg_config.n_bytes << 3;
        spi_msg_ctrl->msg->para[14] = spi_msg_ctrl->spi_msg_config.mode;
        spi_msg_ctrl->msg->para[15] = spi_msg_ctrl->spi_msg_config.tmod;
        spi_msg_ctrl->msg->para[16] = 1;
        spi_msg_ctrl->msg->para[17] = first;

        ret = FSpiMsgSet(spi_msg_ctrl);

        if (ret)
        {
            return ret;
        }

        first = 0;
        spi_msg_ctrl->tx += trans_data_len;

    } while (spi_msg_ctrl->tx_end > spi_msg_ctrl->tx);

    return ret;
}


/**
 * @name: FSpiMsgTransfer
 * @msg: 数据传输接口
 * @return {int} 0表示成功返回，其它返回值表示操作失败
 * @param {FSpiMsgCtrl} *spi_msg_ctrl FSpiMsg驱动控制数据
 * @param {void} *tx_buf 写缓冲区，可以为空
 * @param {void} *rx_buf 读缓冲区, 可以为空
 * @param {fsize_t} len 需要传输的数据长度
    - 使用此函数前需要确保FSPIM驱动初始化成功
 */

int FSpiMsgTransfer(FSpiMsgCtrl *spi_msg_ctrl, const void *tx_buf, void *rx_buf, uint32_t len)
{
    int ret = 0;

    FASSERT(len > 0);

    spi_msg_ctrl->trans_len = len;
    spi_msg_ctrl->tx = (void *)tx_buf;
    spi_msg_ctrl->tx_end = spi_msg_ctrl->tx + len;
    spi_msg_ctrl->rx = (void *)rx_buf;
    spi_msg_ctrl->rx_end = spi_msg_ctrl->rx + len;

    u8 flash_cmd = 0x00;
    if (tx_buf)
    {
        FSPIM_MSG_DEBUG("flash_cmd: %02x\n", *(u8 *)tx_buf);
        flash_cmd = *(u8 *)tx_buf;
        spi_msg_ctrl->flash_cmd = flash_cmd;
        /* 不同命令有不同打包方式，具体分为以下三种方式*/
        /* flash命令为擦除 */
        if (flash_cmd == FSPI_FLASH_CMD_ERASE_4K || flash_cmd == FSPI_FLASH_CMD_ERASE_CHIP)
        {
            ret = FSpiMsgFlashErase(spi_msg_ctrl);
            if (ret != ERR_SPI_SUCEESS)
            {
                FSPIM_MSG_ERROR("flash erase cmd failed!");
            }
        }
        /* flash命令为写数据*/
        else if (flash_cmd == FSPI_FLASH_CMD_PAGE_PROGRAM || flash_cmd == FSPI_FLASH_CMD_PAGE_PROGRAM_4B)
        {
            ret = FSpiMsgWriteFlashData(spi_msg_ctrl);
            if (ret != ERR_SPI_SUCEESS)
            {
                FSPIM_MSG_ERROR("flash write cmd failed!");
            }
        }
        else
        {
            /*flash命令为其他命令*/
            ret = FSpiMsgWriteFlashCmd(spi_msg_ctrl);
            if (ret != ERR_SPI_SUCEESS)
            {
                FSPIM_MSG_ERROR("general flash cmd failed!");
            }
        }
    }
    if (rx_buf)
    {

        ret = FSpiMsgRead(spi_msg_ctrl);
        if (ret != ERR_SPI_SUCEESS)
        {
            FSPIM_MSG_ERROR("flash read cmd failed!");
        }
    }
    return ret;
}


/**
 * @name: FSpiMsgShowMsg
 * @msg: 展示当前发送的消息内容
 * @return {none}
 * @param {FSpiMsgInfo *}， msg，spi消息结构体指针
 * @param {u8} status0, 协议命令状态
 * @param {u8} status1, 某些函数执行后的返回值
 */

void FSpiMsgShowMsg(FSpiMsgInfo *msg)
{
    FSPIM_MSG_DEBUG("cmd:%04x,sub:0x%04x", msg->cmd_type, msg->cmd_subid);
    FSPIM_MSG_DEBUG("para:%02x,%02x,%02x,%02x", msg->para[0], msg->para[1],
                    msg->para[2], msg->para[3]);
    FSPIM_MSG_DEBUG("para:%02x,%02x,%02x,%02x", msg->para[4], msg->para[5],
                    msg->para[6], msg->para[7]);
    FSPIM_MSG_DEBUG("para:%02x,%02x,%02x,%02x", msg->para[8], msg->para[9],
                    msg->para[10], msg->para[11]);
    FSPIM_MSG_DEBUG("para:%02x,%02x,%02x,%02x", msg->para[12], msg->para[13],
                    msg->para[13], msg->para[15]);
    FSPIM_MSG_DEBUG("para:%02x,%02x,%02x,%02x", msg->para[16], msg->para[17],
                    msg->para[18], msg->para[19]);
}
