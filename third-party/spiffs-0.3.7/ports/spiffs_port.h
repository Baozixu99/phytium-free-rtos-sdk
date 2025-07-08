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
 * FilePath: spiffs_port.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:24:52
 * Description:  This files is for giving tolal spiffs init api.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong 2022/8/9   first release
 */

#ifndef SPIFFS_PORT_H
#define SPIFFS_PORT_H

/***************************** Include Files *********************************/
#include "ftypes.h"

#include "spiffs.h"
/************************** Constant Definitions *****************************/

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    FSPIFFS_PORT_TO_FSPIM = 0,
    FSPIFFS_PORT_TO_FQSPI = 1,
} FSpiffsPortType;

enum
{
    FSPIFFS_PORT_OK = 0,
};

/**************************** Type Definitions *******************************/
typedef struct spiffs_t spiffs;
typedef struct
{
    spiffs fs;
    u32    fs_ready;
    u32    fs_addr;
    u32    fs_size;
} FSpiffs;
/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int FSpiffsInitialize(FSpiffs *const instance, FSpiffsPortType type);
void FSpiffsDeInitialize(FSpiffs *const instance);
const spiffs_config *FSpiffsGetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif
