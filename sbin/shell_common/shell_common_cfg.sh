#!/bin/bash

## --------------------------------------------------------
# ʹ�ýű����Ƶ�������������ֹ�ű����ظ����������ļ�������C/C++�б���ͷ�ļ����������

_shell_common_cfg_="__shell_common_cfg_$$__"

if [ -z "${!_shell_common_cfg_}" ]; then
    eval "$_shell_common_cfg_=1"
else
    return 0
fi

## --------------------------------------------------------

## --------------------------------------------------------
# ��������������������Ϊ���ջᱻ������shell��ʹ�ã����SHELL_COMMON_DIR��Ҫָ��
SHELL_COMMON_DIR="${MKHOME}/sbin/shell_common"

source ${SHELL_COMMON_DIR}/shell_common_log.sh

## --------------------------------------------------------

##
## �õ�cfg�����ļ����е�sections����
## cfg�ж��section�ÿ��и���
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
## �õ�cfg�����ļ�����section������keyֵ
## cfg�ж��key�ÿ��и���
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
## �õ�cfg�����ļ�����section������valueֵ
## cfg�ж��value�ÿ��и���
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
## �õ�cfg�����ļ�����section������key=valueֵ
## cfg�ж��key=value�ÿ��и���
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
## �õ�cfg�����ļ�����section����key��valueֵ
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

## ����cfg�ļ��Ķ�ȡ
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

