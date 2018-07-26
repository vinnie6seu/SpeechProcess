/*
 * EnvUtil.hpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_ENVUTIL_HPP_
#define INCLUDE_UTIL_ENVUTIL_HPP_

#include <string>
using namespace std;

class EnvUtil {
public:
	static char* getConfigPath();
	static char* getLogPath();
};


#endif /* INCLUDE_UTIL_ENVUTIL_HPP_ */
