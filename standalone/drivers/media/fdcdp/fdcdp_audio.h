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
 * FilePath: fdcdp_audio.c
 * Created Date: 2023-09-11 09:19:16
 * Last Modified: Tue Oct 31 2023
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ----------------------------- ----
 */
#ifndef FDCDP_AUDIO_H
#define FDCDP_AUDIO_H

/***************************** Include Files *********************************/

#include "ftypes.h"
#include "fdp.h"
#include "fparameters.h"
#ifdef __cplusplus
extern "C"
{
#endif
/**************************** Type Definitions *******************************/

typedef struct
{
    u32 sample_rate;
    u32 link_rate;
    u16 m;
    u16 n;
} FDcDpAudioMN;

/************************** Function Prototypes ******************************/

/*get the m and n value with the link_rate*/
const FDcDpAudioMN *FDpAudioGetMN(FDpCtrl *instance, u32 link_rate);

/*set the audio config */
FError FDpAudioSetPara(FDpCtrl *instance);

/*enable the audio*/
FError FDpAudioEnable(FDpCtrl *instance);

#ifdef __cplusplus
}
#endif

#endif