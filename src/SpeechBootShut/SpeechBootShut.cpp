/*
 * SpeechBootShut.cpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#include <vector>
#include <string>
#include <pthread.h> 

#include "lib_acl.hpp"
#include "acl_cpp_init.hpp"

#include "SpeechBootShut/SpeechBootShut.hpp"

using namespace acl;
using namespace std;

SpeechBootShut::SpeechBootShut() : _cfgUtil("SpeechProcess.cfg") {
	_init_flag = false;
}

SpeechBootShut::~SpeechBootShut() {

}

SpeechBootShut& SpeechBootShut::init() {
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
	logFullPathArr["SpeechBootShut"] = "";
	_cfgUtil.getLogFullPath(logFullPathArr);
	for (map<std::string, std::string>::iterator it = logFullPathArr.begin(); it != logFullPathArr.end(); it++) {
		acl::log::open((*it).second.c_str(), (*it).first.c_str());

		logger("%s log is open....!", (*it).first.c_str());
	}

	// 3.初始化 tts 共享内存
	int shmTextToSpeechKey = _cfgUtil.getShmTextToSpeechKey();
	int shmTextToSpeechNum = _cfgUtil.getShmTextToSpeechNum();
	_shmTextToSpeech.init(shmTextToSpeechKey, shmTextToSpeechNum);

	logger("init shm:[text to speech], key:[%d], num:[%d]", shmTextToSpeechKey, shmTextToSpeechNum);

	// 4.初始化 asr 共享内存
	int shmSpeechToTextKey = _cfgUtil.getShmSpeechToTextKey();
	int shmSpeechToTextNum = _cfgUtil.getShmSpeechToTextNum();
	_shmSpeechToText.init(shmSpeechToTextKey, shmSpeechToTextNum);

	logger("init shm:[speech to text], key:[%d], num:[%d]", shmSpeechToTextKey, shmSpeechToTextNum);

	_init_flag = true;

	return *this;
}

FUN_STATUS SpeechBootShut::shmTextToSpeechCreate() {
	// 创建共享内存
	logger("start to create shm:[text to speech]......");

	if (_shmTextToSpeech.createShm() == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		return FAILURE;
	}

	logger("shm:[text to speech], key:[%d], shmid:[%d], bytes:[%d]",
			_shmTextToSpeech.getShmKey(), _shmTextToSpeech.getShmId(),
			_shmTextToSpeech.getShmSize());

	// 将共享内存加载到进程中，修改共享内存头部
	ShmTextToSpeechHead* shmTextToSpeechHead = NULL;
	if (_shmTextToSpeech.procLoadShm((void**) &shmTextToSpeechHead) == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		return FAILURE;
	}

	shmTextToSpeechHead->_total = _shmTextToSpeech.getShmTextToSpeechItemNum(); // 总的共享内存[文本-语音]记录数
	shmTextToSpeechHead->_free = shmTextToSpeechHead->_total;             // 剩余量
	shmTextToSpeechHead->_pointIndex = -1;                   // 上次写入文本的共享内存记录数组索引
	shmTextToSpeechHead->_offset = sizeof(ShmTextToSpeechHead);

	// 初始化锁用于修改共享内存头部
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	if (pthread_mutex_init(&shmTextToSpeechHead->_headMptr, &mattr) != 0) {
		logger("fail to call pthread_mutex_init for _headMptr ......");
	}

	shmTextToSpeechHead->_shmSize = _shmTextToSpeech.getShmSize(); // 分配的共享内存块大小
	shmTextToSpeechHead->_itemSize = sizeof(ShmTextToSpeechItem);      // 单条记录大小

	// 初始化所有记录的互斥量
	ShmTextToSpeechItem* start =
			(ShmTextToSpeechItem*) ((char*) shmTextToSpeechHead
					+ shmTextToSpeechHead->_offset);

	for (int id = 0; id < shmTextToSpeechHead->_total; id++) {

		if (pthread_mutex_init(&((start + id)->_itemMptr), &mattr) != 0) {
			logger("fail to call pthread_mutex_init for _itemMptr ......");
		}
	}

	logger("end to create shm:[text to speech]......");


	if (_shmTextToSpeech.procDetachShm((void*) shmTextToSpeechHead) == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS SpeechBootShut::shmTextToSpeechDelete() {
	// 删除共享内存
	logger("start to delete shm:[text to speech]......");

	ShmTextToSpeechHead* shmTextToSpeechHead = NULL;
	if (_shmTextToSpeech.procLoadShm((void**) &shmTextToSpeechHead) == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		return FAILURE;
	}

	logger("shm:[text to speech], total:[%d], free:[%d], pointIndex:[%d], offset:[%d], shmSize:[%d], itemSize:[%d]",
			shmTextToSpeechHead->_total, shmTextToSpeechHead->_free,
			shmTextToSpeechHead->_pointIndex, shmTextToSpeechHead->_offset,
			shmTextToSpeechHead->_shmSize, shmTextToSpeechHead->_itemSize);

	// !!!!!!!!!!考虑摧毁互斥量

	if (_shmTextToSpeech.procDetachShm((void*) shmTextToSpeechHead) == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		return FAILURE;
	}

	// 正式删除共享内存
	if (_shmTextToSpeech.deleteShm() == FAILURE) {
		logger_error(_shmTextToSpeech.getShmLastError().c_str());
		return FAILURE;
	}

	logger("end to delete shm:[text to speech]......");

	return SUCCESS;
}

FUN_STATUS SpeechBootShut::shmSpeechToTextCreate() {
	// 创建共享内存
	logger("start to create shm:[speech to text]......");

	if (_shmSpeechToText.createShm() == FAILURE) {
		logger_error(_shmSpeechToText.getShmLastError().c_str());
		return FAILURE;
	}

	logger("shm:[speech to text], key:[%d], shmid:[%d], bytes:[%d]",
			_shmSpeechToText.getShmKey(), _shmSpeechToText.getShmId(),
			_shmSpeechToText.getShmSize());

	// 将共享内存加载到进程中，修改共享内存头部
	ShmSpeechToTextHead* shmSpeechToTextHead = NULL;
	if (_shmSpeechToText.procLoadShm((void**) &shmSpeechToTextHead) == FAILURE) {
		logger_error(_shmSpeechToText.getShmLastError().c_str());
		return FAILURE;
	}

	shmSpeechToTextHead->_total = _shmSpeechToText.getShmSpeechToTextItemNum(); // 总的共享内存[语音-文本]记录数
	shmSpeechToTextHead->_free = shmSpeechToTextHead->_total;                   // 剩余量
	shmSpeechToTextHead->_pointIndex = -1;                                       // 上次写入语音的共享内存记录数组索引
	shmSpeechToTextHead->_offset = sizeof(ShmSpeechToTextHead);

	// 初始化锁用于修改共享内存头部
	pthread_mutexattr_t mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	if (pthread_mutex_init(&shmSpeechToTextHead->_headMptr, &mattr) != 0) {
		logger("fail to call pthread_mutex_init for _headMptr ......");
	}

	shmSpeechToTextHead->_shmSize = _shmSpeechToText.getShmSize();     // 分配的共享内存块大小
	shmSpeechToTextHead->_itemSize = sizeof(ShmSpeechToTextItem);      // 单条记录大小

	// 初始化所有记录的互斥量
	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmSpeechToTextHead + shmSpeechToTextHead->_offset);

	for (int id = 0; id < shmSpeechToTextHead->_total; id++) {
        // 1.
		if (pthread_mutex_init(&((start + id)->_itemMptr), &mattr) != 0) {
			logger("fail to call pthread_mutex_init for _itemMptr ......");
		}

		// 2.
		if (pthread_mutex_init(&((start + id)->_speech_mutex), &mattr) != 0) {
			logger("fail to call pthread_mutex_init for _speech_mutex ......");
		}

		// 3.
		if (pthread_mutex_init(&((start + id)->_text_mutex), &mattr) != 0) {
			logger("fail to call pthread_mutex_init for _text_mutex ......");
		}

		// 4.
		if (pthread_cond_init(&((start + id)->_text_full_cond), NULL) != 0) {
			logger("fail to call pthread_cond_init for _text_full_cond ......");
		}

		// 5.
		if (pthread_cond_init(&((start + id)->_text_empty_cond), NULL) != 0) {
			logger("fail to call pthread_cond_init for _text_empty_cond ......");
		}
	}

	logger("end to create shm:[speech to text]......");


	if (_shmSpeechToText.procDetachShm((void*) shmSpeechToTextHead) == FAILURE) {
		logger_error(_shmSpeechToText.getShmLastError().c_str());
		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS SpeechBootShut::shmSpeechToTextDelete() {
	// 删除共享内存
	logger("start to delete shm:[speech to text]......");

	ShmSpeechToTextHead* shmSpeechToTextHead = NULL;
	if (_shmSpeechToText.procLoadShm((void**) &shmSpeechToTextHead) == FAILURE) {
		logger_error(_shmSpeechToText.getShmLastError().c_str());
		return FAILURE;
	}

	logger("shm:[speech to text], total:[%d], free:[%d], pointIndex:[%d], offset:[%d], shmSize:[%d], itemSize:[%d]",
			shmSpeechToTextHead->_total, shmSpeechToTextHead->_free,
			shmSpeechToTextHead->_pointIndex, shmSpeechToTextHead->_offset,
			shmSpeechToTextHead->_shmSize, shmSpeechToTextHead->_itemSize);

	// !!!!!!!!!!考虑摧毁互斥量
	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmSpeechToTextHead + shmSpeechToTextHead->_offset);

	for (int id = 0; id < shmSpeechToTextHead->_total; id++) {
        // 1.
		if (pthread_mutex_destroy(&((start + id)->_itemMptr)) != 0) {
			logger("fail to call pthread_mutex_destroy for _itemMptr ......");
		}

		// 2.
		if (pthread_mutex_destroy(&((start + id)->_speech_mutex)) != 0) {
			logger("fail to call pthread_mutex_destroy for _speech_mutex ......");
		}

		// 3.
		if (pthread_mutex_destroy(&((start + id)->_text_mutex)) != 0) {
			logger("fail to call pthread_mutex_destroy for _text_mutex ......");
		}

		// 4.
		if (pthread_cond_destroy(&((start + id)->_text_full_cond)) != 0) {
			logger("fail to call pthread_cond_destroy for _text_full_cond ......");
		}

		// 5.
		if (pthread_cond_destroy(&((start + id)->_text_empty_cond)) != 0) {
			logger("fail to call pthread_cond_destroy for _text_empty_cond ......");
		}
	}
	// !!!!!!!!!!考虑摧毁互斥量

	if (_shmSpeechToText.procDetachShm((void*) shmSpeechToTextHead) == FAILURE) {
		logger_error(_shmSpeechToText.getShmLastError().c_str());
		return FAILURE;
	}

	// 正式删除共享内存
	if (_shmSpeechToText.deleteShm() == FAILURE) {
		logger_error(_shmSpeechToText.getShmLastError().c_str());
		return FAILURE;
	}

	logger("end to delete shm:[speech to text]......");

	return SUCCESS;
}
