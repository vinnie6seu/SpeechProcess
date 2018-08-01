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
	} else if (_clientType == CLIENT_TYPE_ASR_PACKET) {
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
		httpJsonStruct.totalSendPacketNum = cur_send_packet_num;
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
	if (_shmSpeechToText.readTextBlockingQueueBatch(_shmSpeechToTextHead, id, _allBlockingShmQueue[id], asr_trans_result_vec, is_exception, speechStatus) == FAILURE) {

		logger_error("fail to call fun readTextBlockingQueueBatch");

		if (is_exception == true) {
			logger_error("google return exception, start to clear ShmQueue");
		}

		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS MrcpClient::asrExceptionHandle(int id) {
	if (_shmSpeechToText.callBackExceptionHandle(_shmSpeechToTextHead, id, _allNoBlockingShmQueue[id], _allBlockingShmQueue[id]) == FAILURE) {
		logger_error("fail to call fun callBackExceptionHandle");
		return FAILURE;
	}

	return SUCCESS;
}


