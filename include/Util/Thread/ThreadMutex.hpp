/*
 * ThreadMutex.hpp
 *
 *  Created on: 2018年7月15日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_THREAD_THREADMUTEX_HPP_
#define INCLUDE_UTIL_THREAD_THREADMUTEX_HPP_

#include <pthread.h>
#include "Util/Exception/throw.hpp"

/**
 * 线程互斥锁
 */
class ThreadMutex {
public:
	/**
	 * 构造函数
	 */
	ThreadMutex(pthread_mutex_t* mutex);

	/**
	 * 析构函数
	 */
	~ThreadMutex(void);

	/**
	 * 对线程锁进行加锁，一直到加锁成功或内部失败(一般不会失败，除非是系统问题)
	 * @exception {ThrowableException} 加锁有问题
	 */
	void lock(void) throw(ThrowableException);

	/**
	 * 尝试性加锁，无论成功与否都会立即返回
	 * @exception {ThrowableException} 表示加锁失败
	 */
	void tryLock(void) throw(ThrowableException);

	/**
	 * 解线程锁
	 * @exception {ThrowableException} 解锁有问题，有可能之前并未加锁成功所致
	 */
	void unLock(void) throw(ThrowableException);

	/**
	 * 获得 acl 中 C 版本的系统类型的线程锁
	 * @return {pthread_mutex_t*}
	 */
	pthread_mutex_t* getMutex(void);

private:
	/**
	 * muduo C++ 要求禁止该类拷贝构造和赋值
	 */
	ThreadMutex(const ThreadMutex&);
	ThreadMutex& operator=(const ThreadMutex&);

	pthread_mutex_t* _mutex;
};

#endif /* INCLUDE_UTIL_THREAD_THREADMUTEX_HPP_ */
