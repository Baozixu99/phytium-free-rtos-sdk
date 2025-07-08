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
 * Date: 2022-08-19 10:09:31
 * LastEditTime: 2022-08-19 10:09:31
 * Description:  This files is for
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/8/26   first release
 */
#ifndef FSPIM_SFUD_CORE_H
#define FSPIM_SFUD_CORE_H

#include "sfud.h"
#include "sfud_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

sfud_err FSpimProbe(sfud_flash *flash);

#ifdef __cplusplus
}
#endif

#endif // !