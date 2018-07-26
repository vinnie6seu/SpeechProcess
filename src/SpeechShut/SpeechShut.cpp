/*
 * SpeechShut.cpp
 *
 *  Created on: 2018年6月3日
 *      Author: yaoqiaobing
 */

#include "SpeechBootShut/SpeechBootShut.hpp"
#include "lib_acl.hpp"

int main(int argc, char* argv[]) {
	SpeechBootShut& speechBootShut = acl::singleton2<SpeechBootShut>::get_instance();

	// 从配置文件中读取共享内存的 key 及 分配 item 数量
	speechBootShut.init();

	// 删除 tts 的共享内存
	speechBootShut.shmTextToSpeechDelete();

	// 删除 asr 的共享内存
	speechBootShut.shmSpeechToTextDelete();

	return 0;
}


