###
 # @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 #  
 # SPDX-License-Identifier: Apache-2.0.
 # 
 # @Date: 2021-07-02 08:17:54
 # @LastEditTime: 2021-07-02 08:37:23
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

CC_NAME=gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf
AARCH32_LINUX_URL=https://releases.linaro.org/components/toolchain/binaries/4.9-2017.01/arm-linux-gnueabihf/$CC_NAME.tar.xz
AARCH32_LINUX_PACK=$CC_NAME.tar.xz
INSTALL_PATH=$FREERTOS_SDK_ROOT/tools
export AARCH32_LINUX_CROSS_PATH=$INSTALL_PATH/$CC_NAME

if [ $OFFLINE_INSTALL -ne 1 ]; then
# do not download if package exists
    [ ! -d $AARCH32_LINUX_CROSS_PATH ] && [ ! -f $INSTALL_PATH/$AARCH32_LINUX_PACK ] && wget -P $INSTALL_PATH/ $AARCH32_LINUX_URL
fi

if [ ! -f  $INSTALL_PATH/$AARCH32_LINUX_PACK ]; then
    echo "Gcc compiler package non found !!!"$INSTALL_PATH/$AARCH32_LINUX_PACK
    exit 1
fi

## LINUX GNU CC
[ ! -d $AARCH32_LINUX_CROSS_PATH ] && sudo tar -C $INSTALL_PATH/ -vxf $INSTALL_PATH/$AARCH32_LINUX_PACK
sed -i '/export AARCH32_LINUX_CROSS_PATH=/d' $PROFILE_PATH
sed -i '/export PATH=\$PATH:\$AARCH32_LINUX_CROSS_PATH/d' $PROFILE_PATH
echo "export AARCH32_LINUX_CROSS_PATH=$AARCH32_LINUX_CROSS_PATH" >> $PROFILE_PATH
echo "export PATH=\$PATH:\$AARCH32_LINUX_CROSS_PATH/bin">> $PROFILE_PATH
