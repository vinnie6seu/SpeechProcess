/*
 * AlgorithmUtil.cpp
 *
 *  Created on: 2018年6月27日
 *      Author: yaoqiaobing
 */

#include <algorithm>
#include <map>

#include "Util/AlgorithmUtil.hpp"

using namespace std;



int AlgorithmUtil::lengthOfLongestSubstring(const string& s) {
	int result = 0;

	if (s.empty()) {
		return result;
	}

	// 用于存储该字符曾经出现过的位置
	int index[256] = {0};

	int startIndex = 0;
	for (int i = 0; i < s.size(); i++) {
		// 移动子串起始index到该字符曾经出现过的位置
		startIndex = max(startIndex, index[s[i]]);
		// 求取最大子串
		result = max(result, i - startIndex + 1);
		// 保存该字符的最新位置
		index[s[i]] = i + 1;
	}

	return result;
}

// Transform S into T.
// For example, S = "abba", T = "^#a#b#b#a#$".
// ^ and $ signs are sentinels appended to each end to avoid bounds checking
string AlgorithmUtil::preProcess(const string& s) {
	int n = s.length();
	if (n == 0)
		return "^$";
	string ret = "^";
	for (int i = 0; i < n; i++)
		ret += "#" + s.substr(i, 1);

	ret += "#$";
	return ret;
}

string AlgorithmUtil::longestPalindrome(const string& s) {
	// static成员函数没有this形参，它可以直接访问所属类的static成员，但不能直接使用非static成员。
	// 类的非static成员函数是可以直接访问类的static和非static成员，而不用作用域操作符。
	string T = preProcess(s);
	int n = T.length();
	int *P = new int[n];
	int C = 0, R = 0;
	for (int i = 1; i < n - 1; i++) {
		int i_mirror = 2 * C - i; // equals to i' = C - (i-C)

		P[i] = (R > i) ? min(R - i, P[i_mirror]) : 0;

		// Attempt to expand palindrome centered at i
		while (T[i + 1 + P[i]] == T[i - 1 - P[i]])
			P[i]++;

		// If palindrome centered at i expand past R,
		// adjust center based on expanded palindrome.
		if (i + P[i] > R) {
			C = i;
			R = i + P[i];
		}
	}

	// Find the maximum element in P.
	int maxLen = 0;
	int centerIndex = 0;
	for (int i = 1; i < n - 1; i++) {
		if (P[i] > maxLen) {
			maxLen = P[i];
			centerIndex = i;
		}
	}
	delete[] P;

	return s.substr((centerIndex - 1 - maxLen) / 2, maxLen);
}

vector<int> AlgorithmUtil::twoSum(vector<int>& nums, int target) {
	vector<int> result;

	if (nums.size() < 2) {
		return result;
	}

	map<int, int> record;
	record.insert(make_pair(nums[0], 0));
	for (int i = 1; i < nums.size(); i++) {
		if (record.find(target - nums[i]) != record.end()) {
			result.push_back(record[target - nums[i]]);
			result.push_back(i);
			break;
		}

		record.insert(make_pair(nums[i], i));
	}

	return result;
}



