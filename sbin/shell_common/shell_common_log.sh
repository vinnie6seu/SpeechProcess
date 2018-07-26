#!/bin/bash  

## --------------------------------------------------------
# ʹ�ýű����Ƶ�������������ֹ�ű����ظ����������ļ�������C/C++�б���ͷ�ļ����������

_shell_common_log_="__shell_common_log_$$__"

if [ -z "${!_shell_common_log_}" ]; then
    eval "$_shell_common_log_=1"
else
    return 0
fi

## --------------------------------------------------------

## --------------------------------------------------------
# ��������������������Ϊ���ջᱻ������shell��ʹ�ã����SHELL_COMMON_DIR��Ҫָ��
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
