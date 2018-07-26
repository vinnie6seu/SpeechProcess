/*
 * NoBlockingShmQueue.hpp
 *
 *  Created on: 2018年7月24日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SHM_NOBLOCKINGSHMQUEUE_HPP_
#define INCLUDE_SHM_NOBLOCKINGSHMQUEUE_HPP_

#include "Util/Thread/ThreadMutex.hpp"
#include "Util/Thread/ThreadMutexGuard.hpp"

class NoBlockingShmQueue {
public:
	/**
	 * 构造函数
	 */
	NoBlockingShmQueue(char* buffer, int buffer_len, int item_len, pthread_mutex_t* speech_mutex);

	/**
	 * 析构函数
	 */
	~NoBlockingShmQueue();

	/**
	 * 写入数据
	 */
	void pushBack(const char* data, int len);

	/**
	 * 取出数据
	 */
	void popFront(char* data, int len);

	/**
	 * 取出数据多个包
	 */
	void popFrontBatch(char* data, int len, int& batch_num);

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
	ThreadMutex _threadMutex;             // 保证队列操作线程安全

	char* _buffer;                        // 共享内存可写位置
	int* _buffer_len;                     // 该块共享内存长度
	int* _item_len;                       // 往该块共享内存写入的包数据长度
	int* _max_size;                       // 该块共享内存可以存放的包数量 _buffer_len / _item_len
	int* _cur_write_index;                // 当前写入的位置
	int* _cur_read_index;                 // 当前读到的位置
};

#endif /* INCLUDE_SHM_NOBLOCKINGSHMQUEUE_HPP_ */
