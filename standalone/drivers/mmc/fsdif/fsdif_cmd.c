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
 * FilePath: fsdif_cmd.c
 * Date: 2022-06-01 14:23:59
 * LastEditTime: 2022-06-01 14:24:00
 * Description:  This file is for sdif command related function
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.1   zhugengyu  2022/6/6     modify according to tech manual.
 * 2.0   zhugengyu  2023/9/16    rename as sdif, support SD 3.0 and rework clock timing
 * 2.1   zhugengyu  2023/10/23   add sdio interrupt handler 
 */
/***************************** Include Files *********************************/

#include "fdrivers_port.h"
#include "fsdif_hw.h"
#include "fsdif.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
#define FSDIF_EXT_APP_CMD 55U

/***************** Macros (Inline Functions) Definitions *********************/
#define FSDIF_DEBUG_TAG   "FSDIF-CMD"
#define FSDIF_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_WARN(format, ...) FT_DEBUG_PRINT_W(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_INFO(format, ...) FT_DEBUG_PRINT_I(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)
#define FSDIF_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FSDIF_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/
extern FError FSdifPIOReadData(FSdif *const instance_p, FSdifData *data_p);

/*****************************************************************************/
FError FSdifSendPrivateCmd11(uintptr base_addr, u32 cmd) /* send CMD11 switch voltage */
{
    u32 reg_val;
    int retries = FSDIF_TIMEOUT;

    FSDIF_DATA_BARRIER(); /* drain writebuffer */

    FSDIF_WRITE_REG(base_addr, FSDIF_CMD_OFFSET, FSDIF_CMD_START | cmd);

    do
    {
        reg_val = FSDIF_READ_REG(base_addr, FSDIF_CMD_OFFSET);
        if (--retries <= 0)
        {
            break;
        }
    } while (FSDIF_CMD_START & reg_val); /* wait until command send done */

    return (retries <= 0) ? FSDIF_ERR_TIMEOUT : FSDIF_SUCCESS;
}

FError FSdifSendPrivateCmd(uintptr base_addr, u32 cmd, u32 arg)
{
    u32 reg_val;
    int retries = FSDIF_TIMEOUT;

    FSDIF_WRITE_REG(base_addr, FSDIF_CMD_ARG_OFFSET, arg);

    FSDIF_DATA_BARRIER(); /* drain writebuffer */

    FSDIF_WRITE_REG(base_addr, FSDIF_CMD_OFFSET, FSDIF_CMD_START | cmd);

    retries = FSDIF_TIMEOUT;
    do
    {
        reg_val = FSDIF_READ_REG(base_addr, FSDIF_CMD_OFFSET);
        if (--retries <= 0)
        {
            break;
        }
    } while (FSDIF_CMD_START & reg_val); /* wait until command send done */

    return (retries <= 0) ? FSDIF_ERR_TIMEOUT : FSDIF_SUCCESS;
}

/**
 * @name: FSdifTransferCmd
 * @msg: pack and transfer command
 * @return {FError} FSDIF_SUCCESS if transfer success
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifCmdData} *cmd_data_p, contents of transfer command and data
 */
FError FSdifTransferCmd(FSdif *const instance_p, FSdifCmdData *const cmd_data_p)
{
    FASSERT(cmd_data_p);
    FError ret = FSDIF_SUCCESS;
    uintptr base_addr = instance_p->config.base_addr;

    FSDIF_DEBUG("============[%s-%d]@0x%x begin ============",
                (FSDIF_EXT_APP_CMD == instance_p->prev_cmd) ? "ACMD" : "CMD",
                cmd_data_p->cmdidx, base_addr);
    FSDIF_DEBUG("    cmd: 0x%x", cmd_data_p->rawcmd);
    FSDIF_DEBUG("    arg: 0x%x", cmd_data_p->cmdarg);

    /* enable related interrupt */
    FSdifSetInterruptMask(instance_p, FSDIF_GENERAL_INTR, FSDIF_INTS_CMD_MASK, TRUE);

    ret = FSdifSendPrivateCmd(base_addr, cmd_data_p->rawcmd, cmd_data_p->cmdarg);
    FSDIF_INFO("cmd send done ...");
    return ret;
}

/**
 * @name: FSdifGetCmdResponse
 * @msg: Get cmd response and received data after wait poll status or interrupt signal
 * @return {FError} FSDIF_SUCCESS if get success
 * @param {FSdif} *instance_p, SDIF controller instance
 * @param {FSdifCmdData} *cmd_data_p, contents of transfer command and data
 */
FError FSdifGetCmdResponse(FSdif *const instance_p, FSdifCmdData *const cmd_data_p)
{
    FASSERT(instance_p);
    FASSERT(cmd_data_p);
    FError ret = FSDIF_SUCCESS;
    const boolean read = !(cmd_data_p->rawcmd & FSDIF_CMD_DAT_WRITE);
    uintptr base_addr = instance_p->config.base_addr;

    if (FT_COMPONENT_IS_READY != instance_p->is_ready)
    {
        FSDIF_ERROR("Device is not yet initialized!!!");
        return FSDIF_ERR_NOT_INIT;
    }

    if ((NULL != cmd_data_p->data_p) && (read))
    {
        if (FSDIF_PIO_TRANS_MODE == instance_p->config.trans_mode)
        {
            ret = FSdifPIOReadData(instance_p, cmd_data_p->data_p);
        }
    }

    /* check response of cmd */
    if (FSDIF_CMD_RESP_EXP & cmd_data_p->rawcmd)
    {
        if (FSDIF_CMD_RESP_LONG & cmd_data_p->rawcmd)
        {
            cmd_data_p->response[0] = FSDIF_READ_REG(base_addr, FSDIF_RESP0_OFFSET);
            cmd_data_p->response[1] = FSDIF_READ_REG(base_addr, FSDIF_RESP1_OFFSET);
            cmd_data_p->response[2] = FSDIF_READ_REG(base_addr, FSDIF_RESP2_OFFSET);
            cmd_data_p->response[3] = FSDIF_READ_REG(base_addr, FSDIF_RESP3_OFFSET);

            FSDIF_DEBUG("    resp: 0x%x-0x%x-0x%x-0x%x", cmd_data_p->response[0],
                        cmd_data_p->response[1], cmd_data_p->response[2],
                        cmd_data_p->response[3]);
        }
        else
        {
            cmd_data_p->response[0] = FSDIF_READ_REG(base_addr, FSDIF_RESP0_OFFSET);
            cmd_data_p->response[1] = 0U;
            cmd_data_p->response[2] = 0U;
            cmd_data_p->response[3] = 0U;
            FSDIF_DEBUG("    resp: 0x%x", cmd_data_p->response[0]);
        }
    }

    FSDIF_DEBUG("============[%s-%d]@0x%x end  ============",
                (FSDIF_EXT_APP_CMD == instance_p->prev_cmd) ? "ACMD" : "CMD",
                cmd_data_p->cmdidx, base_addr);

    /* disable related interrupt */
    FSdifSetInterruptMask(instance_p, FSDIF_GENERAL_INTR,
                          FSDIF_INTS_CMD_MASK | FSDIF_INTS_DATA_MASK, FALSE);
    FSdifSetInterruptMask(instance_p, FSDIF_IDMA_INTR, FSDIF_DMAC_INTS_MASK, FALSE);

    instance_p->prev_cmd = cmd_data_p->cmdidx; /* record previous command */

    return ret;
}
