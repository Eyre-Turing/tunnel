#!/bin/bash

# 开始菜单
menu() {
	if [ "$1" == 1 ]; then
		dialog --backtitle '代理服务器' --title '控制界面' --ok-label '选择' --cancel-label '退出控制界面' --menu '您是想' 0 0 3 'restart' '重启' 'stop' '关闭' 'settings' '业务配置' 2>"$2"
	else
		dialog --backtitle '代理服务器' --title '控制界面' --ok-label '选择' --cancel-label '退出控制界面' --menu '您是想' 0 0 3 'start' '启动' 'settings' '业务配置' 'manage' '管理配置' 2>"$2"
	fi
	
	return 0
}

# 显示用户业务配置框
input_pro() {
	dialog --backtitle '代理服务器' --title '控制界面' --form '配置' 0 0 4 \
	'客户模拟器端口' 1 1 "$1" 1 20 20 0 \
	'用户连接端口' 2 1 "$2" 2 20 20 0 \
	2>"$3"
	return 0
}

# 显示用户管理配置框
input_man() {
	dialog --backtitle '代理服务器' --title '控制界面' --form '管理' 0 0 4 \
	'管理端口' 1 1 "$1" 1 20 20 0 \
	'服务启动标题' 2 1 "$2" 2 20 20 0 \
	2>"$3"
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

	# 先前该服务是否存在
	local service_exist=0
	
	local sockfd=-1
	
	local choice
	local outdata
	
	local manager=$(cat "$ini" | grep 'manager' | awk -F 'manager=' '{print $2}')
	manager=${manager//$'\r'/}
	
	# 确认服务是否在运行
	sockfd=$(($(echo $(ls /proc/self/fd | sort) | awk '{print $NF}')+1))
	if { eval "exec ${sockfd}<>/dev/tcp/127.0.0.1/${manager}"; } 2>/dev/null; then
		service_exist=1
	else
		service_exist=0
	fi

	while :; do
		# 读取原有配置文件
		local client=$(cat "$ini" | grep 'client' | awk -F 'client=' '{print $2}')
		client=${client//$'\r'/}
		local user=$(cat "$ini" | grep 'user' | awk -F 'user=' '{print $2}')
		user=${user//$'\r'/}
		manager=$(cat "$ini" | grep 'manager' | awk -F 'manager=' '{print $2}')	# 需要刷新，因为用户可能会更新管理端口
		manager=${manager//$'\r'/}
		local title=$(cat "$ini" | grep 'title' | awk -F 'title=' '{print $2}')
		title=${title//$'\r'/}
		
		# 开始
		menu "$service_exist" "$outini"
		
		choice=$(cat "$outini")
		
		case $choice in
			'restart')
				eval "echo \"q\" >&${sockfd}"
				eval "cat <&${sockfd}"	# 等待服务关闭
				
				nohup ./server --title "${title}" &>/dev/null &
				;;
			'stop')
				eval "echo \"q\" >&${sockfd}"
				eval "cat <&${sockfd}"	# 等待服务关闭
				;;
			'settings')
				# 用户输入
				input_pro "$client" "$user" "$outini"

				# 读取用户输入写出的配置文件
				outdata=$(cat "$outini")
				
				# 用户是否取消
				if [ -z "$outdata" ]; then
					dialog --backtitle '代理服务器' --title '配置' --msgbox '取消了修改配置' 0 0
					continue
				fi

				# 读入参数并判断合法性
				client=$(echo "$outdata" | tail -n +1 | head -n 1)
				user=$(echo "$outdata" | tail -n +2 | head -n 1)

				# 判断是否为非负整数
				if ! check_uint "$client" "客户模拟器端口"; then
					continue
				fi
				if ! check_uint "$user" "用户连接端口"; then
					continue
				fi
				
				# 判断参数范围
				if ! check_range "$client" "客户模拟器端口"; then
					continue
				fi
				if ! check_range "$user" "用户连接端口"; then
					continue
				fi
				
				# 修改配置
				echo "[listen]" >"$ini"
				echo "client=${client}" >>"$ini"
				echo "user=${user}" >>"$ini"
				echo "manager=${manager}" >>"$ini"
				echo "title=${title}" >>"$ini"
				
				continue
				;;
			'start')
				nohup ./server --title "${title}" &>/dev/null &
				;;
			'manage')
				# 用户输入
				input_man "$manager" "$title" "$outini"

				# 读取用户输入写出的配置文件
				outdata=$(cat "$outini")
				
				# 用户是否取消
				if [ -z "$outdata" ]; then
					dialog --backtitle '代理服务器' --title '配置' --msgbox '取消了修改配置' 0 0
					continue
				fi

				# 读入参数并判断合法性
				manager=$(echo "$outdata" | tail -n +1 | head -n 1)
				title=$(echo "$outdata" | tail -n +2 | head -n 1)

				# 判断是否为非负整数
				if ! check_uint "$manager" "管理端口"; then
					continue
				fi
				
				# 判断参数范围
				if ! check_range "$manager" "管理端口"; then
					continue
				fi
				
				# 修改配置
				echo "[listen]" >"$ini"
				echo "client=${client}" >>"$ini"
				echo "user=${user}" >>"$ini"
				echo "manager=${manager}" >>"$ini"
				echo "title=${title}" >>"$ini"
				
				continue
				;;
			'')
				# 退出
				;;
			*)
				dialog --backtitle '代理服务器' --title '控制界面' --msgbox '找不到这个操作对应要执行的脚本' 0 0
				continue
				;;
		esac
		
		break
	done
	
	# 关闭套接字
	if [ "$service_exist" == 1 ]; then
		eval "exec ${sockfd}<&-"
		eval "exec ${sockfd}>&-"
		sockfd=-1
		service_exist=0
	fi

	# 删除缓存配置文件
	rm -f "$outini"
	
	return 0
}

if which dialog &>/dev/null; then
	main
else
	echo '您的环境目前不支持该控制界面，请试着安装dialog以支持该控制界面'
	echo 'RedHat可以使用'\`'yum install dialog'\''，Debian可以使用'\`'apt-get install dialog'\'
fi
