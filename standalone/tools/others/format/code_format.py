import os
import sys
import subprocess
import tempfile
import json
"""

在项目根目录下执行该脚本
以phytium-standalone-sdk/arch为例:
脚本在phytium-standalone-sdk下执行, 命令为
python3 tools/others/format/code_format.py arch/
clang-format-15 -i -style=file -assume-filename=.clang-format <target_dir>
"""
def find_c_files(directories):
    c_files = []
    project_fir = os.getcwd()
    print(f"Searching for C/C++ files in directories: {directories}")
    for directory in directories:
        directory = os.path.join(project_fir, directory)
        print(f"Searching in directory: {directory}")
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith('.c') or file.endswith('.h'):
                    c_files.append(os.path.join(root, file))
    return c_files

def run_clang_format(file_path, clang_format_path):
    print(f"Formatting file_path:{file_path}")
    result = subprocess.run(
        ['clang-format-15', '-i','-style=file', '-assume-filename=.clang-format', file_path],
        cwd=clang_format_path,
        capture_output=True,
        text=True
    )
    return result.stdout

def diff_files(original, formatted, file_path):
    """
    Compare the original and formatted file contents and return their differences in git diff format.

    This function writes the original and formatted contents to temporary files, then uses `git diff --no-index`
    to generate a diff between them. The output paths in the diff are replaced with the actual file path and
    a label indicating the formatted version.

    Args:
        original (str): The original file content.
        formatted (str): The formatted file content.
        file_path (str): The path to the file being compared (used for display in the diff output).

    Returns:
        str: The diff output in git diff format, showing differences between the original and formatted contents.
    """
    with tempfile.NamedTemporaryFile(delete=False) as orig_tmp, tempfile.NamedTemporaryFile(delete=False) as fmt_tmp:
        orig_tmp.write(original.encode())
        fmt_tmp.write(formatted.encode())
        orig_tmp.flush()
        fmt_tmp.flush()
        diff_cmd = ['git', 'diff', '-U0', '--no-index','--', orig_tmp.name, fmt_tmp.name]
        result = subprocess.run(diff_cmd, capture_output=True, text=True)
        diff_output = result.stdout
    os.unlink(orig_tmp.name)
    os.unlink(fmt_tmp.name)
    diff_output = diff_output.replace(orig_tmp.name, file_path).replace(fmt_tmp.name, file_path + ' (formatted)')
    return diff_output

def main():
    if len(sys.argv) != 2:
        print(f'Usage: {sys.argv[0]} <target_directory>')
        sys.exit(1)
    target_dir = sys.argv[1]
    format_files_cnt = 0
    if isinstance(target_dir, str):
        target_dir = [target_dir]
    
    if 'drivers/part1' in target_dir:
        target_dir = ["drivers/adc", "drivers/can", "drivers/device", "drivers/dma", "drivers/eth", 
        "drivers/i2c", "drivers/i2s", "drivers/i3c", "drivers/iomux", "drivers/ipc", "drivers/media", "drivers/mio",
        "drivers/mmc", "drivers/msg", "drivers/nand"]
        
    if 'drivers/part2' in target_dir:
        target_dir = ["drivers/pcie","drivers/pin","drivers/port","drivers/pwm", "drivers/qspi", "drivers/rtc",
                      "drivers/sata", "drivers/scmi", "drivers/sec", "drivers/serial", "drivers/spi","drivers/timer","drivers/watchdog"]
        
    if 'example/part1' in target_dir:
        target_dir =["example/peripherals/can","example/peripherals/can_spi","example/peripherals/dma","example/peripherals/generic_timer",
                     "example/peripherals/gicv3","example/peripherals/i2c","example/peripherals/i2s","example/peripherals/i3c","example/peripherals/iopad",
                     "example/peripherals/ipc","example/peripherals/pcie","example/peripherals/pin","example/peripherals/pmbus","example/peripherals/pwm"]
    if 'example/part2' in target_dir:
        target_dir =["example/peripherals/qspi","example/peripherals/sata","example/peripherals/sec","example/peripherals/serial","example/peripherals/spi",
                     "example/peripherals/timer","example/peripherals/usb","example/peripherals/wdt"]
        
    if 'example/part3' in target_dir:
        target_dir =["example/ai","example/media","example/network","example/python","example/storage"]
    
    clang_format_path = os.path.dirname(os.path.abspath(__file__))
    
    
    c_files = find_c_files(target_dir)
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(script_dir, 'filter_dir.json')

    
    with open(json_path, 'r') as f:
        filter_config = json.load(f)
    
    # 提取配置规则
    exclude_suffixes = tuple(filter_config["exclude_suffixes"])
    exclude_dir_patterns = filter_config["exclude_dir_patterns"]
    conditional_excludes = filter_config["conditional_excludes"]
    
    print(f"Exclude suffixes: {exclude_suffixes}")
    print(f"Exclude dir patterns: {exclude_dir_patterns}")
    print(f"Conditional excludes: {conditional_excludes}")
    
    # 规则1: 后缀名匹配
    for file_path in c_files:
        if file_path.endswith(exclude_suffixes):
            continue
    # 规则2: 目录模式匹配
        if any(pattern in file_path for pattern in exclude_dir_patterns):
            continue
    # 规则3：子目录匹配        
        skip_file = False
        for condition in conditional_excludes:
            req_str = condition["required_substring"]
            miss_list = condition["exclude_if_missing"]
        
            if req_str in file_path:
            # 如果所有指定字符串都不存在，则排除
                if all(miss not in file_path for miss in miss_list):
                    skip_file = True
                    break
                
        if skip_file:
            continue
        
        
        formatted = run_clang_format(file_path, clang_format_path)
        
        
        format_files_cnt += 1 

    
    print(f"Formatted {format_files_cnt} C/C++ files.")

if __name__ == '__main__':
    main()
