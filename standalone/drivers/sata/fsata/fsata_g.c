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
 * FilePath: fsata_g.c
 * Date: 2022-02-10 14:55:11
 * LastEditTime: 2022-02-18 09:03:08
 * Description:  This file is for static config of sata ctrl
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   wangxiaodong  2022/2/10    first release
 * 1.1   wangxiaodong  2022/10/21   improve functions
 */

#include "fparameters.h"
#include "fsata.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/* configs of pcie ahci ctrl */
const FSataConfig FSataPcieConfigTbl[PLAT_AHCI_HOST_MAX_COUNT] =
{
    [0] =
    {
        .base_addr = AHCI_BASE_0,
        .instance_name = "sata0",
        .irq_num = AHCI_IRQ_0     /* Irq number */

    },
    [1] =
    {
        .base_addr = AHCI_BASE_1,
        .instance_name = "sata1",
        .irq_num = AHCI_IRQ_1     /* Irq number */

    },
    [2] =
    {
        .base_addr = AHCI_BASE_2,
        .instance_name = "sata2",
        .irq_num = AHCI_IRQ_2     /* Irq number */

    },
    [3] =
    {
        .base_addr = AHCI_BASE_3,
        .instance_name = "sata3",
        .irq_num = AHCI_IRQ_3     /* Irq number */

    },
    [4] =
    {
        .base_addr = AHCI_BASE_4,
        .instance_name = "sata4",
        .irq_num = AHCI_IRQ_4     /* Irq number */

    },
};

#if defined(FSATA_NUM)
/* configs of controller ahci ctrl */
const FSataConfig FSataControllerConfigTbl[FSATA_NUM] =
{
    
#if defined(FSATA0_ID)
    [FSATA0_ID] =
    {
        .instance_id = FSATA0_ID,
        .base_addr = FSATA0_BASE_ADDR,
        .instance_name = "sata0",
        .irq_num = FSATA0_IRQ_NUM     /* Irq number */

    },
#endif

#if defined(FSATA1_ID)
    [FSATA1_ID] =
    {
        .instance_id = FSATA1_ID,
        .base_addr = FSATA1_BASE_ADDR,
        .instance_name = "sata1",
        .irq_num = FSATA1_IRQ_NUM     /* Irq number */

    },
#endif

};
#endif

