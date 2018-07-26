#!/bin/bash

## --------------------------------------------------------
# 引入其他公共函数，因为最终会被引入主shell中使用，因此SHELL_COMMON_DIR需要指定
SHELL_COMMON_DIR="${MKHOME}/sbin/shell_common"

source ${SHELL_COMMON_DIR}/shell_common_algorithm.sh

## --------------------------------------------------------



echo
echo "==========================================================================="

echo "test 192. Word Frequency ..."
echo

countWordFrequencyRes=`fun_countWordFrequency ".././make_all.cfg" " "`
echo -n $countWordFrequencyRes | tr ',' '\n'

echo "==========================================================================="
echo


echo
echo "==========================================================================="

echo "test 194. Transpose File ..."
echo

transposeFileRes=`fun_transposeFile ".././make_all.cfg" " "`
echo -n $transposeFileRes | tr ',' '\n'

echo "==========================================================================="
echo