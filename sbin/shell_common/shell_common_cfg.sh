#!/bin/bash

## --------------------------------------------------------
# 使用脚本名称当作变量名，防止脚本被重复引入其他文件，类似C/C++中避免头文件被多次引用

_shell_common_cfg_="__shell_common_cfg_$$__"

if [ -z "${!_shell_common_cfg_}" ]; then
    eval "$_shell_common_cfg_=1"
else
    return 0
fi

## --------------------------------------------------------

## --------------------------------------------------------
# 引入其他公共函数，因为最终会被引入主shell中使用，因此SHELL_COMMON_DIR需要指定
SHELL_COMMON_DIR="${MKHOME}/sbin/shell_common"

source ${SHELL_COMMON_DIR}/shell_common_log.sh

## --------------------------------------------------------

##
## 得到cfg配置文件所有的sections名称
## cfg中多个section用空行隔开
## fun_listCfgSections "filename.cfg"
##
function fun_listCfgSections() {
    cfgfile="$1"
    # echo "cfgfile:${cfgfile}"
    # # exit 1
    if [ $# -ne 1 ] || [ ! -f ${cfgfile} ]
    then
        fun_logError "use para error or file [${cfgfile}] not exist!"
        exit 1 
    else
        sections=`sed -n '/\[*\]/p' ${cfgfile}  |grep -v '^#'|tr -d []`
        echo  "${sections}"
    fi
}

##
## 得到cfg配置文件给定section的所有key值
## cfg中多个key用空行隔开
## fun_listCfgKeys "filename.cfg" "section"
## 
function fun_listCfgKeys() {
    cfgfile="$1"
    section="$2"
    if [ $# -ne 2 ] || [ ! -f ${cfgfile} ]
    then
        fun_logError "use para error or file [${cfgfile}] not exist!"
        exit 1 
    else
        keys=$(sed -n '/\['$section'\]/,/^$/p' $cfgfile|grep -Ev '\[|\]|^$'|awk -F'=' '{print $1}')
        echo ${keys}
    fi
}

##
## 得到cfg配置文件给定section的所有value值
## cfg中多个value用空行隔开
## fun_listCfgValues "filename.cfg" "section"
## 
function fun_listCfgValues() {
    cfgfile="$1"
    section="$2"
    if [ $# -ne 2 ] || [ ! -f ${cfgfile} ]
    then
        fun_logError "use para error or file [${cfgfile}] not exist!"
        exit 1 
    else
        values=$(sed -n '/\['$section'\]/,/^$/p' $cfgfile|grep -Ev '\[|\]|^$'|awk -F'=' '{print $2}')
        echo ${values}
    fi
}

## 
## 得到cfg配置文件给定section的所有key=value值
## cfg中多个key=value用空行隔开
## fun_listCfgKeyValue "filename.cfg" "section"
## 
function fun_listCfgKeysValues() {
    cfgfile="$1"
    section="$2"
    if [ $# -ne 2 ] || [ ! -f ${cfgfile} ]
    then
        fun_logError "use para error or file [${cfgfile}] not exist!"
        exit 1 
    else
        values=$(sed -n '/\['$section'\]/,/^$/p' $cfgfile|grep -Ev '\[|\]|^$'|awk -F'=' '{print $1"="$2}')
        echo ${values}
    fi
}

## 
## 得到cfg配置文件给定section给定key的value值
## fun_listCfgKeyValue "filename.cfg" "section" "key"
## 
function fun_listCfgOneKeyValue() {
    INIFILE=$1
    SECTION=$2
    ITEM=$3
    
	if [ $# -ne 3 ] || [ ! -f ${INIFILE} ]
    then
        fun_logError "use para error or file [${cfgfile}] not exist!"
        exit 1 
    fi
    
    _readIni=`awk -F '=' '/\['$SECTION'\]/{a=1}a==1&&$1~/'$ITEM'/{print $2;exit 1}' $INIFILE`
    echo ${_readIni}
}

## 测试cfg文件的读取
## all_sections=`fun_listCfgSections ${MAKE_ALL_CFG}`
## echo ${all_sections}
## 
## all_keys=`fun_listCfgKeys ${MAKE_ALL_CFG} "MAKE_DIR"`
## echo ${all_keys}
## 
## all_values=`fun_listCfgValues ${MAKE_ALL_CFG} "MAKE_DIR"`
## echo ${all_values}
## 
## all_keys_values=`fun_listCfgKeysValues ${MAKE_ALL_CFG} "MAKE_DIR"`
## echo ${all_keys_values}
## 
## one_value=`fun_listCfgOneKeyValue ${MAKE_ALL_CFG} "MAKE_DIR" "DIR2"`
## echo ${one_value}

