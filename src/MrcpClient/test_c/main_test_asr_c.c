/*
 * main_test_c.c
 *
 *  Created on: 2018年8月2日
 *      Author: yaoqiaobing
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <time.h>

#include "MrcpClientPara.h"

#include "MrcpClientWrapper.hpp"

FUN_STATUS do_work(struct MrcpClientStruct* mrcpClientStruct, const char* fileName, int* id) {
	clock_t start, finish;
	start = clock();

	//////////////////////////////////////////
	int i = 0;

	char speech[SPEECH_LEN];                // asr 的源语音
	char text[TEXT_LEN];                    // 目标文本

	memset(speech, '\0', SPEECH_LEN);
	memset(text, '\0', TEXT_LEN);

	int data_length = 0;

	// 1.读取文件
	FILE* fp = NULL;
	if ((fp = fopen(fileName, "rb")) == NULL) {
		printf("can not open the raw image ");
		return FAILURE;
	} else {
		printf("read OK");
	}

	fseek(fp, 0L, SEEK_END); /* 定位到文件末尾 */
	data_length = ftell(fp); /* 得到文件大小 */

	fseek(fp, 0L, SEEK_SET); /* 定位到文件开头 */
	fread(speech, data_length, 1, fp); /* 一次性读取全部文件内容 */
	speech[data_length] = 0; /* 字符串结束标志 */

	fclose(fp);


	// 2.init，找到当次准备写入的id，向 java 服务端发送[id, MSP_AUDIO_INIT]
	if (asr_send_speech(mrcpClientStruct, id, NULL, 0, 0, MSP_AUDIO_INIT) != SUCCESS) {
		return FAILURE;
	}


	AsrSpeechTransResult* asrTransResult = NULL;
	int asrTransResultLen = 0;
	// 3.continue，向 java 服务端发送[id, MSP_AUDIO_CONTINUE, cur_send_speech_num]，写语音包数据，收取文本包
	int FRAME_LEN = SPEECH_PACKET_LEN;
	int num = 0;
	while (num * FRAME_LEN <= data_length) {
		if ((num + 1) * FRAME_LEN > data_length) {

			printf("id:[%d] speech from[%d] to[%d]\n", *id, num * FRAME_LEN, data_length);

			// 注意这里因为用的是音频文件最后一段肯定不能取整，但是为了满足发送的 packet 大小一致，依然使用SPEECH_PACKET_LEN
			if (asr_send_speech(mrcpClientStruct, id, speech + num * FRAME_LEN, SPEECH_PACKET_LEN, num, MSP_AUDIO_CONTINUE) != SUCCESS) {
				printf_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_CONTINUE]", *id);
				return FAILURE;
			}

		} else {

			printf("id:[%d] speech from[%d] to[%d]\n", *id, num * FRAME_LEN, (num + 1) * FRAME_LEN);

			if (asr_send_speech(mrcpClientStruct, id, speech + num * FRAME_LEN, SPEECH_PACKET_LEN, num, MSP_AUDIO_CONTINUE) != SUCCESS) {
				printf_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_CONTINUE]", *id);
				return FAILURE;
			}

		}
		num++;
		printf("id:[%d] cur_send_speech_num:[%d] success to send [MSP_AUDIO_CONTINUE] packet", *id, num);

		usleep(200 * 1000); // 模拟人说话时间间隙。200ms对应10帧的音频

		printf("id:[%d] status:[MSP_AUDIO_CONTINUE] start to receive text packet", *id);
        // 中间处理过程收取结果
		if (asr_recv_trans_result(mrcpClientStruct, id, &asrTransResult, &asrTransResultLen, MSP_AUDIO_CONTINUE) == FAILURE) {
			printf_error("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_CONTINUE]", *id);
			return FAILURE;
		}
		printf("id:[%d] status:[MSP_AUDIO_CONTINUE] success to receive text packet:[%d]", *id, asrTransResultLen);

		for (i = 0; i < asrTransResultLen; i++) {
			printf("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
					asrTransResult[i]._cur_result._transcript, asrTransResult[i]._cur_result._stability,
					asrTransResult[i]._cur_result._is_final ? "true" : "false", asrTransResult[i]._cur_result._confidence,
					asrTransResult[i]._cur_predict._transcript, asrTransResult[i]._cur_predict._stability,
					asrTransResult[i]._cur_predict._is_final ? "true" : "false", asrTransResult[i]._cur_predict._confidence);
		}

		// 清理
		free(asrTransResult);
		asrTransResult = NULL;
		asrTransResultLen = 0;
	}

	// 4.last，告知 java 服务端[id, MSP_AUDIO_LAST, totalSendPacketNum]
	if (asr_send_speech(mrcpClientStruct, id, NULL, 0, num, MSP_AUDIO_LAST) != SUCCESS) {
		printf_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_LAST]", *id);
		return FAILURE;
	}
	printf("id:[%d] success to send [MSP_AUDIO_LAST] packet", *id);

    // 5.最后收取所有结果
	printf("id:[%d] status:[MSP_AUDIO_LAST] start to receive text packet", *id);

	if (asr_recv_trans_result(mrcpClientStruct, id, &asrTransResult, &asrTransResultLen, MSP_AUDIO_LAST) == FAILURE) {
		printf_error("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", *id);
		return FAILURE;
	}
	printf("id:[%d] status:[MSP_AUDIO_LAST] success to receive text packet:[%d]", *id, asrTransResultLen);

	for (i = 0; i < asrTransResultLen; i++) {
		printf("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
				asrTransResult[i]._cur_result._transcript, asrTransResult[i]._cur_result._stability,
				asrTransResult[i]._cur_result._is_final ? "true" : "false", asrTransResult[i]._cur_result._confidence,
				asrTransResult[i]._cur_predict._transcript, asrTransResult[i]._cur_predict._stability,
				asrTransResult[i]._cur_predict._is_final ? "true" : "false", asrTransResult[i]._cur_predict._confidence);

		if (asrTransResult[i]._cur_result._is_final == true) {
			memcpy(text, asrTransResult[i]._cur_result._transcript, strlen(asrTransResult[i]._cur_result._transcript));
		}
	}


	//////////////////////////////////////////

	finish = clock();

	printf("fileName:[%s] text is [%s] consume time:[%f]s", fileName, text, (float) (finish-start) / CLOCKS_PER_SEC);

	return SUCCESS;
}

int main(int argc, char* argv[]) {

	MrcpClientStruct* mrcpClientStruct = getInstance();

	/**
	 * 1.初始化函数
	 * 注意：在所有线程前调用该函数
	 */
	asr_init(mrcpClientStruct);

	////////////////////////////////////////////////////////////////////////////////
	const char* fileName = "/root/qbyao/my_project/SpeechTranscript/src/main/resources/audio.raw";
	int id = -1;

	// 2.
	if (do_work(mrcpClientStruct, fileName, &id) != SUCCESS) {
		// 3.
		asr_term(mrcpClientStruct, &id);
	}

	////////////////////////////////////////////////////////////////////////////////

	return 0;
}
