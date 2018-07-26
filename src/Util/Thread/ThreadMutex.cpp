/*
 * ThreadMutex.cpp
 *
 *  Created on: 2018年7月15日
 *      Author: yaoqiaobing
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "Util/Thread/ThreadMutex.hpp"


ThreadMutex::ThreadMutex(pthread_mutex_t* mutex) : _mutex(mutex) {
}

ThreadMutex::~ThreadMutex() {
}

void ThreadMutex::lock(void) throw(ThrowableException) {
	int ret = pthread_mutex_lock(_mutex);
	if (ret != 0) {
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_mutex_lock error:" << strerror(ret));
	}
}

void ThreadMutex::tryLock(void) throw(ThrowableException) {
	int ret = pthread_mutex_trylock(_mutex);
	if (ret != 0) {
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_mutex_trylock error:" << strerror(ret));
	}
}

void ThreadMutex::unLock(void) throw(ThrowableException) {
	int ret = pthread_mutex_unlock(_mutex);
	if (ret) {
		ThrowableException e;
		RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_mutex_unlock error:" << strerror(ret));
	}
}

pthread_mutex_t* ThreadMutex::getMutex(void) {
	return _mutex;
}
