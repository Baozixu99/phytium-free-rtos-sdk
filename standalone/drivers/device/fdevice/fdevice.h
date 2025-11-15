/*
 * 
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
 * FilePath: fdevice.h
 * Created Date: 2025-05-23 11:12:20
 * Last Modified: 2025-05-23 16:49:58
 * Description:  This file is for
 * 
 * Modify History:
 *   Ver      Who        Date               Changes
 * -----  ----------  --------  ---------------------------------
 * 1.0    zhangyan    2025/5/23 first release
 */

#ifndef FDEVICE_H
#define FDEVICE_H

#include "ftypes.h"
#include "ferror_code.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define FDEVICE_SUCCESS            FT_SUCCESS
#define FDEVICE_UNKNOWN_DEVICE     FT_MAKE_ERRCODE(ErrModBsp, ErrBspDevice, 1)

#define MAX_DEVICES                16

#define FDEVICE_FLAG_INIT_COMPLETE FT_COMPONENT_IS_READY /* device is closed */

#define FDEVICE_FLAG_RDONLY        0x001 /* read only */
#define FDEVICE_FLAG_WRONLY        0x002 /* write only */
#define FDEVICE_FLAG_RDWR          0x003 /* read and write */

#define FDEVICE_OFLAG_CLOSE        0x000 /* device is closed */
#define FDEVICE_OFLAG_RDONLY       0x001 /* read only access */
#define FDEVICE_OFLAG_WRONLY       0x002 /* write only access */
#define FDEVICE_OFLAG_RDWR         0x003 /* read and write */
#define FDEVICE_OFLAG_OPEN         0x008 /* device is opened */
#define FDEVICE_OFLAG_MASK         0xf0f /* mask of open flag */

typedef struct FBusOps FBusOps;

typedef struct
{
    const char *name;   /* Device name */
    const FBusOps *ops; /* Pointer to device operation functions */
    void *private_data; /* Pointer to device-specific private data */
    u32 init_complete;  /* Flag indicating whether initialization is complete */
    u32 open_flag;      /* Flag indicating device open state and mode */
} FDevice;

struct FBusOps
{
    FError (*init)(FDevice *dev);
    FError (*open)(FDevice *dev, u32 oflag);
    FError (*close)(FDevice *dev);
    fsize_t (*read)(FDevice *dev, void *buffer, fsize_t size);
    fsize_t (*write)(FDevice *dev, const void *buffer, fsize_t size);
    FError (*control)(FDevice *dev, int cmd, void *args);
};

/*registers a device in the device table*/
FError FRegisterDevice(FDevice *dev);
/*get a registered device by its name*/
FDevice *FGetDeviceByName(const char *name);
/*initialize the specified device by calling the device_init handler*/
FError FDeviceInit(FDevice *dev);
/*open the specified device, initialize it if not yet initialized*/
FError FDeviceOpen(FDevice *dev, u32 oflag);
/*close the specified device by calling the device_close handler*/
FError FDeviceClose(FDevice *dev);
/*read data from the specified device*/
fsize_t FDeviceRead(FDevice *dev, void *buffer, fsize_t size);
/*write data to the specified device*/
fsize_t FDeviceWrite(FDevice *dev, void *buffer, fsize_t size);
/*send control command to the specified device*/
FError FDeviceControl(FDevice *dev, int cmd, void *arg);

#ifdef __cplusplus
}
#endif

#endif // !