/*
 * CfgUtil.hpp
 *
 *  Created on: 2018年5月30日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_CFGUTIL_HPP_
#define INCLUDE_CFGUTIL_HPP_

#include <string>
#include <map>
#include <vector>

#include "Define.hpp"
#include "SimpleIni.h"

using namespace std;

class CfgUtil {
public:
	CfgUtil(string cfgName);

	/**
	 * 解析出整个cfg
	 */
	FUN_STATUS parseCfg(map<string, map<string, vector<string> > >& cfg);

	/**
	 * 解析出整个base
	 */
	FUN_STATUS parseCfgBase(string& baseName, map<string, vector<string> >& base);

	/**
	 * 解析出整个base
	 */
	FUN_STATUS parseCfgBase(const char* baseName, map<string, vector<string> >& base);

	/**
	 * 解析出某个base指定的item
	 */
	FUN_STATUS parseCfgBaseItem(string& baseName, string& key, vector<string>& valueArr);

	FUN_STATUS parseCfgBaseItem(const char* baseName, const char* key, vector<string>& valueArr);

	~CfgUtil() {}

	// ----------------------------------------------------

	/**
	 * 获取存放文本转语音共享内存key
	 */
	int getShmTextToSpeechKey();

	/**
	 * 获取共享内存存放文本转语音记录数目
	 */
	int getShmTextToSpeechNum();

	// ----------------------------------------------------

	/**
	 * 获取存放语音转文本共享内存key
	 */
	int getShmSpeechToTextKey();

	/**
	 * 获取共享内存存放语音转文本记录数目
	 */
	int getShmSpeechToTextNum();

	// ----------------------------------------------------

	/**
	 * 获取日志标准输出情况
	 */
	bool getLogStdOut();

	/**
	 * 获取日志输出目录
	 */
	string getLogDir();

	/**
	 * 获取日志文件全路径
	 */
	void getLogFullPath(map<string, string>& logFullPathArr);

	// ----------------------------------------------------

	/**
	 * 获取 socket 服务器ip
	 */
	string getSocketIp();

	/**
	 * 获取 socket 服务器port
	 */
	string getSocketPort();

	// ----------------------------------------------------

	/**
	 * 获取 http 服务器ip
	 */
	string getHttpIp();

	/**
	 * 获取 http 服务器port
	 */
	string getHttpPort();

	/**
	 * 获取 http 发送报文格式
	 */
	string getHttpRequestType();

	/**
	 * 获取 http 发送报文字符集
	 */
	string getHttpRequestCharSet();

	/**
	 * 获取 http 接收报文字符集
	 */
	string getHttpResponseCharSet();

private:
	/**
	 * 去掉首尾空格，分割value值
	 */
	void modifyValue(const char* pVal, vector<string>& valueArr);

	string _cfg_dir;                                // 配置文件所在目录
	string _cfg_name;                               // 配置文件名称
	CSimpleIniA _ini;                               // 用于解析配置文件
};

#endif /* INCLUDE_CFGUTIL_HPP_ */
