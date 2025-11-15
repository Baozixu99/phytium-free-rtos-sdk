/*
* Copyright : (C) 2025 Phytium Information Technology, Inc.
* All Rights Reserved.
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
* FilePath: fsdif_msg_g.h
* Date: 2025-04-10 16:20:52
* LastEditTime: 2025-04-10 16:20:52
* Description:  This file is for sdif user interface definition
*
* Modify History:
*  Ver   Who        Date         Changes
* ----- ------     --------    --------------------------------------
* 1.0   zhugengyu  2025/4/10    init commit
*/
/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fparameters.h"

#include "fsdif_msg.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

const FSdifMsgConfig FSDIF_CONFIG_TBL[FSDIF_NUM] =
{
    [FSDIF0_ID] =
    {
        .instance_id  = FSDIF0_ID,
        .dev_msg = 
        {
            .regfile = FSDIF0_MSG_REGFILE_ADDR,
            .shmem = FSDIF0_MSG_SHARE_MEM_ADDR
        },
        .irq_num       = FSDIF0_IRQ_NUM,
        .non_removable = FALSE,
    },
};


/*****************************************************************************/
