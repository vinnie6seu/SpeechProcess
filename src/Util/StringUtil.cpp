/*
 * StringUtil.cpp
 *
 *  Created on: 2018年6月5日
 *      Author: yaoqiaobing
 */

#include "Util/StringUtil.hpp"

std::vector<std::string> StringUtil::Compact(
		const std::vector<std::string> &tokens) {
	std::vector<std::string> compacted;

	for (size_t i = 0; i < tokens.size(); ++i) {
		if (!tokens[i].empty()) {
			compacted.push_back(tokens[i]);
		}
	}

	return compacted;
}

std::vector<std::string> StringUtil::Split(const std::string &str,
		const std::string &delim, const bool trim_empty) {
	size_t pos, last_pos = 0, len;
	std::vector<std::string> tokens;

	while (true) {
		pos = str.find(delim, last_pos);
		if (pos == std::string::npos) {
			pos = str.size();
		}

		len = pos - last_pos;
		if (!trim_empty || len != 0) {
			tokens.push_back(str.substr(last_pos, len));
		}

		if (pos == str.size()) {
			break;
		} else {
			last_pos = pos + delim.size();
		}
	}

	return tokens;
}

void StringUtil::Split(const std::string &str,
		const std::string &delim, std::vector<std::string>& tokens,
		const bool trim_empty) {
	size_t pos, last_pos = 0, len;

	while (true) {
		pos = str.find(delim, last_pos);
		if (pos == std::string::npos) {
			pos = str.size();
		}

		len = pos - last_pos;
		if (!trim_empty || len != 0) {
			tokens.push_back(str.substr(last_pos, len));
		}

		if (pos == str.size()) {
			break;
		} else {
			last_pos = pos + delim.size();
		}
	}

}

std::string StringUtil::Join(
		const std::vector<std::string> &tokens, const std::string &delim,
		const bool trim_empty) {
	if (trim_empty) {
		return Join(Compact(tokens), delim, false);
	} else {
		std::stringstream ss;
		for (size_t i = 0; i < tokens.size() - 1; ++i) {
			ss << tokens[i] << delim;
		}
		ss << tokens[tokens.size() - 1];

		return ss.str();
	}
}

std::string StringUtil::Trim(const std::string &str) {

	std::string blank = "\r\n\t ";
	size_t begin = str.size(), end = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		if (blank.find(str[i]) == std::string::npos) {
			begin = i;
			break;
		}
	}

	for (size_t i = str.size(); i > 0; --i) {
		if (blank.find(str[i - 1]) == std::string::npos) {
			end = i - 1;
			break;
		}
	}

	if (begin >= end) {
		return "";
	} else {
		return str.substr(begin, end - begin + 1);
	}
}

std::string StringUtil::Repeat(const std::string &str,
		unsigned int times) {
	std::stringstream ss;
	for (unsigned int i = 0; i < times; ++i) {
		ss << str;
	}
	return ss.str();
}

std::string StringUtil::ReplaceAll(const std::string &source,
		const std::string &target, const std::string &replacement) {
	return Join(Split(source, target, false), replacement, false);
}

std::string StringUtil::ToUpper(const std::string &str) {
	std::string s(str);
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

std::string StringUtil::ToLower(const std::string &str) {
	std::string s(str);
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

std::string StringUtil::ReadFile(const std::string &filepath) {
	std::ifstream ifs(filepath.c_str());
	std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
	ifs.close();
	return content;
}

void StringUtil::WriteFile(const std::string &filepath,
		const std::string &content) {
	std::ofstream ofs(filepath.c_str());
	ofs << content;
	ofs.close();
	return;
}


/************************************************************************/
// g++ -DDEBUG_TEST StringUtil.cpp -I/home/vinnie/my_project/SpeechProcess/include/

#ifdef DEBUG_TEST

#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
	std::string buf = "id=90";
	std::string delim = "=";

	std::vector<std::string> tokens;
	StringUtil::Split(buf, delim, tokens, true);
	for (int i = 0; i < tokens.size(); i++) {
		cout << i << " " << tokens[i] << endl;
	}

    return 0;
}

#endif
