#!/bin/bash  

## --------------------------------------------------------
# 使用脚本名称当作变量名，防止脚本被重复引入其他文件，类似C/C++中避免头文件被多次引用

_shell_common_log_="__shell_common_log_$$__"

if [ -z "${!_shell_common_log_}" ]; then
    eval "$_shell_common_log_=1"
else
    return 0
fi

## --------------------------------------------------------

## --------------------------------------------------------
# 引入其他公共函数，因为最终会被引入主shell中使用，因此SHELL_COMMON_DIR需要指定
SHELL_COMMON_DIR="${MKHOME}/sbin/shell_common"


## --------------------------------------------------------

function fun_logInfo() {  
	cur_time=`date "+%Y%m%d %H%M%S"`
	caller_line_no=`caller 0 | awk '{print$1}'`
    echo -e "${cur_time}|\033[4m\033[1m\033[33mINFO \033[0m\033[33m\033[0m|${0}|${caller_line_no} : ${1}"
}  

function fun_logWarn() {  
	cur_time=`date "+%Y%m%d %H%M%S"`
	caller_line_no=`caller 0 | awk '{print$1}'`
    echo -e "${cur_time}|\033[4m\033[1m\033[33mWARN \033[0m\033[33m\033[0m|${0}|${caller_line_no} : ${1}"
}  
 
function fun_logError() {  
	cur_time=`date "+%Y%m%d %H%M%S"`
	caller_line_no=`caller 0 | awk '{print$1}'`
    echo -e "${cur_time}|\033[4m\033[1m\033[33mERROR\033[0m\033[33m\033[0m|${0}|${caller_line_no} : ${1}"
}
