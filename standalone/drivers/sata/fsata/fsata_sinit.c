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
 * FilePath: fsata_sinit.c
 * Date: 2022-02-10 14:55:11
 * LastEditTime: 2022-02-18 09:04:15
 * Description:  This file is for sata static init
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/2/10    first release
 * 1.1   wangxiaodong  2022/10/21   improve functions
 */

/***************************** Include Files *********************************/

#include "fsata.h"
#include "fparameters.h"
#include "fassert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

extern const FSataConfig FSataPcieConfigTbl[PLAT_AHCI_HOST_MAX_COUNT];

#if defined(SOC_TARGET_PE2204) || defined(SOC_TARGET_PE2202) || \
    defined(SOC_TARGET_PD2308) || defined(SOC_TARGET_PD2408)
extern const FSataConfig FSataControllerConfigTbl[FSATA_NUM];
#endif

/*****************************************************************************/
/**
 * @name: FSataLookupConfig
 * @msg: get sata configs by id and type, Support both pcie and SATA controllers
 * @return {FSataConfig *}
 * @param {u32} instance_id, id of sata ctrl
 */
const FSataConfig *FSataLookupConfig(u32 instance_id, u8 type)
{
    const FSataConfig *pconfig = NULL;

    if (type == FSATA_TYPE_CONTROLLER)
    {
#if defined(SOC_TARGET_PE2204) || defined(SOC_TARGET_PE2202) || \
    defined(SOC_TARGET_PD2308) || defined(SOC_TARGET_PD2408)
        FASSERT(instance_id < FSATA_NUM);
        pconfig = &FSataControllerConfigTbl[instance_id];
#else
        FASSERT_MSG(0, "The processor does not include sata controller.");
#endif
    }
    else
    {
        FASSERT(instance_id < PLAT_AHCI_HOST_MAX_COUNT);
        pconfig = &FSataPcieConfigTbl[instance_id];
    }

    return (const FSataConfig *)pconfig;
}
