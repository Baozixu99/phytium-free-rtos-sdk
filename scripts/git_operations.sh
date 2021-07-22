###
 # @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 #  
 # SPDX-License-Identifier: Apache-2.0.
 # 
 # @Date: 2021-07-06 08:46:10
 # @LastEditTime: 2021-07-22 11:00:29
 # @Description:  This files is for 
 # 
 # @Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 
#!/bin/sh

git update-index --chmod=+x ./install.sh
git update-index --chmod=+x ./*.sh
git update-index --chmod=+x ./scripts/*.sh
git update-index --chmod=+x ./make/*.mk
git update-index --chmod=+x ./standalone/lib/Kconfiglib/*.py