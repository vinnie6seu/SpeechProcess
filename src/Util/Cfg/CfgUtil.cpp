
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <list>

#include "lib_acl.h"
#include "Util/Cfg/CfgUtil.hpp"
#include "Util/Cfg/SimpleIni.h"
#include "Util/StringUtil.hpp"
#include "Util/EnvUtil.hpp"
#include "Util/DateUtil.hpp"

using namespace std;

CfgUtil::CfgUtil(string cfgName) {
	_cfg_name = cfgName;

	// 获取环境变量CONFIG_PATH
	char* configPathDirEnv = acl_getenv("CONFIG_PATH");
	if ((configPathDirEnv == NULL) || (strlen(configPathDirEnv) <= 0)) {
		_cfg_dir = "/cygdrive/e/c_dev/sts_project/SpeechProcess/etc/";
	} else {
		_cfg_dir = configPathDirEnv;
	}

	string cfgFullPath = _cfg_dir + _cfg_name;

	// 装载配置文件
	_ini.SetUnicode();
	_ini.LoadFile(cfgFullPath.c_str());
}



/**
 * 解析出整个cfg
 */
FUN_STATUS CfgUtil::parseCfg(map<string, map<string, vector<string> > >& cfg) {
	CSimpleIniA::TNamesDepend sections;
	_ini.GetAllSections(sections);

    list<CSimpleIniA::Entry>::iterator sections_itor = sections.begin();
    while (sections_itor != sections.end()) {
    	// 行号:(*sections_itor).nOrder
    	// base名称:(*sections_itor).pItem

    	map<string, vector<string> > base;

		CSimpleIniA::TNamesDepend sections_keys;
		_ini.GetAllKeys((*sections_itor).pItem, sections_keys);
	    list<CSimpleIniA::Entry>::iterator sections_keys_itor = sections_keys.begin();
	    while (sections_keys_itor != sections_keys.end()) {

	    	// 行号:(*sections_keys_itor).nOrder
	    	// key:(*sections_keys_itor).pItem
	    	// value:_ini.GetValue((*sections_itor).pItem, (*sections_keys_itor).pItem, NULL)

	    	const char* pVal = _ini.GetValue((*sections_itor).pItem, (*sections_keys_itor).pItem, NULL);

	    	vector<string> valueArr;
	    	modifyValue(pVal, valueArr);

	    	base.insert(make_pair((*sections_keys_itor).pItem, valueArr));

	    	sections_keys_itor++;
	    }

	    cfg.insert(make_pair((*sections_itor).pItem, base));

    	sections_itor++;
    }

	return 0;
}

/**
 * 解析出整个base
 */
FUN_STATUS CfgUtil::parseCfgBase(string& baseName, map<string, vector<string> >& base) {

	CSimpleIniA::TNamesDepend keys;
	_ini.GetAllKeys(baseName.c_str(), keys);

    // 遍历所有的key取出value值
    list<CSimpleIniA::Entry>::iterator keys_itor = keys.begin();
    while (keys_itor != keys.end()) {
    	// 行号:(*keys_itor).nOrder
    	// key:(*keys_itor).pItem
    	// value:_ini.GetValue("CUBLWDB", (*keys_itor).pItem, NULL)

    	const char* pVal = _ini.GetValue(baseName.c_str(), (*keys_itor).pItem, NULL);

    	vector<string> valueArr;
    	modifyValue(pVal, valueArr);

    	base.insert(make_pair((*keys_itor).pItem, valueArr));

    	keys_itor++;
    }

	return 0;
}

FUN_STATUS CfgUtil::parseCfgBase(const char* baseName, map<string, vector<string> >& base) {

	CSimpleIniA::TNamesDepend keys;
	_ini.GetAllKeys(baseName, keys);

    // 遍历所有的key取出value值
    list<CSimpleIniA::Entry>::iterator keys_itor = keys.begin();
    while (keys_itor != keys.end()) {
    	// 行号:(*keys_itor).nOrder
    	// key:(*keys_itor).pItem
    	// value:_ini.GetValue("CUBLWDB", (*keys_itor).pItem, NULL)

    	const char* pVal = _ini.GetValue(baseName, (*keys_itor).pItem, NULL);

    	vector<string> valueArr;
    	modifyValue(pVal, valueArr);

    	base.insert(make_pair((*keys_itor).pItem, valueArr));

    	keys_itor++;
    }

	return 0;
}

/**
 * 解析出某个base指定的item
 */
FUN_STATUS CfgUtil::parseCfgBaseItem(string& baseName, string& key, vector<string>& valueArr) {

	const char* pVal = _ini.GetValue(baseName.c_str(), key.c_str(), NULL);

	// base_item.second.push_back(pVal);
	modifyValue(pVal, valueArr);

	return 0;
}

FUN_STATUS CfgUtil::parseCfgBaseItem(const char* baseName, const char* key, vector<string>& valueArr) {

	const char* pVal = _ini.GetValue(baseName, key, NULL);

	// base_item.second.push_back(pVal);
	modifyValue(pVal, valueArr);

	return 0;
}

void CfgUtil::modifyValue(const char* pVal, vector<string>& valueArr) {
	string valueStr = pVal;
	valueStr = StringUtil::Trim(valueStr);

	StringUtil::Split(valueStr, " ", valueArr, true);
}

int CfgUtil::getShmTextToSpeechKey() {
	vector<string> valueArr;

	parseCfgBaseItem("SHM", "TEXT_TO_SPEECH_KEY", valueArr);

	int textToSpeechKey = 0;
	if (valueArr.size() == 1) {
		textToSpeechKey = atoi(valueArr[0].c_str());
	}

	return textToSpeechKey;
}

int CfgUtil::getShmTextToSpeechNum() {
	vector<string> valueArr;

	parseCfgBaseItem("SHM", "TEXT_TO_SPEECH_NUM", valueArr);

	int textToSpeechNum = 0;
	if (valueArr.size() == 1) {
		textToSpeechNum = atoi(valueArr[0].c_str());
	}

	return textToSpeechNum;
}

int CfgUtil::getShmSpeechToTextKey() {
	vector<string> valueArr;

	parseCfgBaseItem("SHM", "SPEECH_TO_TEXT_KEY", valueArr);

	int speechToTextKey = 0;
	if (valueArr.size() == 1) {
		speechToTextKey = atoi(valueArr[0].c_str());
	}

	return speechToTextKey;
}

int CfgUtil::getShmSpeechToTextNum() {
	vector<string> valueArr;

	parseCfgBaseItem("SHM", "SPEECH_TO_TEXT_NUM", valueArr);

	int speechToTextNum = 0;
	if (valueArr.size() == 1) {
		speechToTextNum = atoi(valueArr[0].c_str());
	}

	return speechToTextNum;
}

bool CfgUtil::getLogStdOut() {
	vector<string> valueArr;

	parseCfgBaseItem("LOG", "STDOUT", valueArr);

	bool stdOutFlag = false;
	if (valueArr.size() == 1) {
		stdOutFlag = (valueArr[0] == "1");
	}

	return stdOutFlag;
}

string CfgUtil::getLogDir() {
	// 目录
	vector<string> valueArr;
	parseCfgBaseItem("LOG", "DIR", valueArr);

	string logDir;
	if (valueArr.empty()) {
		logDir = EnvUtil::getLogPath();
	} else {
		logDir = valueArr[0];
	}

	return logDir;
}

void CfgUtil::getLogFullPath(map<string, string>& logFullPathArr) {
	// 目录
	string logDir = getLogDir();

	// 读取整个配置文件
	map<string, map<string, vector<string> > > cfg;
	parseCfg(cfg);

	map<string, map<string, vector<string> > >::iterator it;
	for (it = cfg.begin(); it != cfg.end(); it++) {
		if (it->second.find("APP_NAME") != it->second.end()) {
			string appName;
			if ((it->second)["APP_NAME"].empty()) {

			} else {
				appName = (it->second)["APP_NAME"][0];
			}

			if (logFullPathArr.find(appName) != logFullPathArr.end()) {
				// 日志格式
				string logFileFormat;
				if ((it->second)["FILEFORMAT"].empty()) {
					logFileFormat = "{NAME}-{DATE}";
				} else {
					logFileFormat = (it->second)["FILEFORMAT"][0];
				}

				// 日志名字
				string logName;
				if ((it->second)["NAME"].empty()) {

				} else {
					logName = (it->second)["NAME"][0];
				}

				// 日期格式
				string datePattern;
				if ((it->second)["DATE"].empty()) {
					datePattern = "%Y%m%d";
				} else {
					datePattern = (it->second)["DATE"][0];
				}

				// 需要开始构造日志日志路径
				string logFullPath = logFileFormat;
				logFullPath = logFullPath.replace(logFullPath.find("{NAME}"), 6, logName);
				logFullPath = logFullPath.replace(logFullPath.find("{DATE}"), 6, DateUtil::getCurTimeFromat(datePattern));
				logFullPath = logDir + logFullPath;

				logFullPathArr[appName] = logFullPath;
			}
		}
	}
}

string CfgUtil::getSocketIp() {
	vector<string> valueArr;

	parseCfgBaseItem("SOCKET", "IP", valueArr);

	string ip;
	if (valueArr.size() == 1) {
		ip = valueArr[0];
	}

	return ip;
}

string CfgUtil::getSocketPort() {
	vector<string> valueArr;

	parseCfgBaseItem("SOCKET", "PORT", valueArr);

	string port;
	if (valueArr.size() == 1) {
		port = valueArr[0];
	}

	return port;
}

string CfgUtil::getHttpIp() {
	vector<string> valueArr;

	parseCfgBaseItem("HTTP", "IP", valueArr);

	string ip;
	if (valueArr.size() == 1) {
		ip = valueArr[0];
	}

	return ip;
}

string CfgUtil::getHttpPort() {
	vector<string> valueArr;

	parseCfgBaseItem("HTTP", "PORT", valueArr);

	string port;
	if (valueArr.size() == 1) {
		port = valueArr[0];
	}

	return port;
}

string CfgUtil::getHttpRequestType() {
	vector<string> valueArr;

	parseCfgBaseItem("HTTP", "REQUEST_TYPE", valueArr);

	string requestType;
	if (valueArr.size() == 1) {
		requestType = valueArr[0];
	}

	return requestType;
}

string CfgUtil::getHttpRequestCharSet() {
	vector<string> valueArr;

	parseCfgBaseItem("HTTP", "REQUEST_CHARSET", valueArr);

	string requestCharSet;
	if (valueArr.size() == 1) {
		requestCharSet = valueArr[0];
	}

	return requestCharSet;
}

string CfgUtil::getHttpResponseCharSet() {
	vector<string> valueArr;

	parseCfgBaseItem("HTTP", "RESPONSE_CHARSET", valueArr);

	string responseCharSet;
	if (valueArr.size() == 1) {
		responseCharSet = valueArr[0];
	}

	return responseCharSet;
}
