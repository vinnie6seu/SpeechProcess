/*
 * SpeechSynthesis.cpp
 *
 *  Created on: 2018年5月27日
 *      Author: yaoqiaobing
 */

#include <string>
#include <unistd.h>
#include <pthread.h>

#include "SpeechSynthesis/SpeechSynthesis.hpp"
#include "lib_acl.hpp"
#include "Util/StringUtil.hpp"
#include "SpeechSynthesis/tts_sdk.hpp"

extern "C" {
	#include "IflyTek/msp_errors.h"
}

using namespace acl;
using namespace std;

SpeechSynthesis::SpeechSynthesis() : _cfgUtil("SpeechProcess.cfg") {
	_init_flag = false;
	_shmTextToSpeechHead = NULL;
}

SpeechSynthesis::~SpeechSynthesis() {
	if (_shmTextToSpeech.procDetachShm((void*) _shmTextToSpeechHead) == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
	}
}

SpeechSynthesis& SpeechSynthesis::init() {
	if (_init_flag) {
		return *this;
	}

	// 1.初始化ACL库(尤其是在WIN32下一定要调用此函数，在UNIX平台下可不调用)
	acl_cpp_init();

	// 2.设置日志
	// 设置日志是否标准输出
	bool stdOutFlag = _cfgUtil.getLogStdOut();
	acl::log::stdout_open(stdOutFlag);

	// 打开日志
	map<std::string, std::string> logFullPathArr;
	logFullPathArr["SpeechSynthesis"] = "";
	_cfgUtil.getLogFullPath(logFullPathArr);
	for (map<std::string, std::string>::iterator it = logFullPathArr.begin();
			it != logFullPathArr.end(); it++) {
		acl::log::open((*it).second.c_str(), (*it).first.c_str());

		logger("%s log is open....!", (*it).first.c_str());
	}

	// 3.初始化文本转语音共享内存
	int shmTextToSpeechKey = _cfgUtil.getShmTextToSpeechKey();
	int shmTextToSpeechNum = _cfgUtil.getShmTextToSpeechNum();
	_shmTextToSpeech.init(shmTextToSpeechKey, shmTextToSpeechNum);
	logger("init shm:[text to speech], key:[%d], num:[%d]", shmTextToSpeechKey, shmTextToSpeechNum);

	// 4.将共享内存加载到本进程中
	if (_shmTextToSpeech.procLoadShm((void**) &_shmTextToSpeechHead) == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		// return FAILURE;
	}
	logger("success to add shmTextToSpeech in my proc, proc_id:[%d]", getpid());

	_init_flag = true;

	return *this;
}

void SpeechSynthesis::on_accept(socket_stream* stream) {
	if (stream->format("SpeechSynthesis server start to provide service!\r\n") == -1)
		return ;

	while (true) {
		if (on_read(stream) == FAILURE)
			break;
	}
}

FUN_STATUS SpeechSynthesis::on_read(socket_stream* stream) {
	acl::string request;

	// 收到的请求"\r\n"已经去除
	if (stream->gets(request) == false) {
		logger_error("gets error: %s", acl::last_serror());
		return FAILURE;
	}

	if (request == "quit") {
		stream->puts("SpeechSynthesis server end the service!\r\n");
		return FAILURE;
	}

	if (request.empty()) {
		if (stream->write("\r\n") == -1) {
			logger_error("SpeechSynthesis server write 1 error: %s", acl::last_serror());
			return FAILURE;
		}
	} else {

//		// 调用该函数处理完全文本告知客户端
//		if (do_text_all(request, stream) == FAILURE) {
//			return FAILURE;
//		}

		// 调用该函数一有语音数据即告知客户端
		if (do_text_part(request, stream) == FAILURE) {
			return FAILURE;
		}
	}

	return SUCCESS;
}

FUN_STATUS SpeechSynthesis::do_text_all(acl::string& request, socket_stream* stream) {
	logger("client request:[%s]", request.c_str());

	// 读出客户端写入的文本进行处理     **********************
	std::string requestStr = request.c_str();
	std::string delim = "=";
	std::vector<std::string> tokens;
	StringUtil::Split(requestStr, delim, tokens, true);

	// 从指定共享内存位置读取全文本
	ShmTextToSpeechItem shmTextToSpeechItem;
	_shmTextToSpeech.readTextRecordShm(_shmTextToSpeechHead, atoi(tokens[1].c_str()), shmTextToSpeechItem);
	if (shmTextToSpeechItem._flag != 1) {
		logger_error("shm item flag is [%d] not 1", shmTextToSpeechItem._flag);
		return FAILURE;
	}
	logger("client request:[%s], text:[%s]", request.c_str(), shmTextToSpeechItem._text);

	// 处理好了语音
	if (tts_sdk_all(shmTextToSpeechItem._text, sizeof(shmTextToSpeechItem._text), shmTextToSpeechItem._speech, sizeof(shmTextToSpeechItem._speech)) != MSP_SUCCESS) {
		logger_error("fail to call function tts_sdk");
		return FAILURE;
	}

	// 将处理好的语音全部写入共享内存中
	_shmTextToSpeech.writeSpeechRecordShm(_shmTextToSpeechHead, atoi(tokens[1].c_str()), shmTextToSpeechItem);

	// 告知客户端写在了哪个位置
	std::string response = std::string(request.c_str()) + "\r\n";

	if (stream->write(response.c_str()) == -1) {
		logger_error("SpeechSynthesis server write 2 error: %s, response(%s), len: %d",  acl::last_serror(), response.c_str(), (int ) response.length());
		return FAILURE;
	}

	logger("server response:[%s]", response.c_str());

	return SUCCESS;
}

FUN_STATUS SpeechSynthesis::do_text_part(acl::string& request, socket_stream* stream) {
	logger("client request:[%s]", request.c_str());

	// 读出客户端写入的文本进行处理     **********************
	std::string requestStr = request.c_str();
	std::string delim = "=";
	std::vector<std::string> tokens;
	StringUtil::Split(requestStr, delim, tokens, true);

	// 从指定共享内存位置读取全文本
	ShmTextToSpeechItem shmTextToSpeechItem;
	_shmTextToSpeech.readTextRecordShm(_shmTextToSpeechHead, atoi(tokens[1].c_str()), shmTextToSpeechItem);
	if (shmTextToSpeechItem._flag != 1) {
		logger_error("shm item flag is [%d] not 1", shmTextToSpeechItem._flag);
		return FAILURE;
	}
	logger("client request:[%s], text:[%s]", request.c_str(), shmTextToSpeechItem._text);

	// 进行语音处理
	if (tts_sdk_part(shmTextToSpeechItem._text, sizeof(shmTextToSpeechItem._text), stream, _shmTextToSpeechHead, _shmTextToSpeech, atoi(tokens[1].c_str())) != MSP_SUCCESS) {
		logger_error("fail to call function tts_sdk");
		return FAILURE;
	}

	return SUCCESS;
}

// 基类虚函数：服务进程切换用户身份前调用此函数
void SpeechSynthesis::proc_pre_jail() {
	logger("proc_pre_jail\r\n");
}

// 基类虚函数：服务进程切换用户身份后调用此函数
void SpeechSynthesis::proc_on_init() {
	logger("proc init\r\n");
}

// 基类虚函数：服务进程退出前调用此函数
void SpeechSynthesis::proc_on_exit() {
	logger("proc exit\r\n");
}



/**
 * 参数设置
 */
static char *var_cfg_debug_msg;

static acl::master_str_tbl var_conf_str_tab[] = {
	{ "debug_msg", "test_msg", &var_cfg_debug_msg },

	{ 0, 0, 0 }
};

static int  var_cfg_debug_enable;

static acl::master_bool_tbl var_conf_bool_tab[] = {
	{ "debug_enable", 1, &var_cfg_debug_enable },

	{ 0, 0, 0 }
};

static int  var_cfg_io_timeout;

static acl::master_int_tbl var_conf_int_tab[] = {
	{ "io_timeout", 120, &var_cfg_io_timeout, 0, 0 },

	{ 0, 0 , 0 , 0, 0 }
};

int main(int argc, char* argv[]) {

	// 单利方式启动sever端
	SpeechSynthesis& mp = singleton2<SpeechSynthesis>::get_instance();

	mp.init();

	// 1.设置配置参数表
	mp.set_cfg_int(var_conf_int_tab);
	mp.set_cfg_int64(NULL);
	mp.set_cfg_str(var_conf_str_tab);
	mp.set_cfg_bool(var_conf_bool_tab);

	// 开始运行
	if (argc >= 2 && strcmp(argv[1], "alone") == 0) {
		mp.run_alone("127.0.0.1:5503", NULL, 5); // 单独运行方式
	} else {
		mp.run_daemon(argc, argv);  // acl_master 控制模式运行
	}

	return 0;
}
