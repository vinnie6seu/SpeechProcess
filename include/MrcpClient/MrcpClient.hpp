/*
 * MrcpClient.hpp
 *
 *  Created on: 2018年6月5日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_MRCPCLIENT_MRCPCLIENT_HPP_
#define INCLUDE_MRCPCLIENT_MRCPCLIENT_HPP_

#include <tr1/memory>

#include "Define.hpp"
#include "Shm/tts/ShmTextToSpeech.hpp"
#include "Shm/asr/ShmSpeechToText.hpp"
#include "Util/Cfg/CfgUtil.hpp"
#include "Util/EnumUtil.hpp"
#include "HttpClient/HttpClient.hpp"

/**
 * 将相应的枚举类型转为字符串(这种方法很重要)
 */
#define FOREACH_MRCP_CLIENT_TYPE(CMD) \
        CMD(CLIENT_TYPE_TTS) \
		CMD(CLIENT_TYPE_ASR) \
		CMD(CLIENT_TYPE_ASR_PACKET)

/**
 * 枚举：该 mrcp 客户端是干啥用的
 */
typedef enum {
	FOREACH_MRCP_CLIENT_TYPE(GENERATE_ENUM)
} MRCP_CLIENT_TYPE_ENUM;

/**
 * 数组：该 mrcp 客户端是干啥用的，用于将枚举转成字符串
 */
const char* MRCP_CLIENT_TYPE_ENUM_STRING[] = {
	FOREACH_MRCP_CLIENT_TYPE(GENERATE_STRING)
};

class MrcpClient {
public:
	/**
	 * 构造函数
	 */
	MrcpClient();

	/**
	 * 析构函数
	 */
	~MrcpClient();

	/**
	 * 1.初始化acl库
	 * 2.设置日志
	 * 3.初始化共享内存
	 * 4.将共享内存加载到本进程中
	 */
	MrcpClient& init(MRCP_CLIENT_TYPE_ENUM clientType);

	/**
	 * 1.将文本写共享内存
	 * 2.与服务端 socket 通信，告知服务端写入位置
	 * 3.获取服务端返回信息，从共享内存读语音
	 */
	FUN_STATUS textToSpeechProcessAll(const char* text, char* speech);

	/**
	 * 1.将文本写共享内存
	 * 2.与服务端socket通信，告知服务端写入位置
	 * 3.获取服务端返回信息，从共享内存读部分语音
	 */
	FUN_STATUS textToSpeechProcessPart(const char* text, char* speech);

	/////////////////////////////////////////////////////////////////////

	/**
	 * 1.将语音数据写共享内存
	 * 2.与服务端 http 通信，告知服务端写入位置
	 * 3.获取服务端返回信息，从共享内存读文本
	 */
	FUN_STATUS speechToTextProcessAll(const char* speech, char* text);

	/////////////////////////////////////////////////////////////////////

	/**
	 * 1.init，找到当次准备写入的id，向 java 服务端发送[id, MSP_AUDIO_INIT]
	 * 2.continue，向 java 服务端发送[id, MSP_AUDIO_CONTINUE, cur_send_speech_num]，写语音包数据，收取文本包
	 * 3.last，告知 java 服务端[id, MSP_AUDIO_LAST, totalSendPacketNum]
	 */
	FUN_STATUS asrSpeechPacketSend(int& id, const char* speech, int len, int cur_send_packet_num, ASR_SPEECH_PACK_STATUS_ENUM speechStatus);

	/**
	 * 1.continue，如果有数据包就取走，如果遇到异常就返回失败
	 * 2.last，阻塞取走所有数据包，直到is_final，如果遇到异常就返回失败；最后清理阻塞队列，修改item的flag为0，修改head的free
	 */
	FUN_STATUS asrTextBlockRecv(int id, vector<AsrSpeechTransResult>& asr_trans_result_vec, ASR_SPEECH_PACK_STATUS_ENUM speechStatus);

    /**
     * 对指定 id 的 item 清理
     */
	FUN_STATUS asrExceptionHandle(int id);


	/////////////////////////////

	ShmSpeechToText& getShmSpeechToText() {
		return _shmSpeechToText;
	}

	ShmSpeechToTextHead* getShmSpeechToTextHead() {
		return _shmSpeechToTextHead;
	}

	map<int, std::tr1::shared_ptr<BlockingShmQueue> >& getAllBlockingShmQueue() {
		return 	_allBlockingShmQueue;
	}

private:
	bool _init_flag;                                                             // 已经初始化true，未初始化false
	CfgUtil _cfgUtil;                                                            // 解析配置文件

	MRCP_CLIENT_TYPE_ENUM _clientType;                                           // 该参数指出本客户端是处理 tts 还是 asr 的

	ShmTextToSpeech _shmTextToSpeech;                                            // tts 共享内存操作
	ShmTextToSpeechHead* _shmTextToSpeechHead;                                   // tts shm 头

	//////////////////////////////////////////////////////////////////

	ShmSpeechToText _shmSpeechToText;                                            // asr 共享内存操作
	ShmSpeechToTextHead* _shmSpeechToTextHead;                                   // asr shm 头
	HttpClient _HttpClient;                                                      // 用作 http 通信

	//////////////////////////////////////////////////////////////////

	map<int, std::tr1::shared_ptr<NoBlockingShmQueue> > _allNoBlockingShmQueue;
	map<int, std::tr1::shared_ptr<BlockingShmQueue> > _allBlockingShmQueue;
};


/* ============================== 封装后提供给 C 使用 ============================== */

struct MrcpClientStruct {
	MrcpClient* mrcpClient;
};

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

#endif /* INCLUDE_MRCPCLIENT_MRCPCLIENT_HPP_ */
