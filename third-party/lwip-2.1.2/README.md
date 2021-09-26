<!--
 * @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 *  
 * SPDX-License-Identifier: Apache-2.0.
 * 
 * @Date: 2021-08-17 08:20:38
 * @LastEditTime: 2021-08-17 08:31:48
 * @Description:  This files is for 
 * 
 * @Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
-->
# 移植方法

## 获取源码

[contrib-2.1.0](http://download.savannah.nongnu.org/releases/lwip/contrib-2.1.0.zip)
[lwip-2.1.2](http://download.savannah.nongnu.org/releases/lwip/lwip-2.1.2.zip)

## 放置源码

- 源码路径
```
src --> LwIP 
```

- 源码分组

- api
- core
- netif
- arch

## 移植头文件

- lwipopts.h，替代默认的opt.h
- cc.h
- pref.h