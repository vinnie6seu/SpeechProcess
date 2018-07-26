/*
 * XmlUtil.hpp
 *
 *  Created on: 2018年6月25日
 *      Author: yaoqiaobing
 */

#ifndef XMLUTIL_HPP_
#define XMLUTIL_HPP_

#include <string>
#include <map>
#include <vector>

#include "Define.hpp"
#include "tinyxml2.h"
#include "Util/Exception/throw.hpp"
#include "Util/EnumUtil.hpp"

using namespace std;
using namespace tinyxml2;


/*将相应的枚举类型转为字符串(这种方法很重要)*/
#define FOREACH_XML_CONTENT_TYPE(CMD) \
        CMD(XML_TYPE_ELEMENT) \
		CMD(XML_TYPE_ATTRIBUTE) \
		CMD(XML_TYPE_TEXT) \
		CMD(XML_TYPE_ALL)

/**
 * 枚举：xml文件的组成部分
 */
typedef enum {
	FOREACH_XML_CONTENT_TYPE(GENERATE_ENUM)
} XML_CONTENT_TYPE_ENUM;

/**
 * 数组：xml文件的组成部分，用于将枚举转成字符串
 * 需要加入 static 修饰符，不然该头文件被多次 include 会报错该数组重复定义
 */
static const char* XML_CONTENT_TYPE_ENUM_STRING[] = {
	FOREACH_XML_CONTENT_TYPE(GENERATE_STRING)
};

/**
 * xml每个内容的结构表示
 */
typedef struct {
	string name;
	string value;
	int content_type;
} xml_content;


class XmlUtil {
public:
	/**
	 * 构造函数，装载xml文件
	 */
	XmlUtil(string xmlName) throw(ThrowableException);

	/**
	 * 析构函数
	 */
	~XmlUtil() {}

	/**
	 * 调用readXml取出指定层级的指定内容
	 */
    bool readXmlLevelType(vector< pair<int, vector<xml_content> > >& vecXMlContent, int needType, int needLevel);

    /**
     * 调用readXml取出某个节点下的指定内容
     */
    bool readXmlElementType(vector< pair<int, vector<xml_content> > >& vecXMlContent, int needType, const char* needElementName);

private:

	/**
	 * 递归遍历每一个节点
	 * pSurface           ： 第一个一级节点
	 * vecXMlContent      ： 存储取出的内容
	 * curLevel           ： 当前层级
	 *
	 * needType           ： 指定需要的类型
	 * int needLevel      ： 指定需要的层级
	 * needElementName    ： 指定的元素名称
	 * needFlag           ： 如果是true则不用再比较needElementName，已经是指定节点下的内容
	 */
	void readXml(XMLElement* pSurface, vector< pair<int, vector<xml_content> > >& vecXMlContent, int curLevel, int needType, int needLevel, const char* needElementName, pair<bool, int>& needFlag);


	string _xml_dir;                                // xml文件所在目录
	string _xml_name;                               // xml文件名称

	XMLDocument _doc;                               // xml文件对象
	XMLElement* _pRootElement;                      // 根节点
	XMLElement* _pSurface;                          // 第一个一级节点
};


#endif /* XMLUTIL_HPP_ */
