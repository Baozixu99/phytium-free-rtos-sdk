###
 # @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 #  
 # SPDX-License-Identifier: Apache-2.0.
 # 
 # @Date: 2021-07-06 08:46:10
 # @LastEditTime: 2021-07-06 09:20:35
 # @Description:  This files is for 
 # 
 # @Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 
#!/bin/sh
if [ "$1" = "-dev-remote" ]; then
# 将remote仓库修改为开发仓库
    DEV_REMOTE=https://gitee.com/zhugengyu/phytium-free-rtos-sdk.git
    DEV_BRANCH=master
    git remote rm origin
    git remote add origin $DEV_REMOTE
    git checkout -b $DEV_BRANCH
    echo "Standalone SDK Git Repository is set as"
    git remote -v
fi

if [ "$1" = "-pub-remote" ]; then
    DEV_REMOTE=https://gitee.com/phytium_embedded/phytium-free-rtos-sdk.git
    DEV_BRANCH=master
    git remote rm origin
    git remote add origin $DEV_REMOTE
    git checkout -b $DEV_BRANCH
    echo "Standalone SDK Git Repository is set as"
    git remote -v   
fi