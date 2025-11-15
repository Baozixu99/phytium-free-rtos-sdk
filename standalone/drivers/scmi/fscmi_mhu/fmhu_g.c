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
 * FilePath: fmhu_g.c
 * Date: 2022-12-29 16:41:46
 * LastEditTime: 2022-12-29 16:41:46
 * Description:  This file is for get default configs
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0 liushengming 2022/12/30 init
 */

#include <string.h>
#include "ftypes.h"
#include "fparameters.h"
#include "fmhu.h"
#include "fassert.h"

void FMhuGetDefConfig(FMhuConfig *config_p,u32 chan_id)
{
    FASSERT(NULL != config_p);

    memset(config_p, 0, sizeof(config_p));
    config_p->base_addr = FSCMI_MHU_BASE_ADDR;
    config_p->irq_addr = FSCMI_MHU_IRQ_ADDR;
    config_p->irq_num = FSCMI_MHU_IRQ_NUM;
    config_p->mhu_chan_id = chan_id;
    config_p->work_mode = TRUE;
}
