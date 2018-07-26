/*
 * BlockingShmQueue.cpp
 *
 *  Created on: 2018年7月24日
 *      Author: yaoqiaobing
 */


#include <string.h>

#include "Shm/BlockingShmQueue.hpp"

BlockingShmQueue::BlockingShmQueue(char* buffer, int buffer_len, int item_len,
		pthread_mutex_t* text_mutex, pthread_cond_t* text_full_cond,
		pthread_cond_t* text_empty_cond) :
		_buffer(buffer), _threadMutex(text_mutex),
		_full_threadCond(&_threadMutex, text_full_cond),
		_empty_threadCond(&_threadMutex, text_empty_cond),
		_buffer_len(NULL), _item_len(NULL), _max_size(NULL), _cur_write_index(NULL), _cur_read_index(NULL) {

	// 清理
	memset(buffer, '\0', buffer_len);

	// 在该块共享内存头部分配下面变量的空间
	int* ptr = (int*) _buffer;

	_buffer_len = ptr;                              // 该块共享内存长度
	_item_len = ptr + 1;                            // 往该块共享内存写入的包数据长度
	_max_size = ptr + 2;                            // 该块共享内存可以存放的包数量 _buffer_len / _item_len
	_cur_write_index = ptr + 3;                     // 当前写入的位置
	_cur_read_index = ptr + 4;                      // 当前读到的位置


    // 为变量初始化
	_buffer = (char*) (ptr + 5);
	*_buffer_len = buffer_len - sizeof(int) * 5;
	*_item_len = item_len;

	*_max_size = (*_buffer_len) / (*_item_len);     // 该块共享内存可以存放的包数量 _buffer_len / _item_len
	*_cur_write_index = 0;                          // 当前写入的位置
	*_cur_read_index = 0;                           // 当前读到的位置

}

BlockingShmQueue::~BlockingShmQueue() {
}

void BlockingShmQueue::pushBack(const char* data, int len) {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);

    while ((*_cur_write_index - *_cur_read_index) == *_max_size) {
    	_empty_threadCond.wait();
    }

	memcpy(_buffer + (*_cur_write_index) * (*_item_len), data, *_item_len);
	(*_cur_write_index)++;

	_full_threadCond.notify();
}

void BlockingShmQueue::popFront(char* data, int len) {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);

    while ((*_cur_write_index - *_cur_read_index) == 0) {
    	_full_threadCond.wait();
    }

	memcpy(data, _buffer + (*_cur_read_index) * (*_item_len), *_item_len);
	(*_cur_read_index)++;

	_empty_threadCond.notify();
}

void BlockingShmQueue::clear() {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);
	*_cur_write_index = 0;                // 当前写入的位置
	*_cur_read_index = 0;                 // 当前读到的位置

	memset(_buffer, '\0', *_buffer_len);
}

int BlockingShmQueue::size() {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);
	return (*_cur_write_index - *_cur_read_index);
}

bool BlockingShmQueue::isEmpty() {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);
	return (*_cur_write_index - *_cur_read_index) == 0;
}

bool BlockingShmQueue::isFull() {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);
	return (*_cur_write_index - *_cur_read_index) == *_max_size;
}

int BlockingShmQueue::getReadSize() {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);
	return *_cur_read_index;
}

int BlockingShmQueue::getWriteSize() {
	ThreadMutexGuard threadMutexGuard(&_threadMutex);
	return *_cur_write_index;
}
