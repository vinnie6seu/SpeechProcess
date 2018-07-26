/*
 * StringUtil.hpp
 *
 *  Created on: 2018年6月2日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_STRINGUTIL_HPP_
#define INCLUDE_UTIL_STRINGUTIL_HPP_

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

class StringUtil {
public:
	static std::vector<std::string> Compact(
			const std::vector<std::string> &tokens);

	static std::vector<std::string> Split(const std::string &str,
			const std::string &delim, const bool trim_empty = false);

	static void Split(const std::string &str, const std::string &delim,
			std::vector<std::string>& tokens, const bool trim_empty = false);

	static std::string Join(const std::vector<std::string> &tokens,
			const std::string &delim, const bool trim_empty = false);

	static std::string Trim(const std::string &str);

	static std::string Repeat(const std::string &str,
			unsigned int times);

	static std::string ReplaceAll(const std::string &source,
			const std::string &target, const std::string &replacement);

	static std::string ToUpper(const std::string &str);

	static std::string ToLower(const std::string &str);

	static std::string ReadFile(const std::string &filepath);

	static void WriteFile(const std::string &filepath,
			const std::string &content);
};



#endif /* INCLUDE_UTIL_STRINGUTIL_HPP_ */
