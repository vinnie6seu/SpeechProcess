/*
 * HttpClient.hpp
 *
 *  Created on: 2018年7月7日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_HTTPCLIENT_HTTPCLIENT_HPP_
#define INCLUDE_HTTPCLIENT_HTTPCLIENT_HPP_

#include <string>

#include "Util/Exception/throw.hpp"

using namespace std;


class HttpClient {
public:
	/**
	 * 构造函数
	 */
	HttpClient() {}

	/**
	 * 构造函数
	 */
	HttpClient(const char* server_addr, const char* stype, const char* charset, const char* to_charset);

	/**
	 * 析构函数
	 */
	~HttpClient() {}

	/**
	 * 构造函数未指定参数，该函数配置参数
	 */
	void init(const char* server_addr, const char* stype, const char* charset, const char* to_charset);

	/**
	 * 清理参数配置
	 */
	void clear();

	/**
	 * 向 java server 端发起了 post http 请求，返回 json 报文
	 */
	void postRequest(const string& requestMapping, const string& requestJsonStr, string& responseJsonStr) throw(ThrowableException);

private:
	string _server_addr;	// web 服务器地址
	string _stype;		    // 请求报文格式
	string _charset;	    // 本地请求数据文件的字符集
	string _to_charset;	    // 将服务器响应数据转为本地字符集
};



#endif /* INCLUDE_HTTPCLIENT_HTTPCLIENT_HPP_ */
