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
 * Description:  This files is for providing func that divide sfud api into qspi and spi.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong 2022/8/9   first release
 */

/***************************** Include Files *********************************/

#include "sdkconfig.h"
#include "fassert.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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
static xSemaphoreHandle xSpiffsSemaphore;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void FSpiffsSemLock(void)
{
    xSemaphoreTake(xSpiffsSemaphore, portMAX_DELAY);
}

void FSpiffsSemUnlock(void)
{
    xSemaphoreGive(xSpiffsSemaphore);
}

void FSpiffsSemCreate(void)
{
    xSpiffsSemaphore = xSemaphoreCreateMutex();
}

void FSpiffsSemDelete(void)
{
    vSemaphoreDelete(xSpiffsSemaphore);
}

int FSpiffsInitialize(FSpiffs *const instance, FSpiffsPortType type)
{

    FSpiffsSemCreate();
#if defined(CONFIG_SPIFFS_ON_FSPIM_SFUD) || defined(CONFIG_SPIFFS_ON_FSPIM_V2_SFUD)
    FASSERT(FSPIFFS_PORT_TO_FSPIM == type);
    return FSpiffsSpimInitialize(instance);
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    FASSERT(FSPIFFS_PORT_TO_FQSPI == type);
    return FSpiffsQspiInitialize(instance);
#endif
}

void FSpiffsDeInitialize(FSpiffs *const instance)
{
    FSpiffsSemDelete();
#if defined(CONFIG_SPIFFS_ON_FSPIM_SFUD) || defined(CONFIG_SPIFFS_ON_FSPIM_V2_SFUD)
    FSpiffsSpimDeInitialize(instance);
    return;
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    return FSpiffsQspiDeInitialize(instance);
#endif
}

const spiffs_config *FSpiffsGetDefaultConfig(void)
{
#if defined(CONFIG_SPIFFS_ON_FSPIM_SFUD) || defined(CONFIG_SPIFFS_ON_FSPIM_V2_SFUD)
    return FSpiffsSpimGetDefaultConfig();
#endif
#ifdef CONFIG_SPIFFS_ON_FQSPI_SFUD
    return FSpiffsQspiGetDefaultConfig();
#endif
}
