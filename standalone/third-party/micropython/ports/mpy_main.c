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
 * FilePath: mpy_main.c
 * Created Date: 2023-12-01 15:22:57
 * Last Modified: 2025-05-20 14:19:53
 * Description:  This file is for the main function of micropython
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "shared/runtime/pyexec.h"
#include "mpy_uart.h"
#include "py/stackctrl.h"

#if MICROPY_VFS
#include "extmod/vfs_fat.h"
#include "lib/oofatfs/ff.h"
#include "extmod/vfs.h"
#include "mpy_sd.h"
static void mp_init__filesystem(void);
static const char fresh_boot_py[] = "# hello.py -- run hello.py\r\n";
#endif

static char *stack_top;
#if MICROPY_ENABLE_GC
static char heap[MICROPY_HEAP_SIZE];
#endif

void main_py(void)
{
    mp_stack_ctrl_init();
    gc_init(heap, heap + sizeof(heap));
    mp_init();
    MpySerialConfig();
#if MICROPY_VFS
    {
        mp_init__filesystem();
    }
#endif
    pyexec_friendly_repl();
    gc_sweep_all();
    mp_deinit();
    return;
}

#if MICROPY_ENABLE_GC
void gc_collect(void)
{
    // WARNING: This gc_collect implementation doesn't try to get root
    // pointers from CPU registers, and thus may function incorrectly.
    void *dummy;
    gc_collect_start();
    gc_collect_root(&dummy, ((mp_uint_t)stack_top - (mp_uint_t)&dummy) / sizeof(mp_uint_t));
    gc_collect_end();
    gc_dump_info(&mp_plat_print);
}
#endif

void nlr_jump_fail(void *val)
{
    while (1)
    {
        ;
    }
}

void NORETURN __fatal_error(const char *msg)
{
    while (1)
    {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr)
{
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

#if MICROPY_VFS
static void mp_init__filesystem(void)
{
    fs_user_mount_t *vfs_fat = m_new_obj_maybe(fs_user_mount_t);
    mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
    if (vfs == NULL || vfs_fat == NULL)
    {
        mp_printf(&mp_plat_print, "MPY: can't create vfs filesystem\n");
        return;
    }
    vfs_fat->blockdev.flags = MP_BLOCKDEV_FLAG_FREE_OBJ;
    pyb_sd_init_vfs(vfs_fat);
    FRESULT res = f_mount(&vfs_fat->fatfs);
    if (res != FR_OK)
    {
        uint8_t working_buf[FF_MAX_SS];
        res = f_mkfs(&vfs_fat->fatfs, FM_ANY, 0, working_buf, sizeof(working_buf));
        if (res != FR_OK)
        {
            mp_printf(&mp_plat_print, "MPY: can't create sd filesystem\n");
            return;
        }
    }
    vfs->str = "/sd";
    vfs->len = 3; //the length of str
    vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
    vfs->next = NULL;
    MP_STATE_VM(vfs_mount_table) = vfs;
    MP_STATE_PORT(vfs_cur) = vfs;

    // create /sd/sys, /sd/lib and /sd/cert if they don't exist
    if (FR_OK != f_chdir(&vfs_fat->fatfs, "/sys"))
    {
        f_mkdir(&vfs_fat->fatfs, "/sys");
    }
    if (FR_OK != f_chdir(&vfs_fat->fatfs, "/lib"))
    {
        f_mkdir(&vfs_fat->fatfs, "/lib");
    }
    if (FR_OK != f_chdir(&vfs_fat->fatfs, "/cert"))
    {
        f_mkdir(&vfs_fat->fatfs, "/cert");
    }
    f_chdir(&vfs_fat->fatfs, "/");

    FILINFO fno;
    // make sure we have a /sd/hello.py.  Create it if needed.
    res = f_stat(&vfs_fat->fatfs, "/hello.py", &fno);
    if (res == FR_OK)
    {
        mp_printf(&mp_plat_print, "MPY:  find hello.py\n");
    }
    else
    {
        // doesn't exist, create sd file
        FIL fp;
        f_open(&vfs_fat->fatfs, &fp, "/hello.py", FA_WRITE | FA_CREATE_ALWAYS);
        UINT n;
        f_write(&fp, fresh_boot_py,
                sizeof(fresh_boot_py) - 1 /* don't count null terminator */, &n);
        // TODO check we could write n bytes
        f_close(&fp);
        mp_printf(&mp_plat_print, "MPY: creat hello.py\n");
    }
}
#endif