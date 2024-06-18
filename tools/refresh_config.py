#!/usr/bin/env python
# -*- coding:utf-8 -*-
###
# Filename: f:\pr_i2c_bug\phytium-free-rtos-sdk\refresh_config.py
# Path: f:\pr_i2c_bug\phytium-free-rtos-sdk
# Created Date: Friday, May 31st 2024, 5:00:39 pm
# Author: Simon Liu
# 
# Copyright (c) 2024 Phytium Information Technology, Inc.
###
import os
import subprocess
import sys

# 搜索包含configs目录的项目路径
def search_project_paths(sdk_example_path):
    project_paths = []
    for root, dirs, files in os.walk(sdk_example_path):
        if 'configs' in dirs and 'sdkconfig.h' in files:
            # 获取相对路径
            relative_path = os.path.relpath(root, sdk_example_path)
            project_paths.append(relative_path)
    return project_paths

# 此函数从给定的项目路径节点中收集所有的配置文件
def gather_configs(project_path_node):
    # 配置文件目录路径
    config_dir = os.path.join(project_path_node, 'configs')
    # 列出所有以".config"结尾的配置文件
    return [f for f in os.listdir(config_dir) if f.endswith(".config")]

# 此函数处理每一个项目路径节点
def process_project_path_node(sdk_example_path, project_path_node):
    # 得到项目的完整路径
    full_path = os.path.join(sdk_example_path, project_path_node)
    # 收集所有的配置文件
    configs = gather_configs(full_path)
    
    configs.sort(key=str.lower)
    print("开始刷新", project_path_node)
    # 针对每一个配置文件进行处理
    for config in configs:
        # 去除文件的扩展名
        config_name, _ = os.path.splitext(config)
        out_name = (project_path_node + '_' + config_name)
        out_name = out_name.replace('/', '_')

        # 执行make命令序列
        commands = [
            f"make load_kconfig LOAD_CONFIG_NAME={config_name}",
            "make update_menuconfig",
            "make backup_kconfig",
        ]
        for command in commands:
            result = subprocess.call(command, shell=True, cwd=full_path)
            if result != 0:
                print(f"Error: Command '{command}' failed with exit code {result}", file=sys.stderr)
                sys.exit(result)


def main(sdk_example_path):
    # 动态搜索项目路径
    project_paths = search_project_paths(sdk_example_path)
    
    for project_path_node in project_paths:
        process_project_path_node(sdk_example_path, project_path_node)
    print("\033[92mRefresh Config Successfully\033[0m")

# 程序入口点
if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    sdk_example_path = os.path.abspath(os.path.join(script_dir, "../example"))
    print(f"sdk_example_path = {sdk_example_path}")
    main(sdk_example_path)
