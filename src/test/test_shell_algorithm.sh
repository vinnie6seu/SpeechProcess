#!/bin/bash

## --------------------------------------------------------
# ��������������������Ϊ���ջᱻ������shell��ʹ�ã����SHELL_COMMON_DIR��Ҫָ��
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