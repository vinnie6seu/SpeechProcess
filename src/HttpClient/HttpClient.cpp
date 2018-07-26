/*
 * HttpClient.cpp
 *
 *  Created on: 2018年7月4日
 *      Author: yaoqiaobing
 */

#include <sstream>
#include <vector>
#include <assert.h>
#include <getopt.h>

#include "HttpClient/HttpClient.hpp"

#include "lib_acl.hpp"
//using namespace acl;



HttpClient::HttpClient(const char* server_addr, const char* stype, const char* charset, const char* to_charset) {
	_server_addr = server_addr;	      // web 服务器地址
	_stype = stype;		              // 请求报文格式
	_charset = charset;	              // 本地请求数据文件的字符集
	_to_charset = to_charset;	      // 将服务器响应数据转为本地字符集
}

void HttpClient::init(const char* server_addr, const char* stype, const char* charset, const char* to_charset) {
	_server_addr = server_addr;	      // web 服务器地址
	_stype = stype;		              // 请求报文格式
	_charset = charset;	              // 本地请求数据文件的字符集
	_to_charset = to_charset;	      // 将服务器响应数据转为本地字符集
}

void HttpClient::clear() {
	_server_addr.clear();
	_stype.clear();
	_charset.clear();
	_to_charset.clear();
}

void HttpClient::postRequest(const string& requestMapping, const string& requestJsonStr, string& responseJsonStr) throw(ThrowableException) {
	acl::http_request req(_server_addr.c_str());

	// 添加 HTTP 请求头字段
	stringstream ss;

	ss << "application/" << _stype << "; charset=" << _charset;
	string ctype = ss.str();

	// 完善request头部
	acl::http_header& hdr = req.request_header();  // 请求头对象的引用
	hdr.set_url(requestMapping.c_str());
	hdr.set_content_type(ctype.c_str());

	// 发送 HTTP 请求数据
	if (req.request(requestJsonStr.c_str(), requestJsonStr.length()) == false) {
		// logger_error("send http request to [%s %s] error", _server_addr.c_str(), requestMapping.c_str());
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "send http request to [" << _server_addr << " " << requestMapping << "] error");
	}

	// 取出 HTTP 响应头的 Content-Type 字段
	const char* p = req.header_value("Content-Type");
	if (p == NULL || *p == 0) {
		// logger_error("no Content-Type");
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "no Content-Type");
	}

	// 分析 HTTP 响应头的数据类型
	acl::http_ctype content_type;
	content_type.parse(p);

	// 响应头数据类型的子类型
	const char* stype = content_type.get_stype();

	if ((stype != NULL) && (strcasecmp(stype, "json") == 0)) {
		acl::json body;
		if (req.get_body(body, _to_charset.c_str()) == false) {
			// logger_error("get http response body error");
	        ThrowableException e;
	        RAISE_EXCEPTION(RUN_EXCEPTION, e, "get http response body error");
		}

//        responseJsonStr = body.to_string().c_str();

        acl::json_node* node = body.first_node();
		while (node) {
			if (node->tag_name()) {
				if ((strcmp(node->tag_name(), "code") == 0) && (atoi(node->get_text()) != 0)) {
					// 检查 code 返回码
					// logger_error("response code is [%s]", node->get_text());
			        ThrowableException e;
			        RAISE_EXCEPTION(RUN_EXCEPTION, e, "response code is [" << node->get_text() << "]");
				}

				if ((strcmp(node->tag_name(), "data") == 0) && node->get_text()) {
					// 取出 data中的数据
					responseJsonStr = node->get_text();
				}
			}

			node = body.next_node();
		}

	} else {
		// logger_error("response type is not json");
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, "response type is not json");
	}

}
