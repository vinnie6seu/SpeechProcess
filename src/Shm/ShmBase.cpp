/*
 * ShmBase.cpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#include <string.h>

#include "lib_acl.hpp"
#include "Shm/ShmBase.hpp"


ShmBase::ShmBase() {
    _shmKey = 0;
    _shmSize = 0;
    _shmId = 0;
    _lastErrorBuf.clear();
}

/*
void ShmBase::init(int shmKey, int shmSize) {
    this->_shmKey = shmKey;
    this->_shmSize = shmSize;
}
*/

//////////////////////////////////////////////////

FUN_STATUS ShmBase::createShm() {
	// 不存在就创建，存在继续再调用就报错
//	if (getShmId(_shmSize, 0666 | IPC_CREAT | IPC_EXCL) == FAILURE) {
	if (getShmId(_shmSize, enum_shmId_create) == FAILURE) {
		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS ShmBase::deleteShm() {
	// 查shm id
	if (getShmId(0, enum_shmId_get) == FAILURE) {
		return FAILURE;
	}

	// 删除共享内存
	if (shmctl(_shmId, IPC_RMID, NULL)) {
		_lastErrorBuf = "delete shm error. " + string(strerror(errno));
		return FAILURE;
	}

	return SUCCESS;
}

FUN_STATUS ShmBase::procLoadShm(void** shmMemPoint) {
	void* shm_addr = NULL;

	// 查shm id
	if (getShmId(0, enum_shmId_get) == FAILURE) {
		return FAILURE;
	}

	// 加载
	shm_addr = (void*)shmat(_shmId, NULL, 0);
	if (shm_addr == (void*)-1) {
    	_lastErrorBuf = "proc load shm error. " + string(strerror(errno));
    	return FAILURE;
	}

	*shmMemPoint = shm_addr;

	return SUCCESS;
}

FUN_STATUS ShmBase::procDetachShm(void* shmMemPoint) {
	// 查shm id
	if (getShmId(0, enum_shmId_get) == FAILURE) {
		return FAILURE;
	}

	if (shmdt(shmMemPoint) == -1) {
    	_lastErrorBuf = "proc detach shm error. " + string(strerror(errno));
    	return FAILURE;
	}

	return SUCCESS;
}


FUN_STATUS ShmBase::getShmId(int iSize, int iFlag) {

	if (iFlag == enum_shmId_get) {
		// 查询_shmId
	    if (_shmId > 0) {
	    	// 已经查询过
	        return SUCCESS;
	    }

	    // iSize和iFlag应该都是0
	    if (iSize != 0) {
	    	_lastErrorBuf = "get shm id error. para size != 0";
	    	return FAILURE;
	    }

	    _shmId = shmget(_shmKey, iSize, iFlag);
	    if (_shmId < 0) {
	    	_lastErrorBuf = "get shm id error. " + string(strerror(errno));
	    	return FAILURE;
	    }
	} else if (iFlag == enum_shmId_create) {
		// 创建共享内存
		if (iSize <= 0) {
	        _lastErrorBuf = "create shm error. para size = 0";
	        return FAILURE;
		}
	    _shmId = shmget(_shmKey, iSize, iFlag);
	    if (_shmId < 0) {
	        _lastErrorBuf = "create shm error. " + string(strerror(errno));
	        return FAILURE;
	    }
	} else {
		_lastErrorBuf = "fail to call ShmBase::getShmId, para iFlag is unknow.";
		return FAILURE;
	}

	return SUCCESS;
}
