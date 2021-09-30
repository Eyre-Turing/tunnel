#!/bin/bash

# 显示用户配置框
input_dialog() {
	dialog --backtitle '代理服务器' --title '控制界面' --form '配置' 0 0 4 \
	'客户模拟器端口' 1 1 "$1" 1 20 20 0 \
	'用户连接端口' 2 1 "$2" 2 20 20 0 \
	'管理端口' 3 1 "$3" 3 20 20 0 \
	'服务启动标题' 4 1 "$4" 4 20 20 0 \
	2>"$5"
	return 0
}

# 检查是否为非负整数
check_uint() {
	local tmp=$(echo -n "$1" | grep -E '^[0-9]+$')
	if [ -z "$tmp" ]; then
		dialog --backtitle '代理服务器' --title '错误' --msgbox "参数错误：$2必须为非负整数" 0 0
		return 1
	fi
	return 0
}

# 检查数字范围是否1~65535
check_range() {
	if [ "$1" -gt 65535 ] || [ "$1" -lt 1 ]; then
		dialog --backtitle '代理服务器' --title '错误' --msgbox "参数错误：$2必须为1~65535" 0 0
		return 1
	fi
	return 0
}

main() {
	# 配置文件名
	local ini="config.ini"

	# 缓存配置文件路径
	local outini="$(mktemp)"

	# 启动标题
	local title=""

	while :; do
		# 读取原有配置文件
		local client=$(cat "$ini" | grep 'client' | awk -F 'client=' '{print $2}')
		client=${client//$'\r'/}
		local user=$(cat "$ini" | grep 'user' | awk -F 'user=' '{print $2}')
		user=${user//$'\r'/}
		local manager=$(cat "$ini" | grep 'manager' | awk -F 'manager=' '{print $2}')
		manager=${manager//$'\r'/}
		title=$(cat "$ini" | grep 'title' | awk -F 'title=' '{print $2}')
		title=${title//$'\r'/}
		
		# 用户输入
		input_dialog "$client" "$user" "$manager" "$title" "$outini"

		# 读取用户输入写出的配置文件
		local outdata=$(cat "$outini")
		
		# 用户是否取消
		if [ -z "$outdata" ]; then
			echo "你取消了启动服务"
			return 0
		fi

		# 读入参数并判断合法性
		client=$(echo "$outdata" | tail -n +1 | head -n 1)
		user=$(echo "$outdata" | tail -n +2 | head -n 1)
		manager=$(echo "$outdata" | tail -n +3 | head -n 1)
		title=$(echo "$outdata" | tail -n +4 | head -n 1)

		# 判断是否为非负整数
		if ! check_uint "$client" "客户模拟器端口"; then
			continue
		fi
		if ! check_uint "$user" "用户连接端口"; then
			continue
		fi
		if ! check_uint "$manager" "管理端口"; then
			continue
		fi
		
		# 判断参数范围
		if ! check_range "$client" "客户模拟器端口"; then
			continue
		fi
		if ! check_range "$user" "用户连接端口"; then
			continue
		fi
		if ! check_range "$manager" "管理端口"; then
			continue
		fi
		
		# 修改配置
		echo "[listen]" >"$ini"
		echo "client=${client}" >>"$ini"
		echo "user=${user}" >>"$ini"
		echo "manager=${manager}" >>"$ini"
		echo "title=${title}" >>"$ini"
		
		break
	done

	# 删除缓存配置文件
	rm -f "$outini"
	
	# 停掉之前的服务
	if { exec 8<>/dev/tcp/127.0.0.1/${manager}; } 2>/dev/null; then
		echo "停止之前的服务"
		echo "q" >&8
		exec 8<&-
		exec 8>&-
	fi

	# 启动服务
	nohup ./server --title "${title}" &>/dev/null &
	
	echo "服务启动"
	
	return 0
}

main
