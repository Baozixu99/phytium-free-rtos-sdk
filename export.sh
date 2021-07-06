###
 # @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 #  
 # SPDX-License-Identifier: Apache-2.0.
 # 
 # @Date: 2021-06-30 15:45:19
 # @LastEditTime: 2021-07-05 16:21:25
 # @Description:  This files is for 
 # 
 # @Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 
#!/bin/sh
STANDALONE_SDK_V=v0.0.2
FREERTOS_SDK_V=v0.0.1

export FREERTOS_SDK_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

STANDALONE_PATH=$FREERTOS_SDK_ROOT/standalone-sdk

if [ "$1" = "-online" ]; then
    OFFLINE_INSTALL=0 
else
    OFFLINE_INSTALL=1
fi

if [ ! -d $STANDALONE_PATH ]; then
# 只拉取tag对应的版本，不拉取历史版本
    git clone -b $STANDALONE_SDK_V --depth=1 https://gitee.com/phytium_embedded/phytium-standalone-sdk.git $STANDALONE_PATH
fi
    
# copy cc pack to standalone sdk tools path if it's offline install
if [ $OFFLINE_INSTALL = 1 ]; then
    cp $FREERTOS_SDK_ROOT/tools/gcc-* $STANDALONE_PATH/tools
    $STANDALONE_PATH/export.sh
else
    $STANDALONE_PATH/export.sh $1
fi

# save freertos sdk root path
PROFILE_PATH=~/.profile

sed -i '/export FREERTOS_SDK_ROOT=/d' $PROFILE_PATH
echo "export FREERTOS_SDK_ROOT=$FREERTOS_SDK_ROOT" >> $PROFILE_PATH

source $PROFILE_PATH

echo "FreeRTOS Path is set as "$FREERTOS_SDK_ROOT
