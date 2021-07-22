###
 # @ : Copyright (c) 2021 Phytium Information Technology, Inc. 
 #  
 # SPDX-License-Identifier: Apache-2.0.
 # 
 # @Date: 2021-07-02 10:54:34
 # @LastEditTime: 2021-07-22 15:56:44
 # @Description:  This files is for 
 # 
 # @Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 
#!/bin/sh

version=v0.0.3
branch=master
git update-index --chmod=+x ./install.sh
git update-index --chmod=+x ./*.sh
git update-index --chmod=+x ./scripts/*.sh
git update-index --chmod=+x ./make/*.mk
git add .
git tag -a $version -m "Release $version"
git commit -m "Release $version"
git push origin $branch --tags