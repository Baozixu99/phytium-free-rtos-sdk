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
 * FilePath: mpy_sd.c
 * Created Date: 2023-12-01 15:22:57
 * Last Modified: 2024-03-22 09:22:40
 * Description:  This file is for the sd function of micropython
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */
#ifndef MPY_SD_H
#define MPY_SD_H


#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
 DEFINE PUBLIC TYPES
 ******************************************************************************/
typedef struct
{
    mp_obj_base_t base;
    bool enabled;
} pybsd_obj_t;

/******************************************************************************
 DECLARE EXPORTED DATA
 ******************************************************************************/
extern pybsd_obj_t pybsd_obj;
extern const mp_obj_type_t pyb_sd_type;
void pyb_sd_init_vfs(fs_user_mount_t *vfs);


#ifdef __cplusplus
}
#endif

#endif
