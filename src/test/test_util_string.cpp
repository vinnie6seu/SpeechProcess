/*
 * test_util_string.cpp
 *
 *  Created on: 2018年6月26日
 *      Author: yaoqiaobing
 */

#include <iostream>
#include <string>
#include <vector>

#include "Util/StringUtil.hpp"

using namespace std;


// g++ test_util_string.cpp -o test_util_string -I${MKHOME}/include -L${MKHOME}/lib ${ACLINC} ${ACLLD} -lShm -lUtil -lacl_all -lz -lpthread -ldl
int main(int argc, char* argv[]) {

	std::vector<std::string> result;
	std::string bufStr = "id=10&speechOffset=300&speechLen=66";

	std::string delim1 = "&";
	std::vector<std::string> tokens1;
	StringUtil::Split(bufStr, delim1, tokens1, true);

	for (int i = 0 ; i < tokens1.size(); i++) {
		StringUtil::Split(tokens1[i], "=", result, true);
	}


	for (int i = 0 ; i < result.size(); i++) {
		cout << result[i] << " ";
	}
	cout << endl;

	return 0;
}
