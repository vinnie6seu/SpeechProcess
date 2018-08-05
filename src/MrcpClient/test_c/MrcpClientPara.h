/*
 * Para.h
 *
 *  Created on: 2018年8月5日
 *      Author: yaoqiaobing
 */

#ifndef SRC_MRCPCLIENT_TEST_C_MRCPCLIENTPARA_H_
#define SRC_MRCPCLIENT_TEST_C_MRCPCLIENTPARA_H_

/////////////////////////////////////////////////////////////////////

#define SUCCESS 0
#define FAILURE -1
typedef int FUN_STATUS;

/////////////////////////////////////////////////////////////////////

/**
 * 谷歌 asr 根据每个语音片段转义返回的数据包，一般谷歌一次返回两个包，一个是转义结果 result，一个是预测结果 predict
 */
typedef struct {
	char _transcript[128 + 1];          // 文本数据
	double _stability;                  // 本包数据的稳定性，一般 result 的较高， predict 的较低

	bool _is_final;                     // 谷歌接口以 onComplete 为界，返回的最后结果包，_is_final 为 true，中间过程均是 false
	double _confidence;                 // 最终结果的准确度
} AsrSpeechTransItem;

/**
 * java 服务端给客户端返回的数据
 */
typedef struct {
	AsrSpeechTransItem _cur_result;     // 转义中间过程、最终结果放在其中

	AsrSpeechTransItem _cur_predict;    // 预测结果放在其中

	bool _is_exception;                 // 如果谷歌接口出现异常则 _is_exception 为true，否则为false
	char _exception_str[1024 + 1];      // 谷歌接口告知的异常原因
} AsrSpeechTransResult;

/////////////////////////////////////////////////////////////////////

const int SPEECH_LEN = 638014 + 1;
const int SPEECH_PACKET_LEN = 3200;

const int TEXT_LEN = 638014 + 1;
const int TEXT_PACKET_LEN = sizeof(AsrSpeechTransResult);

/////////////////////////////////////////////////////////////////////

typedef enum {
	MSP_AUDIO_INIT, MSP_AUDIO_CONTINUE, MSP_AUDIO_LAST
} ASR_SPEECH_PACK_STATUS_ENUM;

/////////////////////////////////////////////////////////////////////
#endif /* SRC_MRCPCLIENT_TEST_C_MRCPCLIENTPARA_H_ */
