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
 * FilePath: README.md
 * Date: 2022-02-24 13:42:19
 * LastEditTime: 2022-03-21 17:04:28
 * Description:  This file is for 
 * 
 * Modify History: 
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