/*
 * XmlUtil.cpp
 *
 *  Created on: 2018年6月25日
 *      Author: yaoqiaobing
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <list>
#include <sstream>
#include <iostream>

#include "lib_acl.h"
#include "Util/Xml/XmlUtil.hpp"
#include "Util/Xml/tinyxml2.h"
#include "Util/Exception/throw.hpp"
#include "Util/StringUtil.hpp"
#include "Util/EnvUtil.hpp"
#include "Util/DateUtil.hpp"

using namespace std;

/**
 * 参考 : https://github.com/leethomason/tinyxml2/blob/master/xmltest.cpp
 *       https://blog.csdn.net/qq_35488967/article/details/60963493
 */


XmlUtil::XmlUtil(string xmlName) throw(ThrowableException) {
	_xml_name = xmlName;

	// 获取环境变量XML_PATH
	char* xmlPathDirEnv = acl_getenv("XML_PATH");
	if ((xmlPathDirEnv == NULL) || (strlen(xmlPathDirEnv) <= 0)) {
		_xml_dir = "/cygdrive/g/c_dev/sts_project/SpeechProcess/etc/";
	} else {
		_xml_dir = xmlPathDirEnv;
	}

	string xmlFullPath = _xml_dir + _xml_name;

	// 装载xml文件
    if (_doc.LoadFile(xmlFullPath.c_str()) != XML_SUCCESS) {
        ThrowableException e;
        RAISE_EXCEPTION(RUN_EXCEPTION, e, _doc.ErrorName() << " " << _doc.ErrorStr());
    }

	_pRootElement = _doc.RootElement();                   // 根节点
	_pSurface = _pRootElement->FirstChildElement();       // 节点入口，获取第一个值为value的子节点 默认为空则返回第一个节点
}

bool XmlUtil::readXmlLevelType(vector< pair<int, vector<xml_content> > >& vecXMlContent, int needType, int needLevel) {

	pair<bool, int> needFlag = make_pair(true, 0);
    readXml(_pSurface, vecXMlContent, 1, needType, needLevel, NULL, needFlag);

    return true;
}

bool XmlUtil::readXmlElementType(vector< pair<int, vector<xml_content> > >& vecXMlContent, int needType, const char* needElementName) {

	pair<bool, int> needFlag = make_pair(false, 0);
	readXml(_pSurface, vecXMlContent, 1, needType, 0, needElementName, needFlag);

    return true;
}

void XmlUtil::readXml(XMLElement* pSurface, vector< pair<int, vector<xml_content> > >& vecXMlContent, int curLevel, int needType, int needLevel, const char* needElementName, pair<bool, int>& needFlag) {

	// 若出现乱码，请使用str = utf8ToCString(value.c_str());进行转码
	while (pSurface) {

		if (needFlag.first || (needElementName == NULL) || ((pSurface->Name() != NULL) && (strcmp(pSurface->Name(), needElementName) == 0))) {

			if (!needFlag.first) {
				needFlag = make_pair(true, curLevel);
			}

			// 该层的内容
			vector<xml_content> vecCurLevel;

			// 0.节点名称
			if ((pSurface->Name() != NULL) && (needType == XML_TYPE_ALL || needType == XML_TYPE_ELEMENT) && (needLevel == 0 || needLevel == curLevel)) {
				xml_content xml_content_item;
				xml_content_item.name = pSurface->Name();
				xml_content_item.content_type = XML_TYPE_ELEMENT;

				vecCurLevel.push_back(xml_content_item);
			}

			// 1.获取该节点所有属性值
			const XMLAttribute* attr = pSurface->FirstAttribute();
			while (attr) {
				// _vectorValue.push_back(cStringToString(utf8ToCString(attr->Value()))); // 此处为utf8格式，若显示乱码，请手动转码。

				if ((attr->Value() != NULL) && (needType == XML_TYPE_ALL || needType == XML_TYPE_ATTRIBUTE) && (needLevel == 0 || needLevel == curLevel)) {
					xml_content xml_content_item;
					xml_content_item.name = attr->Name();
					xml_content_item.value = attr->Value();
					xml_content_item.content_type = XML_TYPE_ATTRIBUTE;

					vecCurLevel.push_back(xml_content_item);
				}

				// 获取下一个属性值
				attr = attr->Next();
			}

			// 2.获取该节点的内容
			if ((pSurface->GetText() != NULL) && (needType == XML_TYPE_ALL || needType == XML_TYPE_TEXT) && (needLevel == 0 || needLevel == curLevel)) {
				xml_content xml_content_item;
				xml_content_item.value = pSurface->GetText();
				xml_content_item.content_type = XML_TYPE_TEXT;

				vecCurLevel.push_back(xml_content_item);
			}

			// 保存该层的内容
			if (!vecCurLevel.empty()) {
				vecXMlContent.push_back(make_pair(curLevel, vecCurLevel));
			}
		}

		// 3.查看当前节点是否有子节点
		XMLElement* surface1 = pSurface->FirstChildElement();
		if (surface1 != NULL) {
			// 递归调用
			readXml(surface1, vecXMlContent, curLevel + 1, needType, needLevel, needElementName, needFlag);
		}

		// 4.指向当前节点的下一个兄弟节点
		pSurface = pSurface->NextSiblingElement();

		if (needFlag.first && (needFlag.second >= curLevel)) {
			needFlag = make_pair(false, curLevel);
		}

	}

}
