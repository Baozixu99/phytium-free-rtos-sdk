<!--
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-06-29 16:48:40
 * @LastEditTime: 2021-07-02 11:24:53
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->

# host侧设置
```
sudo service tftpd-hpa restart
```

# 开发板侧设置
```
icache off
dcache off
```

```
setenv ipaddr 192.168.4.20  
setenv serverip 192.168.4.50 
setenv gatewayip 192.168.4.1 
tftpboot 80100000 ft2004_freertos_buffer.bin
go 80100000
```