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
 * FilePath: fnand_option.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:51
 * Description:  This file is for options functions for the fnand component.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */


#include "fnand.h"
#include "fnand_hw.h"

/**
 * @name: FNandSetOption
 * @msg:
 * @note:
 * @param {FNand} *instance_p is the pointer to the FNand instance.
 * @param {u32} options is configuration options ，use FNAND_OPS_INTER_MODE_SELECT to select nand flash interface
 * @param {u32} value is set value
 * @return {FError}  FT_SUCCESS set option is ok ，FNAND_ERR_INVAILD_PARAMETER options is invalid
 */
FError FNandSetOption(FNand *instance_p, u32 options, u32 value)
{
    FNandConfig *config_p;
    FASSERT(instance_p != NULL);
    FASSERT(instance_p->is_ready == FT_COMPONENT_IS_READY);
    config_p = &instance_p->config;

    switch (options)
    {
        case FNAND_OPS_INTER_MODE_SELECT:
            FASSERT(FNAND_TOG_ASYN_DDR >= value);
            FNAND_CLEARBIT(config_p->base_address, FNAND_CTRL0_OFFSET, FNAND_CTRL0_INTER_MODE(3UL));
            FNAND_SETBIT(config_p->base_address, FNAND_CTRL0_OFFSET,
                         FNAND_CTRL0_INTER_MODE((unsigned long)value));
            break;
        default:
            return FNAND_ERR_INVAILD_PARAMETER;
    }

    return FT_SUCCESS;
}
