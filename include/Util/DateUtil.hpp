/*
 * DateUtil.hpp
 *
 *  Created on: 2018年6月3日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_DATEUTIL_HPP_
#define INCLUDE_UTIL_DATEUTIL_HPP_

#include <string>
using namespace std;

class DateUtil {
public:


    /* ************工具方法***************   */

    /**
    * 格式化Timestamp时间到指定日期格式
    */
    static string parseTimestampToStr(int timestamp, string& timeFromat);

    /**
    * 获取当前Timestamp时间
    */
    static int getCurTimestamp();

    /**
    * 获取当前格式化时间
    */
    static string getCurTimeFromat(string& timeFromat);
};


#endif /* INCLUDE_UTIL_DATEUTIL_HPP_ */
