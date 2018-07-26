/*
 * AlgorithmUtil.hpp
 *
 *  Created on: 2018年6月27日
 *      Author: yaoqiaobing
 */

#ifndef INCLUDE_UTIL_ALGORITHMUTIL_HPP_
#define INCLUDE_UTIL_ALGORITHMUTIL_HPP_

#include <vector>
#include <string>
using namespace std;

class AlgorithmUtil {
public:
	/* ----------------- leetcode string tag -----------------*/

	/**
	 * 序号： 3
	 * 标题： Longest Substring Without Repeating Characters
	 * 问题： 求取字符串中最长无重复字符子串的长度
	 *
	 * 思路：滑动窗口法。
	 * 分析： Time complexity : O(n). Index i will iterate n times.
	 *       Space complexity (HashMap) : O(min(m, n)).
	 *       Space complexity (Table): O(m). m is the size of the charset.
	 */
	static int lengthOfLongestSubstring(const string& s);

	/**
	 * 序号： 5
	 * 标题： Longest Palindromic Substring
	 * 问题： 最长对称子串
	 *
	 * 思路：https://articles.leetcode.com/longest-palindromic-substring-part-ii/
	 * 分析：Time complexity : O(n).
	 */
	static string preProcess(const string& s);

	static string longestPalindrome(const string& s);

	/* ----------------- leetcode string tag -----------------*/


	/* ----------------- leetcode array tag -----------------*/

	/**
	 * 序号： 1
	 * 标题： Two Sum
	 * 问题： 在一个未排序的数组中找到两个数字和能够等于指定值target。只有唯一解，返回两个数字的位置。
	 *
	 * 思路： 因为要返回两个数字的位置，因此不能先排序再左右缩进。可以使用一个map记录遍历过的数字和其位置，再用(target - 当前值)是否在
	 *       map中出现过。
	 * 分析：
	 */
	static vector<int> twoSum(vector<int>& nums, int target);

	/* ----------------- leetcode array tag -----------------*/
};


#endif /* INCLUDE_UTIL_ALGORITHMUTIL_HPP_ */
