/*
 * ThreadMutexGuard.hpp
 *
 *  Created on: 2018年7月16日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_THREAD_THREADMUTEXGUARD_HPP_
#define INCLUDE_UTIL_THREAD_THREADMUTEXGUARD_HPP_

class ThreadMutex;

class ThreadMutexGuard {
public:
	/**
	 * 构造函数
	 */
	ThreadMutexGuard(ThreadMutex* threadMutex);

	/**
	 * 析构函数
	 */
	~ThreadMutexGuard(void);
private:
	/**
	 * muduo C++ 要求禁止该类拷贝构造和赋值
	 */
	ThreadMutexGuard(const ThreadMutexGuard&);
	ThreadMutexGuard& operator=(const ThreadMutexGuard&);

	mutable ThreadMutex* _threadMutex;
};

#endif /* INCLUDE_UTIL_THREAD_THREADMUTEXGUARD_HPP_ */
