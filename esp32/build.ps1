
# 获取脚本的参数
$arguments = $args
Write-Host "args: $args"

# 初始化变量
$espIdfPath = ""
$projectPath = ""
$otherParams = @()

# 遍历参数
foreach ($arg in $arguments) {
    if ($arg -match "^--espIdfPath=") {
        # 提取 espIdfPath 的值
        $espIdfPath = $arg -replace "^--espIdfPath=", ""
    } elseif ($arg -match "^--projectPath=") {
        # 提取 projectPath 的值
        $projectPath = $arg -replace "^--projectPath=", ""
    } else {
        # 将其他参数添加到数组中
        $otherParams += $arg
    }
}

# 检查 espIdfPath 是否为空，如果为空则使用默认值
if ([string]::IsNullOrEmpty($espIdfPath)) {
    $espIdfPath = "C:/Users/hzgaoqi1/esp/v5.4.1/esp-idf"
}
if ([string]::IsNullOrEmpty($projectPath)) {
    $projectPath = (Get-Location).Path
}

# 打印参数值以调试
Write-Host "Debug Information:"
Write-Host "espIdfPath: $espIdfPath"
Write-Host "projectPath: $projectPath"
Write-Host "otherParams: $otherParams"

# 设置环境变量
$env:IDF_PATH = $espIdfPath
$env:PATH = "$env:PATH;$env:IDF_PATH/tools"

# 激活 ESP-IDF 环境
. "$env:IDF_PATH/export.ps1"

# 构造 Python 命令
$pythonCommand = "python $projectPath/build.py"
foreach ($arg in $args) {
    $pythonCommand += " $arg"
}

# 打印构造的 Python 命令以调试
Write-Host "Executing command: $pythonCommand"

# 运行 Python 编译脚本并捕获输出和退出状态码
$process = Start-Process -FilePath "python" -ArgumentList "$projectPath/build.py", "$otherParams" -PassThru -NoNewWindow -Wait
$exitCode = $process.ExitCode

# 打印退出状态码
Write-Host "Exit code: $exitCode"

# 退出脚本并返回状态码
exit $exitCode