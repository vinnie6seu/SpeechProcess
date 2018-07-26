/*
 * BlockingShmQueue.hpp
 *
 *  Created on: 2018年7月23日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SHM_BLOCKINGSHMQUEUE_HPP_
#define INCLUDE_SHM_BLOCKINGSHMQUEUE_HPP_

#include "Util/Thread/ThreadMutex.hpp"
#include "Util/Thread/ThreadMutexGuard.hpp"
#include "Util/Thread/ThreadCond.hpp"

class BlockingShmQueue {
public:
	/**
	 * 构造函数
	 */
	BlockingShmQueue(char* buffer, int buffer_len, int item_len, pthread_mutex_t* text_mutex, pthread_cond_t* text_full_cond, pthread_cond_t* text_empty_cond);

	/**
	 * 析构函数
	 */
	~BlockingShmQueue();

	/**
	 * 写入数据
	 */
	void pushBack(const char* data, int len);

	/**
	 * 取出数据
	 */
	void popFront(char* data, int len);

	/**
	 * 重置清理
	 */
	void clear();

	/**
	 * 当前未被取走的包数量
	 */
	int size();

	/**
	 * 是否包数据都被取走
	 */
	bool isEmpty();

	/**
	 * 空间满了
	 */
	bool isFull();

	/**
	 * 获取已读数据量
	 */
	int getReadSize();

	/**
	 * 获取已写数据量
	 */
	int getWriteSize();

private:
	ThreadMutex _threadMutex;             // 保证队列操作安全
	ThreadCond _full_threadCond;          // 用于阻塞写
	ThreadCond _empty_threadCond;         // 用于阻塞读

	char* _buffer;                        // 共享内存可写位置
	int* _buffer_len;                     // 该块共享内存长度
	int* _item_len;                       // 往该块共享内存写入的包数据长度
	int* _max_size;                       // 该块共享内存可以存放的包数量 _buffer_len / _item_len
	int* _cur_write_index;                // 当前写入的位置
	int* _cur_read_index;                 // 当前读到的位置
};

#endif /* INCLUDE_SHM_BLOCKINGSHMQUEUE_HPP_ */
