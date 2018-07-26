/*
 * test_shm_speech.cpp
 *
 *  Created on: 2018年6月10日
 *      Author: yaoqiaobing
 */

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include "Shm/tts/ShmTextToSpeech.hpp"
#include "lib_acl.hpp"
#include "acl_cpp_init.hpp"
#include "MrcpClient/MrcpClient.hpp"
#include "Util/StringUtil.hpp"

using namespace acl;
using namespace std;

int g_shm_id = 0;
void* g_shm_addr = NULL;

int try_shm(key_t key) {
	if ((g_shm_id = shmget(key, 0, 0660)) == -1) {
		/*NCSLOG("shmget(%d %d): %s.\n", cfg.ipc_key, shm_size, ERRSTR);*/
		return 0;
	}
	return 1;
}

int at_shm(key_t key) {
	if ((g_shm_id = shmget(key, 0, 0660)) == -1) {
		/*NCSLOG("shmget(%d %d): %s.\n", cfg.ipc_key, shm_size, ERRSTR);*/
		return -1;
	}
	if ((g_shm_addr = shmat(g_shm_id, NULL, 0)) == (void *) -1) {
		/*NCSLOG("shmat(%d): %s.\n", shm_id, ERRSTR);*/
		return -1;
	}
	return 0;
}

int dt_shm() {
	int ret;
	ret = shmdt(g_shm_addr);
	if (ret != -1)
		g_shm_addr = NULL;
	return ret;
}

int rm_shm() {
	int ret;
	ret = shmctl(g_shm_id, IPC_RMID, NULL);
	if (ret != -1)
		g_shm_id = -1;
	return ret;
}

int get_shm_natt() {
	struct shmid_ds sds;
	if (shmctl(g_shm_id, IPC_STAT, &sds) == -1) {
		return -1;
	}
	return (int) (sds.shm_nattch);
}

 // g++ test_shm_speech.cpp -o test_shm_speech -I${MKHOME}/include -L${MKHOME}/lib ${ACLINC} ${ACLLD} -lShm -lUtil -lacl_all -lz -lpthread -ldl
int main(int argc, char* argv[]) {

	// 1.初始化ACL库(尤其是在WIN32下一定要调用此函数，在UNIX平台下可不调用)
	acl_cpp_init();

	// 2.设置日志
	CfgUtil _cfgUtil("SpeechProcess.cfg");

	// 3.初始化文本转语音共享内存
	ShmTextToSpeech _shmTextToSpeech;
	int shmTextToSpeechKey = _cfgUtil.getShmTextToSpeechKey();
	int shmTextToSpeechNum = _cfgUtil.getShmTextToSpeechNum();
	// 告知_shmTextToSpeech共享内存的key值及语音条目数
	_shmTextToSpeech.init(shmTextToSpeechKey, shmTextToSpeechNum);
	printf("init shm:[text to speech], key:[%d], num:[%d]", shmTextToSpeechKey, shmTextToSpeechNum);

	// 4.将共享内存加载到本进程中
	ShmTextToSpeechHead* shmTextToSpeechHead = NULL;
	if (_shmTextToSpeech.procLoadShm((void**) &shmTextToSpeechHead) == FAILURE) {
		printf(_shmTextToSpeech.getShmLastError().c_str());
		// return FAILURE;
	}
	printf("success to add shmTextToSpeech\n");

	printf(
			"shm:[text to speech], total:[%d], free:[%d], pointIndex:[%d], offset:[%d], shmSize:[%d], itemSize:[%d]\n",
			shmTextToSpeechHead->_total, shmTextToSpeechHead->_free,
			shmTextToSpeechHead->_pointIndex, shmTextToSpeechHead->_offset,
			shmTextToSpeechHead->_shmSize, shmTextToSpeechHead->_itemSize);

	return 0;
}








