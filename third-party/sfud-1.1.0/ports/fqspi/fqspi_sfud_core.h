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
 * FilePath: fqspi_sfud_core.h
 * Date: 2022-02-10 14:53:44
 * LastEditTime: 2022-02-25 11:46:38
 * Description:  This files is for providing sfud api based on qspi
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0  wangxiaodong 2022/8/9   first commit
 */


#ifndef FQSPI_SFUD_CORE_H
#define FQSPI_SFUD_CORE_H

#include "sfud.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "sfud_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FQSPI0_SFUD_NAME "FQSPI0"

sfud_err FQspiProbe(sfud_flash *flash) ;

#ifdef __cplusplus
}
#endif

#endif // !
