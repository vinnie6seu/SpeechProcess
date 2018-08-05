/*
 * MrcpClientWrapper.cpp
 *
 *  Created on: 2018年8月5日
 *      Author: yaoqiaobing
 */

#include "lib_acl.hpp"
#include "MrcpClient/MrcpClient.hpp"

/* ============================== 封装后提供给 C 使用 ============================== */

#ifdef __cplusplus
extern "C" {
#endif

struct MrcpClientStruct {
	MrcpClient* mrcpClient;
};

struct MrcpClientStruct* getInstance() {
	return (struct MrcpClientStruct*) malloc(sizeof(struct MrcpClientStruct));
}

/**
 * 初始化函数
 * 注意：在所有线程前调用该函数
 */
void asr_init(struct MrcpClientStruct* mrcpClientStruct) {
	// 0.初始化 mrcp 客户端
	MrcpClient& mrcpClient = acl::singleton2<MrcpClient>::get_instance();

	mrcpClient.init(CLIENT_TYPE_ASR_PACKET);

	mrcpClientStruct->mrcpClient = &mrcpClient;
}

/**
 * 发包函数
 */
FUN_STATUS asr_send_speech(struct MrcpClientStruct* mrcpClientStruct, int* id, const char* speech, int len, int num, ASR_SPEECH_PACK_STATUS_ENUM speechStatus) {

	MrcpClient* mrcpClient = mrcpClientStruct->mrcpClient;

	if (mrcpClient->asrSpeechPacketSend((*id), speech, len, num, speechStatus) != SUCCESS) {
		return FAILURE;
	}

	return SUCCESS;

}

/**
 * 获取结果函数
 * 注意：asrTransResult 空间是在本函数内分配的，因此使用完了必须注意释放对应空间
 */
FUN_STATUS asr_recv_trans_result(struct MrcpClientStruct* mrcpClientStruct, int* id, AsrSpeechTransResult** asrTransResult, int* len, ASR_SPEECH_PACK_STATUS_ENUM speechStatus) {

	MrcpClient* mrcpClient = mrcpClientStruct->mrcpClient;

	vector<AsrSpeechTransResult> asr_trans_result_vec;
	if (mrcpClient->asrTextBlockRecv((*id), asr_trans_result_vec, speechStatus) == FAILURE) {
		return FAILURE;
	}

	if (!asr_trans_result_vec.empty()) {
		*len = asr_trans_result_vec.size();

		*asrTransResult = (AsrSpeechTransResult*) malloc(sizeof(AsrSpeechTransResult) * asr_trans_result_vec.size());
	}

	return SUCCESS;

}

/**
 * 销毁/清理函数
 * 注意：检查到 FAILURE 后请调用该函数
 */
FUN_STATUS asr_term(struct MrcpClientStruct* mrcpClientStruct, int* id) {

	MrcpClient* mrcpClient = mrcpClientStruct->mrcpClient;

	if (mrcpClient->asrExceptionHandle((*id)) == FAILURE) {
		return FAILURE;
	}

	return SUCCESS;

}

#ifdef __cplusplus
}
#endif

/* ============================== 封装后提供给 C 使用 ============================== */
