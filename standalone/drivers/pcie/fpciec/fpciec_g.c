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
 * FilePath: fpcie_c_g.c
 * Created Date: 2023-08-01 22:55:30
 * Last Modified: 2023-08-11 15:05:10
 * Description:  This file is for pciec configuration 
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0     huanghe    2023/08/06        first release
 * 1.1     huangjin   2023/12/26        Adaptive pd2308
 */


#include "fpciec.h"
#include "fparameters.h"

FPcieCConfig FPcieCConfigTable[FPCIEC_INSTANCE_NUM] =
{
    [0] =  {
        .instance_id = 0, /* Id of device*/
        
        .config_index = FPCIEC_INSTANCE0_NUM,
        .config_space  = FPCIEC_INSTANCE0_CONFIG_BASE, /* hbp base */
        .control_space = FPCIEC_INSTANCE0_CONTROL_BASE, /* control base */

        .dma_engine_base = FPCIEC_INSTANCE0_DMABASE ,
        .dma_max_num =  FPCIEC_INSTANCE0_DMA_NUM, 
        .misc_irq_num  = FPCIEC_INSTANCE0_MISC_IRQ_NUM ,
    } ,
    #if defined(FPCIEC_INSTANCE1_NUM)
    [1] =  {
        .instance_id = 1, /* Id of device*/
        
        .config_index = FPCIEC_INSTANCE1_NUM,
        .config_space  = FPCIEC_INSTANCE1_CONFIG_BASE, /* hbp base */
        .control_space = FPCIEC_INSTANCE1_CONTROL_BASE, /* control base */

        .dma_engine_base = FPCIEC_INSTANCE1_DMABASE ,
        .dma_max_num =  FPCIEC_INSTANCE1_DMA_NUM, 
    } ,
    #endif
    #if defined(FPCIEC_INSTANCE2_NUM)
    [2] =  {
        .instance_id = 2, /* Id of device*/
        
        .config_index = FPCIEC_INSTANCE2_NUM,
        .config_space  = FPCIEC_INSTANCE2_CONFIG_BASE, /* hbp base */
        .control_space = FPCIEC_INSTANCE2_CONTROL_BASE, /* control base */

        .dma_engine_base = FPCIEC_INSTANCE2_DMABASE ,
        .dma_max_num =  FPCIEC_INSTANCE2_DMA_NUM,
        .misc_irq_num = FPCIEC_INSTANCE3_MISC_IRQ_NUM,
    } ,
    #endif
    #if defined(FPCIEC_INSTANCE3_NUM)
    [3] =  {
        .instance_id = 3, /* Id of device*/
        
        .config_index = FPCIEC_INSTANCE3_NUM,
        .config_space  = FPCIEC_INSTANCE3_CONFIG_BASE, /* hbp base */
        .control_space = FPCIEC_INSTANCE3_CONTROL_BASE, /* control base */

        .dma_engine_base = FPCIEC_INSTANCE3_DMABASE ,
        .dma_max_num =  FPCIEC_INSTANCE3_DMA_NUM, 
    } ,
    #endif
    #if defined(FPCIEC_INSTANCE4_NUM)
    [4] =  {
        .instance_id = 4, /* Id of device*/
        
        .config_index = FPCIEC_INSTANCE4_NUM,
        .config_space  = FPCIEC_INSTANCE4_CONFIG_BASE, /* hbp base */
        .control_space = FPCIEC_INSTANCE4_CONTROL_BASE, /* control base */

        .dma_engine_base = FPCIEC_INSTANCE4_DMABASE ,
        .dma_max_num =  FPCIEC_INSTANCE4_DMA_NUM, 
    } ,
    #endif
    #if defined(FPCIEC_INSTANCE5_NUM)
    [5] =  {
        .instance_id = 5, /* Id of device*/
        
        .config_index = FPCIEC_INSTANCE5_NUM,
        .config_space  = FPCIEC_INSTANCE5_CONFIG_BASE, /* hbp base */
        .control_space = FPCIEC_INSTANCE5_CONTROL_BASE, /* control base */

        .dma_engine_base = FPCIEC_INSTANCE5_DMABASE ,
        .dma_max_num =  FPCIEC_INSTANCE5_DMA_NUM, 
    } 
    #endif
} ;


