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
 * Last Modified: 2025-05-20 14:24:53
 * Description:  This file is for the sd function of micropython
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 *  1.0  Wangzq     2023/12/07   Modify the format and establish the version
 */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "py/mpconfig.h"
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "lib/oofatfs/ff.h"
#include "lib/oofatfs/diskio.h"
#include "extmod/vfs_fat.h"

#include "mpy_sd.h"
#include "fparameters.h"
#include "fsl_sdmmc.h"
#if defined(SOC_TARGET_PE220X)
#include "fsdif_timing.h"
#endif

static sdmmchost_config_t s_inst_config;
static sdmmc_sd_t s_inst;
/******************************************************************************
 DEFINE PRIVATE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DECLARE PUBLIC DATA
 ******************************************************************************/
#define SD_SECTOR_SIZE 512
#define SD_MAX_RW_BLK  1024U
#define FSDIF1_ID      1
#define FSDIF0_ID      0
/******************************************************************************
 DECLARE PRIVATE DATA
 ******************************************************************************/
/******************************************************************************
 DECLARE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC status_t pyb_ft_sd_hw_init(pybsd_obj_t *self);
STATIC mp_obj_t pyb_sd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw,
                                const mp_obj_t *args);
STATIC mp_obj_t pyb_sd_deinit(mp_obj_t self_in);
STATIC status_t pyb_sd_read_contact(mp_obj_t buf, uint32_t block_num, uint32_t num_blocks);
STATIC mp_obj_t pyb_sd_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf);

STATIC const mp_obj_base_t pyb_sd_obj = {&pyb_sd_type};

/// Initializes the sd card hardware driver
STATIC status_t pyb_ft_sd_hw_init(pybsd_obj_t *self)
{
    status_t err = 0;
    SDMMC_OSAInit();
    memset(&s_inst_config, 0, sizeof(s_inst_config));
    memset(&s_inst, 0, sizeof(s_inst));
#if defined(SOC_TARGET_PE220X)
    FSdifTimingInit();
    s_inst_config.hostId = FSDIF1_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDIF;
#elif defined(SOC_TARGET_PD1904) || defined(SOC_TARGET_PD2008)
    s_inst_config.hostId = FSDMMC0_ID;
    s_inst_config.hostType = kSDMMCHOST_TYPE_FSDMMC;
#endif

    s_inst_config.cardType = kSDMMCHOST_CARD_TYPE_MICRO_SD;
    s_inst_config.enableDMA = TRUE;
    s_inst_config.enableIrq = TRUE;

    s_inst_config.endianMode = kSDMMCHOST_EndianModeLittle;
    s_inst_config.maxTransSize = SD_MAX_RW_BLK * SD_SECTOR_SIZE;
    s_inst_config.defBlockSize = SD_SECTOR_SIZE;
    s_inst_config.cardClock = SD_CLOCK_50MHZ;
    s_inst_config.isUHSCard = FALSE;

    err = SD_CfgInitialize(&s_inst, &s_inst_config);
    if (kStatus_Success != err)
    {
        mp_printf(&mp_plat_print, "Init SD failed, err = %d !!!", err);
        return err;
    }

    self->enabled = true;
    return err;
}

/******************************************************************************
 DEFINE PRIVATE FUNCTIONS
 ******************************************************************************/
STATIC mp_obj_t pyb_sd_init_helper(pybsd_obj_t *self, const mp_map_t *args)
{
    status_t err = 0;
    err = pyb_ft_sd_hw_init(self);
    if (err != 0)
    {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}

/******************************************************************************/
// MicroPython bindings
STATIC mp_obj_t pyb_sd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw,
                                const mp_obj_t *all_args)
{
    if (n_args > 0)
    {
        if (all_args[0] != MP_OBJ_NEW_SMALL_INT(0))
        {
            mp_raise_OSError(MP_ENODEV);
        }
        --n_args;
        ++all_args;
    }
    // parse args
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);

    mp_obj_t self = MP_OBJ_NULL;

    return self;
}

STATIC mp_obj_t pyb_sd_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{

    pybsd_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    return pyb_sd_init_helper(self, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_sd_init_obj, 1, pyb_sd_init);

STATIC mp_obj_t pyb_sd_deinit(mp_obj_t self_in)
{
    pybsd_obj_t *self = self_in;
    // disable the peripheral
    self->enabled = false;
    SD_Deinit(&s_inst.card);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_sd_deinit_obj, pyb_sd_deinit);

STATIC status_t pyb_sd_read_contact(mp_obj_t buf, uint32_t block_num, uint32_t num_blocks)
{
    status_t err = 0;

    err = SD_ReadBlocks(&s_inst.card, buf, block_num, num_blocks);

    return err;
}

STATIC mp_obj_t pyb_sd_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf)
{
    status_t err = 0;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    err = pyb_sd_read_contact(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SD_SECTOR_SIZE);
    return MP_OBJ_NEW_SMALL_INT(err != RES_OK); // return of 0 means success
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sd_readblocks_obj, pyb_sd_readblocks);

STATIC status_t pyb_sd_write_contact(mp_obj_t buf, uint32_t block_num, uint32_t num_blocks)
{
    status_t err = 0;
    err = SD_WriteBlocks(&s_inst.card, buf, block_num, num_blocks);
    return err;
}

STATIC mp_obj_t pyb_sd_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf)
{
    status_t err = 0;
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    err = pyb_sd_write_contact(bufinfo.buf, mp_obj_get_int(block_num), bufinfo.len / SD_SECTOR_SIZE);
    return MP_OBJ_NEW_SMALL_INT(err != RES_OK); // return of 0 means success
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sd_writeblocks_obj, pyb_sd_writeblocks);

STATIC mp_obj_t pyb_sd_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in)
{
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd)
    {
        case MP_BLOCKDEV_IOCTL_INIT:
            pyb_ft_sd_hw_init(self);
            return MP_OBJ_NEW_SMALL_INT(0);
        case MP_BLOCKDEV_IOCTL_DEINIT:
        case MP_BLOCKDEV_IOCTL_SYNC:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT:
            return MP_OBJ_NEW_SMALL_INT(s_inst.card.blockCount);

        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE:
            return MP_OBJ_NEW_SMALL_INT(s_inst.card.blockSize);

        default:                             // unknown command
            return MP_OBJ_NEW_SMALL_INT(-1); // error
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(pyb_sd_ioctl_obj, pyb_sd_ioctl);

STATIC const mp_rom_map_elem_t pyb_sd_locals_dict_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sd)},
    {MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&pyb_sd_init_obj)},
    {MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&pyb_sd_deinit_obj)},
    // block device protocol
    {MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&pyb_sd_readblocks_obj)},
    {MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&pyb_sd_writeblocks_obj)},
    {MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&pyb_sd_ioctl_obj)},
};

STATIC MP_DEFINE_CONST_DICT(pyb_sd_locals_dict, pyb_sd_locals_dict_table);

const mp_obj_module_t mp_module_sd = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&pyb_sd_locals_dict,
};

MP_REGISTER_MODULE(MP_QSTR_sd, mp_module_sd);

MP_DEFINE_CONST_OBJ_TYPE(pyb_sd_type, MP_QSTR_SD, MP_TYPE_FLAG_NONE, make_new,
                         pyb_sd_make_new, locals_dict, &pyb_sd_locals_dict);

void pyb_sd_init_vfs(fs_user_mount_t *vfs)
{
    vfs->base.type = &mp_fat_vfs_type;
    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_NATIVE | MP_BLOCKDEV_FLAG_HAVE_IOCTL;
    vfs->fatfs.drv = vfs;
    vfs->blockdev.readblocks[0] = (mp_obj_t)&pyb_sd_readblocks_obj;
    vfs->blockdev.readblocks[1] = (mp_obj_t)&pyb_sd_obj;
    vfs->blockdev.readblocks[2] = (mp_obj_t)pyb_sd_read_contact; // native version
    vfs->blockdev.writeblocks[0] = (mp_obj_t)&pyb_sd_writeblocks_obj;
    vfs->blockdev.writeblocks[1] = (mp_obj_t)&pyb_sd_obj;
    vfs->blockdev.writeblocks[2] = (mp_obj_t)pyb_sd_write_contact; // native version
    vfs->blockdev.u.ioctl[0] = (mp_obj_t)&pyb_sd_ioctl_obj;
    vfs->blockdev.u.ioctl[1] = (mp_obj_t)&pyb_sd_obj;
}
