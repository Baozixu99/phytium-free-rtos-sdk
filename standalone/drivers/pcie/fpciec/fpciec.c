/*
 * Copyright (C) 2023, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fpciec.c
 * Created Date: 2023-08-01 20:30:10
 * Last Modified: 2023-08-06 09:44:56
 * Description:  This file is for pciec initialization api
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 */

#include "fpciec.h"
#include "ftypes.h"
#include "string.h"
#include "fassert.h"

/**
 * @name: FPcieCCfgInitialize
 * @msg: Initializes an instance of FPcieC using the provided configuration.
 * @param {FPcieC *} instance_p - Pointer to the FPcieC instance.
 * @param {FPcieCConfig *} config_p - Pointer to the FPcieCConfig configuration.
 * @return {FError} - Returns FT_SUCCESS if the initialization is successful.
 */
FError FPcieCCfgInitialize(FPcieC *instance_p, FPcieCConfig *config_p)
{
    /* Assert arguments */
    FASSERT(instance_p != NULL);
    FASSERT(config_p != NULL);

    /* Clear instance memory and make copy of configuration */
    memset(instance_p, 0, sizeof(FPcieC));
    memcpy(&instance_p->config, config_p, sizeof(FPcieCConfig));

    instance_p->is_ready = FT_COMPONENT_IS_READY;

    return FT_SUCCESS;
}