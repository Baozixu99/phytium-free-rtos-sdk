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
 * FilePath: fboard_port.h
 * Created Date: 2023-10-27 17:02:35
 * Last Modified: 2023-10-27 09:22:20
 * Description:  This file is for board layer code decoupling
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 *  1.0     zhangyan  2023/10/27    first release
 */
#ifndef FBOARD_PORT_H
#define FBOARD_PORT_H

#include "fdebug.h"
#include "sdkconfig.h"

#ifndef FT_DEBUG_PRINT_I
#define FT_DEBUG_PRINT_I(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_E
#define FT_DEBUG_PRINT_E(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_D
#define FT_DEBUG_PRINT_D(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_W
#define FT_DEBUG_PRINT_W(TAG, format, ...)
#endif

#ifndef FT_DEBUG_PRINT_V
#define FT_DEBUG_PRINT_V(TAG, format, ...)
#endif

#endif