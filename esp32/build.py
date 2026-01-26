import os
import shutil
import subprocess
import argparse
import platform

# 设置 ESP-IDF 路径
esp_idf_path = {
    "Windows": "C:/Users/hzgaoqi1/esp/v5.4.1/esp-idf",
    "Darwin": "/path/to/esp-idf"  # macOS路径，需要根据实际情况修改
}

def run_command(command, cwd=None):
    """运行命令并捕获输出"""
    try:
        subprocess.run(command, check=True, cwd=cwd, shell=True)
    except subprocess.CalledProcessError as e:
        print(f"Error: {e}")
        exit(1)

def activate_espidf_env(esp_idf_path):
    """激活ESP-IDF环境"""
    if platform.system() == "Windows":
        # # 直接在Python中设置环境变量
        # os.environ["IDF_PATH"] = esp_idf_path["Windows"]
        # os.environ["PATH"] = f"{os.environ['PATH']};{os.path.join(esp_idf_path['Windows'], 'tools')}"
        print(f"Activating ESP-IDF environment for Windows at build.ps1")
    elif platform.system() == "Darwin":  # macOS
        # os.environ["IDF_PATH"] = esp_idf_path["Darwin"]
        # os.environ["PATH"] = f"{os.environ['PATH']}:{os.path.join(esp_idf_path['Darwin'], 'tools')}"
        print(f"Activating ESP-IDF environment for Mac at build.sh")

def build_project(target, build_dir, project_path, esp_idf_path):
    """编译项目"""
    print(f"Compiling {target} project...")
    # 删除旧的 sdkconfig 文件
    sdkconfig_path = os.path.join(project_path, "sdkconfig")
    if os.path.exists(sdkconfig_path):
        os.remove(sdkconfig_path)
    
    # 根据目标芯片复制默认的 sdkconfig 文件
    sdkconfig_defaults_path = os.path.join(project_path, f"sdkconfig.defaults.{target}")
    if not os.path.exists(sdkconfig_defaults_path):
        print(f"Error: sdkconfig.defaults.{target} not found in {project_path}")
        exit(1)
    shutil.copy(sdkconfig_defaults_path, sdkconfig_path)
    
    # 激活 ESP-IDF 环境
    activate_espidf_env(esp_idf_path)
    
    # 运行编译命令
    build_command = f"idf.py -B {build_dir} -DIDF_TARGET={target} build"
    run_command(build_command, cwd=project_path)
    print(f"{target} compilation completed successfully. Output in {build_dir}")

def main():
    parser = argparse.ArgumentParser(description="Build ESP-IDF projects for different targets.")
    parser.add_argument("--esp32s3", action="store_true", help="Build for ESP32-S3")
    parser.add_argument("--esp32c3", action="store_true", help="Build for ESP32-C3")
    parser.add_argument("--all", action="store_true", help="Build for both ESP32-S3 and ESP32-C3 (default)")
    
    args = parser.parse_args()
    
    # 设置默认行为为 --all
    if not (args.esp32s3 or args.esp32c3 or args.all):
        args.all = True
    
    # 设置路径
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_path = script_dir

    # # 删除旧的 out 目录
    # out_dir = os.path.join(script_dir, "out")
    # if os.path.exists(out_dir):
    #     shutil.rmtree(out_dir)
    #     os.makedirs(out_dir)
    
    try:
        if args.all or args.esp32s3:
            build_dir = os.path.join(script_dir, "out", "esp32s3")
            build_project("esp32s3", build_dir, project_path, esp_idf_path)
        
        if args.all or args.esp32c3:
            build_dir = os.path.join(script_dir, "out", "esp32c3")
            build_project("esp32c3", build_dir, project_path, esp_idf_path)
    except Exception as e:
        print(f"Error: {e}")
        exit(1)

if __name__ == "__main__":
    main()