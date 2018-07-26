/*
 * test_util_xml.cpp
 *
 *  Created on: 2018年6月10日
 *      Author: yaoqiaobing
 */

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

#include "lib_acl.hpp"
#include "Util/Xml/XmlUtil.hpp"
#include "Util/EnumUtil.hpp"

using namespace acl;
using namespace std;


// g++ test_util_xml.cpp -o test_util_xml -I${MKHOME}/include -L${MKHOME}/lib ${ACLINC} ${ACLLD} -lShm -lUtil -lacl_all -lz -lpthread -ldl
int main(int argc, char* argv[]) {

	// 1.初始化ACL库(尤其是在WIN32下一定要调用此函数，在UNIX平台下可不调用)
	acl_cpp_init();

	try {
		// 2.装载xml文件
		XmlUtil _xmlUtil("SpeechProcess.xml");

		vector< pair<int, vector<xml_content> > > vecXMlContent;

		// 3.获取所有层级的所有类型内容
		vecXMlContent.clear();
		_xmlUtil.readXmlLevelType(vecXMlContent, XML_TYPE_ALL, 0);

		cout << "------------- 获取所有层级的所有类型内容 -------------" << endl;
		for (int i = 0; i < vecXMlContent.size(); i++) {
			cout << std::string(vecXMlContent[i].first, '\t');
			for (int j = 0; j < vecXMlContent[i].second.size(); j++) {
				cout << "[" << vecXMlContent[i].second[j].name << " : " << vecXMlContent[i].second[j].value << " : " << XML_CONTENT_TYPE_ENUM_STRING[vecXMlContent[i].second[j].content_type] << "] ";
			}
			cout << endl;
		}

		// 4.获取第1层级的属性类型内容
		vecXMlContent.clear();
		_xmlUtil.readXmlLevelType(vecXMlContent, XML_TYPE_ATTRIBUTE, 1);

		cout << "------------- 获取第1层级的属性类型内容 -------------" << endl;
		for (int i = 0; i < vecXMlContent.size(); i++) {
			cout << std::string(vecXMlContent[i].first, '\t');
			for (int j = 0; j < vecXMlContent[i].second.size(); j++) {
				cout << "[" << vecXMlContent[i].second[j].name << " : " << vecXMlContent[i].second[j].value << " : " << XML_CONTENT_TYPE_ENUM_STRING[vecXMlContent[i].second[j].content_type] << "] ";
			}
			cout << endl;
		}

		// 5.获取第2层级的文本类型内容
		vecXMlContent.clear();
		_xmlUtil.readXmlLevelType(vecXMlContent, XML_TYPE_TEXT, 2);

		cout << "------------- 获取第2层级的文本类型内容 -------------" << endl;
		for (int i = 0; i < vecXMlContent.size(); i++) {
			cout << std::string(vecXMlContent[i].first, '\t');
			for (int j = 0; j < vecXMlContent[i].second.size(); j++) {
				cout << "[" << vecXMlContent[i].second[j].name << " : " << vecXMlContent[i].second[j].value << " : " << XML_CONTENT_TYPE_ENUM_STRING[vecXMlContent[i].second[j].content_type] << "] ";
			}
			cout << endl;
		}

		// 6.获取指定元素的所有属性内容
		vecXMlContent.clear();
		_xmlUtil.readXmlElementType(vecXMlContent, XML_TYPE_ATTRIBUTE, "surface");

		cout << "------------- 获取指定元素的所有属性内容 -------------" << endl;
		for (int i = 0; i < vecXMlContent.size(); i++) {
			cout << std::string(vecXMlContent[i].first, '\t');
			for (int j = 0; j < vecXMlContent[i].second.size(); j++) {
				cout << "[" << vecXMlContent[i].second[j].name << " : " << vecXMlContent[i].second[j].value << " : " << XML_CONTENT_TYPE_ENUM_STRING[vecXMlContent[i].second[j].content_type] << "] ";
			}
			cout << endl;
		}

		// 7.获取指定元素的所有内容
		vecXMlContent.clear();
		_xmlUtil.readXmlElementType(vecXMlContent, XML_TYPE_ALL, "surface");

		cout << "------------- 获取指定元素的所有内容 -------------" << endl;
		for (int i = 0; i < vecXMlContent.size(); i++) {
			cout << std::string(vecXMlContent[i].first, '\t');
			for (int j = 0; j < vecXMlContent[i].second.size(); j++) {
				cout << "[" << vecXMlContent[i].second[j].name << " : " << vecXMlContent[i].second[j].value << " : " << XML_CONTENT_TYPE_ENUM_STRING[vecXMlContent[i].second[j].content_type] << "] ";
			}
			cout << endl;
		}

		// 8.获取指定元素的所有文本内容
		vecXMlContent.clear();
		_xmlUtil.readXmlElementType(vecXMlContent, XML_TYPE_TEXT, "yeah");

		cout << "------------- 获取指定元素的所有文本内容 -------------" << endl;
		for (int i = 0; i < vecXMlContent.size(); i++) {
			cout << std::string(vecXMlContent[i].first, '\t');
			for (int j = 0; j < vecXMlContent[i].second.size(); j++) {
				cout << "[" << vecXMlContent[i].second[j].name << " : " << vecXMlContent[i].second[j].value << " : " << XML_CONTENT_TYPE_ENUM_STRING[vecXMlContent[i].second[j].content_type] << "] ";
			}
			cout << endl;
		}

	} catch (ThrowableException& e) {
		cout << e.what() << endl;
	}

	return 0;
}








