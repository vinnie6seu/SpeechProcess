#!/bin/bash

## --------------------------------------------------------
# ��������������������Ϊ���ջᱻ������shell��ʹ�ã����SHELL_COMMON_DIR��Ҫָ��
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
    echo "    ==  ����${MAKE_ALL_CFG}�е�����Ŀ¼          : sh make_all.sh -a"
    echo "    ==  ����${MAKE_ALL_CFG}�е�����Ŀ¼          : sh make_all.sh -c"
    echo "    ==  ����������${MAKE_ALL_CFG}�е�����Ŀ¼  : sh make_all.sh -ca"
    echo "    ==  ��ʾ${MAKE_ALL_CFG}�е�����Ŀ¼�������  : sh make_all.sh -s"
              
    echo "    ==  ָ��Ŀ¼����                             : sh make_all.sh -a dir_name"
    echo "    ==  ָ��Ŀ¼����                             : sh make_all.sh -c dir_name"
    echo "    ==  ָ��Ŀ¼����������                     : sh make_all.sh -ca dir_name"
    echo "    ==  ָ��Ŀ¼��ʾ�������                     : sh make_all.sh -s dir_name"
    
    echo "==========================================================================="
    echo    
    
    exit 1
fi


echo
echo "==========================================================================="

## 1.ȷ��������Ŀ¼
all_make_dirs=(${cfg_all_make_dirs[@]})

# ���ָ����Ŀ¼�Ƿ��������ļ����Ѿ���ӵ�
if [ "${DIRNAME}" != "" ]; then
    flag="false"
    for subDir in ${cfg_all_make_dirs[@]}; do
        if [ "${subDir}" = "${DIRNAME}" ]; then
        	  flag="true"
            break	
        fi
    done
    
    if [ "${flag}" = "false" ]; then
    	  echo "�����ļ���û�������Ŀ¼[${DIRNAME}]"
    	  exit 1
    fi  
    
    all_make_dirs=(${DIRNAME})
fi


# echo ${#cfg_all_make_dirs[@]} ${cfg_all_make_dirs[@]}
# echo ${#all_make_dirs[@]} ${all_make_dirs[@]}  


## 2.ȷ�������Ķ���
TARGET=("all")
TARGET_LOG=("����")
case "${REQUIRE}" in
    "-a")
        TARGET=("all")
        TARGET_LOG=("����")
        ;;
    "-c")
        TARGET=("clean")
        TARGET_LOG=("����")
        ;;
    "-ca")
        TARGET=("clean" "all")
        TARGET_LOG=("����" "����")
        ;;
    "-s")
        TARGET=("show")
        TARGET_LOG=("��ʾ�������")
        ;;
    *)
        #��������
        echo "δ��ʶ�����[${REQUIRE}]"
        ;;
esac

# echo ${#TARGET[@]}

## 3.��������
echo "${TARGET_LOG[@]} ---- ${all_make_dirs[@]}"
echo "==========================================================================="
echo

## 4.��ʼ����
for subDir in ${all_make_dirs[@]}; do
    for(( index=0;index<${#TARGET[@]};index++)); do
        echo "${TARGET_LOG[${index}]} [${subDir}] ......"
        make ${TARGET[${index}]} -C ${subDir}
        
        if [ $? -ne 0 ]; then
            echo "${TARGET_LOG[${index}]} [${subDir}] ������......"
            echo
            exit 1
        fi        
        echo        
    done
done

echo "==========================================================================="
echo