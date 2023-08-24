#!/bin/sh
# 后缀 .app 格式转 .ipa 格式文件
# 脚本目录和xxx.app要在同一个目录，如果想要配置，请填入包含后缀名的全称
g_app_name=""

# 在当前目录下找到 .app 文件
AutoFindAppFolder() {
	for p_file in ./*; do
		#判断是否为文件夹
		if [ -d "$p_file" ]; then
			file_name=$(basename "$p_file")
			#判断后缀名称是否是app
			file_last=${file_name##*.}
            if [ "$file_last" = "app" ]; then
                g_app_name="$file_name"
            fi
		fi
	done
}

# 读取用户输入
ReadUserSelectPara() {
	if [ -z "$g_app_name" ]; then
	    # 文件名称为空
	    read -r input_name
		sleep 0.1
		g_app_name=$input_name
		ReadUserSelectPara
	else
	    # 文件名称有效
	    echo "*** 当前.app 文件:$g_app_name ***"
	fi
}

# 打包ipa
AppToIpa() {
    app_file_name=$1
    # 先删除里面当前的IPAFolder文件夹
    rm -rf IPAFolder
    # 再创建IPAFolder文件夹
    mkdir IPAFolder
    # 在文件夹里面创建Payload文件夹
    mkdir IPAFolder/Payload
    # 将当前目录下的App_Name.app复制到Payload里面
    cp -r "$app_file_name" IPAFolder/Payload/"$app_file_name"
    # 进入IPAFolder文件夹
    cd IPAFolder
    # 压缩多个目录zip FileName.zip 目录1 目录2 目录3....
    file_head=${app_file_name%.*}
    zip -r "$file_head.ipa" Payload
    echo "*** ipa 文件已经创建到 IPAFolder 文件夹下 ***"
}

# 脚本主函数
Main() {
    # 如果未设置，则自动获取一次
    if [ -z "$g_app_name" ]; then
        AutoFindAppFolder
    fi
    # 如果自动获取仍旧为空，则要求用户输入
    if [ -z "$g_app_name" ]; then
        echo "*** 请输入.app全名称，包含后缀名 ***"
        ReadUserSelectPara
    else
        # 转换
        AppToIpa "$g_app_name"
    fi
}

cd "$(dirname "$0")" || exit 0

# 判断有无传递参数，Python脚本中传递有参数
if [ -z "$1" ]; then
    Main
fi
