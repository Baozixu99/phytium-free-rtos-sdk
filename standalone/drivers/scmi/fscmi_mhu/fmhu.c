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
 * FilePath: fmhu.c
 * Date: 2022-12-29 18:07:32
 * LastEditTime: 2022-12-29 18:07:32
 * Description:  This file is for mhu channel binding
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2023/01/04 init
 */

#include "fassert.h"
#include "fmhu.h"
#include "fmhu_hw.h"
/**
 * @name: 
 * @msg: 
 * @return {*}
 * @param {FScmiMhu} *instance_p
 * @param {int} chan
 */
void FMhuChanProbe(FScmiMhu *instance_p)
{
    FASSERT(instance_p);

    instance_p->tx_complete = FALSE;
    instance_p->msg_count = 0U;

    instance_p->send_data = FMhuSendData;
    instance_p->startup = FMhuStartup;
    instance_p->shutdown = FMhuShutdown;
    instance_p->last_tx_done = FMhuLastTxDone;

    instance_p->mhu.is_ready = FT_COMPONENT_IS_READY;

    instance_p->startup(instance_p->mhu.config.base_addr);
    return;
}

/**
 * @name: 
 * @msg: 
 * @return {*}
 * @param {FScmiMhu} *instance_p
 * @param {int} chan
 */
void FMhuChanRemove(FScmiMhu *instance_p)
{
    FASSERT(instance_p);

    instance_p->tx_complete = TRUE;

    instance_p->send_data = NULL;
    instance_p->startup = NULL;
    instance_p->shutdown = NULL;
    instance_p->last_tx_done = NULL;

    instance_p->mhu.is_ready = 0U;

    instance_p->shutdown(instance_p->mhu.config.base_addr);
    return;
}
