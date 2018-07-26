/*
 * EnvUtil.cpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#include "lib_acl.h"
#include "Util/EnvUtil.hpp"

char* EnvUtil::getConfigPath() {
	return acl_getenv("CONFIG_PATH");
}

char* EnvUtil::getLogPath() {
	return acl_getenv("LOG_PATH");
}
