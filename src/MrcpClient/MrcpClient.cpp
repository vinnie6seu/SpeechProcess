/*
 * MrcpClient.cpp
 *
 *  Created on: 2018年6月5日
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



MrcpClient::MrcpClient() : _cfgUtil("SpeechProcess.cfg") {
	_init_flag = false;
	_shmTextToSpeechHead = NULL;    // tts shm 头
	_shmSpeechToTextHead = NULL;    // asr shm 头

	_clientType = CLIENT_TYPE_TTS;
}

MrcpClient::~MrcpClient() {

	if (_clientType == CLIENT_TYPE_TTS) {
		if (_shmTextToSpeech.procDetachShm((void*) _shmTextToSpeechHead) == FAILURE) {
			logger_error(_shmTextToSpeech.getShmLastError().c_str());
		}
	} else if (_clientType == CLIENT_TYPE_ASR) {
		if (_shmSpeechToText.procDetachShm((void*) _shmSpeechToTextHead) == FAILURE) {
			logger_error(_shmSpeechToText.getShmLastError().c_str());
		}
	}

}

MrcpClient& MrcpClient::init(MRCP_CLIENT_TYPE_ENUM clientType) {
	if (_init_flag) {
		return *this;
	}

	// 0.记录本客户端的是做 tts 还是 asr
	_clientType = clientType;

	// 1.初始化ACL库(尤其是在WIN32下一定要调用此函数，在UNIX平台下可不调用)
	acl_cpp_init();

	// 2.设置日志
	// 设置日志是否标准输出
	bool stdOutFlag = _cfgUtil.getLogStdOut();
	acl::log::stdout_open(stdOutFlag);

	// 打开日志
	map<std::string, std::string> logFullPathArr;
	logFullPathArr["MrcpClient"] = "";
	_cfgUtil.getLogFullPath(logFullPathArr);
	for (map<std::string, std::string>::iterator it = logFullPathArr.begin();
			it != logFullPathArr.end(); it++) {
		acl::log::open((*it).second.c_str(), (*it).first.c_str());

		logger("%s log is open....!", (*it).first.c_str());
	}

	if (_clientType == CLIENT_TYPE_TTS) {

		// 3.1.1.初始化文本转语音共享内存
		int shmTextToSpeechKey = _cfgUtil.getShmTextToSpeechKey();
		int shmTextToSpeechNum = _cfgUtil.getShmTextToSpeechNum();
		_shmTextToSpeech.init(shmTextToSpeechKey, shmTextToSpeechNum);
		logger("init shm:[text to speech], key:[%d], num:[%d]", shmTextToSpeechKey, shmTextToSpeechNum);

		// 3.1.2.将共享内存加载到本进程中
		if (_shmTextToSpeech.procLoadShm((void**) &_shmTextToSpeechHead) == FAILURE) {
			logger_error(_shmTextToSpeech.getShmLastError().c_str());
			// return FAILURE;
		}
		logger("success to add shmTextToSpeech in my proc, proc_id:[%d]", (int)getpid());

	} else if (_clientType == CLIENT_TYPE_ASR) {

		// 3.2.1.初始化语音转文本共享内存
		int shmSpeechToTextKey = _cfgUtil.getShmSpeechToTextKey();
		int shmSpeechToTextNum = _cfgUtil.getShmSpeechToTextNum();
		_shmSpeechToText.init(shmSpeechToTextKey, shmSpeechToTextNum);
		logger("init shm:[speech to text], key:[%d], num:[%d]", shmSpeechToTextKey, shmSpeechToTextNum);

		// 3.2.2.将共享内存加载到本进程中
		if (_shmSpeechToText.procLoadShm((void**) &_shmSpeechToTextHead) == FAILURE) {
			logger_error(_shmSpeechToText.getShmLastError().c_str());
			// return FAILURE;
		}
		logger("success to add shmSpeechToText in my proc, proc_id:[%d]", (int)getpid());

		// 3.2.3.初始化 http 客户端
		std::string server_addr = _cfgUtil.getHttpIp() + ":" + _cfgUtil.getHttpPort();	// web 服务器地址
		std::string stype = _cfgUtil.getHttpRequestType();		                        // 请求报文格式
		std::string charset = _cfgUtil.getHttpRequestCharSet();	                        // 本地请求数据文件的字符集
		std::string to_charset = _cfgUtil.getHttpResponseCharSet();	                    // 将服务器响应数据转为本地字符集
		_HttpClient.init(server_addr.c_str(), stype.c_str(), charset.c_str(), to_charset.c_str());
		logger("success to init http client, server_addr:[%s], request_type:[%s]", server_addr.c_str(), stype.c_str(), charset.c_str());
	} else if (_clientType == CLIENT_TYPE_ASR_PACKET) {

		// 3.3.1.初始化语音转文本共享内存
		int shmSpeechToTextKey = _cfgUtil.getShmSpeechToTextKey();
		int shmSpeechToTextNum = _cfgUtil.getShmSpeechToTextNum();
		_shmSpeechToText.init(shmSpeechToTextKey, shmSpeechToTextNum);
		logger("init shm:[speech to text], key:[%d], num:[%d]", shmSpeechToTextKey, shmSpeechToTextNum);

		// 3.3.2.将共享内存加载到本进程中
		if (_shmSpeechToText.procLoadShm((void**) &_shmSpeechToTextHead) == FAILURE) {
			logger_error(_shmSpeechToText.getShmLastError().c_str());
			// return FAILURE;
		}
		logger("success to add shmSpeechToText in my proc, proc_id:[%d]", (int)getpid());

		// 3.3.3.初始化 http 客户端
		std::string server_addr = _cfgUtil.getHttpIp() + ":" + _cfgUtil.getHttpPort();	// web 服务器地址
		std::string stype = _cfgUtil.getHttpRequestType();		                        // 请求报文格式
		std::string charset = _cfgUtil.getHttpRequestCharSet();	                        // 本地请求数据文件的字符集
		std::string to_charset = _cfgUtil.getHttpResponseCharSet();	                    // 将服务器响应数据转为本地字符集
		_HttpClient.init(server_addr.c_str(), stype.c_str(), charset.c_str(), to_charset.c_str());
		logger("success to init http client, server_addr:[%s], request_type:[%s]", server_addr.c_str(), stype.c_str(), charset.c_str());

		// 3.3.4.构造共享内存块处理模式
		if (_shmSpeechToText.allItemShmQueueInit((void*) _shmSpeechToTextHead, _allNoBlockingShmQueue, _allBlockingShmQueue) == FAILURE) {
			logger_error(_shmSpeechToText.getShmLastError().c_str());
			// return FAILURE;
		}
		logger("success to call function allItemShmQueueInit");
	}

	_init_flag = true;

	return *this;
}

FUN_STATUS MrcpClient::textToSpeechProcessAll(const char* text, char* speech) {
	// 1.将文本写共享内存
	if (_shmTextToSpeechHead == NULL) {
		logger_error("fail to add shmTextToSpeech in my proc");
		return FAILURE;
	}

	if ((text == NULL) || (speech == NULL)) {
		logger_error("text or speech is NULL");
		return FAILURE;
	}

	ShmTextToSpeechItem shmTextToSpeechItem;
	memcpy(shmTextToSpeechItem._text, text, sizeof(shmTextToSpeechItem._text));
	_shmTextToSpeech.writeTextRecordShm((void*)_shmTextToSpeechHead, shmTextToSpeechItem);

	// 2.与服务端socket通信，告知服务端写入位置     **********************
	stringstream ss;
	ss << "id=" << shmTextToSpeechItem._id << "\r\n";
	std::string request = ss.str();


	std::string server_addr = _cfgUtil.getSocketIp() + ":" + _cfgUtil.getSocketPort();
	int conn_timeout = 10;
	/* 连接服务器超时时间，单位：秒 */
	int rw_timeout = 10 /* 网络 IO 超时时间，单位：秒 */;
	acl::socket_stream conn;

	acl::string buf;
	// 连接远程服务器
	if (conn.open(server_addr.c_str(), conn_timeout, rw_timeout) == false) {
		logger_error("connect server: %s error: %s", server_addr.c_str(), acl::last_serror());
		return FAILURE;
	} else {
		// 从服务器读一行数据，注：第二个参数为默认的 true，意思是获得
		// 一行数据后自动将尾部的 \r\n 去掉
		buf.clear();
		if (conn.gets(buf) == false) {
			logger_error("gets one line from server error: %s", acl::last_serror());
			return FAILURE;
		}

		// 打印server端的连接上的通知
		logger(buf.c_str());
	}


	// 向服务器写一行数据，同时从服务器读一行数据

	// 向服务器发送一行数据
	if (conn.write(request.c_str(), strlen(request.c_str())) == -1) {
		logger_error("write request to server error: %s", acl::last_serror());
		return FAILURE;
	}

	// 从服务器读一行数据，注：第二个参数为默认的 true，意思是获得
	// 一行数据后自动将尾部的 \r\n 去掉
	std::string bufStr;

	buf.clear();
	if (conn.gets(buf) == false) {
		logger_error("gets one line from server error: %s", acl::last_serror());
		return FAILURE;
	} else {
		// 获得服务器返回
		bufStr = buf.c_str();

		// 断开与服务端的连接
		// 向服务器发送一行数据
		if (conn.write("quit\r\n", strlen("quit\r\n")) == -1) {
			logger_error("write request to server error: %s", acl::last_serror());
			return FAILURE;
		}

		// 从服务器读一行数据，注：第二个参数为默认的 true，意思是获得
		// 一行数据后自动将尾部的 \r\n 去掉
		buf.clear();
		if (conn.gets(buf) == false) {
			logger_error("gets one line from server error: %s", acl::last_serror());
			return FAILURE;
		}

		// 打印server端的接受断开的通知
		logger(buf.c_str());
	}

	// 3.获取服务端返回信息，从共享内存读语音     **********************
	std::string delim = "=";
	std::vector<std::string> tokens;
	StringUtil::Split(bufStr, delim, tokens, true);

	_shmTextToSpeech.readSpeechRecordShm(_shmTextToSpeechHead, atoi(tokens[1].c_str()), shmTextToSpeechItem);
	logger("read shm at id:[%d], speechLen:[%s]", atoi(tokens[1].c_str()), strlen(shmTextToSpeechItem._speech));
	if (shmTextToSpeechItem._flag != 2) {
		logger_error("shm item flag is not 2");
		return FAILURE;
	}
	memcpy(speech, shmTextToSpeechItem._speech, sizeof(shmTextToSpeechItem._speech));

	return SUCCESS;
}

FUN_STATUS MrcpClient::textToSpeechProcessPart(const char* text, char* speech) {
	// 1.将文本写共享内存
	if (_shmTextToSpeechHead == NULL) {
		logger_error("fail to add shmTextToSpeech in my proc");
		return FAILURE;
	}

	if ((text == NULL) || (speech == NULL)) {
		logger_error("text or speech is NULL");
		return FAILURE;
	}

	ShmTextToSpeechItem shmTextToSpeechItem;
	memcpy(shmTextToSpeechItem._text, text, sizeof(shmTextToSpeechItem._text));
	_shmTextToSpeech.writeTextRecordShm((void*)_shmTextToSpeechHead, shmTextToSpeechItem);

	// 2.与服务端socket通信，告知服务端写入位置     **********************
	stringstream ss;
	ss << "id=" << shmTextToSpeechItem._id << "\r\n";
	std::string request = ss.str();


	std::string server_addr = _cfgUtil.getSocketIp() + ":" + _cfgUtil.getSocketPort();
	int conn_timeout = 10;
	/* 连接服务器超时时间，单位：秒 */
	int rw_timeout = 10 /* 网络 IO 超时时间，单位：秒 */;
	acl::socket_stream conn;

	acl::string buf;
	// 连接远程服务器
	if (conn.open(server_addr.c_str(), conn_timeout, rw_timeout) == false) {
		logger_error("connect server: %s error: %s", server_addr.c_str(), acl::last_serror());
		return FAILURE;
	} else {
		// 从服务器读一行数据，注：第二个参数为默认的 true，意思是获得
		// 一行数据后自动将尾部的 \r\n 去掉
		buf.clear();
		if (conn.gets(buf) == false) {
			logger_error("gets one line from server error: %s", acl::last_serror());
			return FAILURE;
		}

		// 打印server端的连接上的通知
		logger(buf.c_str());
	}


	// 向服务器写一行数据，同时从服务器读一行数据

	// 向服务器发送一行数据，告知服务端文本数据位置
	if (conn.write(request.c_str(), strlen(request.c_str())) == -1) {
		logger_error("write request to server error: %s", acl::last_serror());
		return FAILURE;
	}


	////////////////////////////////////////////////////////////////////

	// 从服务器读一行数据，注：第二个参数为默认的 true，意思是获得
	// 一行数据后自动将尾部的 \r\n 去掉
	std::string bufStr;

	// 此时flag是1表示服务端正在处理，当flag为2表示服务端已经完成处理完毕了
	int flag = shmTextToSpeechItem._flag;
	while (flag != 2) {
		buf.clear();
		if (conn.gets(buf) == false) {
			logger_error("gets one line from server error: %s", acl::last_serror());
			return FAILURE;
		} else {
			// 获得服务器返回
			bufStr = buf.c_str();
		}

		// 3.获取服务端返回信息，从共享内存读部分语音     **********************
		// "id=10&speechOffset=300&speechLen=66"
		std::vector<std::string> result;

		std::string delim1 = "&";
		std::vector<std::string> tokens1;
		StringUtil::Split(bufStr, delim1, tokens1, true);

		for (int i = 0 ; i < tokens1.size(); i++) {
			StringUtil::Split(tokens1[i], "=", result, true);
		}

		int speechOffset = atoi(result[3].c_str());
		int speechLen = atoi(result[5].c_str());
		flag = atoi(result[7].c_str());

		// 按照服务端的指示读取部分语音数据
		_shmTextToSpeech.readSpeechRecordShmPart(_shmTextToSpeechHead, atoi(result[1].c_str()), speechOffset, speechLen, shmTextToSpeechItem);
		logger("read shm at id:[%d], speechOffset:[%d], speechLen:[%d], flag:[%d]", atoi(result[1].c_str()), speechOffset, speechLen, flag);

		memcpy(speech + speechOffset, shmTextToSpeechItem._speech + speechOffset, speechLen);

//		// 获取服务端处理的情况
//		flag = shmTextToSpeechItem._flag;
	}
	// 设置客户端读取完毕的标识
	_shmTextToSpeech.setSpeechRecordShmReadOK(_shmTextToSpeechHead, shmTextToSpeechItem._id);

	// 断开与服务端的连接
	// 向服务器发送一行数据
	if (conn.write("quit\r\n", strlen("quit\r\n")) == -1) {
		logger_error("write request to server error: %s", acl::last_serror());
		return FAILURE;
	}

	// 从服务器读一行数据，注：第二个参数为默认的 true，意思是获得
	// 一行数据后自动将尾部的 \r\n 去掉
	buf.clear();
	if (conn.gets(buf) == false) {
		logger_error("gets one line from server error: %s", acl::last_serror());
		return FAILURE;
	}

	// 打印server端的接受断开的通知
	logger(buf.c_str());

	////////////////////////////////////////////////////////////////////

	return SUCCESS;
}

FUN_STATUS MrcpClient::speechToTextProcessAll(const char* speech, char* text) {
	logger("step into speechToTextProcessAll.....");

	// 1.将语音写共享内存
	if (_shmSpeechToTextHead == NULL) {
		logger_error("fail to add shmSpeechToText in my proc");
		return FAILURE;
	}

	if ((text == NULL) || (speech == NULL)) {
		logger_error("text or speech is NULL");
		return FAILURE;
	}

	logger("start to write speech data in shmKey:[%d] shmId:[%d]", _shmSpeechToText.getShmKey(), _shmSpeechToText.getShmId());

	ShmSpeechToTextItem shmSpeechToTextItem;
	memcpy(shmSpeechToTextItem._speech, speech, sizeof(shmSpeechToTextItem._speech));
	_shmSpeechToText.writeSpeechRecordShm((void*) _shmSpeechToTextHead, shmSpeechToTextItem);

	logger("end to write speech data in shmKey:[%d] shmId:[%d] id:[%d] speechLen:[%d]", _shmSpeechToText.getShmKey(), _shmSpeechToText.getShmId(), shmSpeechToTextItem._id, sizeof(shmSpeechToTextItem._speech));


	// 2.与服务端 http 通信，告知服务端语音写入位置，并得到服务端返回信息
	HttpJson httpJsonStruct;
//	memset(&httpJsonStruct, '\0', sizeof(HttpJson));	// 不要这样清空，否则下面的acl::gson(json_http, httpJsonStruct);会出错
	httpJsonStruct.id = shmSpeechToTextItem._id;

	try {
		// 2.1.将结构体序列化成需要发送的 json 报文字符串
		acl::json json_http;
		// 将 httpJson 对象转换为 json 对象
		acl::json_node& node = acl::gson(json_http, httpJsonStruct);

		// 2.2.发送给服务端信息，并收到返回 json 报文
		std::string requestJsonStr = node.to_string().c_str();
		std::string responseJsonStr;

		logger("requestJsonStr:[%s]", requestJsonStr.c_str());

		_HttpClient.postRequest("/SpeechTranscript/AsrGoogle/speechToText", requestJsonStr, responseJsonStr);

		logger("responseJsonStr:[%s]", responseJsonStr.c_str());

		// 2.3.解析返回的 json 报文
//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));
		json_http.clear();

		// 放入字符串
		json_http.update(responseJsonStr.c_str());

		// 将 json 对象转换为结构体对象
		std::pair<bool, std::string> ret = acl::gson(json_http.get_root(), httpJsonStruct);

		// 如果转换失败，则打印转换失败原因
		if (ret.first == false)
			logger_error("error: %s\r\n", ret.second.c_str());
		else
			logger("http server response is [%s]\r\n", responseJsonStr.c_str());

	} catch (ThrowableException& e) {
		logger_error(e.what().c_str());
		return FAILURE;
	}

	// 3.从服务端告知的位置取出 text
	_shmSpeechToText.readTextRecordShm(_shmSpeechToTextHead, httpJsonStruct.id, shmSpeechToTextItem);
	logger("read shm at id:[%d], textLen:[%d]", httpJsonStruct.id, strlen(shmSpeechToTextItem._text));
	if (shmSpeechToTextItem._flag != ITEM_FLAG_CAN_GET_TEXT) {
		logger_error("shm item flag is not [%s]", SHM_ASR_ITEM_FLAG_ENUM_STRING[ITEM_FLAG_CAN_GET_TEXT]);
		return FAILURE;
	}
	memcpy(text, shmSpeechToTextItem._text, sizeof(shmSpeechToTextItem._text));

	return SUCCESS;
}

/////////////////////////////////////////////////////////////////////


FUN_STATUS MrcpClient::asrSpeechPacketSend(int& id, const char* speech, int len, int cur_send_packet_num, ASR_SPEECH_PACK_STATUS_ENUM speechStatus) {

	// 0.将语音写共享内存
	if (_shmSpeechToTextHead == NULL) {
		logger_error("fail to add shmSpeechToText in my proc");
		return FAILURE;
	}

	if (speechStatus == MSP_AUDIO_INIT) {
		// 1.init 状态在共享内存找到写入 id
		if (_shmSpeechToText.findWriteSpeechDest((void*) _shmSpeechToTextHead, id) != SUCCESS) {
			logger_error("fail to call fun findWriteSpeechDest");
			return FAILURE;
		}


		// 向服务端发送请求要求其创建 Google client 发送配置 ，告知服务端语音准备写入位置，并得到服务端返回信息
		HttpJson httpJsonStruct;
//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));	// 不要这样清空，否则下面的acl::gson(json_http, httpJsonStruct);会出错
		httpJsonStruct.id = id;
		httpJsonStruct.asrSpeechPackStatus = MSP_AUDIO_INIT;

		try {
			// 2.1.将结构体序列化成需要发送的 json 报文字符串
			acl::json json_http;
			// 将 httpJson 对象转换为 json 对象
			acl::json_node& node = acl::gson(json_http, httpJsonStruct);

			// 2.2.发送给服务端信息，并收到返回 json 报文
			std::string requestJsonStr = node.to_string().c_str();
			std::string responseJsonStr;

			logger("requestJsonStr:[%s]", requestJsonStr.c_str());

			_HttpClient.postRequest("/SpeechTranscript/AsrGoogle/speechToTextPacket", requestJsonStr, responseJsonStr);

			logger("responseJsonStr:[%s]", responseJsonStr.c_str());

			// 2.3.解析返回的 json 报文
	//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));
			json_http.clear();

			// 放入字符串
			json_http.update(responseJsonStr.c_str());

			// 将 json 对象转换为结构体对象
			std::pair<bool, std::string> ret = acl::gson(json_http.get_root(), httpJsonStruct);

			// 如果转换失败，则打印转换失败原因
			if (ret.first == false)
				logger_error("error: %s\r\n", ret.second.c_str());
			else
				logger("http server response is [%s]\r\n", responseJsonStr.c_str());

		} catch (ThrowableException& e) {
			logger_error(e.what().c_str());
			return FAILURE;
		}

	} else if (speechStatus == MSP_AUDIO_CONTINUE) {
		// 2.continue 状态将得到的 speech 写到非阻塞队列中
		if (id == -1) {
			logger_error("fail to call fun findWriteSpeechDest");
			return FAILURE;
		}

		// 发送语音包
		if (_allNoBlockingShmQueue.find(id) == _allNoBlockingShmQueue.end()) {
			logger_error("fail to find right noBlockingShmQueue");
			return FAILURE;
		}

		if (_shmSpeechToText.writeSpeechNoBlockingQueue(_allNoBlockingShmQueue[id], speech, len) != SUCCESS) {
			logger_error("fail to call fun writeSpeechNoBlockingQueue");
			return FAILURE;
		}

		// 向服务端发送请求告知已有语音数据包发送出来了
		HttpJson httpJsonStruct;
//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));	// 不要这样清空，否则下面的acl::gson(json_http, httpJsonStruct);会出错
		httpJsonStruct.id = id;
		httpJsonStruct.curSendPacketNum = cur_send_packet_num;
		httpJsonStruct.asrSpeechPackStatus = MSP_AUDIO_CONTINUE;

		try {
			// 2.1.将结构体序列化成需要发送的 json 报文字符串
			acl::json json_http;
			// 将 httpJson 对象转换为 json 对象
			acl::json_node& node = acl::gson(json_http, httpJsonStruct);

			// 2.2.发送给服务端信息，并收到返回 json 报文
			std::string requestJsonStr = node.to_string().c_str();
			std::string responseJsonStr;

			logger("requestJsonStr:[%s]", requestJsonStr.c_str());

			_HttpClient.postRequest("/SpeechTranscript/AsrGoogle/speechToTextPacket", requestJsonStr, responseJsonStr);

			logger("responseJsonStr:[%s]", responseJsonStr.c_str());

			// 2.3.解析返回的 json 报文
	//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));
			json_http.clear();

			// 放入字符串
			json_http.update(responseJsonStr.c_str());

			// 将 json 对象转换为结构体对象
			std::pair<bool, std::string> ret = acl::gson(json_http.get_root(), httpJsonStruct);

			// 如果转换失败，则打印转换失败原因
			if (ret.first == false)
				logger_error("error: %s\r\n", ret.second.c_str());
			else
				logger("http server response is [%s]\r\n", responseJsonStr.c_str());

		} catch (ThrowableException& e) {
			logger_error(e.what().c_str());
			return FAILURE;
		}

	} else if (speechStatus == MSP_AUDIO_LAST) {
		// 3.last 状态通知到服务端
		if (id == -1) {
			logger_error("fail to call fun findWriteSpeechDest");
			return FAILURE;
		}

		// 向服务端发送请求告知其已经发送完毕所有语音数据包
		HttpJson httpJsonStruct;
//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));	// 不要这样清空，否则下面的acl::gson(json_http, httpJsonStruct);会出错
		httpJsonStruct.id = id;
		httpJsonStruct.asrSpeechPackStatus = MSP_AUDIO_LAST;

		try {
			// 2.1.将结构体序列化成需要发送的 json 报文字符串
			acl::json json_http;
			// 将 httpJson 对象转换为 json 对象
			acl::json_node& node = acl::gson(json_http, httpJsonStruct);

			// 2.2.发送给服务端信息，并收到返回 json 报文
			std::string requestJsonStr = node.to_string().c_str();
			std::string responseJsonStr;

			logger("requestJsonStr:[%s]", requestJsonStr.c_str());

			_HttpClient.postRequest("/SpeechTranscript/AsrGoogle/speechToTextPacket", requestJsonStr, responseJsonStr);

			logger("responseJsonStr:[%s]", responseJsonStr.c_str());

			// 2.3.解析返回的 json 报文
	//		memset(&httpJsonStruct, '\0', sizeof(HttpJson));
			json_http.clear();

			// 放入字符串
			json_http.update(responseJsonStr.c_str());

			// 将 json 对象转换为结构体对象
			std::pair<bool, std::string> ret = acl::gson(json_http.get_root(), httpJsonStruct);

			// 如果转换失败，则打印转换失败原因
			if (ret.first == false)
				logger_error("error: %s\r\n", ret.second.c_str());
			else
				logger("http server response is [%s]\r\n", responseJsonStr.c_str());

		} catch (ThrowableException& e) {
			logger_error(e.what().c_str());
			return FAILURE;
		}

	}

	return SUCCESS;
}

FUN_STATUS MrcpClient::asrTextBlockRecv(int id, vector<AsrSpeechTransResult>& asr_trans_result_vec, ASR_SPEECH_PACK_STATUS_ENUM speechStatus) {

	// 语音写共享内存
	if (_shmSpeechToTextHead == NULL) {
		logger_error("fail to add shmSpeechToText in my proc");
		return FAILURE;
	}

	if (id == -1) {
		logger_error("fail to call fun findWriteSpeechDest");
		return FAILURE;
	}

	if (_allBlockingShmQueue.find(id) == _allBlockingShmQueue.end()) {
		logger_error("fail to find right BlockingShmQueue");
		return FAILURE;
	}

	// 读转义后生成的文本数据包
	bool is_exception = false;
	if (_shmSpeechToText.readTextBlockingQueueBatch(_shmTextToSpeechHead, id, _allBlockingShmQueue[id], asr_trans_result_vec, is_exception, speechStatus) == FAILURE) {

		logger_error("fail to call fun readTextBlockingQueueBatch");

		if (is_exception == true) {
			logger_error("google return exception, start to clear ShmQueue");
		}

		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS MrcpClient::asrExceptionHandle(int id) {
	if (_shmSpeechToText.callBackExceptionHandle(_shmTextToSpeechHead, id, _allNoBlockingShmQueue[id], _allBlockingShmQueue[id]) == FAILURE) {
		logger_error("fail to call fun callBackExceptionHandle");
		return FAILURE;
	}

	return SUCCESS;
}


/************************************************************************/
// g++ -DDEBUG_TEST MrcpClient.cpp -I/home/vinnie/my_project/SpeechProcess/include/

// #ifdef DEBUG_TEST

#include "lib_acl.hpp"

//////////////////////////////////////////////////////////////////////////

// tts 子线程类定义
class myThreadTTS: public acl::thread {
public:
	myThreadTTS(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient) {}

	~myThreadTTS() {}
protected:
	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {
		const char* myname = "run";
		logger("%s: thread id: %lu, %lu\r\n", myname, thread_id(), acl::thread::thread_self());

		char text[256 + 1];
//		sprintf(text, "进行全文本处理，my thread id is %lu", thread_id());
		sprintf(text, "start test, my thread id is %lu", thread_id());
		char speech[251552 + 1];

//		_mrcpClient.textToSpeechProcessAll(text, speech);
		_mrcpClient.textToSpeechProcessPart(text, speech);

		stringstream ss;
		ss << thread_id() << ".wav";
		speech_to_wav(speech, sizeof(speech), ss.str().c_str());

		return NULL;
	}

	virtual void speech_to_wav(const char* speech, int len_speech,
			const char* des_path) {
		FILE* fp = fopen(des_path, "wb");
		if (NULL == fp) {
			logger_error("open %s error.\n", des_path);
			return;
		}

		fwrite(speech, len_speech, 1, fp);
		fclose(fp);
		fp = NULL;
	}

private:
	MrcpClient& _mrcpClient;
};

static void test_thread_tts(MrcpClient& mrcpClient, int theadNum) {
	int max_threads = theadNum;

	// 创建一组子线程
	std::vector<myThreadTTS*> threads;
	for (int i = 0; i < max_threads; i++) {
		myThreadTTS* thread = new myThreadTTS(mrcpClient);
		threads.push_back(thread);
		thread->set_detachable(false);
		thread->start();
	}

	// 等待所有子线程正常退出
	std::vector<myThreadTTS*>::iterator it = threads.begin();
	for (; it != threads.end(); ++it) {
		(*it)->wait();
		delete (*it);
	}

/*
	const char* myname = "test_thread";
	myThreadTTS thr(mrcpClient);  // 子线程对象实例

	// 设置线程的属性为非分离方式，以便于下面可以调用 wait
	// 等待线程结束
	thr.set_detachable(false);

	// 启动一个子线程
	if (thr.start() == false) {
		logger_error("start thread failed\r\n");
		return;
	}

	logger("%s: thread id is %lu, main thread id: %lu\r\n", myname, thr.thread_id(), acl::thread::thread_self());

	// 等待子线程运行结束
	if (thr.wait(NULL) == false)
		logger_error("wait thread failed\r\n");
	else
		logger("wait thread ok\r\n");
*/
}

//////////////////////////////////////////////////////////////////////////

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

/**
 * 获取指定目录下，指定后缀的文件
 */
FUN_STATUS getFileVec(const char* dirName, std::vector<std::string>& fileNameVec, const char* suffix) {
	DIR *dip;
	struct dirent *dit;
	int i = 0;

	/* DIR *opendir(const char *name);
	 *
	 * Open a directory stream to argv[1] and make sure
	 * it's a readable and valid (directory) */
	if ((dip = opendir(dirName)) == NULL) {
		logger_error("fail to call opendir");
		return FAILURE;
	}
	logger("Directory stream is now open\n");

	/* struct dirent *readdir(DIR *dir);
	 *
	 * Read in the files from argv[1] and print */
	while ((dit = readdir(dip)) != NULL) {

		bool flag = false;

		std::string file = std::string(dirName) + dit->d_name;
		if (file.find(suffix) != std::string::npos) {
			flag = true;
		}

		if (flag) {
			i++;
			fileNameVec.push_back(file.c_str());
		}

	}
	logger("\n\nreaddir() [%s] found total [%d] [%s] files\n", dirName, i, suffix);

	/* int closedir(DIR *dir);
	 *
	 * Close the stream to argv[1]. And check for errors. */
	if (closedir(dip) == -1) {
		logger_error("fail to call closedir");
		return FAILURE;
	}
	logger("\nDirectory stream is now closed\n");

	return SUCCESS;
}

// asr 子线程类定义
class myThreadASR: public acl::thread {
public:
	myThreadASR(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient) {}

	~myThreadASR() {}

	void setFileName(std::string fileName) {
		_fileName = fileName;
	}
protected:
	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {
		const char* myname = "run";

		char speech[638014 + 1] = {'\0'};                // asr转成的源语音
		char text[1024 + 1] = {'\0'};                    // 目标文本

		// 读取音频文件内容
		acl::string fileBody;
		if (acl::ifstream::load(_fileName.c_str(), &fileBody) == false) {
			logger_error("load %s error", _fileName.c_str());
			return NULL;
		}

		// 检查 wav 的格式头部
		wave_pcm_hdr wavHeadStruct;
		memcpy(&wavHeadStruct, fileBody.c_str(), 44);

		logger("PCM:[%d] channels:[%d] data_size:[%d]", wavHeadStruct.format_tag, wavHeadStruct.channels, wavHeadStruct.data_size);

		// 向 http 服务端请求
		memcpy(speech, fileBody.c_str(), wavHeadStruct.size_8 + 8);

		clock_t start, finish;
		start = clock();

		_mrcpClient.speechToTextProcessAll(speech, text);

		finish = clock();

		logger("fileName:[%s] text is [%s] consume time:[%f]s", _fileName.c_str(), text, (float) (finish-start) / CLOCKS_PER_SEC);

		return NULL;
	}

private:
	MrcpClient& _mrcpClient;
	std::string _fileName;
};

static void test_thread_asr(MrcpClient& mrcpClient, std::string path) {
	std::vector<std::string> fileNameVec;
	getFileVec(path.c_str(), fileNameVec, ".wav");

	// 创建一组子线程
	std::vector<myThreadASR*> threads;
	for (int i = 0; i < fileNameVec.size(); i++) {
		myThreadASR* thread = new myThreadASR(mrcpClient);
		threads.push_back(thread);
		thread->set_detachable(false);
		thread->setFileName(fileNameVec[i].c_str());
		thread->start();
	}

	// 等待所有子线程正常退出
	std::vector<myThreadASR*>::iterator it = threads.begin();
	for (; it != threads.end(); ++it) {
		(*it)->wait();
		delete (*it);
	}
}

//////////////////////////////////////////////////////////////////////////

class myThreadASRPacket: public acl::thread {
public:
	myThreadASRPacket(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient), _id(-1) {}

	~myThreadASRPacket() {}

	void setFileName(std::string fileName) {
		_fileName = fileName;
	}
protected:
	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {

		if (work_function() != SUCCESS) {

			// 需要清理
			_mrcpClient.asrExceptionHandle(_id);

			return NULL;
		}

		return NULL;
	}

	virtual FUN_STATUS work_function() {
		clock_t start, finish;
		start = clock();

		//////////////////////////////////////////

		char speech[SPEECH_LEN] = {'\0'};                // asr 的源语音
		char text[TEXT_LEN] = {'\0'};                    // 目标文本

		// 1.读取音频文件内容
		acl::string fileBody;
		if (acl::ifstream::load(_fileName.c_str(), &fileBody) == false) {
			logger_error("load %s error", _fileName.c_str());
			return FAILURE;
		}

		// 2.检查 wav 的格式头部
		wave_pcm_hdr wavHeadStruct;
		memcpy(&wavHeadStruct, fileBody.c_str(), 44);

		logger("PCM:[%d] channels:[%d] data_size:[%d]", wavHeadStruct.format_tag, wavHeadStruct.channels, wavHeadStruct.data_size);

        // 3.拷贝语音数据到 speech 数组中
		memcpy(speech, fileBody.c_str(), wavHeadStruct.size_8 + 8);

		// 4.请求 java 收取 asr 结果
		vector<AsrSpeechTransResult> asr_trans_result_vec;

		// 4.1 init，找到当次准备写入的id，向 java 服务端发送[id, MSP_AUDIO_INIT]
		if (_mrcpClient.asrSpeechPacketSend(_id, NULL, 0, 0, MSP_AUDIO_INIT) != SUCCESS) {
			logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_INIT]", _id);
			return FAILURE;
		}
		logger("id:[%d] success to send [MSP_AUDIO_INIT] packet", _id);

		// 4.2 continue，向 java 服务端发送[id, MSP_AUDIO_CONTINUE, cur_send_speech_num]，写语音包数据，收取文本包
		int data_length = wavHeadStruct.size_8 + 8;
		int FRAME_LEN = SPEECH_PACKET_LEN;
		int num = 0;
		while (num * FRAME_LEN <= data_length) {
			if ((num + 1) * FRAME_LEN > data_length) {

				logger("id:[%d] speech from[%d] to[%d]\n", _id , num * FRAME_LEN, data_length);

				// 注意这里因为用的是音频文件最后一段肯定不能取整，但是为了满足发送的 packet 大小一致，依然使用SPEECH_PACKET_LEN
				if (_mrcpClient.asrSpeechPacketSend(_id, speech + num * FRAME_LEN, SPEECH_PACKET_LEN, num, MSP_AUDIO_CONTINUE) != SUCCESS) {
					logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_CONTINUE]", _id);
					return FAILURE;
				}

			} else {

				logger("id:[%d] speech from[%d] to[%d]\n", _id , num * FRAME_LEN, (num + 1) * FRAME_LEN);

				if (_mrcpClient.asrSpeechPacketSend(_id, speech + num * FRAME_LEN, SPEECH_PACKET_LEN, num, MSP_AUDIO_CONTINUE) != SUCCESS) {
					logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_CONTINUE]", _id);
					return FAILURE;
				}

			}
			num++;
			logger("id:[%d] cur_send_speech_num:[%d] success to send [MSP_AUDIO_CONTINUE] packet", _id, num);

			usleep(200 * 1000); // 模拟人说话时间间隙。200ms对应10帧的音频

			logger("id:[%d] status:[MSP_AUDIO_CONTINUE] start to receive text packet", _id);
            // 中间处理过程收取结果
			asr_trans_result_vec.clear();
			if (_mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_CONTINUE) == FAILURE) {
				logger_error("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_CONTINUE]", _id);
				return FAILURE;
			}
			logger("id:[%d] status:[MSP_AUDIO_CONTINUE] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

			for (int i = 0; i < asr_trans_result_vec.size(); i++) {
				logger("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
						asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
						asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
						asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
						asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);
			}
		}

		// 4.3 last，告知 java 服务端[id, MSP_AUDIO_LAST]
		if (_mrcpClient.asrSpeechPacketSend(_id, NULL, 0, 0, MSP_AUDIO_LAST) != SUCCESS) {
			logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_LAST]", _id);
			return FAILURE;
		}
		logger("id:[%d] success to send [MSP_AUDIO_LAST] packet", _id);

        // 4.4 最后收取所有结果
		logger("id:[%d] status:[MSP_AUDIO_LAST] start to receive text packet", _id);

		asr_trans_result_vec.clear();
		if (_mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_LAST) == FAILURE) {
			logger_error("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", _id);
			return FAILURE;
		}
		logger("id:[%d] status:[MSP_AUDIO_LAST] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

		for (int i = 0; i < asr_trans_result_vec.size(); i++) {
			logger("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
					asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
					asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
					asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
					asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);

			if (asr_trans_result_vec[i]._cur_result._is_final == true) {
				memcpy(text, asr_trans_result_vec[i]._cur_result._transcript, strlen(asr_trans_result_vec[i]._cur_result._transcript));
			}
		}


		//////////////////////////////////////////

		finish = clock();

		logger("fileName:[%s] text is [%s] consume time:[%f]s", _fileName.c_str(), text, (float) (finish-start) / CLOCKS_PER_SEC);

		return SUCCESS;
	}

private:
	MrcpClient& _mrcpClient;
	std::string _fileName;
	int _id;                        // 当前线程写入数据的 item id
};

static void test_thread_asr_packet(MrcpClient& mrcpClient, std::string path) {
	std::vector<std::string> fileNameVec;
	getFileVec(path.c_str(), fileNameVec, ".wav");

	// 创建一组子线程
	std::vector<myThreadASRPacket*> threads;
	for (int i = 0; i < fileNameVec.size(); i++) {
		myThreadASRPacket* thread = new myThreadASRPacket(mrcpClient);
		threads.push_back(thread);
		thread->set_detachable(false);
		thread->setFileName(fileNameVec[i].c_str());
		thread->start();
	}

	// 等待所有子线程正常退出
	std::vector<myThreadASRPacket*>::iterator it = threads.begin();
	for (; it != threads.end(); ++it) {
		(*it)->wait();
		delete (*it);
	}
}

//////////////////////////////////////////////////////////////////////////

static void usage(const char* procname)
{
	printf("usage: %s [-h help] [-t test_type tts/asr/asr_packet] [-n tts_client_thread_num] [-d asr_client_find_wav_path]\r\n", procname);
	printf("para options:\r\n");
	printf("\t %s -t tts -n 20\r\n", procname);
	printf("\t %s -t asr -d .\/\r\n", procname);
	printf("\t %s -t asr_packet -d .\/\r\n", procname);
}

int main(int argc, char* argv[]) {

	int ch;
	std::string type;
	int theadNum;
	std::string path;
	while ((ch = getopt(argc, argv, "ht:n:d:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 't':
			type = optarg;
			break;
		case 'n':
			theadNum = atoi(optarg);
			break;
		case 'd':
			path = optarg;
			break;
		default:
			usage(argv[0]);
			return 0;
		}
	}

	// 0.初始化 mrcp 客户端
	MrcpClient& mrcpClient = acl::singleton2<MrcpClient>::get_instance();

	if (type == "tts") {
		mrcpClient.init(CLIENT_TYPE_TTS);

		// 1.多线程测试 tts 程序，运行若干线程请求 socket 服务端，给出文本转成语音
		test_thread_tts(mrcpClient, theadNum);
	} else if (type == "asr") {
		mrcpClient.init(CLIENT_TYPE_ASR);
		// 2.多线程测试 asr 程序，将制度目录下的 wav 音频数据取出发给 http 服务端，给出语音转成文本
		test_thread_asr(mrcpClient, path);
	} else if (type == "asr_packet") {
		mrcpClient.init(CLIENT_TYPE_ASR_PACKET);
		// 3.多线程测试 asr 程序，将制度目录下的 wav 音频数据取出[分割数据包]发给 http 服务端，给出语音转成文本
		test_thread_asr_packet(mrcpClient, path);
	} else {
		usage(argv[0]);
		return 0;
	}

	return 0;
}

// #endif



