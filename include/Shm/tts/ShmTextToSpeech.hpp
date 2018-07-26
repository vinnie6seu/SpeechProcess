/*
 * ShmTextToSpeech.hpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SPEECHSYNTHESIS_SHMTEXTTOSPEECH_HPP_
#define INCLUDE_SPEECHSYNTHESIS_SHMTEXTTOSPEECH_HPP_

#include <pthread.h>
#include "Shm/ShmBase.hpp"
#include "Util/EnumUtil.hpp"

/******************************** tts shm  ********************************/

// 共享内存所存一条数据
typedef struct ShmTextToSpeechItem {
	int _id;                                 // 数组编号，从0开始
	int _flag;                               // 0可写入_text，1正在调用tts转成_speech，2可以取用_speech
	pthread_mutex_t _itemMptr;               // 行锁控制修改_flag
	char _text[256 + 1];                     // 输入文本
	char _speech[251552 + 1];                // tts转成的目标语音
} ShmTextToSpeechItem;

typedef struct ShmTextToSpeechHead {
	int _total;                              // 总的共享内存[文本-语音]记录数
	int _free;                               // 剩余量
	int _pointIndex;                         // 上次写入文本的共享内存记录数组索引
	int _offset;                             // 等待分配空间的记录数组与起始位置偏移
	
	pthread_mutex_t _headMptr;               // 修改共享内存头部锁

	int _shmSize;                            // 分配的共享内存块大小
	int _itemSize;                           // 单条记录大小
} ShmTextToSpeechHead;


class ShmTextToSpeech : public ShmBase {
public:
	/*
	 * 初始化key和共享内存大小
	 */
    void init(int shmKey, int shmTextToSpeechItemNum);

	/*
	 * 共享内存存放总记录数目
	 */
	int getShmTextToSpeechItemNum() {
		return _shmTextToSpeechItemNum;
	}

	/*
	 * 1.找到空闲区域flag == 0写入文本记录
	 * 2.修改item的flag为1
	 * 3.修改head的free和pointIndex
	 */
 	FUN_STATUS writeTextRecordShm(void* shmMemPointStart, ShmTextToSpeechItem& shmTextToSpeechItem);

 	/**
 	 * 1.在指定位置写入语音数据
 	 * 2.修改item的flag为2
 	 */
 	FUN_STATUS writeSpeechRecordShm(void* shmMemPointStart, int id, ShmTextToSpeechItem& shmTextToSpeechItem);

 	/**
 	 * ------------------------------------------------
 	 * 1.在指定位置写入一部分语音数据
 	 */
 	FUN_STATUS writeSpeechRecordShmPart(void* shmMemPointStart, int id, int speechOffset, int speechLen, ShmTextToSpeechItem& shmTextToSpeechItem);

 	/**
 	 * 2.修改item的flag为2
 	 * ------------------------------------------------
 	 */
 	FUN_STATUS setSpeechRecordShmWriteOK(void* shmMemPointStart, int id);

 	/**
 	 * 1.在指定位置读取文本数据数据
 	 */
 	FUN_STATUS readTextRecordShm(void* shmMemPointStart, int id, ShmTextToSpeechItem& shmTextToSpeechItem);

	/*
	 * 1.取走处理完生成的语音数据
	 * 2.修改item的flag为0
	 * 3.修改head的free
	 */
 	FUN_STATUS readSpeechRecordShm(void* shmMemPointStart, int id, ShmTextToSpeechItem& shmTextToSpeechItem);

 	/**
 	 * ------------------------------------------------
 	 * 1.取走处理完生成的部分语音数据
 	 */
 	FUN_STATUS readSpeechRecordShmPart(void* shmMemPointStart, int id, int speechOffset, int speechLen, ShmTextToSpeechItem& shmTextToSpeechItem);

 	/**
	 * 2.修改item的flag为0
	 * 3.修改head的free
 	 * ------------------------------------------------
 	 */
 	FUN_STATUS setSpeechRecordShmReadOK(void* shmMemPointStart, int id);

private:

	int _shmTextToSpeechItemNum;                // 配置文件设置的共享内存存储总行数
};

#endif /* INCLUDE_SPEECHSYNTHESIS_SHMTEXTTOSPEECH_HPP_ */
