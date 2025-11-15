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
 * FilePath: fspim_sfud_core.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:52
 * Description:  This file is for providing qspi based on sfud api;
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu    2021/12/10   first commit
 * 1.0.1 wangxiaodong 2022/12/1    parameter naming change
 */


#ifndef FSPIM_SFUD_H
#define FSPIM_SFUD_H

#include "sfud.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

sfud_err FSpimProbe(sfud_flash *flash);


#ifdef __cplusplus
}
#endif


#endif
