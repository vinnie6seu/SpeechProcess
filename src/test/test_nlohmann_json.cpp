/*
 * test_nlohmann_json.cpp
 *
 *  Created on: 2018年7月28日
 *      Author: yaoqiaobing
 */
#include <iostream>
#include <string>
#include "Shm/asr/ShmSpeechToText.hpp"
#include ".././Shm/jni/json.hpp"
using json = nlohmann::json;
using namespace std;

int main(void) {

	AsrSpeechTransResult asrSpeechTransResult;

	string json_cstr = "{\"cur_predict\":{\"confidence\":0.0,\"is_final\":false,\"stability\":0.0},\"cur_result\":{\"confidence\":0.9854616522789001,\"is_final\":true,\"stability\":0.0,\"transcript\":\"how old is the Brooklyn Bridge how old is the Brooklyn Bridge\"},\"is_exception\":false}";

	try {

		cout << "------1-----" << endl;

		//	json jsonObject = "{ \"happy\": true, \"pi\": 3.141 }"_json;
		json jsonObject = json::parse(json_cstr);

		cout << "------2-----" << endl;

		//////

		if (jsonObject.find("is_exception") != jsonObject.end()) {
			// there is an entry with key "pi"
			cout << *jsonObject.find("is_exception") << endl;

			asrSpeechTransResult._is_exception = *jsonObject.find("is_exception");
		}

		if (jsonObject.find("exception_str") != jsonObject.end()) {
			// there is an entry with key "pi"
			cout << *jsonObject.find("exception_str") << endl;

			string tmp = *jsonObject.find("exception_str");
			memcpy(asrSpeechTransResult._exception_str, tmp.c_str(), sizeof(asrSpeechTransResult._exception_str) - 1);
		}

		//////
		if (jsonObject.find("cur_result") != jsonObject.end()) {
			// there is an entry with key "pi"
			json resultJsonObject =  *jsonObject.find("cur_result");

			if (resultJsonObject.find("transcript") != resultJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *resultJsonObject.find("transcript") << endl;

				string tmp = *resultJsonObject.find("transcript");
				memcpy(asrSpeechTransResult._cur_result._transcript, tmp.c_str(), sizeof(asrSpeechTransResult._cur_result._transcript) - 1);
			}

			if (resultJsonObject.find("stability") != resultJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *resultJsonObject.find("stability") << endl;

				asrSpeechTransResult._cur_result._stability = *resultJsonObject.find("stability");
			}

			if (resultJsonObject.find("is_final") != resultJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *resultJsonObject.find("is_final") << endl;

				asrSpeechTransResult._cur_result._is_final = *resultJsonObject.find("is_final");
			}

			if (resultJsonObject.find("confidence") != resultJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *resultJsonObject.find("confidence") << endl;

				asrSpeechTransResult._cur_result._confidence = *resultJsonObject.find("confidence");
			}
		}

		////////
		if (jsonObject.find("cur_predict") != jsonObject.end()) {
			// there is an entry with key "pi"
			json predictJsonObject =  *jsonObject.find("cur_predict");

			if (predictJsonObject.find("transcript") != predictJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *predictJsonObject.find("transcript") << endl;

				string tmp = *predictJsonObject.find("transcript");
				memcpy(asrSpeechTransResult._cur_predict._transcript, tmp.c_str(), sizeof(asrSpeechTransResult._cur_predict._transcript) - 1);
			}

			if (predictJsonObject.find("stability") != predictJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *predictJsonObject.find("stability") << endl;

				asrSpeechTransResult._cur_predict._stability = *predictJsonObject.find("stability");
			}

			if (predictJsonObject.find("is_final") != predictJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *predictJsonObject.find("is_final") << endl;

				asrSpeechTransResult._cur_predict._is_final = *predictJsonObject.find("is_final");
			}

			if (predictJsonObject.find("confidence") != predictJsonObject.end()) {
				// there is an entry with key "pi"
				cout << *predictJsonObject.find("confidence") << endl;

				asrSpeechTransResult._cur_predict._confidence = *predictJsonObject.find("confidence");
			}
		}

		cout << "------3-----" << endl;

		printf("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
				asrSpeechTransResult._cur_result._transcript, asrSpeechTransResult._cur_result._stability,
				asrSpeechTransResult._cur_result._is_final ? "true" : "false", asrSpeechTransResult._cur_result._confidence,
				asrSpeechTransResult._cur_predict._transcript, asrSpeechTransResult._cur_predict._stability,
				asrSpeechTransResult._cur_predict._is_final ? "true" : "false", asrSpeechTransResult._cur_predict._confidence);

	} catch (std::exception& e) {
		cout << e.what() << endl;
	}


	return 0;
}
