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
 * FilePath: ff_utils.h
 * Date: 2022-07-21 13:21:43
 * LastEditTime: 2022-07-21 13:21:44
 * Description:  This file is for fatfs test utility definition
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2022/12/3   init commit
 */

#ifndef  FF_UTILS_H
#define  FF_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* file system */
#include "ff.h"

typedef struct
{
    FATFS fs;
    BYTE  work[FF_MAX_SS]; /* working buffer used for format process */
    DWORD is_ready;
} ff_fatfs;

FRESULT ff_setup(ff_fatfs *fatfs, const TCHAR *mount_point, const MKFS_PARM *opt, UINT force_format);
void ff_dump_info(const TCHAR *mount_point);
FRESULT ff_append_test (const TCHAR* path);
FRESULT ff_read_test (const TCHAR* path);
FRESULT ff_delete_test (const TCHAR* path);
FRESULT ff_list_test (TCHAR* path);

FRESULT ff_basic_test (const TCHAR* mount_point, const TCHAR* file_name);
int ff_cycle_test (const TCHAR *mount_point, UINT ncyc);
int ff_diskio_speed_bench(const TCHAR *mount_point, DWORD start_sector, BYTE *test_buf, QWORD test_buf_len, QWORD test_tot_len);
int ff_fileio_speed_test(const TCHAR *mount_point, BYTE *test_buf, QWORD test_buf_len, QWORD test_tot_len);
FRESULT ff_big_file_test(const TCHAR* path, UINT sz_mb);

#ifdef __cplusplus
}
#endif

#endif