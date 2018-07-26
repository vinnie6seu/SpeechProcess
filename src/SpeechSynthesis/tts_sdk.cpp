/*
 * tts_sdk.cpp
 *
 *  Created on: 2018年6月10日
 *      Author: yaoqiaobing
 */

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern "C" {
	#include "IflyTek/qtts.h"
	#include "IflyTek/msp_cmn.h"
	#include "IflyTek/msp_errors.h"
}

#include "lib_acl.hpp"
#include "SpeechSynthesis/SpeechSynthesis.hpp"
#include "SpeechSynthesis/tts_sdk.hpp"

using namespace std;

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr {
	char riff[4];                      // = "RIFF"
	int size_8;                        // = FileSize - 8
	char wave[4];                      // = "WAVE"
	char fmt[4];                       // = "fmt "
	int fmt_size;		               // = 下一个结构体的大小 : 16

	short int format_tag;              // = PCM : 1
	short int channels;                // = 通道数 : 1
	int samples_per_sec;               // = 采样率 : 8000 | 6000 | 11025 | 16000
	int avg_bytes_per_sec;             // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int block_align;             // = 每采样点字节数 : wBitsPerSample / 8
	short int bits_per_sample;         // = 量化比特数: 8 | 16

	char data[4];                      // = "data";
	int data_size;                     // = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;

/* 默认wav音频头部数据 */
wave_pcm_hdr default_wav_hdr = {
		{ 'R', 'I', 'F', 'F' },
		0,
		{ 'W', 'A', 'V', 'E' },
		{ 'f', 'm', 't', ' ' },
		16,
		1,
		1,
		8000,
		16000,
//		16000,
//		32000,
		2,
		16,
		{ 'd', 'a', 't', 'a' },
		0
};

/* 文本合成 */
int text_to_speech_all(const char* src_text, int src_text_len, char* des_speech, int des_speech_len, const char* params) {
	int ret = -1;
//	FILE* fp = NULL;
	const char* sessionID = NULL;
	unsigned int audio_len = 0;
	wave_pcm_hdr wav_hdr = default_wav_hdr;
	int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	if (NULL == src_text || NULL == des_speech) {
		logger_error("params is error!\n");
		return ret;
	}

	// 指向可写入起始位置
	char* p_des_speech = des_speech;

//	fp = fopen(des_path, "wb");
//	if (NULL == fp) {
//		printf("open %s error.\n", des_path);
//		return ret;
//	}

	/* 开始合成 */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret) {
		logger_error("QTTSSessionBegin failed, error code: %d.\n", ret);
//		fclose(fp);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int) strlen(src_text), NULL);
	if (MSP_SUCCESS != ret) {
		logger_error("QTTSTextPut failed, error code: %d.\n", ret);
		QTTSSessionEnd(sessionID, "TextPutError");
//		fclose(fp);
		return ret;
	}

	/* 正在合成 */
	logger("start to work ...\n");
//	fwrite(&wav_hdr, sizeof(wav_hdr), 1, fp); // 添加wav音频头，使用采样率为8000
	memcpy(p_des_speech, &wav_hdr, sizeof(wav_hdr)); // 添加wav音频头，使用采样率为8000
	p_des_speech += sizeof(wav_hdr);
	if ((p_des_speech - des_speech) >= des_speech_len) {
		logger_error("no space to write speech");
		return MSP_ERROR_FAIL;
	}
	logger("end write wav_hdr ...\n");

	while (1) {
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret) {
			break;
		}

		if (NULL != data) {

//			fwrite(data, audio_len, 1, fp);
			memcpy(p_des_speech, data, audio_len); // 写入语音数据
			p_des_speech += audio_len;
			if ((p_des_speech - des_speech) >= des_speech_len) {
				logger_error("no space to write speech");
				return MSP_ERROR_FAIL;
			}

			wav_hdr.data_size += audio_len; // 计算data_size大小

			logger("success to write speech data %d ...\n", wav_hdr.data_size);
		}

		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;

//		printf(">\n");
//		usleep(150 * 1000); // 防止频繁占用CPU
	}
//	printf("\n");

	logger("end write speech data ...\n");

	if (MSP_SUCCESS != ret) {
		logger_error("QTTSAudioGet failed, error code: %d.\n", ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
//		fclose(fp);
		return ret;
	}

	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);

	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
//	fseek(fp, 4, 0);
//	fwrite(&wav_hdr.size_8, sizeof(wav_hdr.size_8), 1, fp); // 写入size_8的值
//	fseek(fp, 40, 0); // 将文件指针偏移到存储data_size值的位置
//	fwrite(&wav_hdr.data_size, sizeof(wav_hdr.data_size), 1, fp); // 写入data_size的值
//	fclose(fp);
//	fp = NULL;

	p_des_speech = des_speech;
	// 写入size_8的值
	memcpy(p_des_speech + 4, &wav_hdr.size_8, sizeof(wav_hdr.size_8));
	// 写入data_size的值
	memcpy(p_des_speech + 40, &wav_hdr.data_size, sizeof(wav_hdr.data_size));

	logger("end modify wav_hdr ...\n");

	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret) {
		logger_error("QTTSSessionEnd failed, error code: %d.\n", ret);
	}

	return ret;
}

int tts_sdk_all(char* clientText, int clientTextLen, char* serverSpeech, int serverSpeechLen) {
	int ret = MSP_SUCCESS;
	const char* login_params = "appid = 5b0d5271, work_dir = ."; // 登录参数,appid与msc库绑定,请勿随意改动
	/*
	 * rdn:           合成音频数字发音方式
	 * volume:        合成音频的音量
	 * pitch:         合成音频的音调
	 * speed:         合成音频对应的语速
	 * voice_name:    合成发音人
	 * sample_rate:   合成音频采样率
	 * text_encoding: 合成文本编码格式
	 *
	 */
	const char* session_begin_params = "voice_name = xiaoyan, text_encoding = utf8, sample_rate = 8000, speed = 50, volume = 50, pitch = 50, rdn = 2";
//	const char* session_begin_params = "voice_name = xiaoyan, text_encoding = utf8, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 2";

	//	const char* filename = "tts_sample.wav"; // 合成的语音文件名称
	const char* text = clientText; // 需要合成的文本

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); // 第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	if (MSP_SUCCESS != ret) {
		//登录失败，退出登录
		logger_error("MSPLogin failed, error code: %d.\n", ret);
		goto exit;
	}

//	logger("\n###########################################################################\n");
//	logger("## 语音合成（Text To Speech，TTS）技术能够自动将任意文字实时转换为连续的 ##\n");
//	logger("## 自然语音，是一种能够在任何时间、任何地点，向任何人提供语音信息服务的  ##\n");
//	logger("## 高效便捷手段，非常符合信息时代海量数据、动态更新和个性化查询的需求。  ##\n");
//	logger("###########################################################################\n\n");

	/* 文本合成 */
	logger("call function text_to_speech ...\n");
	ret = text_to_speech_all(text, clientTextLen, serverSpeech, serverSpeechLen, session_begin_params);
	if (MSP_SUCCESS != ret) {
		logger_error("text_to_speech failed, error code: %d.\n", ret);
	}
	logger("合成完毕\n");

exit:
	MSPLogout(); //退出登录

	return ret;
}

/* ------------------------------ 两种不同的处理方式 ------------------------------ */

/* 文本合成 */
int text_to_speech_part(const char* src_text, int src_text_len, const char* params, socket_stream* stream, ShmTextToSpeechHead* _shmTextToSpeechHead, ShmTextToSpeech& _shmTextToSpeech, int id) {

	int ret = -1;
	const char* sessionID = NULL;
	unsigned int audio_len = 0;
	wave_pcm_hdr wav_hdr = default_wav_hdr;
	int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;

	ShmTextToSpeechItem shmTextToSpeechItem;
	memset(&shmTextToSpeechItem, '\0', sizeof(ShmTextToSpeechItem));

	char* des_speech = shmTextToSpeechItem._speech;
	int des_speech_len = sizeof(shmTextToSpeechItem._speech);

	if (NULL == src_text) {
		logger_error("params is error!\n");
		return ret;
	}

	// 指向可写入起始位置
	char* p_des_speech = des_speech;

	/* 开始合成 */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret) {
		logger_error("QTTSSessionBegin failed, error code: %d.\n", ret);
		return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int) strlen(src_text), NULL);
	if (MSP_SUCCESS != ret) {
		logger_error("QTTSTextPut failed, error code: %d.\n", ret);
		QTTSSessionEnd(sessionID, "TextPutError");
		return ret;
	}

	// 给client端的回复
	stringstream ss;

	int speechOffset = 0;

	/* 正在合成 */
	logger("start to work ...\n");

	memcpy(p_des_speech, &wav_hdr, sizeof(wav_hdr)); // 添加wav音频头，使用采样率为8000
	p_des_speech += sizeof(wav_hdr);
	if ((p_des_speech - des_speech) >= des_speech_len) {
		logger_error("no space to write speech");
		return MSP_ERROR_FAIL;
	}

	ss.clear();
	ss.str("");
	ss << "id=" << id << "&speechOffset=" << speechOffset << "&speechLen=" << sizeof(wav_hdr) << "&flag=1" << "\r\n";
    // 在共享内存中写入已经处理好的部分语音数据
	_shmTextToSpeech.writeSpeechRecordShmPart(_shmTextToSpeechHead, id, speechOffset, sizeof(wav_hdr), shmTextToSpeechItem);
	speechOffset += sizeof(wav_hdr);
    // 告知客户端已经处理好的部分语音数据
	if (stream->write(ss.str().c_str()) == -1) {
		logger_error("SpeechSynthesis server write 2 error: %s, response(%s), len: %d",  acl::last_serror(), ss.str().c_str(), (int ) ss.str().length());
		return FAILURE;
	}

	logger("end write wav_hdr ...\n");

	while (1) {
		/* 获取合成音频 */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret) {
			break;
		}

		if (NULL != data) {
			memcpy(p_des_speech, data, audio_len); // 写入语音数据
			p_des_speech += audio_len;
			if ((p_des_speech - des_speech) >= des_speech_len) {
				logger_error("no space to write speech");
				return MSP_ERROR_FAIL;
			}

			ss.clear();
			ss.str("");
			ss << "id=" << id << "&speechOffset=" << speechOffset << "&speechLen=" << audio_len << "&flag=1" << "\r\n";
			// 在共享内存中写入已经处理好的部分语音数据
			_shmTextToSpeech.writeSpeechRecordShmPart(_shmTextToSpeechHead, id, speechOffset, audio_len, shmTextToSpeechItem);
			speechOffset += audio_len;
			// 告知客户端已经处理好的部分语音数据
			if (stream->write(ss.str().c_str()) == -1) {
				logger_error("SpeechSynthesis server write 2 error: %s, response(%s), len: %d",  acl::last_serror(), ss.str().c_str(), (int ) ss.str().length());
				return FAILURE;
			}

			wav_hdr.data_size += audio_len; // 计算data_size大小

			logger("success to write speech data %d ...\n", wav_hdr.data_size);
		}

		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;

//		printf(">\n");
//		usleep(150 * 1000); // 防止频繁占用CPU
	}
	logger("end write speech data ...\n");

	if (MSP_SUCCESS != ret) {
		logger_error("QTTSAudioGet failed, error code: %d.\n", ret);
		QTTSSessionEnd(sessionID, "AudioGetError");
		return ret;
	}

	/* 修正wav文件头数据的大小 */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);

	/* 将修正过的数据写回文件头部,音频文件为wav格式 */
	p_des_speech = des_speech;
	// 写入size_8的值
	memcpy(p_des_speech + 4, &wav_hdr.size_8, sizeof(wav_hdr.size_8));
	// 写入data_size的值
	memcpy(p_des_speech + 40, &wav_hdr.data_size, sizeof(wav_hdr.data_size));

	ss.clear();
	ss.str("");
	speechOffset = 0;
	ss << "id=" << id << "&speechOffset=" << speechOffset << "&speechLen=" << sizeof(wav_hdr) << "&flag=2" << "\r\n";
	// 在共享内存中写入已经处理好的部分语音数据
	_shmTextToSpeech.writeSpeechRecordShmPart(_shmTextToSpeechHead, id, speechOffset, sizeof(wav_hdr), shmTextToSpeechItem);
	speechOffset += sizeof(wav_hdr);

	// 告知客户端已经处理好的部分语音数据
	if (stream->write(ss.str().c_str()) == -1) {
		logger_error("SpeechSynthesis server write 2 error: %s, response(%s), len: %d",  acl::last_serror(), ss.str().c_str(), (int ) ss.str().length());
		return FAILURE;
	}

	logger("end modify wav_hdr ...\n");

    // 所有语音处理合成完毕后，修改共享内存flag标识为2，不该使用[客户端在见到该标识后主动发送命令与本进程服务断开]
	// 因为客户端取这个flag标识可能不及时或者太早造成程序逻辑错误，直接用报文告知客户端该flag标识
	_shmTextToSpeech.setSpeechRecordShmWriteOK(_shmTextToSpeechHead, id);

	/* 合成完毕 */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret) {
		logger_error("QTTSSessionEnd failed, error code: %d.\n", ret);
	}

	return ret;
}

int tts_sdk_part(char* clientText, int clientTextLen, socket_stream* stream, ShmTextToSpeechHead* _shmTextToSpeechHead, ShmTextToSpeech& _shmTextToSpeech, int id) {
	int ret = MSP_SUCCESS;
	const char* login_params = "appid = 5b0d5271, work_dir = ."; // 登录参数,appid与msc库绑定,请勿随意改动
	/*
	 * rdn:           合成音频数字发音方式
	 * volume:        合成音频的音量
	 * pitch:         合成音频的音调
	 * speed:         合成音频对应的语速
	 * voice_name:    合成发音人
	 * sample_rate:   合成音频采样率
	 * text_encoding: 合成文本编码格式
	 *
	 */
	const char* session_begin_params = "voice_name = xiaoyan, text_encoding = utf8, sample_rate = 8000, speed = 50, volume = 50, pitch = 50, rdn = 2";
	const char* text = clientText; // 需要合成的文本

	/* 用户登录 */
	ret = MSPLogin(NULL, NULL, login_params); // 第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://www.xfyun.cn注册获取
	if (MSP_SUCCESS != ret) {
		//登录失败，退出登录
		logger_error("MSPLogin failed, error code: %d.\n", ret);
		goto exit;
	}

	/* 文本合成 */
	logger("call function text_to_speech_part ...\n");
	ret = text_to_speech_part(text, clientTextLen, session_begin_params, stream, _shmTextToSpeechHead, _shmTextToSpeech, id);
	if (MSP_SUCCESS != ret) {
		logger_error("text_to_speech_part failed, error code: %d.\n", ret);
	}
	logger("合成完毕\n");

exit:
	MSPLogout(); //退出登录

	return ret;
}
