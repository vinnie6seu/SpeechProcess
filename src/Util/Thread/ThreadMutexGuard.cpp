/*
 * ThreadMutexGuard.cpp
 *
 *  Created on: 2018年7月16日
 *      Author: yaoqiaobing
 */

#include <assert.h>

#include "Util/Thread/ThreadMutex.hpp"
#include "Util/Thread/ThreadMutexGuard.hpp"

ThreadMutexGuard::ThreadMutexGuard(ThreadMutex* threadMutex) : _threadMutex(threadMutex) {
	try {
		_threadMutex->lock();
	} catch (ThrowableException& e) {
		assert(false);
	}
}

ThreadMutexGuard::~ThreadMutexGuard(void) {
	try {
		_threadMutex->unLock();
	} catch (ThrowableException& e) {
		assert(false);
	}
}
