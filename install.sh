###
 # @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 #  
 # SPDX-License-Identifier: Apache-2.0.
 # 
 # @Date: 2021-06-30 15:45:19
 # @LastEditTime: 2021-07-22 16:03:21
 # @Description:  This files is for 
 # 
 # @Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 
#!/bin/sh
if [ "$1" = "-online" ]; then
    OFFLINE_INSTALL=0 
else
    OFFLINE_INSTALL=1
fi

# profile to save environment variables
PROFILE_PATH=~/.profile
chmod +x $PROFILE_PATH

# get absoulte path of script
FREERTOS_SDK_ABS=$(readlink -f "$0")
export FREERTOS_SDK_ROOT=$(dirname $FREERTOS_SDK_ABS)
echo "[1]: FreeRTOS SDK at "$FREERTOS_SDK_ROOT

#export FREERTOS_SDK_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
INSTALL_PATH=$FREERTOS_SDK_ROOT/tools

# remove environment variables
sed -i '/export FREERTOS_AARCH32_CROSS_PATH=/d' $PROFILE_PATH
sed -i '/export PATH=\$FREERTOS_AARCH32_CROSS_PATH/d' $PROFILE_PATH
sed -i '/export FREERTOS_AARCH64_CROSS_PATH=/d' $PROFILE_PATH
sed -i '/export PATH=\$FREERTOS_AARCH64_CROSS_PATH/d' $PROFILE_PATH
sed -i '/export FREERTOS_STANDALONE=/d' $PROFILE_PATH
sed -i '/export FREERTOS_SDK_ROOT=/d' $PROFILE_PATH
echo "[2]: Reset Environment"

#####################################Install BSP####################################################
STANDALONE_SDK_V=v0.0.5
STANDALONE_PATH=$FREERTOS_SDK_ROOT/standalone
STANDALONE_BRANCHE=master
STANDALONE_REMOTE=https://gitee.com/phytium_embedded/phytium-standalone-sdk.git

# no fetch history commits
if [ ! -d $STANDALONE_PATH ]; then
    echo "[3]: Fetch BSP"
    git clone $STANDALONE_REMOTE $STANDALONE_PATH
else
    echo "[3]: BSP Already Exists"
fi

if [ $? -ne 0 ] && [ -d $STANDALONE_PATH ]; then
    echo "[3]: Failed when Fetch Bsp !!!"
    exit
fi

# fetch part of dirs
cd $STANDALONE_PATH
# enable checkout part of standalone
git config core.sparsecheckout true
# supress part checkout warnning
git config advice.detachedHead false
echo "bsp/" >> .git/info/sparse-checkout
echo "lib/" >> .git/info/sparse-checkout
echo "[3]: BSP Version is" $STANDALONE_SDK_V
git checkout $STANDALONE_SDK_V

if [ $? -ne 0 ]; then
    echo "[3]: Failed when Checkout Bsp !!!"
    exit
fi

# assign execute premission
# make sure bsp scripts are executable
chmod +x $FREERTOS_SDK_ROOT/*.sh --silent
chmod +x $FREERTOS_SDK_ROOT/scripts/*.sh --silent
chmod +x $FREERTOS_SDK_ROOT/make/*.mk --silent
chmod +x $STANDALONE_PATH/lib/Kconfiglib/*.py --silent
#find $STANDALONE_PATH/lib/Kconfiglib/*.py -type f -exec dos2unix {} \;
if [ $? -ne 0 ]; then
    echo "[3]: Failed when assign file premission !!!"
    exit
fi

#####################################Install GCC Tools##############################################

# name of cc
AARCH32_CC=gcc-arm-x86_64-none-eabi-10-2020-q4-major
AARCH64_CC=gcc-arm-x86_64-aarch64-none-elf-10.2-2020.11

# url of cc package
AARCH32_URL=https://gitee.com/phytium_embedded/phytium-standalone-sdk/attach_files/776847/download/gcc-arm-x86_64-none-eabi-10-2020-q4-major.tar.xz
AARCH64_URL=https://gitee.com/phytium_embedded/phytium-standalone-sdk/attach_files/776846/download/gcc-arm-x86_64-aarch64-none-elf-10.2-2020.11.tar.xz

# cc package name
AARCH32_CC_PACK=$AARCH32_CC.tar.xz
AARCH64_CC_PACK=$AARCH64_CC.tar.xz

# cc package name during download
AARCH32_CC_DL=$AARCH32_CC.tmp.tar.xz
AARCH64_CC_DL=$AARCH64_CC.tmp.tar.xz

# cc install path
export FREERTOS_AARCH32_CROSS_PATH=$INSTALL_PATH/$AARCH32_CC
export FREERTOS_AARCH64_CROSS_PATH=$INSTALL_PATH/$AARCH64_CC

echo "[4]: Install CC...."

# remove tmp load package 
[ -f $INSTALL_PATH/$AARCH32_CC_DL ] && rm $INSTALL_PATH/$AARCH32_CC_DL
[ -f $INSTALL_PATH/$AARCH64_CC_DL ] && rm $INSTALL_PATH/$AARCH64_CC_DL

# do not download if package exists
# do not download if compiler already installed
if [ $OFFLINE_INSTALL -ne 1 ]; then
    if [ ! -d $FREERTOS_AARCH32_CROSS_PATH ] && [ ! -f $INSTALL_PATH/$AARCH32_CC_PACK ]; then
        wget $AARCH32_URL -O $INSTALL_PATH/$AARCH32_CC_DL -c -t 0
# rename downlaod cc package in case download process is intrrupted
        if [ -f $INSTALL_PATH/$AARCH32_CC_DL ]; then
            mv $INSTALL_PATH/$AARCH32_CC_DL $INSTALL_PATH/$AARCH32_CC_PACK
            echo "[4]: Got CC "$INSTALL_PATH/$AARCH32_CC_PACK
        fi
    else
        echo "[4]: AARCH32 CC Already Exists"
    fi
    
    if [ ! -d $FREERTOS_AARCH64_CROSS_PATH ] && [ ! -f $INSTALL_PATH/$AARCH64_CC_PACK ]; then 
        wget $AARCH64_URL -O $INSTALL_PATH/$AARCH64_CC_DL -c -t 0
        if [ -f $INSTALL_PATH/$AARCH64_CC_DL ]; then
            mv $INSTALL_PATH/$AARCH64_CC_DL $INSTALL_PATH/$AARCH64_CC_PACK
            echo "[4]: Got CC "$INSTALL_PATH/$AARCH64_CC_PACK
        fi
    else
        echo "[4]: AARCH64 CC Already Exists"
    fi
fi

if [ ! -f $INSTALL_PATH/$AARCH32_CC_PACK ]; then
    echo "[4]: Failed, AARCH32 CC Package Non Found !!!"
    exit
fi
if [ ! -f $INSTALL_PATH/$AARCH64_CC_PACK ]; then
    echo "[4]: Failed, AARCH64 CC Package Non Found !!!"
    exit    
fi

# untar aarch32 cc
echo "[4]: Untar AARCH32 CC in process.."
if [ ! -d $FREERTOS_AARCH32_CROSS_PATH ]; then
    tar -C $INSTALL_PATH/ -jxf $INSTALL_PATH/$AARCH32_CC_PACK
    if [ $? -ne 0 ]; then
        echo "[4]: Failed when Untar AARCH32 CC !!!"
        exit
    fi
fi

# write aarch32 cc path
echo "[4]: AARCH32 CC Install Success at "$FREERTOS_AARCH32_CROSS_PATH

# untar aarch64 cc
echo "[4]: Untar AARCH64 CC in process.."
if [ ! -d $FREERTOS_AARCH64_CROSS_PATH ]; then
    tar -C $INSTALL_PATH/ -vxf $INSTALL_PATH/$AARCH64_CC_PACK
    if [ $? -ne 0 ]; then
        echo "[4]: Failed when Untar AARCH64 CC !!!"
        exit
    fi
fi

# write aarch64 cc path
echo "[4]: AARCH64 CC Install Success at "$FREERTOS_AARCH64_CROSS_PATH

# write environment variables
echo "export FREERTOS_AARCH32_CROSS_PATH=$FREERTOS_AARCH32_CROSS_PATH" >> $PROFILE_PATH 
echo "export PATH=\$FREERTOS_AARCH32_CROSS_PATH/bin:\$PATH" >> $PROFILE_PATH
echo "export FREERTOS_AARCH64_CROSS_PATH=$FREERTOS_AARCH64_CROSS_PATH" >> $PROFILE_PATH
echo "export PATH=\$FREERTOS_AARCH64_CROSS_PATH/bin:\$PATH" >> $PROFILE_PATH
echo "export FREERTOS_STANDALONE=$STANDALONE_PATH" >> $PROFILE_PATH
echo "export FREERTOS_SDK_ROOT=$FREERTOS_SDK_ROOT" >> $PROFILE_PATH
if [ $? -ne 0 ]; then
    echo "[4]: Failed when Update Environment Variables !!!"
    exit
fi

# show success info
echo "[5]: Success!!! FreeRTOS SDK is Install at "$FREERTOS_SDK_ROOT
echo "[5]: Phytium FreeRTOS SDK Setup Done!!!"