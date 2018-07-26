/*
 * ShmSpeechToText.cpp
 *
 *  Created on: 2018年7月9日
 *      Author: yaoqiaobing
 */

#include "Shm/asr/ShmSpeechToText.hpp"

/******************************** asr shm  ********************************/

void ShmSpeechToText::init(int shmKey, int shmSpeechToTextItemNum) {
	_shmKey = shmKey;
	_shmSpeechToTextItemNum = shmSpeechToTextItemNum;

	_shmSize = _shmSpeechToTextItemNum * sizeof(ShmSpeechToTextItem);
}

FUN_STATUS ShmSpeechToText::writeSpeechRecordShm(void* shmMemPointStart, ShmSpeechToTextItem& shmSpeechToTextItem) {

	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int totalRecordsNum = shmSpeechToTextHead->_total;                                  // 总的共享内存[语音-文本]记录数
	int freeRecordsNum = shmSpeechToTextHead->_free;                                    // 剩余量
	int pointRecordsIndex = (shmSpeechToTextHead->_pointIndex + 1) % totalRecordsNum;   // 上次写入语音的共享内存记录数组索引

	if (freeRecordsNum <= 0) {
		_lastErrorBuf = "write shm error. no space for write";
		return FAILURE;
	}

	// 最近能写入的
	int count = 0;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	while (count < freeRecordsNum) {
		if ((start + pointRecordsIndex)->_flag == ITEM_FLAG_CAN_WRITE_SPEECH) {
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
	(start + pointRecordsIndex)->_flag = ITEM_FLAG_CALL_ASR_WORKING;

	if (pthread_mutex_lock(&(shmSpeechToTextHead->_headMptr)) != 0) {

	}

	// 修改head中free + 1
	shmSpeechToTextHead->_free = freeRecordsNum - 1;
	// 修改head中最近写入指向pointRecordsIndex
	shmSpeechToTextHead->_pointIndex = pointRecordsIndex;

	if (pthread_mutex_unlock(&(shmSpeechToTextHead->_headMptr)) != 0) {

	}

	if (pthread_mutex_unlock(&((start + pointRecordsIndex)->_itemMptr)) != 0) {

	}

	// 修改item中id
	(start + pointRecordsIndex)->_id = pointRecordsIndex;

	// 写入语音到item中
	memcpy((start + pointRecordsIndex)->_speech, shmSpeechToTextItem._speech, sizeof(shmSpeechToTextItem._speech));

	// 告知调用者写在了那个index中
	shmSpeechToTextItem._id = pointRecordsIndex;
	shmSpeechToTextItem._flag = ITEM_FLAG_CALL_ASR_WORKING;

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::writeTextRecordShm(void* shmMemPointStart, int id, ShmSpeechToTextItem& shmSpeechToTextItem) {

	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	// 在指定位置写文本数据
	memcpy((start + id)->_text, shmSpeechToTextItem._text, sizeof(shmSpeechToTextItem._text));
	// 修改flag状态为2
	(start + id)->_flag = ITEM_FLAG_CAN_GET_TEXT;

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::readSpeechRecordShm(void* shmMemPointStart, int id, ShmSpeechToTextItem& shmSpeechToTextItem) {
	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	// 读走item中共享内存语音数据
	memcpy(shmSpeechToTextItem._speech, (start + id)->_speech, sizeof(shmSpeechToTextItem._speech));
	// 读走item中共享内存flag状态
	shmSpeechToTextItem._flag = (start + id)->_flag;

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::readTextRecordShm(void* shmMemPointStart, int id, ShmSpeechToTextItem& shmSpeechToTextItem) {

	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	// 读走item中共享内存文本数据
	memcpy(shmSpeechToTextItem._text, (start + id)->_text, sizeof(shmSpeechToTextItem._text));
	shmSpeechToTextItem._flag = (start + id)->_flag;

	if (pthread_mutex_lock(&((start + id)->_itemMptr)) != 0) {

	}

	// 修改item中flag状态为0
	(start + id)->_flag = ITEM_FLAG_CAN_WRITE_SPEECH;

	if (pthread_mutex_lock(&(shmSpeechToTextHead->_headMptr)) != 0) {

	}
	// 修改head中free + 1
	shmSpeechToTextHead->_free = shmSpeechToTextHead->_free + 1;

	if (pthread_mutex_unlock(&(shmSpeechToTextHead->_headMptr)) != 0) {

	}

	if (pthread_mutex_unlock(&((start + id)->_itemMptr)) != 0) {

	}

	return SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

FUN_STATUS ShmSpeechToText::allItemShmQueueInit(void* shmMemPointStart,
		map<int, std::tr1::shared_ptr<NoBlockingShmQueue> >& allNoBlockingShmQueue,
		map<int, std::tr1::shared_ptr<BlockingShmQueue> >& allBlockingShmQueue) {

	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;
	int offset = shmSpeechToTextHead->_offset;
	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmSpeechToTextHead + offset);

	for (int index = 0; index < shmSpeechToTextHead->_total; index++) {
		// 构造共享内存块处理模式
//		std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue = std::make_shared<NoBlockingShmQueue>((start + index)->_speech, SPEECH_LEN, SPEECH_PACKET_LEN, &((start + index)->_speech_mutex));
//		std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue = std::make_shared<BlockingShmQueue>((start + index)->_text, TEXT_LEN, TEXT_PACKET_LEN, &((start + index)->_text_mutex), &((start + index)->_text_full_cond), &((start + index)->_text_empty_cond));

		std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue(new NoBlockingShmQueue((start + index)->_speech, SPEECH_LEN, SPEECH_PACKET_LEN, &((start + index)->_speech_mutex)));
		std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue(new BlockingShmQueue((start + index)->_text, TEXT_LEN, TEXT_PACKET_LEN, &((start + index)->_text_mutex), &((start + index)->_text_full_cond), &((start + index)->_text_empty_cond)));

		allNoBlockingShmQueue.insert(make_pair(index, noBlockingShmQueue));
		allBlockingShmQueue.insert(make_pair(index, blockingShmQueue));

//		NoBlockingShmQueue* noBlockingShmQueue = new NoBlockingShmQueue((start + index)->_speech, SPEECH_LEN, SPEECH_PACKET_LEN, &((start + index)->_speech_mutex));
//		BlockingShmQueue* blockingShmQueue = new BlockingShmQueue((start + index)->_text, TEXT_LEN, TEXT_PACKET_LEN, &((start + index)->_text_mutex), &((start + index)->_text_full_cond), &((start + index)->_text_empty_cond));
	}

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::findWriteSpeechDest(void* shmMemPointStart, int& id) {

	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int totalRecordsNum = shmSpeechToTextHead->_total;                                  // 总的共享内存[语音-文本]记录数
	int freeRecordsNum = shmSpeechToTextHead->_free;                                    // 剩余量
	int pointRecordsIndex = (shmSpeechToTextHead->_pointIndex + 1) % totalRecordsNum;   // 上次写入语音的共享内存记录数组索引

	if (freeRecordsNum <= 0) {
		_lastErrorBuf = "write shm error. no space for write";
		return FAILURE;
	}

	// 最近能写入的
	int count = 0;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	while (count < freeRecordsNum) {
		if ((start + pointRecordsIndex)->_flag == ITEM_FLAG_CAN_WRITE_SPEECH) {
			break;
		}
		pointRecordsIndex = (pointRecordsIndex + 1) % totalRecordsNum;
		count++;
	}
	if (count >= freeRecordsNum) {
		_lastErrorBuf = "write shm error. no space for write";
		return FAILURE;
	}



	{
		ThreadMutex shmSpeechToTextItemMutex(&((start + pointRecordsIndex)->_itemMptr));
		ThreadMutexGuard threadMutexGuard(&shmSpeechToTextItemMutex);

		if ((start + pointRecordsIndex)->_flag != ITEM_FLAG_CAN_WRITE_SPEECH) {
			_lastErrorBuf = "find dest not for write";
			return FAILURE;
		} else {
			// 修改item中flag为1
			(start + pointRecordsIndex)->_flag = ITEM_FLAG_CALL_ASR_WORKING;

			// 修改item中id
			(start + pointRecordsIndex)->_id = pointRecordsIndex;
		}

		{
			ThreadMutex shmSpeechToTextHeadMutex(&(shmSpeechToTextHead->_headMptr));
			ThreadMutexGuard threadMutexGuard(&shmSpeechToTextHeadMutex);

			// 修改head中free + 1
			shmSpeechToTextHead->_free = freeRecordsNum - 1;
			// 修改head中最近写入指向pointRecordsIndex
			shmSpeechToTextHead->_pointIndex = pointRecordsIndex;
		}
	}


	id = pointRecordsIndex;

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::writeSpeechNoBlockingQueue(std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, const char* speech, int len) {

	if (noBlockingShmQueue->isFull() == false) {
		noBlockingShmQueue->pushBack(speech, len);
	} else {
		_lastErrorBuf = "no space for write speech";
		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::writeTextBlockingQueue(void* shmMemPointStart, int id, std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue, const AsrSpeechTransResult& asr_trans_result) {
	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	// 写文本数据
	if (blockingShmQueue->isFull() == false) {
		blockingShmQueue->pushBack((char*) (&asr_trans_result), TEXT_PACKET_LEN);
	} else {
		_lastErrorBuf = "no space for write text";
		return FAILURE;
	}

	// 修改flag状态为2
	(start + id)->_flag = ITEM_FLAG_CAN_GET_TEXT;

	return SUCCESS;
}


FUN_STATUS ShmSpeechToText::readSpeechNoBlockingQueueBatch(std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, char* speech, int len, bool is_complete_send, int total_send_packet_num, int& batch_num, bool& is_complete_receive) {


	// 1.取出语音包
	// 2.告知java本次是否取到数据
	// 3.检查终止标志，清理

	// 批量取出数据，如果没有数据可以取，该函数略过 batch_num 为0
	noBlockingShmQueue->popFrontBatch(speech, len, batch_num);

	// 检查【发送完成标志comlete && 已收到包数量 == 总发送包数量】
	is_complete_receive = false;
	if (is_complete_send && (total_send_packet_num == noBlockingShmQueue->getReadSize())) {
		is_complete_receive = true;
		// 被清理后不会再次进入该清理
		noBlockingShmQueue->clear();
	}

	return SUCCESS;
}

//FUN_STATUS ShmSpeechToText::clearSpeechNoBlockingQueue(std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, bool is_complete, int total_send_packet_num) {
//
//	if (is_complete && (total_send_packet_num == noBlockingShmQueue->getReadSize())) {
//		noBlockingShmQueue->clear();
//	}
//
//	return SUCCESS;
//}


FUN_STATUS ShmSpeechToText::readTextBlockingQueueBatch(void* shmMemPointStart, int id, std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue, vector<AsrSpeechTransResult>& asr_trans_result_vec, bool& is_exception, ASR_SPEECH_PACK_STATUS_ENUM speechStatus) {
	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);


	AsrSpeechTransResult asr_trans_result_item;
	if (speechStatus == MSP_AUDIO_CONTINUE) {

		// 如果有数据包就取走
		while (blockingShmQueue->isEmpty() == false) {

			memset(&asr_trans_result_item, '\0', sizeof(AsrSpeechTransResult));
			blockingShmQueue->popFront((char*) (&asr_trans_result_item), TEXT_PACKET_LEN);

			asr_trans_result_vec.push_back(asr_trans_result_item);

			// 检查数据包是否是异常
			if (asr_trans_result_item._is_exception == true) {
				is_exception = true;
				return FAILURE;
			}
		}

	} else if (speechStatus == MSP_AUDIO_LAST) {

		// 1.阻塞取走所有数据包
		while (true) {
			memset(&asr_trans_result_item, '\0', sizeof(AsrSpeechTransResult));

			blockingShmQueue->popFront((char*) (&asr_trans_result_item), TEXT_PACKET_LEN);
			asr_trans_result_vec.push_back(asr_trans_result_item);

			// 检查数据包是否是异常
			if (asr_trans_result_item._is_exception == true) {
				is_exception = true;
				return FAILURE;
			}

			// 如果是最后一个数据包退出
			if (asr_trans_result_item._cur_result._is_final == true) {
				break;
			}
		}

		// 2.清理阻塞队列
		blockingShmQueue->clear();

		// 3.修改标识状态
		if (pthread_mutex_lock(&((start + id)->_itemMptr)) != 0) {

		}

		// 修改item中flag状态为0
		(start + id)->_flag = ITEM_FLAG_CAN_WRITE_SPEECH;

		if (pthread_mutex_lock(&(shmSpeechToTextHead->_headMptr)) != 0) {

		}
		// 修改head中free + 1
		shmSpeechToTextHead->_free = shmSpeechToTextHead->_free + 1;

		if (pthread_mutex_unlock(&(shmSpeechToTextHead->_headMptr)) != 0) {

		}

		if (pthread_mutex_unlock(&((start + id)->_itemMptr)) != 0) {

		}
	}

	return SUCCESS;
}

FUN_STATUS ShmSpeechToText::callBackExceptionHandle(void* shmMemPointStart, int id, std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue) {
	ShmSpeechToTextHead* shmSpeechToTextHead = (ShmSpeechToTextHead*) shmMemPointStart;

	int offset = shmSpeechToTextHead->_offset;

	ShmSpeechToTextItem* start = (ShmSpeechToTextItem*) ((char*) shmMemPointStart + offset);

	// 1.清理非阻塞队列
	noBlockingShmQueue->clear();

	// 2.清理阻塞队列
	blockingShmQueue->clear();


	// 3.修改标识状态
	if (pthread_mutex_lock(&((start + id)->_itemMptr)) != 0) {

	}

	// 修改item中flag状态为0
	(start + id)->_flag = ITEM_FLAG_CAN_WRITE_SPEECH;

	if (pthread_mutex_lock(&(shmSpeechToTextHead->_headMptr)) != 0) {

	}
	// 修改head中free + 1
	shmSpeechToTextHead->_free = shmSpeechToTextHead->_free + 1;

	if (pthread_mutex_unlock(&(shmSpeechToTextHead->_headMptr)) != 0) {

	}

	if (pthread_mutex_unlock(&((start + id)->_itemMptr)) != 0) {

	}

	return SUCCESS;
}
