/*
 * ThreadCond.hpp
 *
 *  Created on: 2018年7月16日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_THREAD_THREADCOND_HPP_
#define INCLUDE_UTIL_THREAD_THREADCOND_HPP_

#include <pthread.h>

// 类声明只能用来定义引用或者指针，如果要定义对象则必须引入头文件让该类定义在前
// 如果使用引入或者指针访问成员，也必须引入头文件让该类定义在前，不然编译器不清楚该类有哪些成员
class ThreadMutex;

/**
 * 线程条件变量
 */
class ThreadCond {
public:
	/**
	 * 构造方法
	 * @param mutex {ThreadMutex*} 内部自动引用该线程锁
	 */
	ThreadCond(ThreadMutex* threadMutex, pthread_cond_t* cond);

	/**
	 * 析构方法，注意是否要释放内部创建线程锁
	 */
	~ThreadCond(void);

	/**
	 * 内部使用pthread_cond_wait
	 * @exception {ThrowableException} 线程条件等待有问题
	 *
	 * 使用示例：
	 * ThreadMutex threadMutex;
	 * ThreadCond threadCond(&threadMutex);
	 *
	 * thread A ：
	 * {
	 *     ThreadMutexGuard threadMutexGuard(&threadMutex);
	 *     // 多个消费者使用while，单个消费者使用if
	 *     if (tid == -1) {
	 *         threadCond.wait();
	 *     }
	 * }
	 *
	 * thread B ：
	 * {
	 *     ThreadMutexGuard threadMutexGuard(&threadMutex);
	 *     tid = 500;
	 *     threadCond.notify();
	 * }
	 */
	void wait() throw(ThrowableException);

	/**
	 * 等待线程条件变量就绪
	 * @param timeOut_ms {long long} 等待条件变量就绪的超时时间(毫秒级)
	 *   > 0 时表示等待超时的时间
	 *   == 0，不等待
	 *   < 0 则一直等待直到条件变量就绪，退化成wait函数
	 * @exception {ThrowableException} 线程条件等待有问题或计算等待时间有问题
	 */
	void waitTimeOut(long long timeOut_ms) throw(ThrowableException);

	/**
	 * 通知一个或几个等待在线程条件变量上的线程，表示条件变量就结
	 * @exception {ThrowableException} 表示通知失败
	 */
	void notify(void) throw(ThrowableException);

	/**
	 * 通知所有等待在线程条件变量上的线程，表示条件变量就结
	 * @exception {ThrowableException} 表示通知失败
	 */
	void notifyAll(void) throw(ThrowableException);

	/**
	 * 获得与该线程条件变量绑定的线程互斥锁
	 * @return {ThreadMutex*}
	 */
	ThreadMutex* getMutex(void);

	/**
	 * 获得系统类型的线程条件变量对象
	 * @return {pthread_cond_t*}
	 */
	pthread_cond_t* getCond(void);
private:
	mutable ThreadMutex* _threadMutex;

	pthread_cond_t* _cond;
};


#endif /* INCLUDE_UTIL_THREAD_THREADCOND_HPP_ */
