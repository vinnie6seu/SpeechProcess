/*
 * SpeechSynthesis.hpp
 *
 *  Created on: 2018年5月29日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_SPEECHSYNTHESIS_SPEECHSYNTHESIS_HPP_
#define INCLUDE_SPEECHSYNTHESIS_SPEECHSYNTHESIS_HPP_

#include "lib_acl.hpp"
#include "Define.hpp"
#include "Shm/tts/ShmTextToSpeech.hpp"
#include "Util/Cfg/CfgUtil.hpp"

using namespace acl;


/**
 * 语音合成模块
 */
class SpeechSynthesis: public master_proc {
public:
	SpeechSynthesis();

	~SpeechSynthesis();

	/**
	 * 1.初始化acl库
	 * 2.设置日志
	 * 3.初始化文本转语音共享内存
	 * 4.将共享内存加载到本进程中
	 */
	SpeechSynthesis& init();

protected:
	/**
	 * 基类纯虚函数：当接收到一个客户端连接时调用此函数
	 * @param stream {aio_socket_stream*} 新接收到的客户端异步流对象
	 * 注：该函数返回后，流连接将会被关闭，用户不应主动关闭该流
	 */
	virtual void on_accept(socket_stream* stream);

	/**
	 * 监听客户端的请求进行处理
	 */
	FUN_STATUS on_read(socket_stream* stream);

	// 基类虚函数：服务进程切换用户身份前调用此函数
	virtual void proc_pre_jail();

	// 基类虚函数：服务进程切换用户身份后调用此函数
	virtual void proc_on_init();

	// 基类虚函数：服务进程退出前调用此函数
	virtual void proc_on_exit();

private:

	/**
	 * 读取客户端传来的文本，将整段文本全部转成语音后告知客户端取走
	 */
	FUN_STATUS do_text_all(acl::string& request, socket_stream* stream);

	/**
	 * 读取客户端传来的文本，将科大讯飞处理好的语音立即告知客户端，不等待全部文本的处理完成
	 */
	FUN_STATUS do_text_part(acl::string& request, socket_stream* stream);

	bool _init_flag;                         // 已经初始化true，未初始化false
	CfgUtil _cfgUtil;                        // 解析配置文件
	ShmTextToSpeech _shmTextToSpeech;
	ShmTextToSpeechHead* _shmTextToSpeechHead;
};



#endif /* INCLUDE_SPEECHSYNTHESIS_SPEECHSYNTHESIS_HPP_ */
