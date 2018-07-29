/*
 * com_fuwei_asr_SpeechTranscript_modular_service_JniShmService.cpp
 *
 *  Created on: 2018年7月9日
 *      Author: yaoqiaobing
 */

#include <sstream>
#include <string>

#include "Define.hpp"
#include "Shm/asr/ShmSpeechToText.hpp"
#include "MrcpClient/MrcpClient.hpp"
#include "com_fuwei_asr_SpeechTranscript_modular_service_JniShmService.h"
#include "jni_md.h"
#include "jni.h"

#include "json.hpp"
using json = nlohmann::json;

// 调用 java 中函数打印日志
#define LOG_INFO(pEnv, object, info) callback_printLogInfo(pEnv, object, info)
#define LOG_ERROR(pEnv, object, error) callback_printLogError(pEnv, object, error)
//#define LOG_INFO(pEnv, object, info)
//#define LOG_ERROR(pEnv, object, error)

// 控制 .so 共享内存参数
bool initFlag = false;
ShmSpeechToText shmSpeechToText;
ShmSpeechToTextHead* shmSpeechToTextHead = NULL;

map<int, std::tr1::shared_ptr<NoBlockingShmQueue> > allNoBlockingShmQueue;
map<int, std::tr1::shared_ptr<BlockingShmQueue> > allBlockingShmQueue;

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_shmInit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1shmInit
  (JNIEnv *pEnv, jobject object, jint shmSpeechToTextKey, jint shmSpeechToTextNum, jint mrcpClientType) {

	if (initFlag) {
		return ;
	}

	// 1.初始化语音转文本共享内存
	shmSpeechToText.init(shmSpeechToTextKey, shmSpeechToTextNum);

	std::stringstream info;
	info << "success to init shm:[speech to text], key:[" << shmSpeechToTextKey << "], num:[" << shmSpeechToTextNum << "]";
	LOG_INFO(pEnv, object, info.str().c_str());

	// 2.将共享内存加载到本进程中
	if (shmSpeechToText.procLoadShm((void**) &shmSpeechToTextHead) == FAILURE) {
		LOG_ERROR(pEnv, object, shmSpeechToText.getShmLastError().c_str());
		return ;
	}

	LOG_INFO(pEnv, object, "success to add shmSpeechToText");

	int CLIENT_TYPE_ASR_PACKET_INT = CLIENT_TYPE_ASR_PACKET;
	info.clear();
	info.str("");
	info << "client type is " << mrcpClientType << " CLIENT_TYPE_ASR_PACKET is " << CLIENT_TYPE_ASR_PACKET;
	LOG_INFO(pEnv, object, info.str().c_str());

	if (mrcpClientType == CLIENT_TYPE_ASR_PACKET_INT) {
		// 3.调用allItemShmQueueInit

		LOG_INFO(pEnv, object, "start to call function allItemShmQueueInit");

		// SPEECH_LEN, SPEECH_PACKET_LEN TEXT_LEN, TEXT_PACKET_LEN
		info.clear();
		info.str("");
		info << "SPEECH_LEN is " << SPEECH_LEN << " SPEECH_PACKET_LEN is " << SPEECH_PACKET_LEN << " TEXT_LEN is " << TEXT_LEN << " TEXT_PACKET_LEN is " << TEXT_PACKET_LEN;
		LOG_INFO(pEnv, object, info.str().c_str());

		shmSpeechToText.allItemShmQueueInit(shmSpeechToTextHead, allNoBlockingShmQueue, allBlockingShmQueue);

		LOG_INFO(pEnv, object, "success to call function allItemShmQueueInit");
	}

	initFlag = true;
}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_readSpeechRecordShm
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1readSpeechRecordShm
  (JNIEnv *pEnv, jobject object, jint id) {

	ShmSpeechToTextItem shmSpeechToTextItem;
	memset(&shmSpeechToTextItem, '\0', sizeof(ShmSpeechToTextItem));

	shmSpeechToText.readSpeechRecordShm(shmSpeechToTextHead, id, shmSpeechToTextItem);

	std::stringstream info;
	info << "success to read speech at id:[" << id << "] speechLen:[" << sizeof(shmSpeechToTextItem._speech) << "]";
	LOG_INFO(pEnv, object, info.str().c_str());

	return pEnv->NewStringUTF(shmSpeechToTextItem._speech);

}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_readSpeechRecordByteArrShm
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1readSpeechRecordByteArrShm
  (JNIEnv *pEnv, jobject object, jint id) {

	ShmSpeechToTextItem shmSpeechToTextItem;
	memset(&shmSpeechToTextItem, '\0', sizeof(ShmSpeechToTextItem));

	shmSpeechToText.readSpeechRecordShm(shmSpeechToTextHead, id, shmSpeechToTextItem);

	std::stringstream info;
	info << "success to read speech at id:[" << id << "] speechLen:[" << sizeof(shmSpeechToTextItem._speech) << "]";
	LOG_INFO(pEnv, object, info.str().c_str());

	// 将语音数据放入 byte[] 中
	jbyteArray speechByteArr = pEnv->NewByteArray(sizeof(shmSpeechToTextItem._speech));
	pEnv->SetByteArrayRegion(speechByteArr, 0, sizeof(shmSpeechToTextItem._speech) - 1, (const signed char *)shmSpeechToTextItem._speech);

	return speechByteArr;

}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_writeTextRecordShm
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1writeTextRecordShm
  (JNIEnv *pEnv, jobject object, jint id, jstring text) {

	ShmSpeechToTextItem shmSpeechToTextItem;
	memset(&shmSpeechToTextItem, '\0', sizeof(ShmSpeechToTextItem));

	const char *text_cstr = (char *)pEnv->GetStringUTFChars(text, 0);
	memcpy(shmSpeechToTextItem._text, text_cstr, strlen(text_cstr));

	shmSpeechToText.writeTextRecordShm(shmSpeechToTextHead, id, shmSpeechToTextItem);

	std::stringstream info;
	info << "success to write text at id:[" << id << "] textLen:[" << strlen(text_cstr) << "]";
	LOG_INFO(pEnv, object, info.str().c_str());
}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_shmTerm
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1shmTerm
  (JNIEnv *pEnv, jobject object) {

	if (shmSpeechToText.procDetachShm((void*) shmSpeechToTextHead) == FAILURE) {
		LOG_ERROR(pEnv, object, shmSpeechToText.getShmLastError().c_str());
	}
	shmSpeechToTextHead = NULL;
	initFlag = false;

	LOG_INFO(pEnv, object, "success to delete shmSpeechToText");
}

void callback_printLogInfo(JNIEnv *pEnv, jobject object, const char* info) {
    // 1、得到字节码 FindClass(pEnv, 类的全路径);
    jclass codeClass = (*pEnv).FindClass("com/fuwei/asr/SpeechTranscript/modular/service/JniShmService");
    // 2、得到方法 GetMethodID(pEnv, codeClass, 方法名, 方法签名);
    // 方法签名是要调用的方法名对应的签名
    jmethodID methodID = (*pEnv).GetMethodID(codeClass, "printLogInfo", "(Ljava/lang/String;)V");
    // 3、实例化该类
    jobject obj = (*pEnv).AllocObject(codeClass);
    // 4.调用Java类中的方法
    (*pEnv).CallVoidMethod(obj, methodID, pEnv->NewStringUTF(info));
}

void callback_printLogError(JNIEnv *pEnv, jobject object, const char* error) {
    // 1、得到字节码 FindClass(pEnv, 类的全路径);
    jclass codeClass = (*pEnv).FindClass("com/fuwei/asr/SpeechTranscript/modular/service/JniShmService");
    // 2、得到方法 GetMethodID(pEnv, codeClass, 方法名, 方法签名);
    // 方法签名是要调用的方法名对应的签名
    jmethodID methodID = (*pEnv).GetMethodID(codeClass, "printLogError", "(Ljava/lang/String;)V");
    // 3、实例化该类
    jobject obj = (*pEnv).AllocObject(codeClass);
    // 4.调用Java类中的方法
    (*pEnv).CallVoidMethod(obj, methodID, pEnv->NewStringUTF(error));
}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_shmSpeechPacketReceive
 * Signature: (IZILjava/lang/Integer;Ljava/lang/Integer;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1shmSpeechPacketReceive
  (JNIEnv *pEnv, jobject object, jint id, jboolean is_complete_send, jint total_send_packet_num, jobject batch_num, jobject is_complete_receive) {

	std::stringstream info;

	// 1.读取语音数据
	char speech[SPEECH_LEN] = {'\0'};
	int batch_num_int = 0;
	bool is_complete_receive_bool = false;

	shmSpeechToText.readSpeechNoBlockingQueueBatch(allNoBlockingShmQueue[id], speech, SPEECH_LEN, is_complete_send, total_send_packet_num, batch_num_int, is_complete_receive_bool);

	// 2.修改变量 batch_num 和 is_complete_receive
	info.clear();
	info.str("");
	info << "success to call readSpeechNoBlockingQueueBatch id:[" << id << "] get batch_num_int:[" << batch_num_int << "] get is_complete_receive_bool:[" << is_complete_receive_bool << "]";
	LOG_INFO(pEnv, object, info.str().c_str());

	// -----------------------------------------
	jclass integerClass;
	jfieldID valueIntegerFieldId;

	integerClass = pEnv->FindClass("java/lang/Integer");
	if (integerClass == NULL) {
		LOG_INFO(pEnv, object, "FindClass failed");
		return NULL;
	}

	valueIntegerFieldId = pEnv->GetFieldID(integerClass, "value", "I");
	if (id == NULL) {
		LOG_INFO(pEnv, object, "GetFiledID failed");
		return NULL;
	}

	pEnv->SetIntField(batch_num, valueIntegerFieldId, batch_num_int);
	LOG_INFO(pEnv, object, "set batch_num_int");
	// -----------------------------------------
	jclass boolean_to_integerClass;
	jfieldID valueBoolean_To_IntegerFieldId;

	boolean_to_integerClass = pEnv->FindClass("java/lang/Integer");
	if (boolean_to_integerClass == NULL) {
		LOG_INFO(pEnv, object, "FindClass failed");
		return NULL;
	}

	valueBoolean_To_IntegerFieldId = pEnv->GetFieldID(boolean_to_integerClass, "value", "I");
	if (id == NULL) {
		LOG_INFO(pEnv, object, "GetFiledID failed");
		return NULL;
	}

	if (is_complete_receive_bool) {
		int bool_true = 1;
		pEnv->SetIntField(is_complete_receive, valueBoolean_To_IntegerFieldId, bool_true);
	} else {
		int bool_false = 0;
		pEnv->SetBooleanField(is_complete_receive, valueBoolean_To_IntegerFieldId, bool_false);
	}
	LOG_INFO(pEnv, object, "set is_complete_receive");
    // -----------------------------------------

	// 3.返回语音数据
	info.clear();
	info.str("");
	info << "success to read speech at id:[" << id << "] batch_num:[" << batch_num_int << "] is_complete_receive:[" << is_complete_receive_bool << "]";
	LOG_INFO(pEnv, object, info.str().c_str());

	// 将语音数据放入 byte[] 中
	jbyteArray speechBytePacketArr = pEnv->NewByteArray(batch_num_int * SPEECH_PACKET_LEN);
	pEnv->SetByteArrayRegion(speechBytePacketArr, 0, batch_num_int * SPEECH_PACKET_LEN, (const signed char *)speech);

	return speechBytePacketArr;
}

typedef struct {
	jclass asrSpeechTransItemClass;

	jfieldID transcriptFieldId;
	jfieldID stabilityFieldId;

	jfieldID is_finalFieldId;
	jfieldID confidenceFieldId;
} AsrSpeechTransItemFieldIds;

typedef struct {
	jclass asrSpeechTransResultClass;

	jfieldID _cur_resultFieldId;
	AsrSpeechTransItemFieldIds _cur_result;

	jfieldID _cur_predictFieldId;
	AsrSpeechTransItemFieldIds _cur_predict;

	jfieldID is_exceptionFieldId;
	jfieldID exception_strFieldId;
} AsrSpeechTransResultFieldIds;

static void transAsrResponseData(JNIEnv *pEnv, jobject& asrResponse, AsrSpeechTransResult& asr_trans_result) {
/*
	// -----------------------------------------
	// 1.取出类成员字段 id
	AsrSpeechTransResultFieldIds asrSpeechTransResultFieldIds;

	asrSpeechTransResultFieldIds.asrSpeechTransResultClass = pEnv->GetObjectClass(asrResponse);

	asrSpeechTransResultFieldIds._cur_resultFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds.asrSpeechTransResultClass, "_cur_result", "com/fuwei/asr/SpeechTranscript/modular/entity/AsrShmResponseItem");
	jobject cur_result_jobject = pEnv->GetObjectField(asrResponse, asrSpeechTransResultFieldIds._cur_resultFieldId);
	asrSpeechTransResultFieldIds._cur_result.asrSpeechTransItemClass = pEnv->GetObjectClass(cur_result_jobject);
	// asrSpeechTransResultFieldIds._cur_result.asrSpeechTransItemClass = pEnv->FindClass("com/fuwei/asr/SpeechTranscript/modular/entity/AsrShmResponseItem");
	asrSpeechTransResultFieldIds._cur_result.transcriptFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_result.asrSpeechTransItemClass, "transcript", "Ljava/lang/String;");
	asrSpeechTransResultFieldIds._cur_result.stabilityFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_result.asrSpeechTransItemClass, "stability", "D");
	asrSpeechTransResultFieldIds._cur_result.is_finalFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_result.asrSpeechTransItemClass, "is_final", "Z");
	asrSpeechTransResultFieldIds._cur_result.confidenceFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_result.asrSpeechTransItemClass, "confidence", "D");

	asrSpeechTransResultFieldIds._cur_predictFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds.asrSpeechTransResultClass, "_cur_predict", "com/fuwei/asr/SpeechTranscript/modular/entity/AsrShmResponseItem");
	jobject cur_predict_jobject = pEnv->GetObjectField(asrResponse, asrSpeechTransResultFieldIds._cur_predictFieldId);
	asrSpeechTransResultFieldIds._cur_predict.asrSpeechTransItemClass = pEnv->GetObjectClass(cur_predict_jobject);
	// asrSpeechTransResultFieldIds._cur_predict.asrSpeechTransItemClass = pEnv->FindClass("com/fuwei/asr/SpeechTranscript/modular/entity/AsrShmResponseItem");
	asrSpeechTransResultFieldIds._cur_predict.transcriptFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_predict.asrSpeechTransItemClass, "transcript", "Ljava/lang/String;");
	asrSpeechTransResultFieldIds._cur_predict.stabilityFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_predict.asrSpeechTransItemClass, "stability", "D");
	asrSpeechTransResultFieldIds._cur_predict.is_finalFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_predict.asrSpeechTransItemClass, "is_final", "Z");
	asrSpeechTransResultFieldIds._cur_predict.confidenceFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds._cur_predict.asrSpeechTransItemClass, "confidence", "D");

	asrSpeechTransResultFieldIds.is_exceptionFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds.asrSpeechTransResultClass, "is_exception", "Z");
	asrSpeechTransResultFieldIds.exception_strFieldId = pEnv->GetFieldID(asrSpeechTransResultFieldIds.asrSpeechTransResultClass, "exception_str", "Ljava/lang/String;");

	// -----------------------------------------
	// 2.取值
	jstring cur_result_transcript = (jstring) pEnv->GetObjectField(cur_result_jobject, asrSpeechTransResultFieldIds._cur_result.transcriptFieldId);
	const char *cur_result_transcript_cstr = (char *)pEnv->GetStringUTFChars(cur_result_transcript, 0);
	memcpy(asr_trans_result._cur_result._transcript, cur_result_transcript_cstr, strlen(cur_result_transcript_cstr));
	asr_trans_result._cur_result._stability = pEnv->GetDoubleField(cur_result_jobject, asrSpeechTransResultFieldIds._cur_result.stabilityFieldId);
	asr_trans_result._cur_result._is_final = pEnv->GetBooleanField(cur_result_jobject, asrSpeechTransResultFieldIds._cur_result.is_finalFieldId);
	asr_trans_result._cur_result._confidence = pEnv->GetDoubleField(cur_result_jobject, asrSpeechTransResultFieldIds._cur_result.confidenceFieldId);

	jstring cur_predict_transcript = (jstring) pEnv->GetObjectField(cur_predict_jobject, asrSpeechTransResultFieldIds._cur_predict.transcriptFieldId);
	const char *cur_predict_transcript_cstr = (char *)pEnv->GetStringUTFChars(cur_predict_transcript, 0);
	memcpy(asr_trans_result._cur_predict._transcript, cur_predict_transcript_cstr, strlen(cur_predict_transcript_cstr));
	asr_trans_result._cur_predict._stability = pEnv->GetDoubleField(cur_predict_jobject, asrSpeechTransResultFieldIds._cur_predict.stabilityFieldId);
	asr_trans_result._cur_predict._is_final = pEnv->GetBooleanField(cur_predict_jobject, asrSpeechTransResultFieldIds._cur_predict.is_finalFieldId);
	asr_trans_result._cur_predict._confidence = pEnv->GetDoubleField(cur_predict_jobject, asrSpeechTransResultFieldIds._cur_predict.confidenceFieldId);


	asr_trans_result._is_exception = pEnv->GetBooleanField(asrResponse, asrSpeechTransResultFieldIds.is_exceptionFieldId);
	jstring exception_str = (jstring) pEnv->GetObjectField(asrResponse, asrSpeechTransResultFieldIds.exception_strFieldId);
	const char *exception_cstr = (char *)pEnv->GetStringUTFChars(exception_str, 0);
	memcpy(asr_trans_result._exception_str, exception_cstr, strlen(exception_cstr));

	// -----------------------------------------

*/
}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_shmTextPacketSend
 * Signature: (ILcom/fuwei/asr/SpeechTranscript/modular/entity/AsrShmResponse;)V
 */
JNIEXPORT void JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1shmTextPacketSend__ILcom_fuwei_asr_SpeechTranscript_modular_entity_AsrShmResponse_2
  (JNIEnv *pEnv, jobject object, jint id, jobject asrResponse) {

	// 填写包数据
	AsrSpeechTransResult asr_trans_result;
	transAsrResponseData(pEnv, asrResponse, asr_trans_result);

	// 发送包数据
	shmSpeechToText.writeTextBlockingQueue(shmSpeechToTextHead, id, allBlockingShmQueue[id], asr_trans_result);

	std::stringstream info;
	info << "success to send text packet at id:[" << id << "] text:[" << asr_trans_result._cur_result._transcript << "]";
	LOG_INFO(pEnv, object, info.str().c_str());
}

/**
 * 解析 json
 */
static void transAsrResponseData(JNIEnv *pEnv, jobject object, string& json_cstr, AsrSpeechTransResult& asrSpeechTransResult) {

	try {
		json jsonObject = json::parse(json_cstr);

		if (jsonObject.find("is_exception") != jsonObject.end()) {

			asrSpeechTransResult._is_exception = *jsonObject.find("is_exception");
		}

		if (jsonObject.find("exception_str") != jsonObject.end()) {

			string tmp = *jsonObject.find("exception_str");
			memcpy(asrSpeechTransResult._exception_str, tmp.c_str(),
					sizeof(asrSpeechTransResult._exception_str) - 1);
		}

		//////
		if (jsonObject.find("cur_result") != jsonObject.end()) {
			// there is an entry with key "pi"
			json resultJsonObject = *jsonObject.find("cur_result");

			if (resultJsonObject.find("transcript") != resultJsonObject.end()) {

				string tmp = *resultJsonObject.find("transcript");
				memcpy(asrSpeechTransResult._cur_result._transcript, tmp.c_str(),
						sizeof(asrSpeechTransResult._cur_result._transcript) - 1);
			}

			if (resultJsonObject.find("stability") != resultJsonObject.end()) {

				asrSpeechTransResult._cur_result._stability =
						*resultJsonObject.find("stability");
			}

			if (resultJsonObject.find("is_final") != resultJsonObject.end()) {

				asrSpeechTransResult._cur_result._is_final = *resultJsonObject.find(
						"is_final");
			}

			if (resultJsonObject.find("confidence") != resultJsonObject.end()) {

				asrSpeechTransResult._cur_result._confidence =
						*resultJsonObject.find("confidence");
			}
		}

		////////
		if (jsonObject.find("cur_predict") != jsonObject.end()) {
			// there is an entry with key "pi"
			json predictJsonObject = *jsonObject.find("cur_predict");

			if (predictJsonObject.find("transcript") != predictJsonObject.end()) {

				string tmp = *predictJsonObject.find("transcript");
				memcpy(asrSpeechTransResult._cur_predict._transcript, tmp.c_str(),
						sizeof(asrSpeechTransResult._cur_predict._transcript) - 1);
			}

			if (predictJsonObject.find("stability") != predictJsonObject.end()) {

				asrSpeechTransResult._cur_predict._stability =
						*predictJsonObject.find("stability");
			}

			if (predictJsonObject.find("is_final") != predictJsonObject.end()) {

				asrSpeechTransResult._cur_predict._is_final =
						*predictJsonObject.find("is_final");
			}

			if (predictJsonObject.find("confidence") != predictJsonObject.end()) {

				asrSpeechTransResult._cur_predict._confidence =
						*predictJsonObject.find("confidence");
			}
		}
	} catch (std::exception& e) {
		LOG_ERROR(pEnv, object, e.what());
	}

}

/*
 * Class:     com_fuwei_asr_SpeechTranscript_modular_service_JniShmService
 * Method:    JNI_shmTextPacketSend
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_fuwei_asr_SpeechTranscript_modular_service_JniShmService_JNI_1shmTextPacketSend__ILjava_lang_String_2
  (JNIEnv *pEnv, jobject object, jint id, jstring asrResponseJsonStr) {

	std::stringstream info;

	// 填写包数据
	AsrSpeechTransResult asr_trans_result;

	string json_cstr = (char *)pEnv->GetStringUTFChars(asrResponseJsonStr, 0);
	info.clear();
	info.str("");
	info << "==== response json string:[" << json_cstr << "] ====";
	LOG_INFO(pEnv, object, info.str().c_str());

	transAsrResponseData(pEnv, object, json_cstr, asr_trans_result);

	LOG_INFO(pEnv, object, "==== success to trans json to struct ====");

	// 发送包数据
	shmSpeechToText.writeTextBlockingQueue(shmSpeechToTextHead, id, allBlockingShmQueue[id], asr_trans_result);

	info.clear();
	info.str("");
	info << "success to send text packet at id:[" << id << "] text:[" << asr_trans_result._cur_result._transcript << "]";
	LOG_INFO(pEnv, object, info.str().c_str());
}
