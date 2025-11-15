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
 * FilePath: lfs_testbd_port.c
 * Date: 2022-04-07 08:41:39
 * LastEditTime: 2022-04-07 08:41:39
 * Description:  This file is for little fs testbd port
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/4/7     init commit
 */

/***************************** Include Files *********************************/
#include "fkernel.h"
#include "fassert.h"
#include "fdebug.h"

#include "sdkconfig.h"

#include "sfud.h"
#include "lfs_testbd.h"
#include "lfs_testbd_port.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FLFS_DEBUG_TAG          "LFS-TESTBD-PORT"
#define FLFS_ERROR(format, ...) FT_DEBUG_PRINT_E(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_WARN(format, ...)  FT_DEBUG_PRINT_W(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_INFO(format, ...)  FT_DEBUG_PRINT_I(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)
#define FLFS_DEBUG(format, ...) FT_DEBUG_PRINT_D(FLFS_DEBUG_TAG, format, ##__VA_ARGS__)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
const struct lfs_testbd_config bdcfg = {
    .erase_value = 255,
    .erase_cycles = 1000000, /* 假设一个块擦写1000000次后会变成坏块 */
    .badblock_behavior = LFS_TESTBD_BADBLOCK_PROGERROR,
    .power_cycles = 1000000, /* 假设1000000次后出现一个掉电事件 */
};

/*****************************************************************************/
const struct lfs_config *FLfsTestBDGetDefaultConfig(void)
{
    static lfs_testbd_t bd;
    static const struct lfs_config config = {
        .context = &bd,
        .read = lfs_testbd_read,
        .prog = lfs_testbd_prog,
        .erase = lfs_testbd_erase,
        .sync = lfs_testbd_sync,
        .read_size = 16,
        .prog_size = 16,
        .block_size = 512,
        .block_count = 256,
        .block_cycles = 16,
        .cache_size = 16,
        .lookahead_size = 16,
    };

    return &config;
}

int FLfsTestBDInitialize(FLfs *const instance, const char *lfs_testbd_path)
{
    FASSERT(instance);

    const struct lfs_config *config = FLfsTestBDGetDefaultConfig();
    int ret = FLFS_DRY_RUN_PORT_OK;
    int err = lfs_testbd_createcfg(config, lfs_testbd_path, &bdcfg);

    if (0 != err)
    {
        FLFS_ERROR("Failed to create tested: %d", err);
        ret = FLFS_DRY_RUN_PORT_INIT_FAILED;
    }

    return ret;
}

void FLfsTestBDDeInitialize(FLfs *const instance)
{
    FASSERT(instance);

    const struct lfs_config *config = FLfsTestBDGetDefaultConfig();
    int err = lfs_testbd_destroy(config);
    if (0 != err)
    {
        FLFS_ERROR("Failed to destroy tested: %d", err);
    }

    return;
}