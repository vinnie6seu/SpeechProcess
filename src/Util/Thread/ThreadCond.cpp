/*
 * ThreadCond.cpp
 *
 *  Created on: 2018年7月17日
 *      Author: yaoqiaobing
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>

#include "Util/Thread/ThreadMutex.hpp"
#include "Util/Thread/ThreadCond.hpp"

ThreadCond::ThreadCond(ThreadMutex* threadMutex, pthread_cond_t* cond) : _threadMutex(threadMutex), _cond(cond) {
}

ThreadCond::~ThreadCond(void) {
}

void ThreadCond::wait() throw(ThrowableException) {
	int ret = pthread_cond_wait(_cond, _threadMutex->getMutex()) ;
	if (ret != 0) {
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_cond_wait error:" << strerror(ret));
	}
}

void ThreadCond::waitTimeOut(long long timeOut_ms) throw(ThrowableException) {
	if (timeOut_ms < 0) {
		wait();
		return;
	}

	struct timeval tv;
	int ret = gettimeofday(&tv, NULL);
	if (ret != 0) {
		ThrowableException e;
		RAISE_EXCEPTION(RUN_EXCEPTION, e, "gettimeofday error:" << strerror(ret));
	}

	struct timespec ts;
	uint64_t nsec = uint64_t(tv.tv_usec) * 1000 + uint64_t(timeOut_ms) * 1000 * 1000;
	ts.tv_sec = tv.tv_sec + (nsec / 1000000000LL);
	ts.tv_nsec = nsec % 1000000000LL;

	ret = pthread_cond_timedwait(_cond, _threadMutex->getMutex(), &ts);
	if (ret != 0) {
		ThrowableException e;
		RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_cond_timedwait error:" << strerror(ret));
	}
}

void ThreadCond::notify(void) throw(ThrowableException) {
	int ret = pthread_cond_signal(_cond);
	if (ret != 0) {
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_cond_signal error:" << strerror(ret));
	}
}

void ThreadCond::notifyAll(void) throw(ThrowableException) {
	int ret = pthread_cond_broadcast(_cond);
	if (ret != 0) {
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "pthread_cond_broadcast error:" << strerror(ret));
	}
}

ThreadMutex* ThreadCond::getMutex(void) {
	return _threadMutex;
}

pthread_cond_t* ThreadCond::getCond(void) {
	return _cond;
}

