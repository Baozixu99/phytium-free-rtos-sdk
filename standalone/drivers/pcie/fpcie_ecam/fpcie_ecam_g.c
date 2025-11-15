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
 * FilePath: fpcie_ecam_g.c
 * Created Date: 2023-08-01 08:59:53
 * Last Modified: 2023-08-08 19:14:03
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 */

#include "fpcie_ecam.h"
#include "fparameters.h"



FPcieEcamConfig FPcieEcamConfigTable[FPCIE_ECAM_INSTANCE_NUM] =
{
    {
        .instance_id = FPCIE_ECAM_INSTANCE0,
        .ecam = FPCI_ECAM_CONFIG_BASE_ADDR,
        
        .io_base_addr = FPCI_ECAM_IO_CONFIG_BASE_ADDR,
        .io_size  = FPCI_ECAM_IO_CONFIG_REG_LENGTH,
        .npmem_base_addr = FPCI_ECAM_MEM32_BASE_ADDR,
        .npmem_size = FPCI_ECAM_MEM32_REG_LENGTH,
        .pmem_base_addr = FPCI_ECAM_MEM64_BASE_ADDR, /* Prefetchable memory */
        .pmem_size = FPCI_ECAM_MEM64_REG_LENGTH,

        .inta_irq_num = FPCIE_ECAM_INTA_IRQ_NUM,
        .intb_irq_num = FPCIE_ECAM_INTB_IRQ_NUM,
        .intc_irq_num = FPCIE_ECAM_INTC_IRQ_NUM,
        .intd_irq_num = FPCIE_ECAM_INTD_IRQ_NUM,


        .max_bus_num = FPCIE_ECAM_CFG_MAX_NUM_OF_BUS ,
        .max_dev_num = FPCIE_ECAM_CFG_MAX_NUM_OF_DEV ,
        .max_fun_num = FPCIE_ECAM_CFG_MAX_NUM_OF_FUN ,
#ifdef FPCI_ECAM_INTX_NEED_EOI

        .control_intx_base = 
        {
            [0]= FPCI_ECAM_INTX_CONTROL_STATUS_REG0,
            [1]= FPCI_ECAM_INTX_CONTROL_STATUS_REG1
        },
        .config_intx_base =
        {
            [0]= FPCI_ECAM_INTX_CONFIG_ISTATUS_REG0,
            [1]= FPCI_ECAM_INTX_CONFIG_ISTATUS_REG1,
            [2]= FPCI_ECAM_INTX_CONFIG_ISTATUS_REG2,
            [3]= FPCI_ECAM_INTX_CONFIG_ISTATUS_REG3,
            [4]= FPCI_ECAM_INTX_CONFIG_ISTATUS_REG4,
            [5]= FPCI_ECAM_INTX_CONFIG_ISTATUS_REG5,
        },
#endif

    }
};