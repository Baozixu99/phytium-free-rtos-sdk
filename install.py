#!/usr/bin/env python3

import os
import platform
import re

### platform constant
platform_tags = ["Linux_X86_64" "Linux_AARCH64" "Windows_x64"]
linux_x86 = 0
linux_aarch64 = 1
windows_x64 = 2

# STEP 1: Check environment
if (platform.system() == 'Linux' ) and (platform.machine() == 'x86_64'):
    install_platform = linux_x86
elif (platform.system() == 'Linux' ) and (platform.machine() == 'aarch64'): # Arm64 computer
    install_platform = linux_aarch64
elif (platform.system() == 'Windows') and (platform.machine() == 'AMD64'):
    install_platform = windows_x64
else:
    print("Platform not support !!! ")
    exit()

# get absoulte path current pwd to install sdk
install_path, install_script = os.path.split(os.path.abspath(__file__))
curr_path = os.getcwd()
# get absolute path of sdk install dir
freertos_sdk_path = install_path

# Add standalone sdk
standalone_sdk_v="16c19e5a8b157e1fedb6f40a014a2c4a95f804a9"
if (install_platform == windows_x64):
    standalone_path=freertos_sdk_path  + '\\standalone'
else:
    standalone_path=freertos_sdk_path  + '/standalone'
standalone_branch="master"
standalone_remote="https://gitee.com/phytium_embedded/phytium-standalone-sdk"

if not os.path.exists(standalone_path):
    current_path = os.getcwd()

    os.system("git clone -b {} {} {}".format(standalone_branch, standalone_remote,standalone_path))
    os.chdir(standalone_path)# 切换工作路径至standalone 路径
    os.system("git config core.sparsecheckout true")
    os.system("git config advice.detachedHead false")
    os.system("git sparse-checkout init")

    # 适配 windows 环境，路径不兼容
    os.system("git sparse-checkout set /arch \
                                       /board \
                                       /common \
                                       /drivers \
                                       /standalone.mk \
                                       /lib \
                                       /doc \
                                       /third-party \
                                       !/third-party/lwip-2.1.2/ports/arch \
                                       /tools \
                                       /standalone.kconfig \
                                       /soc")

    os.system("git checkout {}".format(standalone_sdk_v))
    print('Standalone SDK download is succeed')
else:
    # Check if the existing version matches the required version
    current_path = os.getcwd()
    os.chdir(standalone_path)
    current_hash = os.popen('git rev-parse HEAD').read().strip()
    os.chdir(current_path)
    
    if current_hash != standalone_sdk_v:
        print(f"\033[93mWarning: Existing standalone SDK version ({current_hash}) doesn't match required version ({standalone_sdk_v})\033[0m")
        print(f"\033[93mRecommend: Delete the existing standalone SDK and run install.py to download the required version\033[0m")
        # Optionally, you could add code here to update the repository to the correct version
        # os.chdir(standalone_path)
        # os.system(f"git checkout {standalone_sdk_v}")
        # os.chdir(current_path)
        # print("Standalone SDK has been updated to the required version")
        exit()
    else:
        # in case user call this script not from current path
        if (curr_path != install_path):
            exit()
        print("Existing standalone SDK version matches the required version")
    pass

## STEP 2: display success message and enable environment
print("Success!!! Phytium FreeRTOS SDK is Install at {}".format(freertos_sdk_path))
