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
 * FilePath: fspim_msg_sfud.h
 * Date: 2025-04-15 10:09:31
 * LastEditTime: 2025-04-15 10:09:31
 * Description:  This files is for spi msg sfud header.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   liyilun  2025-04-15   first release
 */
#ifndef FSPIM_MSG_SFUD_H
#define FSPIM_MSG_SFUD_H

#include "sfud.h"
#include "sfud_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

sfud_err FSpimMsgProbe(sfud_flash *flash);

#ifdef __cplusplus
}
#endif

#endif // !