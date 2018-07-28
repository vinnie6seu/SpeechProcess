/*
 * test_blockqueue.cpp
 *
 *  Created on: 2018年7月28日
 *      Author: yaoqiaobing
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctime>

#include "lib_acl.hpp"
#include "acl_cpp_init.hpp"
#include "MrcpClient/MrcpClient.hpp"
#include "Util/StringUtil.hpp"

// 注意gson.h需要放在最后不然会编译报错
#include "Entity/HttpJson.h"
#include "Entity/SocketJson.h"
#include "Entity/gson.h"

using namespace acl;
using namespace std;

// g++ test_blockqueue.cpp -o test_blockqueue -I${MKHOME}/include -L${MKHOME}/lib ${ACLINC} ${ACLLD} -lShm -lUtil -lacl_all -lz -lpthread -ldl
int main() {

	int _id = 999;
	char text[1024 + 1] = {'\0'};
	vector<AsrSpeechTransResult> asr_trans_result_vec;

	// 0.初始化 mrcp 客户端
	MrcpClient& _mrcpClient = acl::singleton2<MrcpClient>::get_instance();

	printf("id:[%d] status:[MSP_AUDIO_LAST] start to receive text packet", _id);

	asr_trans_result_vec.clear();
	if (_mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_LAST) == FAILURE) {
		printf("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", _id);
		return FAILURE;
	}
	printf("id:[%d] status:[MSP_AUDIO_LAST] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

	for (int i = 0; i < asr_trans_result_vec.size(); i++) {
		printf("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
				asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
				asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
				asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
				asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);

		if (asr_trans_result_vec[i]._cur_result._is_final == true) {
			memcpy(text, asr_trans_result_vec[i]._cur_result._transcript, strlen(asr_trans_result_vec[i]._cur_result._transcript));
		}
	}

	printf("text is:[%s]", text);

	return 0;
}


