#!/bin/bash

## --------------------------------------------------------
# 使用脚本名称当作变量名，防止脚本被重复引入其他文件，类似C/C++中避免头文件被多次引用

_shell_algorithm_cfg_="__shell_algorithm_cfg_$$__"

if [ -z "${!_shell_algorithm_cfg_}" ]; then
    eval "$_shell_algorithm_cfg_=1"
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
## LeetCode
## 192. Word Frequency
## 功能：统计文件中的单词出现个数
## 返回：字符串是"word" "frequency","word" "frequency",.........
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
## 功能：转置文本内容
## 返回：数组元素每行用","分隔，!!!!如下代码总是报错内存溢出，待改进!!!!
##
function fun_transposeFile() {
    fileName="$1"
    delim="$2"
    
    # declare -a arr=()
    
    # -a指的是将行内容分成数组读到line中
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

