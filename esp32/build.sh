#!/bin/bash

# 获取脚本的参数
arguments=("$@")
echo "args: ${arguments[@]}"

# 初始化变量
espIdfPath=""
projectPath=""
otherParams=()

# 遍历参数
for arg in "${arguments[@]}"; do
    if [[ $arg =~ ^--espIdfPath= ]]; then
        # 提取 espIdfPath 的值
        espIdfPath="${arg#*=}"
    elif [[ $arg =~ ^--projectPath= ]]; then
        # 提取 projectPath 的值
        projectPath="${arg#*=}"
    else
        # 将其他参数添加到数组中
        otherParams+=("$arg")
    fi
done

# 检查 espIdfPath 是否为空，如果为空则使用默认值
if [ -z "$espIdfPath" ]; then
    espIdfPath="/Users/yunxin/project/esp/esp-idf"
fi

# 检查 projectPath 是否为空，如果为空则使用当前目录
if [ -z "$projectPath" ]; then
    projectPath="$(pwd)"
fi

# 打印参数值以调试
echo "Debug Information:"
echo "espIdfPath: $espIdfPath"
echo "projectPath: $projectPath"
echo "otherParams: ${otherParams[@]}"

# 设置环境变量
export IDF_PATH="$espIdfPath"
export PATH="$PATH:$IDF_PATH/tools"

# 激活 ESP-IDF 环境
cd $IDF_PATH
. ./export.sh
cd -

# 构造 Python 命令
pythonCommand="python $projectPath/build.py"
for arg in "${otherParams[@]}"; do
    pythonCommand+=" $arg"
done

# 打印构造的 Python 命令以调试
echo "Executing command: $pythonCommand"

# 运行 Python 编译脚本并捕获输出和退出状态码
python $projectPath/build.py "${otherParams[@]}"
exitCode=$?

# 打印退出状态码
echo "Exit code: $exitCode"

# 退出脚本并返回状态码
exit $exitCode