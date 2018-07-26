#!/bin/bash  

## --------------------------------------------------------
# 使用脚本名称当作变量名，防止脚本被重复引入其他文件，类似C/C++中避免头文件被多次引用

_shell_common_mysql_="_shell_common_mysql_$$__"

if [ -z "${!_shell_common_mysql_}" ]; then
    eval "$_shell_common_mysql_=1"
else
    return 0
fi

## --------------------------------------------------------

## --------------------------------------------------------
# 引入其他公共函数，因为最终会被引入主shell中使用，因此SHELL_COMMON_DIR需要指定
SHELL_COMMON_DIR="${MKHOME}/sbin/shell_common"

source ${SHELL_COMMON_DIR}/shell_common_log.sh

## --------------------------------------------------------

MYSQL_CONN=""

function fun_mysqlConnectInit() {
	# 数据库连接信息
	IP=172.21.33.125
	PORT=60010
	USER=cup_dba
	DBNAME=CUBLWDB
	PASSWD=OStem@00	
	
    MYSQL_CONN="mysql -h${IP} -P${PORT} -u${USER} -p${PASSWD} -D${DBNAME} -A -N -s -e"
}

function fun_mysqlOperate() {
	if [ "${MYSQL_CONN}" = "" ]; then
		fun_logError "not init mysql connect"
		exit 1
	fi
	
	operate_sql="$1"
	if [ "${operate_sql}" = "" ]; then
		fun_logError "not give execute sql"
		exit 1
	fi
	
	fun_logInfo "need to execute is [${MYSQL_CONN} \"${operate_sql}\"]"
	
	sql_res=`${MYSQL_CONN} "${operate_sql}"`	
	if [ "$?" != "0" ]; then
		fun_logError "fail to execute sql:[${operate_sql}]"
		exit 1
	fi
	
	echo ${sql_res}
}


