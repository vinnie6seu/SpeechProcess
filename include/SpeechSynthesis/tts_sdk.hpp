/*
 * tts_sdk.hpp
 *
 *  Created on: 2018年6月10日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SPEECHSYNTHESIS_TTS_SDK_HPP_
#define INCLUDE_SPEECHSYNTHESIS_TTS_SDK_HPP_

#include "lib_acl.hpp"
#include "Define.hpp"
#include "Shm/tts/ShmTextToSpeech.hpp"

using namespace acl;

int tts_sdk_all(char* clientText, int clientTextLen, char* serverSpeech, int serverSpeechLen);

int tts_sdk_part(char* clientText, int clientTextLen, socket_stream* stream, ShmTextToSpeechHead* _shmTextToSpeechHead, ShmTextToSpeech& _shmTextToSpeech, int id);

#endif /* INCLUDE_SPEECHSYNTHESIS_TTS_SDK_HPP_ */
