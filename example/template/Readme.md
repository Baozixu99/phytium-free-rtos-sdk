<!--
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: Readme.md
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:02:58
 * Description:  This file is for 
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->


# host侧设置
- 重启host侧tftp服务器
```
sudo service tftpd-hpa restart
```

# 开发板侧设置

## 使用go命令跳转
- 关闭D-cache和I-cache
```
icache off
dcache off
```

- 下载新版本到FT2000-4
```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 0x80100000 freertos.bin
dcache flush
go 0x80100000
```


## 使用bootelf命令跳转
- 下载新版本到FT2000-4
```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 0x90100000 freertos.elf
bootelf -p 0x90100000
```

