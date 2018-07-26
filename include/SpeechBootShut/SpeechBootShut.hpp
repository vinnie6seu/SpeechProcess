/*
 * SpeechBoot.hpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SPEECHBOOTSHUT_SPEECHBOOTSHUT_HPP_
#define INCLUDE_SPEECHBOOTSHUT_SPEECHBOOTSHUT_HPP_

#include "Shm/tts/ShmTextToSpeech.hpp"
#include "Shm/asr/ShmSpeechToText.hpp"
#include "Util/Cfg/CfgUtil.hpp"

class SpeechBootShut {
public:
	SpeechBootShut();

	~SpeechBootShut();

	/**
	 * 1.初始化acl库
	 * 2.设置日志
	 * 3.初始化 tts 共享内存
	 * 4.初始化 asr 共享内存
	 */
	SpeechBootShut& init();

	/**
	 * 分配 tts 共享内存
	 */
	FUN_STATUS shmTextToSpeechCreate();

	/**
	 * 卸载 tts 共享内存
	 */
	FUN_STATUS shmTextToSpeechDelete();

	/**
	 * 分配 asr 共享内存
	 */
	FUN_STATUS shmSpeechToTextCreate();

	/**
	 * 卸载 asr 共享内存
	 */
	FUN_STATUS shmSpeechToTextDelete();

private:
	bool _init_flag;                                                   // 已经初始化true，未初始化false
	CfgUtil _cfgUtil;                                                  // 解析配置文件

	ShmTextToSpeech _shmTextToSpeech;                                  // tts shm
	ShmSpeechToText _shmSpeechToText;                                  // asr shm
};

#endif /* INCLUDE_SPEECHBOOTSHUT_SPEECHBOOTSHUT_HPP_ */
