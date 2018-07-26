/*
 * ShmSpeechToText.hpp
 *
 *  Created on: 2018年7月9日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SHM_SHMSPEECHTOTEXT_HPP_
#define INCLUDE_SHM_SHMSPEECHTOTEXT_HPP_

#include <tr1/memory>
#include <pthread.h>
#include <map>

#include "Shm/ShmBase.hpp"
#include "Shm/NoBlockingShmQueue.hpp"
#include "Shm/BlockingShmQueue.hpp"
#include "Util/EnumUtil.hpp"

/******************************** asr shm  ********************************/

/**
 * 将相应的枚举类型转为字符串(这种方法很重要)
 */
#define FOREACH_SHM_ASR_ITEM_FLAG(CMD) \
        CMD(ITEM_FLAG_CAN_WRITE_SPEECH) \
		CMD(ITEM_FLAG_CALL_ASR_WORKING) \
		CMD(ITEM_FLAG_CAN_GET_TEXT)

/**
 * 枚举：asr 在共享内存中单条 item 的 flag 表示
 */
typedef enum {
	FOREACH_SHM_ASR_ITEM_FLAG(GENERATE_ENUM)
} SHM_ASR_ITEM_FLAG_ENUM;

/**
 * 数组：asr 在共享内存中单条 item 的 flag 表示，用于将枚举转成字符串
 * 需要加入 static 修饰符，不然该头文件被多次 include 会报错该数组重复定义
 */
static const char* SHM_ASR_ITEM_FLAG_ENUM_STRING[] = {
	FOREACH_SHM_ASR_ITEM_FLAG(GENERATE_STRING)
};

/////////////////////////////////////////////////////////////////////

/**
 * 将相应的枚举类型转为字符串(这种方法很重要)
 */
#define FOREACH_ASR_SPEECH_PACK_STATUS(CMD) \
        CMD(MSP_AUDIO_INIT) \
		CMD(MSP_AUDIO_CONTINUE) \
		CMD(MSP_AUDIO_LAST)

/**
 * 枚举：asr 发送语音包状态
 */
typedef enum {
	FOREACH_ASR_SPEECH_PACK_STATUS(GENERATE_ENUM)
} ASR_SPEECH_PACK_STATUS_ENUM;

/**
 * 数组：asr 发送语音包状态，用于将枚举转成字符串
 * 需要加入 static 修饰符，不然该头文件被多次 include 会报错该数组重复定义
 */
static const char* ASR_SPEECH_PACK_STATUS_ENUM_STRING[] = {
	FOREACH_ASR_SPEECH_PACK_STATUS(GENERATE_STRING)
};

/////////////////////////////////////////////////////////////////////

/**
 * 谷歌 asr 根据每个语音片段转义返回的数据包，一般谷歌一次返回两个包，一个是转义结果 result，一个是预测结果 predict
 */
typedef struct {
	char _transcript[128 + 1];          // 文本数据
	double _stability;                  // 本包数据的稳定性，一般 result 的较高， predict 的较低

	bool _is_final;                     // 谷歌接口以 onComplete 为界，返回的最后结果包，_is_final 为 true，中间过程均是 false
	double _confidence;                 // 最终结果的准确度
} AsrSpeechTransItem;

/**
 * java 服务端给客户端返回的数据
 */
typedef struct {
	AsrSpeechTransItem _cur_result;     // 转义中间过程、最终结果放在其中

	AsrSpeechTransItem _cur_predict;    // 预测结果放在其中

	bool _is_exception;                 // 如果谷歌接口出现异常则 _is_exception 为true，否则为false
	char _exception_str[1024 + 1];      // 谷歌接口告知的异常原因
} AsrSpeechTransResult;

/////////////////////////////////////////////////////////////////////

const int SPEECH_LEN = 638014 + 1;
const int SPEECH_PACKET_LEN = 3200;

const int TEXT_LEN = 1024 + 1;
const int TEXT_PACKET_LEN = sizeof(AsrSpeechTransResult);

/////////////////////////////////////////////////////////////////////

// 共享内存所存一条数据
typedef struct ShmSpeechToTextItem {
	int _id;                                 // 数组编号，从0开始
	SHM_ASR_ITEM_FLAG_ENUM _flag;            // 0可写入_speech，1正在调用 asr 转成 _text，2可以取用 _text
	pthread_mutex_t _itemMptr;               // 行锁控制修改_flag
	char _speech[SPEECH_LEN];                // tts转成的目标语音
	char _text[TEXT_LEN];                    // 输入文本


	pthread_mutex_t _speech_mutex;

	pthread_mutex_t _text_mutex;
	pthread_cond_t _text_full_cond;
	pthread_cond_t _text_empty_cond;

} ShmSpeechToTextItem;

typedef struct ShmSpeechToTextHead {
	int _total;                              // 总的共享内存[语音-文本]记录数
	int _free;                               // 剩余量
	int _pointIndex;                         // 上次写入语音的共享内存记录数组索引
	int _offset;                             // 等待分配空间的记录数组与起始位置偏移

	pthread_mutex_t _headMptr;               // 修改共享内存头部锁

	int _shmSize;                            // 分配的共享内存块大小
	int _itemSize;                           // 单条记录大小
} ShmSpeechToTextHead;


class ShmSpeechToText : public ShmBase {
public:
	/*
	 * 初始化key和共享内存大小
	 */
    void init(int shmKey, int shmSpeechToTextItemNum);

	/*
	 * 共享内存存放总记录数目
	 */
	int getShmSpeechToTextItemNum() {
		return _shmSpeechToTextItemNum;
	}

	/*
	 * 1.找到空闲区域flag == 0写入语音记录
	 * 2.修改item的flag为1
	 * 3.修改head的free和pointIndex
	 */
 	FUN_STATUS writeSpeechRecordShm(void* shmMemPointStart, ShmSpeechToTextItem& shmSpeechToTextItem);

 	/**
 	 * 1.在指定位置写入文本数据
 	 * 2.修改item的flag为2
 	 */
 	FUN_STATUS writeTextRecordShm(void* shmMemPointStart, int id, ShmSpeechToTextItem& shmSpeechToTextItem);

 	/**
 	 * 1.在指定位置读取语音数据
 	 */
 	FUN_STATUS readSpeechRecordShm(void* shmMemPointStart, int id, ShmSpeechToTextItem& shmSpeechToTextItem);

	/*
	 * 1.取走处理完生成的文本数据
	 * 2.修改item的flag为0
	 * 3.修改head的free
	 */
 	FUN_STATUS readTextRecordShm(void* shmMemPointStart, int id, ShmSpeechToTextItem& shmSpeechToTextItem);

 	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * 初始化所有内存块的操作
	 */
 	FUN_STATUS allItemShmQueueInit(void* shmMemPointStart, map<int, std::tr1::shared_ptr<NoBlockingShmQueue> >& allNoBlockingShmQueue,
			map<int, std::tr1::shared_ptr<BlockingShmQueue> >& allBlockingShmQueue);

	/*
	 * 1.找到空闲区域flag == 0用于写入语音记录
	 * 2.修改item的flag为1
	 * 3.修改head的free和pointIndex
	 */
 	FUN_STATUS findWriteSpeechDest(void* shmMemPointStart, int& id);

 	// 往非阻塞 队列中写 speech 数据
 	FUN_STATUS writeSpeechNoBlockingQueue(std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, const char* speech, int len);

 	/**
 	 * 1.在指定位置写入文本数据
 	 * 2.修改item的flag为2
 	 */
 	FUN_STATUS writeTextBlockingQueue(void* shmMemPointStart, int id, std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue, const AsrSpeechTransResult& asr_trans_result);

 	/**
 	 * 1.在指定位置读取语音数据，得到语音数据、读到的语音包数、是否全部读完了[发送完成标志comlete && 已收到包数量 == 总发送包数量]
 	 */
 	FUN_STATUS readSpeechNoBlockingQueueBatch(std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, char* speech, int len, bool is_complete_send, int total_send_packet_num, int& batch_num, bool& is_complete_receive);

// 	FUN_STATUS clearSpeechNoBlockingQueue(std::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, bool is_complete, int total_send_packet_num);

	/*
	 * 1.MSP_AUDIO_CONTINUE：如果有数据包就取走，如果遇到异常就返回失败
	 * 2.MSP_AUDIO_LAST：阻塞取走所有数据包，直到is_final，如果遇到异常就返回失败；最后清理阻塞队列，修改item的flag为0，修改head的free
	 */
 	FUN_STATUS readTextBlockingQueueBatch(void* shmMemPointStart, int id, std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue, vector<AsrSpeechTransResult>& asr_trans_result_vec, bool& is_exception, ASR_SPEECH_PACK_STATUS_ENUM speechStatus);

 	// 收到异常结果进行一些清理
 	// 1.clear队列状态
 	// 2.修改item的flag为0
 	// 3.修改head的free
 	FUN_STATUS callBackExceptionHandle(void* shmMemPointStart, int id, std::tr1::shared_ptr<NoBlockingShmQueue> noBlockingShmQueue, std::tr1::shared_ptr<BlockingShmQueue> blockingShmQueue);

private:

	int _shmSpeechToTextItemNum;                // 配置文件设置的共享内存存储总行数
};

#endif /* INCLUDE_SHM_SHMSPEECHTOTEXT_HPP_ */
