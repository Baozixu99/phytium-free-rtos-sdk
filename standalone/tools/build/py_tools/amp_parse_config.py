#!/usr/bin/env python3
import json
import sys
from itertools import combinations
import math
import os
import re
import subprocess
import shutil
class ConfigManager:
    def __init__(self, required_vars, combined_data):
        self.required_vars = required_vars
        self.combined_data = combined_data
        self.overlap_code = None
        self.config_file_path = None
        
    def prase_and_check_configs(self):

            # 基于 `self.required_vars` 中指定的必需变量比较配置。
            # 此方法检查不同路径中的配置是否对所需变量有一致的设置。
            config_manager.compare_configs()

            # 检查配置参数 "CONFIG_IMAGE_LOAD_ADDRESS" 和 "CONFIG_IMAGE_MAX_LENGTH" 是否存在重叠。
            # 此方法确保这些参数在不同配置中没有重叠的值，这可能导致冲突。
            overlap_code = config_manager.check_overlap()
            
            # 检索配置文件的路径。
            # 此方法提取并返回配置文件所在的路径。
            config_file_path = config_manager.configs_path_get()
            
            # 从配置文件中提取值。
            # 此方法读取指定的配置文件并提取关键值，特别是那些用于引导适应的必需值。
            config_values = config_manager.extract_config_values()
            
            # 返回一个元组，包含组合配置数据、重叠状态、配置文件路径和提取的配置值。
            # 这些返回数据封装了该方法执行的各种检查和解析操作的结果。
            return overlap_code, config_file_path, config_values
        
    def read_config_values(self, file_path, variables, para):
        values = {}
        with open(file_path, 'r') as file:
            for line in file:
                for var in variables:
                    if line.startswith(var):
                        if para == 16:
                            values[var] = int(line.strip().split('=')[1], 16)
                        else:
                            values[var] = line.strip().split('=')[1]
                        break
        return values
    
    def compare_configs(self):
        """
        比较指定路径下的配置文件。

        对每一对配置文件,比较它们是否在所有必需的变量(required_vars)上具有相同的值。
        如果所有对比的文件在所有必需的变量上值都相同，则认为它们匹配。

        参数:
            self.required_vars (list): 需要比较的变量名列表。

        返回:
            tuple: 包含比较结果的字典和所有文件是否匹配的布尔值。
        """
        # 存储每个配置文件中的变量值
        config_values = {}

        # 读取每个配置文件中的变量值
        for name, data in self.combined_data.items():
            path = data[0]
            full_path = os.path.join(path, 'sdkconfig')
            config_values[full_path] = self.read_config_values(full_path, self.required_vars, None)

        # 比较变量值
        comparison_result = {}
        all_match = True
        
        for file1, file2 in combinations(config_values.keys(), 2):
            mismatches = {}
            for var in self.required_vars:
                if config_values[file1].get(var) != config_values[file2].get(var):
                    mismatches[var] = (config_values[file1].get(var), config_values[file2].get(var))
                    all_match = False

            if mismatches:
                comparison_result[(file1, file2)] = mismatches
            else:
                comparison_result[(file1, file2)] = "Match"    
                
            # 配置文件检查
    
        if all_match:
            print('比较成功')
            pass
        else:
            print(self.format_comparison_result(all_match,comparison_result))
            sys.exit("error: 参数对比失败 (Error: Parameter comparison failed)")

        return comparison_result ,all_match
    
    def format_comparison_result(self,all_match, comparison_result):
        if all_match:
            return "所有配置文件完全匹配。"

        formatted_output = ["配置文件比较结果:\n"]
        for file, result in comparison_result.items():
            if result == "Base file for comparison":
                formatted_output.append(f"基准文件: {file}")
            else:
                formatted_output.append(f"文件: {file}")
                if result == "Match":
                    formatted_output.append("  匹配基准文件")
                else:
                    for var, (val1, val2) in result.items():
                        formatted_output.append(f"  变量不匹配: {var} (基准值: {val1}, 当前文件值: {val2})")

        return '\n'.join(formatted_output)
    
    def check_overlap(self):
        """
        检查配置文件中指定的内存范围是否存在重叠。

        此方法遍历所有提供的配置文件，并检查它们的 'CONFIG_IMAGE_LOAD_ADDRESS' 和
        'CONFIG_IMAGE_MAX_LENGTH' 变量以确定内存地址范围。它检查这些范围是否
        在任何配置文件对中重叠。

        参数:
            config_files (dict): 包含配置文件路径和文件名的字典。

        返回:
            tuple: 包含重叠检查结果的布尔值和解析出的配置值字典。
        """
        config_values = {}

        # 读取每个配置文件中的变量值
        for name, data in self.combined_data.items():
            path = data[0]
            full_path = os.path.join(path, 'sdkconfig')
            # 处理关心的["CONFIG_IMAGE_LOAD_ADDRESS", "CONFIG_IMAGE_MAX_LENGTH"]
            config_values[full_path] = self.read_config_values(full_path, ["CONFIG_IMAGE_LOAD_ADDRESS", "CONFIG_IMAGE_MAX_LENGTH"],16)
            
        overlap_check = True
        file_paths = list(config_values.keys())
        for i in range(len(file_paths) - 1):
            for j in range(i + 1, len(file_paths)):
                load_addr1, max_length1 = config_values[file_paths[i]].values()
                load_addr2, max_length2 = config_values[file_paths[j]].values()
                if not (load_addr1 + max_length1 <= load_addr2 or load_addr2 + max_length2 <= load_addr1):
                    overlap_check = False
        print(config_manager.format_overlap_result(config_values, overlap_check))
    
        if not overlap_check :
            sys.exit("error: 配置范围存在重叠 (Error: Configuration range overlaps)")
        self.overlap_code = config_values 
        return config_values
    
    
    def format_overlap_result(self,config_values, overlap_check):
        formatted_output = ["配置文件范围比较结果:\n"]
        for file, values in config_values.items():
            load_address = values["CONFIG_IMAGE_LOAD_ADDRESS"]
            max_length = values["CONFIG_IMAGE_MAX_LENGTH"]
            formatted_output.append(f"文件: {file}")
            formatted_output.append(f"  LOAD_ADDRESS: {hex(load_address)}, MAX_LENGTH: {hex(max_length)}")

        if overlap_check:
            formatted_output.append("\n所有配置文件的范围均不重叠。")
        else:
            formatted_output.append("\n存在范围重叠。请调整以下参数以避免重叠:")
            formatted_output.append("  1. CONFIG_IMAGE_LOAD_ADDRESS")
            formatted_output.append("  2. CONFIG_IMAGE_MAX_LENGTH")
            formatted_output.append("确保两个配置文件中的LOAD_ADDRESS + MAX_LENGTH范围不重叠。")

        return '\n'.join(formatted_output)
    
    def configs_path_get(self):
        # 读取每个配置文件中的变量值
        for name, data in self.combined_data.items():
            path = data[0]
            full_path = os.path.join(path, 'sdkconfig')
            self.config_file_path = full_path
            return full_path
            
    def extract_config_values(self):
        file_path = self.config_file_path
        variables = self.required_vars
        values = {}
        with open(file_path, 'r') as file:
            for line in file:
                for var in variables:
                    if line.startswith(var):
                        key, value = line.strip().split('=', 1)
                        values[key] = value
                        break
        # 返回值为最终用于boot 自适应配置的值
        return values


#*****************************************************************************************************
def remove_config_line(path, line_to_remove,file_name):
    config_file_path = os.path.join(path, file_name)
    print(config_file_path)
    with open(config_file_path, 'r') as file:
        lines = file.readlines()
    with open(config_file_path, 'w') as file:
        for line in lines:
            if line.strip() != line_to_remove:
                file.write(line)

        
def find_elf_files(combined_data):
    elf_files = []
    for name, data in combined_data.items():
        path = data[0]
        file = data[3]

        # 检查每个路径下的所有文件
        full_path = os.path.abspath(path)
        if os.path.isdir(full_path):
            for file in os.listdir(full_path):
                if file.endswith('.elf'):
                    elf_file_path = os.path.join(full_path, file)
                    elf_files.append(elf_file_path)
        else:
            sys.exit(f"错误：路径 {full_path} 不存在。")

    if not elf_files:
        sys.exit(f"错误：在{name}:{path} 中未找到任何 .elf 文件。")

    return elf_files

def pack_elf_files(elf_files, output_file):
    with open(output_file, 'wb') as packed:
        for elf_file in elf_files:
            with open(elf_file, 'rb') as file:
                packed.write(file.read())
            print(f"将{elf_file} 已添加到 {output_file}。")

def create_build_and_file(build_path, filename):
    # 检查 build 文件夹是否存在，如果不存在则创建它
    build_path = os.path.abspath(f"{build_path}/build")
    if not os.path.exists(build_path):
        os.makedirs(build_path)
        print(f"Created directory: {build_path}")
    else:
        print(f"Directory already exists: {build_path}")
        shutil.rmtree(build_path)
        os.makedirs(build_path)
        print(f"Removed and created directory: {build_path}")

    # 创建 amp_pack.c 文件的完整路径
    file_path = os.path.join(build_path, filename)
    if os.path.exists(file_path):  # 检查文件是否存在
        print(f"{filename} exists in {build_path}.")
    else:
        with open(file_path, 'w') as f:
            pass
        print(f"Creating file: {file_path}")

def execute_make_commands(combined_group, additional_config):
    master_flag = 0 # master_flag = 1 表示该combined_group已经有host core
    out_elf_path = []
    bootstrap_flag = 0 # bootstrap_flag = 1 表示该combined_group已经有bootstrap core
    for name, data in combined_group.items():
        if name == "bootstrap":
            bootstrap_flag = 1
            continue
        path = data[0]
        core = data[1]
        master = data[2]
        file = data[3]
        config_name = file.replace('.config', '')

        # 特定处理逻辑
        if additional_config.get('CONFIG_ARCH_EXECUTION_STATE') == '"aarch32"':
            print(f"{path}:CONFIG_USE_AARCH64_L1_TO_AARCH32=y is remove")
            remove_config_line(path, "CONFIG_USE_AARCH64_L1_TO_AARCH32=y","sdkconfig")
            result = subprocess.call("make gen_kconfig", shell=True, cwd=path)
            if result != 0:
                print(f"Error: Command 'make gen_kconfig' failed with exit code {result}", file=sys.stderr)
                sys.exit(result)

        if (master == 1):
            master_flag = master_flag + 1
            if core == -1:
                print(f"Error: {name}:{path}")
                print(f"master = {master} ,master core need to select a core.")
                sys.exit(result)
            if (master_flag > 1):
                print(f"Error: {name}:{path}")
                print(f"master_flag = {master_flag} ,master core need to select only one core.")
                sys.exit(result)
            master_path = path
            master_core = core
            master_config_name = config_name
            continue

        # 继续执行 make all
        cmd = [f"make all -j BUILD_IMAGE_CORE_NUM={core} BUILD_AMP_CORE=y"]
        result = subprocess.call(cmd, shell=True, cwd=path)
        if result != 0:
            print(f"Error: Command 'make all -j' failed with exit code {result}", file=sys.stderr)
            sys.exit(result)
        for file in os.listdir(path):
                if file.endswith('.elf'):
                    elf_file_path = os.path.join(path, file)
                    out_elf_path.append(elf_file_path)
    # 找到组的所有子镜像的elf文件
    # out_elf_path = find_elf_files(combined_group)        
    if master_flag == 1:#需要进行第一轮主从核的打包
        print("****************************************************************.")
        print(f"master_flag = {master_flag}, master core need to build now.")
        print(f"master_path = {master_path}, master_core = {master_core}, master_config_name = {master_config_name}")
        print("****************************************************************.")
        # 创建特征文件 user_make.mk 会检查
        pack_group_bin_file_name = f"{master_path}/packed.bin" #组的多个子镜像打包后elf文件名
        # 打包组的elf文件
        pack_elf_files(out_elf_path,pack_group_bin_file_name)
        # 创建特征文件 user_make.mk 会检查
        create_build_and_file(master_path,'amp_packed.c')
        # 继续执行 make all
        cmd = [f"make all -j BUILD_IMAGE_CORE_NUM={master_core} BUILD_AMP_CORE=y"]
        result = subprocess.call(cmd, shell=True, cwd=master_path)
        if result != 0:
            print(f"Error: Command 'make all -j' failed with exit code {result}", file=sys.stderr)
            sys.exit(result)
        out_elf_path.clear()
        # 检查每个路径下的所有文件
        full_path = os.path.abspath(master_path)
        if os.path.isdir(full_path):
            for file in os.listdir(full_path):
                if file.endswith('.elf'):
                    elf_file_path = os.path.join(full_path, file)
                    out_elf_path.append(elf_file_path)
        else:
            sys.exit(f"错误：路径 {full_path} 不存在。")
    else:#非主从核的打包组，每个镜像需要配置核心id
        if bootstrap_flag == 0:
            for name, data in combined_group.items():
                core = data[1]
                path = data[0]
                if (core == -1):
                    print(f"Error: {name}:{path}")
                    print(f"master = {master} ,master core need to select a core.")
                    sys.exit(result)
    print(f"out_elf_path:{out_elf_path}")
    return out_elf_path

def replace_config_values(config_path, config_data):
    # 读取配置文件
    try:
        with open(config_path, 'r') as file:
            lines = file.readlines()
    except IOError:
        print(f"无法打开文件 {config_path}")
        return
    existing_keys = set()
    # 替换配置值
    updated_lines = []

    # 替换已存在的配置值
    for line in lines:
        for key in config_data.keys():
            if line.startswith(key + "="):
                line = f"{key}={config_data[key]}\n"
                existing_keys.add(key)
                break
        updated_lines.append(line)
    
    for key, value in config_data.items():
        if key not in existing_keys:
            updated_lines.append(f"{key}={value}\n")

    # 写回配置文件
    try:
        with open(config_path, 'w') as file:
            file.writelines(updated_lines)
        print(f"配置文件 {config_path} 已更新。")
    except IOError:
        print(f"无法写入文件 {config_path}")

def make_clean_commands(config_data, configs_key):
    if configs_key in config_data:
        configs = config_data[configs_key]['configs']
        
        for path, config_files in configs.items():
            #执行 make clean
            result = subprocess.call("make clean", shell=True, cwd=path)
            if result != 0:
                print(f"Error: Command 'make clean' failed with exit code {result}", file=sys.stderr)
                sys.exit(result)
            else:
                print(f"Success clean {path}.")
    else:
        print(f"未找到配置项 {configs_key}")


def check_amp_config_in_json(second_person, amp_config_env):
    if amp_config_env in second_person["configs"]:
        return second_person["configs"][amp_config_env]
    return None

def load_check_update_kconfig(combined_data):
    # 检查路径是否存在相同项目
    path_items = []
    for name, data in combined_data.items():
        path_items.append(data[0])
    print(f"path_items:{path_items}")
    duplicates = [item for item in set(path_items) if path_items.count(item) > 1]
    if duplicates:
        sys.exit(f"Duplicate path_items found:{duplicates}")

    # 检查core是否相同
    core_items = []
    for name, data in combined_data.items():
        core_items.append(data[1])
    print(f"core_items:{core_items}")
    duplicates = [item for item in set(core_items) if core_items.count(item) > 1 and item != -1]
    if duplicates:
        sys.exit(f"Duplicate core_items found:{duplicates}")
    # 检查配置项是否相同
    config_items = []
    for name, data in combined_data.items():
        name = data[3].replace('.config', '')
        config_items.append(name)
    print(f"config_items:{config_items}")
    # 比较path_items.count(item) 和 core_items.count(item) 以及 config_items.count(item) 数量相等
    if len(path_items) != len(core_items) or len(path_items) != len(config_items):
        sys.exit(f"path_items.count(item) != core_items.count(item) or path_items.count(item) != config_items.count(item)")
    
    # 根据配置项数量，创建配置组
    for i in range(0,len(path_items)):
        print(f"开始加载第{i}个配置组")
        commands = [
            "make clean",
            f"make load_kconfig LOAD_CONFIG_NAME={config_items[i]}"
        ]

        # 执行命令并检查结果
        for command in commands:
            result = subprocess.call(command, shell=True, cwd=path_items[i])
            if result != 0:
                print(f"Error: Command '{command}' failed with exit code {result}", file=sys.stderr)
                sys.exit(result)
    #加载完默认配置到工程sdkconfig，返回配置项的路径，方便后续处理
    return path_items    

def read_config(file_path):
        # 用于存储配置项的字典
        config = {}

        # 打开并读取配置文件
        with open(file_path, 'r') as file:
            for line in file:
                # 跳过注释行（以 '#' 开头的行）
                if line.startswith('#') or line.strip() == "":
                    continue

                # 移除行末的换行符并解析配置项和值
                line = line.strip()

                # 只处理带有 '=' 的行
                if '=' in line:
                    key, value = line.split('=', 1)
                    config[key.strip()] = value.strip()

        return config

def check_config_values(config,variables,core):
    # 检查是否包含 CONFIG_USE_MSDF 和 CONFIG_MSDF_CORE_ID
    use_msdf = config.get(variables[0])
    msdf_core_id = config.get(variables[1])

    # 判断 CONFIG_USE_MSDF 是否等于 y
    if use_msdf == 'y':
        print("CONFIG_USE_MSDF is enabled (y).")
    else:
        sys.exit("CONFIG_USE_MSDF is not enabled.")

    
    # 输出 CONFIG_MSDF_CORE_ID 的值
    if int(msdf_core_id) != core:
        sys.exit(f"CONFIG_MSDF_CORE_ID is set to {msdf_core_id},not match with core id {core}.")
    else:
        print("CONFIG_MSDF_CORE_ID set OK.")
    
if __name__ == "__main__":
    # 从命令行参数中读取库文件路径、JSON脚本路径和JSON脚本名称
    cmd_step = ""
    if len(sys.argv) == 2:
        cmd_step = sys.argv[1] #新增auto_test command
    
    build_path = os.getcwd()
    file_path = os.path.join(build_path, 'amp_config.json')
    # 检查amp_config.json是否存在
    if not os.path.exists(file_path):
        sys.exit("amp_config.json 文件不存在。")

    # 打开文件并加载 JSON 数据
    with open(file_path, 'r') as file:
        data = json.load(file)
    # 获取第二个对象,第一个对象为配置举例
    second_person = data[1]
    
    # 检查环境变量AMP_CONFIG
    amp_config_env = os.environ.get('AMP_CONFIG')
        
    if amp_config_env:
        print(f"找到环境变量AMP_CONFIG: {amp_config_env} (Found environment variable AMP_CONFIG: {amp_config_env})")
        config_line = check_amp_config_in_json(second_person, amp_config_env)
        if config_line:
            # ... 继续其他逻辑 ...
            print("*********************************************************     配置如下    ***********************************************************************")
            
            print(f"{amp_config_env}:",config_line)
            print("************************************************************************************************************************************************")
            pass
        else:
            sys.exit("请检查amp_config.json中的配置项。请使用 make amp_make AMP_CONFIG=config<num> (Please check the configuration items in amp_config.json. Use make amp_make AMP_CONFIG=config<num>)")
    else:
        print("打印amp_config.json中的config<num>配置：(Printing config<num> configurations from amp_config.json:)")
        for config in second_person["configs"]:
            print(f"{config}:",second_person["configs"][config])
        sys.exit("error:  环境变量AMP_CONFIG不存在。请使用 make amp_make AMP_CONFIG=config<num> (Error: The AMP_CONFIG environment variable does not exist. Use make amp_make AMP_CONFIG=config<num>)")
        
    # 配置文件解析
    # 需要对比提取的规则
    required_vars = ["CONFIG_ARCH_NAME", "CONFIG_BOARD_NAME", "CONFIG_ARCH_EXECUTION_STATE", "CONFIG_SOC_NAME", "CONFIG_TARGET_TYPE_NAME","CONFIG_SOC_CORE_NUM"]
    num_members = len(second_person["configs"][amp_config_env])

    combined_data = {}
    for i in range(0,num_members):#合并配置项，用于统一检查
        print(f"开始解析第{i}个配置组")
        combined_data.update(second_person["configs"][amp_config_env][i])
    print(combined_data)
    project_path = load_check_update_kconfig(combined_data)
    
    # 实例化配置管理器
    config_manager = ConfigManager(required_vars, combined_data)
    overlap_code,config_file_path,config_values = config_manager.prase_and_check_configs()
    

    # 新建一个list，用于存储所有配置项的路径
    build_elf_paths = []
    if num_members > 1:
        # 开始根据规则对配置进行逐个加载与编译进行,0作为bootstrap引导最后打包
        for i in range(1,num_members):
            combined_group = second_person["configs"][amp_config_env][i]
            build_elf_paths.extend(execute_make_commands(combined_group,config_values))
    
    # 对主工程进行打包
    main_prj_slave = second_person["configs"][amp_config_env][0]
    if len(main_prj_slave) != 1:
        build_elf_paths.extend(execute_make_commands(main_prj_slave,config_values))
    
    print(f"从工程编译完成，生成的elf文件路径：{build_elf_paths}")
    pack_elf_file_name = "packed.bin"
    if os.path.exists(pack_elf_file_name):
        os.remove(pack_elf_file_name)
        # 之前的打包文件已经打包完成，删除
        print(f"已删除旧的{pack_elf_file_name}文件")
    
    print(f"开始将{build_elf_paths}打包进{pack_elf_file_name}文件")
    pack_elf_files(build_elf_paths,pack_elf_file_name)
    
    main_prj = second_person["configs"][amp_config_env][0]
    bootstrap_error_flag = 0
    print(f"main_prj:{main_prj}")
    for name, data in main_prj.items():
        if name == 'bootstrap':
            path = data[0]
            core = data[1]
            master = data[2]
            full_path = os.path.join(path, 'sdkconfig')
            bootstrap_error_flag = 1
    if bootstrap_error_flag == 0:
        sys.exit(f"error name found:{name}")
    # 读取配置文件
    config_data = read_config(full_path)
    # 检查配置项的值
    check_config_values(config_data,["CONFIG_USE_MSDF", "CONFIG_MSDF_CORE_ID"],core)

    sdk_path = os.environ.get('SDK_DIR')
    # 创建特征文件 user_make.mk 会检查
    create_build_and_file(build_path,'amp_packed.c')
    if cmd_step == "test":
        cmd = [f"make all -j BUILD_IMAGE_CORE_NUM={core} BUILD_AMP_CORE=y  IMAGE_OUT_NAME=packed_image BUILD_AMP_CORE_TEST=y"]
    else:
        cmd = [f"make all -j BUILD_IMAGE_CORE_NUM={core} BUILD_AMP_CORE=y  IMAGE_OUT_NAME=packed_image "]
    result = subprocess.call(cmd, shell=True, cwd=build_path)
    if result != 0:
        print(f"Error: Command 'make all -j' failed with exit code {result}", file=sys.stderr)
        sys.exit(result)
        