/*
 * ShmBase.hpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SHM_SHMBASE_HPP_
#define INCLUDE_SHM_SHMBASE_HPP_


#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "Define.hpp"

using namespace std;

// 调用shmget使用的flag
enum shmIdFlag {
	enum_shmId_get = 0660,                         // 查询shmId使用flag
	enum_shmId_create = IPC_CREAT | 0660           // 创建共享内存时使用flag
};

class ShmBase {
public:

	ShmBase();

	virtual ~ShmBase() {}

	/*
	 * 得到共享内存key
	 */
	int getShmKey() {
		return _shmKey;
	}

	/*
	 * 得到共享内存大小
	 */
	int getShmSize() {
		return _shmSize;
	}

	/*
	 * 得到共享内存id
	 */
	int getShmId() {
		return _shmId;
	}

	/*
	 * 得到出错原因
	 */
	string& getShmLastError() {
		return _lastErrorBuf;
	}

	/*
	 * 初始化key和共享内存大小
	 */
// 	void init(int shmKey, int shmSize);
	virtual void init(int shmKey, int shmTextToSpeechItemNum) = 0;

	/*
	 * 创建共享内存
	 */
	FUN_STATUS createShm();

	/*
	 * 删除共享内存
	 */
	FUN_STATUS deleteShm();

	/*
	 * 共享内存加载到进程中
	 */
	FUN_STATUS procLoadShm(void** shmMemPoint);

	/*
	 * 共享内存从进程中剥离
	 */
	FUN_STATUS procDetachShm(void* shmMemPoint);

	/*
	 * 找到空闲区域写入记录
	 */
// 	FUN_STATUS writeRecordShm();

	/*
	 * 取走处理完的数据
	 */
// 	FUN_STATUS readRecordShm();

protected:

	/*
	 * 只是获取shm id：iSize = 0 iFlag = 0，若共享内存不存在则函数会报错
	 *
	 * 创建共享内存：
	 * iSize > 0 iFlag = IPC_CREAT(如果内核中不存在键值与key相等的共享内存，则新建一个共享内存；如果存在这样的共享内存，返回此共享内存的标识符)
	 * iSize > 0 iFlag = IPC_CREAT | IPC_EXCL(如果内核中不存在键值 与key相等的共享内存，则新建一个共享内存；如果存在这样的共享内存则报错)
	 *
	 * @return FUN_STATUS
	 */
	FUN_STATUS getShmId(int iSize, int iFlag);

	int _shmKey;
	int _shmSize;
	int _shmId;
	string _lastErrorBuf;
};


#endif /* INCLUDE_SHM_SHMBASE_HPP_ */
