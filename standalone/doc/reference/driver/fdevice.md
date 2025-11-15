# FDEVICE 驱动程序

## 1. 概述

设备管理基础模块,  本模块提供统一的设备操作接口，包括设备的注册、初始化、打开/关闭、读写操作以及控制命令执行等功能。所有设备通过函数指针实现统一管理，支持不同类型设备以相同方式进行操作。

## 2. 功能

每个设备可定义自身的 init、open、close、read、write 和 control处理函数，这些函数在注册设备时进行设置。本模块旨在提供模块化、抽象化和可扩展的设备管理框架，适用于嵌入式系统中多种硬件或虚拟设备的统一操作。

驱动相关的源文件包括，
```
.
├── fdevice.c
└── fdevice.h
```

## 3. API参考

### 3.1. 用户数据结构

- fdevice控制数据

```c
typedef struct {
    const char *name;           /* Device name */
    const FBusOps *ops;         /* Pointer to device operation functions */
    void *private_data;         /* Pointer to device-specific private data */
    u32 init_complete;          /* Flag indicating whether initialization is complete */
    u32 open_flag;              /* Flag indicating device open state and mode */
} FDevice;
```

- FBusOps定义了一个设备操作接口的函数指针集合，每个成员对应设备的一个基本操作方法。具体设备驱动可通过实现这些函数，实现与上层的统一接口对接

```c
struct FBusOps {
    FError (*init)(FDevice *dev);
    FError (*open)(FDevice *dev, u32 oflag);
    FError (*close)(FDevice *dev);
    fsize_t (*read)(FDevice *dev, void *buffer, fsize_t size);
    fsize_t (*write)(FDevice *dev, const void *buffer, fsize_t size);
    FError (*control)(FDevice *dev, int cmd, void *args);
};
```

### 3.2  错误码定义

- FDEVICE_SUCCESS 执行成功               
- FDEVICE_ERR_INVALID_PARAMETER  设备参数错误

### 3.3. 用户API接口

#### FRegisterDevice

- 注册一个设备

```c
FError FRegisterDevice(FDevice *dev);
```

Note:

- 注册一个设备在device_table中

Input:

- {FDevice *} 指向要注册的 FDevice 实例的指针

Return:

- {FError} 设备注册的错误码信息，FDEVICE_SUCCESS 表示设备注册成功，其它返回值表示设备注册失败

#### FGetDeviceByName

- 根据设备名称，获取一个已经注册过的设备

```c
FDevice *FGetDeviceByName(const char *name);
```

Input:

- {const char *} 设备名称

Return:

- {FDevice} 设备获取成功后返回指向该设备的指针，否则然会NULL

#### FDeviceInit

- 初始化一个设备

```c
FError FDeviceInit(FDevice *dev);
```

Input:

- {FDevice *} 指向已经注册的 FDevice 实例的指针

Return:

- {FError} 设备初始化的错误码信息，FDEVICE_SUCCESS 表示设备初始化成功，其它返回值表示设备初始化失败

#### FDeviceOpen

- 打开一个设备

```c
FError FDeviceOpen(FDevice *dev, u32 oflag);
```

Input:

- {FDevice *} 指向已经注册的 FDevice 实例的指针
- {u32 } oflag, 设备的打开模式标志

Return:

- {FError} 设备的错误码信息，FDEVICE_SUCCESS 表示设备打开成功，其它返回值表示设备打开失败

#### FDeviceClose

- 关闭一个设备

```c
FError FDeviceClose(FDevice *dev);
```

Input:

- {FDevice *} 指向已经注册的 FDevice 实例的指针

Return:

- {FError} 设备的错误码信息，FDEVICE_SUCCESS 表示设备关闭成功，其它返回值表示设备关闭失败

#### FDeviceRead

- 通过设备进行读操作

```c
fsize_t FDeviceRead(FDevice *dev, void *buffer, fsize_t size);
```

Input:

- {FDevice *} 指向已经注册的 FDevice 实例的指针
- {void * } 用于存放读取数据的缓冲区指针
- {fsize_t } 请求读取的数据字节数

Return:

- {fsize_t} 实际读取的字节数，若读取失败，返回值为 0

#### FDeviceWrite

- 通过设备进行写操作

```c
fsize_t FDeviceWrite(FDevice *dev, void *buffer, fsize_t size);
```

Input:

- {FDevice *} 指向已经注册的 FDevice 实例的指针
- {void * } 用于存放写入数据的缓冲区指针
- {fsize_t } 请求写入的数据字节数

Return:

- {fsize_t} 实际写入的字节数，若写入失败，返回值为 0

#### FDeviceControl

- 打开一个设备

```c
FError FDeviceControl(FDevice *dev, int cmd, void *arg);
```

Input:

- {FDevice *} 指向已经注册的 FDevice 实例的指针
- {int} cmd：控制命令编号（由具体设备定义）
- {void *} arg：命令参数指针（可为输入或输出参数，取决于命令类型）

Return:

- {FError} 设备的错误码信息，FDEVICE_SUCCESS 表示控制命令执行成功，其它返回值表示控制命令执行失败