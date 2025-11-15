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
 * FilePath: spiffs_port.c
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:47
 * Description:  This file is for providing func that divide sfud api into qspi and spi. 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  zhugengyu   2022/4/15    first commit, support Spiffs
 */

/***************************** Include Files *********************************/

#include "sdkconfig.h"
#include "fassert.h"

#include "spiffs_port.h"

#ifdef CONFIG_SPIFFS_ON_FSPIM_SFUD
#include "fspim_spiffs_port.h"
#endif
#ifdef CONFIG_SPIFFS_ON_FSPIM_V2_SFUD
#include "fspim_spiffs_port.h"
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
#include "fqspi_spiffs_port.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSpiffsInitialize(FSpiffs *const instance, fsize_t flash_id)
{
    int ret = -1;

    if (FSPIFFS_PORT_TO_FSPIM == instance->type)
    {
        ret = FSpiffsSpimInitialize(instance, flash_id);
    }
    else if (FSPIFFS_PORT_TO_FQSPI == instance->type)
    {
        ret = FSpiffsQspiInitialize(instance, flash_id);
    }

    return ret;
}

void FSpiffsDeInitialize(FSpiffs *const instance)
{
    if (FSPIFFS_PORT_TO_FSPIM == instance->type)
    {
        return FSpiffsSpimDeInitialize(instance);
    }

    if (FSPIFFS_PORT_TO_FQSPI == instance->type)
    {
        return FSpiffsQspiDeInitialize(instance);
    }
}

const spiffs_config *FSpiffsGetDefaultConfig(FSpiffsPortType type)
{
    const spiffs_config *pconfig = NULL;

    if (FSPIFFS_PORT_TO_FSPIM == type)
    {
        pconfig = FSpiffsSpimGetDefaultConfig();
    }
    else if (FSPIFFS_PORT_TO_FQSPI == type)
    {
        pconfig = FSpiffsQspiGetDefaultConfig();
    }

    return pconfig;
}