/*
 * MrcpClientWrapper.hpp
 *
 *  Created on: 2018年8月5日
 *      Author: yaoqiaobing
 */

#ifndef SRC_MRCPCLIENT_TEST_C_MRCPCLIENTWRAPPER_HPP_
#define SRC_MRCPCLIENT_TEST_C_MRCPCLIENTWRAPPER_HPP_

#include "MrcpClientPara.h"

/* ============================== 封装后提供给 C 使用 ============================== */

struct MrcpClientStruct;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 初始化函数
 * 注意：在所有线程前调用该函数
 */
void asr_init(struct MrcpClientStruct* mrcpClientStruct);

/**
 * 发包函数
 */
FUN_STATUS asr_send_speech(struct MrcpClientStruct* mrcpClientStruct, int* id, const char* speech, int len, int num, ASR_SPEECH_PACK_STATUS_ENUM speechStatus);

/**
 * 获取结果函数
 * 注意：asrTransResult 空间是在本函数内分配的，因此使用完了必须注意释放对应空间
 */
FUN_STATUS asr_recv_trans_result(struct MrcpClientStruct* mrcpClientStruct, int* id, AsrSpeechTransResult** asrTransResult, int* len, ASR_SPEECH_PACK_STATUS_ENUM speechStatus);

/**
 * 销毁/清理函数
 */
FUN_STATUS asr_term(struct MrcpClientStruct* mrcpClientStruct, int* id);

#ifdef __cplusplus
}
#endif

/* ============================== 封装后提供给 C 使用 ============================== */
#endif /* SRC_MRCPCLIENT_TEST_C_MRCPCLIENTWRAPPER_HPP_ */
