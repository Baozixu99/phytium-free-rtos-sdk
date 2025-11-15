/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
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
 * FilePath: fcpu_affinity_mask.c
 * Date: 2023-11-6 10:33:28
 * LastEditTime: 2023-11-6 10:33:28
 * Description:  This file is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhangyan    2023/11/6       init commit
 */
#include "fparameters.h"
#include "ftypes.h"

const int cluster_ids[FCORE_NUM] = {CORE0_AFF,  CORE1_AFF,  CORE2_AFF,  CORE3_AFF,
                                    CORE4_AFF,  CORE5_AFF,  CORE6_AFF,  CORE7_AFF,
                                    CORE8_AFF,  CORE9_AFF,  CORE10_AFF, CORE11_AFF,
                                    CORE12_AFF, CORE13_AFF, CORE14_AFF, CORE15_AFF};

/**
 * @name: GetCpuMaskToAffval
 * @msg:  Convert information in cpu_mask to cluster_ID and target_list
 * @param {u32} *cpu_mask is each bit of cpu_mask represents a selected CPU, for example, 0x3 represents core0 and CORE1 .
 * @param {u32} *cluster_id is information about the cluster in which core resides ,format is
 * |--------[bit31-24]-------[bit23-16]-------------[bit15-8]-----------[bit7-0]
 * |--------Affinity level3-----Affinity level2-----Affinity level1-----Affinity level0
 * @param {u32} *target_list  is core mask in cluster
 * @return {u32} 0 indicates that the conversion was not successful , 1 indicates that the conversion was successful
 */
u32 GetCpuMaskToAffval(u32 *cpu_mask, u32 *cluster_id, u32 *target_list)
{
    if (*cpu_mask == 0)
    {
        return 0;
    }

    *target_list = 0;
    *cluster_id = 0;

    for (int i = 0; i < sizeof(cluster_ids) / sizeof(cluster_ids[0]); i++)
    {
        if (*cpu_mask & (1 << i))
        {
            *cpu_mask &= ~(1 << i);
            *cluster_id = cluster_ids[i];
            *target_list = 1;
            break;
        }
    }

    return 1;
}
