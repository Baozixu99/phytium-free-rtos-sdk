/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.   All Rights Reserved.
 * 
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 *      https://opensource.org/licenses/BSD-3-Clause
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * 
 * FilePath: fdevice.c
 * Created Date: 2025-05-23 11:12:07
 * Last Modified: 2025-05-23 16:53:55
 * Description:  This file is for
 * 
 * Modify History:
 *  Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0    zhangyan    2025/5/23 first release
 */

#include "fdevice.h"
#include "fassert.h"
#include "fdebug.h"

#define FT_DEVICE_DEBUG_TAG "FT_DEVICE"
#define FDEVICE_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FT_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDEVICE_INFO(format, ...) \
    FT_DEBUG_PRINT_I(FT_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDEVICE_WARN(format, ...) \
    FT_DEBUG_PRINT_W(FT_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)
#define FDEVICE_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FT_DEVICE_DEBUG_TAG, format, ##__VA_ARGS__)

#define FDEVICE_INIT    (dev->ops ? dev->ops->init : NULL)
#define FDEVICE_OPEM    (dev->ops ? dev->ops->open : NULL)
#define FDEVICE_CLOSE   (dev->ops ? dev->ops->close : NULL)
#define FDEVICE_READ    (dev->ops ? dev->ops->read : NULL)
#define FDEVICE_WRITE   (dev->ops ? dev->ops->write : NULL)
#define FDEVICE_CONTROL (dev->ops ? dev->ops->control : NULL)

static FDevice *device_table[MAX_DEVICES];

/**
 * @name: FRegisterDevice
 * @msg: registers a device in the device table
 * @param {FDevice} *dev, dev Pointer to the FDevice instance to register
 * @return err code information, FDEVICE_SUCCESS indicates success，others indicates failed
 */
FError FRegisterDevice(FDevice *dev)
{
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (device_table[i] == NULL)
        {
            device_table[i] = dev;
            return FDEVICE_SUCCESS;
        }
    }

    FDEVICE_ERROR("To register device failed.");
    return FDEVICE_UNKNOWN_DEVICE;
}

/**
 * @name: FGetDeviceByName
 * @msg: get a registered device by its name
 * @param {const char*} name, pointer to the name string of the device
 * @return pointer to FDevice if found, NULL if not found
 */
FDevice *FGetDeviceByName(const char *name)
{
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (device_table[i] && strcmp(device_table[i]->name, name) == 0)
        {
            return device_table[i];
        }
    }
    return NULL;
}

/**
 * @name: FDeviceInit
 * @msg: initialize the specified device by calling the FDEVICE_INIT handler
 * @param {FDevice*} dev, pointer to the device instance to initialize
 * @return error code information, FDEVICE_SUCCESS indicates success, others indicate failure
 */
FError FDeviceInit(FDevice *dev)
{
    FASSERT(dev);
    FError ret = FDEVICE_SUCCESS;

    if (FDEVICE_INIT != NULL)
    {
        ret = FDEVICE_INIT(dev);
        if (ret != FDEVICE_SUCCESS)
        {
            FDEVICE_ERROR("To initialize device:%.*s failed. The error code is %d", dev->name, ret);
        }

        dev->init_complete = FDEVICE_FLAG_INIT_COMPLETE;
    }

    return ret;
}

/**
 * @name: FDeviceOpen
 * @msg: open the specified device, initialize it if not yet initialized
 * @param {FDevice*} dev, pointer to the device instance to open
 * @param {u32} oflag, open mode flags for the device
 * @return error code information, FDEVICE_SUCCESS indicates success, others indicate failure
 */
FError FDeviceOpen(FDevice *dev, u32 oflag)
{
    FASSERT(dev);
    FError ret = FDEVICE_SUCCESS;

    if (!(dev->init_complete & FDEVICE_FLAG_INIT_COMPLETE))
    {
        if (FDEVICE_INIT != NULL)
        {
            ret = FDEVICE_INIT(dev);
            if (ret != FDEVICE_SUCCESS)
            {
                FDEVICE_ERROR("To initialize device:%.*s failed. The error code is %d",
                              dev->name, ret);
                return ret;
            }
            dev->init_complete = FDEVICE_FLAG_INIT_COMPLETE;
        }
    }
    /* Check if the device is not already opened, or the open flags differ from the requested flags */
    if (!(dev->open_flag & FDEVICE_OFLAG_OPEN) ||
        ((dev->open_flag & FDEVICE_OFLAG_MASK) != ((oflag & FDEVICE_OFLAG_MASK) | FDEVICE_OFLAG_OPEN)))
    {
        if (FDEVICE_OPEM != NULL)
        {
            ret = FDEVICE_OPEM(dev, oflag);
            if (ret != FDEVICE_SUCCESS)
            {
                FDEVICE_ERROR("To open device:%.*s failed. The error code is %d", dev->name, ret);
            }
            else
            {
                dev->open_flag |= FDEVICE_OFLAG_OPEN;
            }
        }
        else
        {
            /* set open flag */
            dev->open_flag = (oflag & FDEVICE_OFLAG_MASK);
        }
    }

    return ret;
}

/**
 * @name: FDeviceClose
 * @msg: close the specified device by calling the FDEVICE_CLOSE handler
 * @param {FDevice*} dev, pointer to the device instance to close
 * @return error code information, FDEVICE_SUCCESS indicates success, others indicate failure
 */
FError FDeviceClose(FDevice *dev)
{
    FASSERT(dev);
    FError ret = FDEVICE_SUCCESS;

    if (FDEVICE_CLOSE != NULL)
    {
        ret = FDEVICE_CLOSE(dev);
        if (ret != FDEVICE_SUCCESS)
        {
            FDEVICE_ERROR("To close device:%.*s failed. The error code is %d", dev->name, ret);
        }
        else
        {
            dev->open_flag = FDEVICE_OFLAG_CLOSE;
        }
    }

    return ret;
}

/**
 * @name: FDeviceRead
 * @msg: read data from the specified device
 * @param {FDevice*} dev, pointer to the device instance
 * @param {void*} buffer, pointer to the buffer to store read data
 * @param {fsize_t} size, number of bytes to read
 * @return number of bytes actually read
 */
fsize_t FDeviceRead(FDevice *dev, void *buffer, fsize_t size)
{
    FASSERT(dev);
    fsize_t read_len = 0;

    if (FDEVICE_READ != NULL)
    {
        read_len = FDEVICE_READ(dev, buffer, size);
    }

    return read_len;
}

/**
 * @name: FDeviceWrite
 * @msg: write data to the specified device
 * @param {FDevice*} dev, pointer to the device instance
 * @param {void*} buffer, pointer to the data buffer to write
 * @param {fsize_t} size, number of bytes to write
 * @return number of bytes actually written
 */
fsize_t FDeviceWrite(FDevice *dev, void *buffer, fsize_t size)
{
    FASSERT(dev);
    fsize_t write_len = 0;

    if (FDEVICE_WRITE != NULL)
    {
        write_len = FDEVICE_WRITE(dev, buffer, size);
    }

    return write_len;
}

/**
 * @name: FDeviceControl
 * @msg: send control command to the specified device
 * @param {FDevice*} dev, pointer to the device instance
 * @param {int} cmd, control command code
 * @param {void*} arg, argument for the control command
 * @return error code information, FDEVICE_SUCCESS indicates success, others indicate failure
 */
FError FDeviceControl(FDevice *dev, int cmd, void *arg)
{
    FASSERT(dev);
    FError ret = FDEVICE_SUCCESS;

    if (FDEVICE_CONTROL != NULL)
    {
        ret = FDEVICE_CONTROL(dev, cmd, arg);
        if (ret != FDEVICE_SUCCESS)
        {
            FDEVICE_ERROR("To control device:%.*s failed. The error code is %d", dev->name, ret);
        }
    }

    return ret;
}
