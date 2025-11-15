import os
import subprocess
import glob
from pathlib import Path
import argparse

start_path = os.path.dirname(os.path.abspath(__file__))
ALL_MISRAC_RULE = [
        '1.1', '1.2', '1.3', '1.4', '2.1',
        '2.2', '2.3', '2.4', '2.5', '2.6',
        '2.7', '3.1', '3.2', '4.1', '4.2',
        '5.1', '5.2', '5.3', '5.4', '5.5',
        '5.6', '5.7', '5.8', '5.9', '6.1',
        '6.2', '7.1', '7.2', '7.3', '7.4',
        '8.1', '8.2', '8.3', '8.4', '8.5',
        '8.6', '8.7', '8.8', '8.9', '8.10',
        '8.11', '8.12', '8.13', '8.14', '9.1',
        '9.2', '9.3', '9.4', '9.5', '10.1',
        '10.2', '10.3', '10.4', '10.5', '10.6',
        '10.7', '10.8', '11.1', '11.2', '11.3',
        '11.4', '11.5', '11.6', '11.7', '11.8',
        '11.9', '12.1', '12.2', '12.3', '12.4',
        '13.1', '13.2', '13.3', '13.4', '13.5',
        '13.6', '14.1', '14.2', '14.3', '14.4',
        '15.1', '15.2', '15.3', '15.4', '15.5',
        '15.6', '15.7', '16.1', '16.2', '16.3',
        '16.4', '16.5', '16.6', '16.7', '17.1',
        '17.2', '17.3', '17.4', '17.5', '17.6',
        '17.7', '17.8', '18.1', '18.2', '18.3',
        '18.4', '18.5', '18.6', '18.7', '18.8',
        '19.1', '19.2', '20.1', '20.2', '20.3',
        '20.4', '20.5', '20.6', '20.7', '20.8',
        '20.9', '20.10', '20.11', '20.12', '20.13',
        '20.14', '21.1', '21.2', '21.3', '21.4',
        '21.5', '21.6', '21.7', '21.8', '21.9',
        '21.10', '21.11', '21.12', '21.14', '21.15',
        '21.16', '21.19', '21.20', '21.21', '21.12',
        '22.1', '22.2', '22.3', '22.4', '22.5',
        '22.6', '22.7', '22.8', '22.9', '22.10',
    ]

def clear_output_file(filename):
    """Clear the output file"""
    with open(filename, 'w') as f:
        pass

def clone_repo_at_commit(repo_url, branch, commit_hash, target_dir):
    original_dir = os.getcwd()  # Save the current working directory

    try:
        if not os.path.exists(target_dir):
            os.makedirs(target_dir)
        elif os.listdir(target_dir):
            print(f"The directory {target_dir} already exists and is not empty, skip cloning operation.\n")
            return
        # 执行 git clone 命令
        subprocess.run(['git', 'clone', '--branch', branch, '--single-branch', repo_url, target_dir], stderr=subprocess.STDOUT, text=True)
        os.chdir(target_dir)
        # 执行 git checkout 命令
        subprocess.run(['git', 'checkout', commit_hash], stderr=subprocess.STDOUT, text=True)
        print(f"Cloned {repo_url} to {target_dir} and checked out branch {branch}, commit {commit_hash}\n")
    
    finally:
        os.chdir(original_dir)  # Change back to the original directory

def run_cppcheck(path, include_dirs, ignore_dirs, output_file, sdk_path):
    """运行 cppcheck 命令"""
    cmd = ['cppcheck', '--std=c99', '--dump', '--quiet', '--force', '--enable=all', f'--include={sdk_path}/example/system/template/sdkconfig.h']
    for dir in ignore_dirs:
        cmd.extend(['-i', dir.rstrip('/') + '/'])

    for dir in include_dirs:
        cmd.extend(['-I', dir.rstrip('/') + '/'])

    cmd.append(path)
    # print("执行命令:")
    # print(" ".join(cmd))
    # 执行 cppcheck 命令
    with open(output_file, 'a') as f:
        subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, text=True)

def run_misra_check(start_path, path, output_file, suppress_misra_rules):
    cmd = ['python3', f'{start_path}/cppcheck/addons/misra.py', f'--rule-texts={start_path}/MISRA_C_2012.txt']
    dump_files = glob.glob(os.path.join(path, '**', '*.dump'), recursive=True)
    if not dump_files:
        with open(output_file, 'a') as f:
            f.write(f"'.dump' file not found in {path}\n")
        return
    
    if suppress_misra_rules:
        suppress_rules_str = ','.join(suppress_misra_rules)
        cmd.extend(['--suppress-rules', suppress_rules_str])
    
    # 执行 misra.py 命令
    cmd = cmd + dump_files
    # print(" ".join(cmd))
    with open(output_file, 'a') as f:
        subprocess.run(cmd, stdout=f, stderr=subprocess.STDOUT, text=True)

def cleanup_files(directory, extensions):
    dir_path = Path(directory)
    for ext in extensions:
        for file in dir_path.rglob(f'*{ext}'):
            if file.is_file():
                file.unlink()

def find_header_directories(directory, ignore_dirs):
    header_dirs = set()

    # 遍历给定目录及其子目录
    for root, dirs, files in os.walk(directory):
        # 检查当前目录是否在忽略列表中
        if any(ignored in root for ignored in ignore_dirs):
            continue

        for file in files:
            # 检查文件扩展名是否为 .h
            if file.endswith('.h'):
                # 获取上一层文件夹路径
                header_dirs.add(os.path.normpath(root))

    return header_dirs

def update_header_dirs(header_dirs, new_dirs):
    # 用于存储重复出现的路径
    seen_dirs = set()
    to_remove = set()

    for dir in new_dirs:
        if dir in header_dirs:
            # 如果新路径在已有路径中，标记为删除
            to_remove.add(dir)
        elif dir in seen_dirs:
            # 如果在当前新路径集合中已经出现过，标记为删除
            to_remove.add(dir)
        else:
            # 如果是新路径，添加到seen_dirs集合
            seen_dirs.add(dir)

    # 删除标记的路径
    header_dirs -= to_remove
    header_dirs.update(seen_dirs)

def find_project_root(start_path, marker_dir):
    current_path = os.path.abspath(start_path)
    while current_path != os.path.abspath(os.sep):
        if os.path.isdir(os.path.join(current_path, marker_dir)):
            return current_path  # 返回当前路径而不是上一层
        current_path = os.path.dirname(current_path)
    
    raise FileNotFoundError(f"未找到 '{marker_dir}' 目录。")

def parse_arguments():
    parser = argparse.ArgumentParser(description='MISRA C Check Script')
    parser.add_argument(
        '--misra-rules',
        nargs='+',
        required=True,
        help='List of MISRA rules to check (e.g., 10.3 17.7)'
    )
    parser.add_argument(
        '--files-path',
        nargs='+',
        required=True,
        help='List of file paths to run MISRA checks on'
    )
    return parser.parse_args()

def remove_keywords(file_path, keywords):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        for keyword in keywords:
            content = content.replace(keyword, '')
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(content)

    except FileNotFoundError:
        print(f"文件 {file_path} 未找到。")
    except Exception as e:
        print(f"处理文件时发生错误: {e}")

if __name__ == "__main__":
    REPO_URL = "https://gitee.com/mirrors/cppcheck.git"
    BRANCH = "main"
    COMMIT_HASH = "e7811ed9d312d66c0fc2c4c0a0d1f1bfd9b6a27c"
    OUTPUT_FILE = 'misrac_check_output.txt'

    FILES_PATH_TO_RUN_CPP_CHECK = find_project_root(start_path, '.gitlab')
    CPPCHECK_CLONE_DIR = os.path.join(start_path, 'cppcheck')
    
    GET_HEADER_PATH_DIRECTORIES_TO_IGNORE = [
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'lib/'), 
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'third-party/'), 
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'tools/'), 
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'doc/'),
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'scripts/'),
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'drivers/'),
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'example/'),
    ]

    DIRECTORIES_TO_IGNORE = [
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'lib/'), 
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'third-party/'), 
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'tools/'), 
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'doc/'),
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, 'scripts/'),
    ]

    args = parse_arguments()
    MISRAC_RULE = args.misra_rules
    FILES_PATH_TO_RUN_MISRAC_CHECK = [
        os.path.join(FILES_PATH_TO_RUN_CPP_CHECK, path) for path in args.files_path
    ]
    SUPPRESS_MISRAC_RULES = [rule for rule in ALL_MISRAC_RULE if rule not in MISRAC_RULE]
    # Clear the output.txt file
    clear_output_file(OUTPUT_FILE)
    # Clone the repository and check out the specified commit
    clone_repo_at_commit(REPO_URL, BRANCH, COMMIT_HASH, CPPCHECK_CLONE_DIR)

    for path in FILES_PATH_TO_RUN_MISRAC_CHECK:
        header_dirs = set()
        header_dirs.update(find_header_directories(FILES_PATH_TO_RUN_CPP_CHECK, GET_HEADER_PATH_DIRECTORIES_TO_IGNORE))
        new_dirs = find_header_directories(path, [])
        update_header_dirs(header_dirs, new_dirs)
        # Run cppcheck
        run_cppcheck(path, header_dirs, DIRECTORIES_TO_IGNORE, OUTPUT_FILE, FILES_PATH_TO_RUN_CPP_CHECK)
        # Run misra check
    for path in FILES_PATH_TO_RUN_MISRAC_CHECK:
        run_misra_check(start_path, path, OUTPUT_FILE, SUPPRESS_MISRAC_RULES)
    # Cleanup .ctu-info and .dump files
    cleanup_files(FILES_PATH_TO_RUN_CPP_CHECK, ['.ctu-info', '.dump'])

    combined_keyword = ''.join(FILES_PATH_TO_RUN_CPP_CHECK) + '/'
    remove_keywords(OUTPUT_FILE, [combined_keyword])
    found_keywords = False
    with open(OUTPUT_FILE, 'r', encoding='utf-8') as file:
        for line in file:
            if any(keyword in line for keyword in MISRAC_RULE):
                print(line.strip())
                found_keywords = True
    
    if found_keywords:
        print('There is code in the detection results that does not comply with the specified misrac_2012 specification, please check.')
        exit(1)
    else:
        print('The detection code complies with the specified misrac_2012 specification.')

