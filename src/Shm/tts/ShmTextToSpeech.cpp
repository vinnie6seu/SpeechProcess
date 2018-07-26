/*
 * ShmTextToSpeech.cpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#include "Shm/tts/ShmTextToSpeech.hpp"

/******************************** tts shm  ********************************/

void ShmTextToSpeech::init(int shmKey, int shmTextToSpeechItemNum) {
	_shmKey = shmKey;
	_shmTextToSpeechItemNum = shmTextToSpeechItemNum;

	_shmSize = _shmTextToSpeechItemNum * sizeof(ShmTextToSpeechItem);
}

FUN_STATUS ShmTextToSpeech::writeTextRecordShm(void* shmMemPointStart, ShmTextToSpeechItem& shmTextToSpeechItem) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int totalRecordsNum = shmTextToSpeechHead->_total;                                  // 总的共享内存[文本-语音]记录数
	int freeRecordsNum = shmTextToSpeechHead->_free;                                    // 剩余量
	int pointRecordsIndex = (shmTextToSpeechHead->_pointIndex + 1) % totalRecordsNum;   // 上次写入文本的共享内存记录数组索引

	if (freeRecordsNum <= 0) {
		_lastErrorBuf = "write shm error. no space for write";
		return FAILURE;
	}

	// 最近能写入的
	int count = 0;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	while (count < freeRecordsNum) {
		if ((start + pointRecordsIndex)->_flag == 0) {
			break;
		}
		pointRecordsIndex = (pointRecordsIndex + 1) % totalRecordsNum;
		count++;
	}
	if (count >= freeRecordsNum) {
		_lastErrorBuf = "write shm error. no space for write";
		return FAILURE;
	}

	if (pthread_mutex_lock(&((start + pointRecordsIndex)->_itemMptr)) != 0) {

	}

	// 修改item中flag为1
	(start + pointRecordsIndex)->_flag = 1;

	if (pthread_mutex_lock(&(shmTextToSpeechHead->_headMptr)) != 0) {

	}

	// 修改head中free + 1
	shmTextToSpeechHead->_free = freeRecordsNum - 1;
	// 修改head中最近写入指向pointRecordsIndex
	shmTextToSpeechHead->_pointIndex = pointRecordsIndex;

	if (pthread_mutex_unlock(&(shmTextToSpeechHead->_headMptr)) != 0) {

	}

	if (pthread_mutex_unlock(&((start + pointRecordsIndex)->_itemMptr)) != 0) {

	}

	// 修改item中id
	(start + pointRecordsIndex)->_id = pointRecordsIndex;

	// 写入文本到item中
	memcpy((start + pointRecordsIndex)->_text, shmTextToSpeechItem._text, sizeof(shmTextToSpeechItem._text));

	// 告知调用者写在了那个index中
	shmTextToSpeechItem._id = pointRecordsIndex;
	shmTextToSpeechItem._flag = 1;

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::writeSpeechRecordShm(void* shmMemPointStart, int id, ShmTextToSpeechItem& shmTextToSpeechItem) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	// 在指定位置写语音数据
	memcpy((start + id)->_speech, shmTextToSpeechItem._speech, sizeof(shmTextToSpeechItem._speech));
	// 修改flag状态为2
	(start + id)->_flag = 2;

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::writeSpeechRecordShmPart(void* shmMemPointStart, int id, int speechOffset, int speechLen, ShmTextToSpeechItem& shmTextToSpeechItem) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	// 在指定位置写入部分语音数据
	memcpy((start + id)->_speech + speechOffset, shmTextToSpeechItem._speech + speechOffset, speechLen);

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::setSpeechRecordShmWriteOK(void* shmMemPointStart, int id) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	// 修改flag状态为2
	(start + id)->_flag = 2;

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::readTextRecordShm(void* shmMemPointStart, int id, ShmTextToSpeechItem& shmTextToSpeechItem) {
	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	// 读走item中共享内存文本数据
	memcpy(shmTextToSpeechItem._text, (start + id)->_text, sizeof(shmTextToSpeechItem._text));
	// 读走item中共享内存flag状态
	shmTextToSpeechItem._flag = (start + id)->_flag;

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::readSpeechRecordShm(void* shmMemPointStart, int id, ShmTextToSpeechItem& shmTextToSpeechItem) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	// 读走item中共享内存语音数据
	memcpy(shmTextToSpeechItem._speech, (start + id)->_speech, sizeof(shmTextToSpeechItem._speech));
	shmTextToSpeechItem._flag = (start + id)->_flag;

	if (pthread_mutex_lock(&((start + id)->_itemMptr)) != 0) {

	}

	// 修改item中flag状态为0
	(start + id)->_flag = 0;

	if (pthread_mutex_lock(&(shmTextToSpeechHead->_headMptr)) != 0) {

	}
	// 修改head中free + 1
	shmTextToSpeechHead->_free = shmTextToSpeechHead->_free + 1;

	if (pthread_mutex_unlock(&(shmTextToSpeechHead->_headMptr)) != 0) {

	}

	if (pthread_mutex_unlock(&((start + id)->_itemMptr)) != 0) {

	}

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::readSpeechRecordShmPart(void* shmMemPointStart, int id, int speechOffset, int speechLen, ShmTextToSpeechItem& shmTextToSpeechItem) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	// 读走item中共享内存语音数据
	memcpy(shmTextToSpeechItem._speech + speechOffset, (start + id)->_speech + speechOffset, speechLen);
	shmTextToSpeechItem._id = id;
	shmTextToSpeechItem._flag = (start + id)->_flag;

	return SUCCESS;
}

FUN_STATUS ShmTextToSpeech::setSpeechRecordShmReadOK(void* shmMemPointStart, int id) {

	ShmTextToSpeechHead* shmTextToSpeechHead = (ShmTextToSpeechHead*) shmMemPointStart;

	int offset = shmTextToSpeechHead->_offset;

	ShmTextToSpeechItem* start = (ShmTextToSpeechItem*) ((char*) shmMemPointStart + offset);

	if (pthread_mutex_lock(&((start + id)->_itemMptr)) != 0) {

	}

	// 修改item中flag状态为0
	(start + id)->_flag = 0;

	if (pthread_mutex_lock(&(shmTextToSpeechHead->_headMptr)) != 0) {

	}
	// 修改head中free + 1
	shmTextToSpeechHead->_free = shmTextToSpeechHead->_free + 1;

	if (pthread_mutex_unlock(&(shmTextToSpeechHead->_headMptr)) != 0) {

	}

	if (pthread_mutex_unlock(&((start + id)->_itemMptr)) != 0) {

	}

	return SUCCESS;
}
