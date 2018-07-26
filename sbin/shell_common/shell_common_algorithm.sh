#!/bin/bash

## --------------------------------------------------------
# ʹ�ýű����Ƶ�������������ֹ�ű����ظ����������ļ�������C/C++�б���ͷ�ļ����������

_shell_algorithm_cfg_="__shell_algorithm_cfg_$$__"

if [ -z "${!_shell_algorithm_cfg_}" ]; then
    eval "$_shell_algorithm_cfg_=1"
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
## LeetCode
## 192. Word Frequency
## ���ܣ�ͳ���ļ��еĵ��ʳ��ָ���
## ���أ��ַ�����"word" "frequency","word" "frequency",.........
##
function fun_countWordFrequency() {
    fileName="$1"    
    delim="$2"
    
    cat $fileName | tr -s "$delim" '\n' | sort | uniq -c | sort -rn | awk '{print $2,$1}' | tr '\n' ','
}

# countWordFrequencyRes=`fun_countWordFrequency "words.txt" " "`
# echo -n $countWordFrequencyRes | tr ',' '\n'


##
## LeetCode
## 194. Transpose File
## ���ܣ�ת���ı�����
## ���أ�����Ԫ��ÿ����","�ָ���!!!!���´������Ǳ����ڴ���������Ľ�!!!!
##
function fun_transposeFile() {
    fileName="$1"
    delim="$2"
    
    # declare -a arr=()
    
    # -aָ���ǽ������ݷֳ��������line��
    while read line; do
        lineArr=(`echo -n ${line} | tr "$delim" ' '`)
        
        # echo -n "${lineArr[@]}"
        # echo "${#lineArr[@]}"

        for ((i = 0; i < "${#lineArr[@]}"; ++i)); do
            if [ -z "${arr[$i]}" ]; then
                arr[$i]="${lineArr[$i]}"
            else
                arr[$i]="${arr[$i]} ${lineArr[$i]}"
            fi
        done
    done < $fileName

    res=""
    for ((i = 0; i < ${#arr[@]}; ++i)); do
        res="${res}""${arr[i]}"","
    done   
    
    echo $res
}

# transposeFileRes=`fun_transposeFile "words.txt" " "`
# echo -n $transposeFileRes | tr ',' '\n'

