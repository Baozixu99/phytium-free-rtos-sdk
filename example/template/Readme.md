<!--
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-06-23 18:47:28
 * @LastEditTime: 2021-07-21 11:21:41
 * @Description:  This files is for 
 * 
 * @Modify History: 
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

