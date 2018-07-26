/*
 * test_tools_gson.cpp
 *
 *  Created on: 2018年7月6日
 *      Author: yaoqiaobing
 */

#include <list>
#include <vector>
#include <map>
#include <stdio.h>
#include <iostream>
#include <time.h>

#include "lib_acl.hpp"
#include "lib_acl.h"

// 注意gson.h需要放在最后不然会编译报错
#include "Entity/SocketJson.h"
#include "Entity/HttpJson.h"
#include "Entity/gson.h"

// 序列化过程
static void serialize(void) {
	SocketJson socketJson;

	socketJson.id = 12;
	socketJson.speechLen = 300;
	socketJson.speechOffset = 600;
	socketJson.ttsFlag = 2;

//	user u;
//
//	u.name = "zsxxsz";
//	u.domain = "263.net";
//	u.age = 11;
//	u.male = true;

	acl::json json_socket;

	// 将 socketJson 对象转换为 json 对象
	acl::json_node& node = acl::gson(json_socket, socketJson);

	printf("serialize:\r\n");
	printf("json_socket: %s\r\n", node.to_string().c_str());
	printf("\r\n");
}

// 反序列化过程
static void deserialize(void) {
	const char *s = "{\"id\": 1, \"speechLen\": 22, \"speechOffset\": 45, \"ttsFlag\": 0}";
	printf("deserialize:\r\n");

	acl::json json;
	json.update(s);

	SocketJson socketJson;

	// 将 json 对象转换为 user 对象
	std::pair<bool, std::string> ret = acl::gson(json.get_root(), socketJson);

	// 如果转换失败，则打印转换失败原因
	if (ret.first == false)
		printf("error: %s\r\n", ret.second.c_str());
	else
		printf("id: %d, speechLen: %d, speechOffset: %d, ttsFlag: %d\r\n", socketJson.id,
				socketJson.speechLen, socketJson.speechOffset, socketJson.ttsFlag);

	///////////////////////////////////////////
	acl::json json_http;
	HttpJson httpJsonStruct;
	std::string responseJsonStr = "{\"id\":18}";

	json_http.update(responseJsonStr.c_str());

	// 将 json 对象转换为结构体对象
	ret = acl::gson(json_http.get_root(), httpJsonStruct);

	// 如果转换失败，则打印转换失败原因
	if (ret.first == false)
		printf("error: %s\r\n", ret.second.c_str());
	else {
		printf("http server response is [%s]\r\n", responseJsonStr.c_str());
		printf("id: %d, speechLen: %d, speechOffset: %d, asrFlag: %d\r\n", httpJsonStruct.id,
				httpJsonStruct.speechLen, httpJsonStruct.speechOffset, httpJsonStruct.asrFlag);
	}

}

// g++ test_tools_gson.cpp -o test_tools_gson -I${MKHOME}/include/ ${ACLINC} ${ACLLD} -L${MKHOME}/lib -lSerialize -lacl_all -lz -lpthread -ldl
int main(void) {
	serialize();
	deserialize();
	return 0;
}

