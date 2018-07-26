#!/bin/bash

## --------------------------------------------------------
# 引入其他公共函数，因为最终会被引入主shell中使用，因此SHELL_COMMON_DIR需要指定
SHELL_COMMON_DIR="${MKHOME}/sbin/shell_common"

source ${SHELL_COMMON_DIR}/shell_common_cfg.sh

## --------------------------------------------------------

MAKE_ALL_CFG="make_all.cfg"


REQUIRE="$1"
DIRNAME="$2"

cfg_all_make_dirs=`fun_listCfgValues ${MAKE_ALL_CFG} "MAKE_DIR"`
cfg_all_make_dirs=(${cfg_all_make_dirs})


if [ $# -lt 1 ]; then
    echo
    echo "==========================================================================="
    
    echo "    usage:"
    echo "    ==  编译${MAKE_ALL_CFG}中的所有目录          : sh make_all.sh -a"
    echo "    ==  清理${MAKE_ALL_CFG}中的所有目录          : sh make_all.sh -c"
    echo "    ==  先清理后编译${MAKE_ALL_CFG}中的所有目录  : sh make_all.sh -ca"
    echo "    ==  显示${MAKE_ALL_CFG}中的所有目录编译变量  : sh make_all.sh -s"
              
    echo "    ==  指定目录编译                             : sh make_all.sh -a dir_name"
    echo "    ==  指定目录清理                             : sh make_all.sh -c dir_name"
    echo "    ==  指定目录先清理后编译                     : sh make_all.sh -ca dir_name"
    echo "    ==  指定目录显示编译变量                     : sh make_all.sh -s dir_name"
    
    echo "==========================================================================="
    echo    
    
    exit 1
fi


echo
echo "==========================================================================="

## 1.确定操作的目录
all_make_dirs=(${cfg_all_make_dirs[@]})

# 检查指定的目录是否在配置文件中已经添加的
if [ "${DIRNAME}" != "" ]; then
    flag="false"
    for subDir in ${cfg_all_make_dirs[@]}; do
        if [ "${subDir}" = "${DIRNAME}" ]; then
        	  flag="true"
            break	
        fi
    done
    
    if [ "${flag}" = "false" ]; then
    	  echo "配置文件中没有输入的目录[${DIRNAME}]"
    	  exit 1
    fi  
    
    all_make_dirs=(${DIRNAME})
fi


# echo ${#cfg_all_make_dirs[@]} ${cfg_all_make_dirs[@]}
# echo ${#all_make_dirs[@]} ${all_make_dirs[@]}  


## 2.确定操作的动作
TARGET=("all")
TARGET_LOG=("编译")
case "${REQUIRE}" in
    "-a")
        TARGET=("all")
        TARGET_LOG=("编译")
        ;;
    "-c")
        TARGET=("clean")
        TARGET_LOG=("清理")
        ;;
    "-ca")
        TARGET=("clean" "all")
        TARGET_LOG=("清理" "编译")
        ;;
    "-s")
        TARGET=("show")
        TARGET_LOG=("显示编译变量")
        ;;
    *)
        #其它输入
        echo "未能识别参数[${REQUIRE}]"
        ;;
esac

# echo ${#TARGET[@]}

## 3.工作内容
echo "${TARGET_LOG[@]} ---- ${all_make_dirs[@]}"
echo "==========================================================================="
echo

## 4.开始工作
for subDir in ${all_make_dirs[@]}; do
    for(( index=0;index<${#TARGET[@]};index++)); do
        echo "${TARGET_LOG[${index}]} [${subDir}] ......"
        make ${TARGET[${index}]} -C ${subDir}
        
        if [ $? -ne 0 ]; then
            echo "${TARGET_LOG[${index}]} [${subDir}] 出错了......"
            echo
            exit 1
        fi        
        echo        
    done
done

echo "==========================================================================="
echo