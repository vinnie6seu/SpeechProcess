/*
 * main_test.cpp
 *
 *  Created on: 2018年8月1日
 *      Author: yaoqiaobing
 */

/************************************************************************/
// g++ -DDEBUG_TEST MrcpClient.cpp -I/home/vinnie/my_project/SpeechProcess/include/

// #ifdef DEBUG_TEST

#include <iostream>

#include "lib_acl.hpp"
#include "MrcpClient/MrcpClient.hpp"

//////////////////////////////////////////////////////////////////////////

// tts 子线程类定义
class myThreadTTS: public acl::thread {
public:
	myThreadTTS(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient) {}

	~myThreadTTS() {}
protected:
	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {
		const char* myname = "run";
		logger("%s: thread id: %lu, %lu\r\n", myname, thread_id(), acl::thread::thread_self());

		char text[256 + 1];
//		sprintf(text, "进行全文本处理，my thread id is %lu", thread_id());
		sprintf(text, "start test, my thread id is %lu", thread_id());
		char speech[251552 + 1];

//		_mrcpClient.textToSpeechProcessAll(text, speech);
		_mrcpClient.textToSpeechProcessPart(text, speech);

		stringstream ss;
		ss << thread_id() << ".wav";
		speech_to_wav(speech, sizeof(speech), ss.str().c_str());

		return NULL;
	}

	virtual void speech_to_wav(const char* speech, int len_speech,
			const char* des_path) {
		FILE* fp = fopen(des_path, "wb");
		if (NULL == fp) {
			logger_error("open %s error.\n", des_path);
			return;
		}

		fwrite(speech, len_speech, 1, fp);
		fclose(fp);
		fp = NULL;
	}

private:
	MrcpClient& _mrcpClient;
};

static void test_thread_tts(MrcpClient& mrcpClient, int theadNum) {
	int max_threads = theadNum;

	// 创建一组子线程
	std::vector<myThreadTTS*> threads;
	for (int i = 0; i < max_threads; i++) {
		myThreadTTS* thread = new myThreadTTS(mrcpClient);
		threads.push_back(thread);
		thread->set_detachable(false);
		thread->start();
	}

	// 等待所有子线程正常退出
	std::vector<myThreadTTS*>::iterator it = threads.begin();
	for (; it != threads.end(); ++it) {
		(*it)->wait();
		delete (*it);
	}

/*
	const char* myname = "test_thread";
	myThreadTTS thr(mrcpClient);  // 子线程对象实例

	// 设置线程的属性为非分离方式，以便于下面可以调用 wait
	// 等待线程结束
	thr.set_detachable(false);

	// 启动一个子线程
	if (thr.start() == false) {
		logger_error("start thread failed\r\n");
		return;
	}

	logger("%s: thread id is %lu, main thread id: %lu\r\n", myname, thr.thread_id(), acl::thread::thread_self());

	// 等待子线程运行结束
	if (thr.wait(NULL) == false)
		logger_error("wait thread failed\r\n");
	else
		logger("wait thread ok\r\n");
*/
}

//////////////////////////////////////////////////////////////////////////

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr {
	char riff[4];                      // = "RIFF"
	int size_8;                        // = FileSize - 8
	char wave[4];                      // = "WAVE"
	char fmt[4];                       // = "fmt "
	int fmt_size;		               // = 下一个结构体的大小 : 16

	short int format_tag;              // = PCM : 1
	short int channels;                // = 通道数 : 1
	int samples_per_sec;               // = 采样率 : 8000 | 6000 | 11025 | 16000
	int avg_bytes_per_sec;             // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int block_align;             // = 每采样点字节数 : wBitsPerSample / 8
	short int bits_per_sample;         // = 量化比特数: 8 | 16

	char data[4];                      // = "data";
	int data_size;                     // = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;

/**
 * 获取指定目录下，指定后缀的文件
 */
FUN_STATUS getFileVec(const char* dirName, std::vector<std::string>& fileNameVec, const char* suffix) {
	DIR *dip;
	struct dirent *dit;
	int i = 0;

	/* DIR *opendir(const char *name);
	 *
	 * Open a directory stream to argv[1] and make sure
	 * it's a readable and valid (directory) */
	if ((dip = opendir(dirName)) == NULL) {
		logger_error("fail to call opendir");
		return FAILURE;
	}
	logger("Directory stream is now open\n");

	/* struct dirent *readdir(DIR *dir);
	 *
	 * Read in the files from argv[1] and print */
	while ((dit = readdir(dip)) != NULL) {

		bool flag = false;

		std::string file = std::string(dirName) + dit->d_name;
		if (file.find(suffix) != std::string::npos) {
			flag = true;
		}

		if (flag) {
			i++;
			fileNameVec.push_back(file.c_str());
		}

	}
	logger("\n\nreaddir() [%s] found total [%d] [%s] files\n", dirName, i, suffix);

	/* int closedir(DIR *dir);
	 *
	 * Close the stream to argv[1]. And check for errors. */
	if (closedir(dip) == -1) {
		logger_error("fail to call closedir");
		return FAILURE;
	}
	logger("\nDirectory stream is now closed\n");

	return SUCCESS;
}

// asr 子线程类定义
class myThreadASR: public acl::thread {
public:
	myThreadASR(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient) {}

	~myThreadASR() {}

	void setFileName(std::string fileName) {
		_fileName = fileName;
	}
protected:
	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {
		const char* myname = "run";

		char speech[638014 + 1] = {'\0'};                // asr转成的源语音
		char text[1024 + 1] = {'\0'};                    // 目标文本

		// 读取音频文件内容
		acl::string fileBody;
		if (acl::ifstream::load(_fileName.c_str(), &fileBody) == false) {
			logger_error("load %s error", _fileName.c_str());
			return NULL;
		}

		// 检查 wav 的格式头部
		wave_pcm_hdr wavHeadStruct;
		memcpy(&wavHeadStruct, fileBody.c_str(), 44);

		logger("PCM:[%d] channels:[%d] data_size:[%d]", wavHeadStruct.format_tag, wavHeadStruct.channels, wavHeadStruct.data_size);

		// 向 http 服务端请求
		memcpy(speech, fileBody.c_str(), wavHeadStruct.size_8 + 8);

		clock_t start, finish;
		start = clock();

		_mrcpClient.speechToTextProcessAll(speech, text);

		finish = clock();

		logger("fileName:[%s] text is [%s] consume time:[%f]s", _fileName.c_str(), text, (float) (finish-start) / CLOCKS_PER_SEC);

		return NULL;
	}

private:
	MrcpClient& _mrcpClient;
	std::string _fileName;
};

static void test_thread_asr(MrcpClient& mrcpClient, std::string path) {
	std::vector<std::string> fileNameVec;
	getFileVec(path.c_str(), fileNameVec, ".wav");

	// 创建一组子线程
	std::vector<myThreadASR*> threads;
	for (int i = 0; i < fileNameVec.size(); i++) {
		myThreadASR* thread = new myThreadASR(mrcpClient);
		threads.push_back(thread);
		thread->set_detachable(false);
		thread->setFileName(fileNameVec[i].c_str());
		thread->start();
	}

	// 等待所有子线程正常退出
	std::vector<myThreadASR*>::iterator it = threads.begin();
	for (; it != threads.end(); ++it) {
		(*it)->wait();
		delete (*it);
	}
}

//////////////////////////////////////////////////////////////////////////

class myThreadASRPacket: public acl::thread {
public:
	myThreadASRPacket(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient), _id(-1) {}

	~myThreadASRPacket() {}

	void setFileName(std::string fileName) {
		_fileName = fileName;
	}
protected:
	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {

		if (work_function() != SUCCESS) {

			// 需要清理
			_mrcpClient.asrExceptionHandle(_id);

			return NULL;
		}

		return NULL;
	}

	virtual FUN_STATUS work_function() {
		clock_t start, finish;
		start = clock();

		//////////////////////////////////////////

		char speech[SPEECH_LEN] = {'\0'};                // asr 的源语音
		char text[TEXT_LEN] = {'\0'};                    // 目标文本

		int data_length = 0;


		if (_fileName.find("wav") != std::string::npos) {
			// 1.读取音频文件内容
			acl::string fileBody;
			if (acl::ifstream::load(_fileName.c_str(), &fileBody) == false) {
				logger_error("load %s error", _fileName.c_str());
				return FAILURE;
			}

			// 2.检查 wav 的格式头部
			wave_pcm_hdr wavHeadStruct;
			memcpy(&wavHeadStruct, fileBody.c_str(), 44);

			logger("PCM:[%d] channels:[%d] data_size:[%d]", wavHeadStruct.format_tag, wavHeadStruct.channels, wavHeadStruct.data_size);

	        // 3.拷贝语音数据到 speech 数组中
			memcpy(speech, fileBody.c_str(), wavHeadStruct.size_8 + 8);

			// 4.音频长度
			data_length = wavHeadStruct.size_8 + 8;

		} else if (_fileName.find("raw") != std::string::npos) {
			// 1.读取音频文件内容
			acl::string fileBody;
			if (acl::ifstream::load(_fileName.c_str(), &fileBody) == false) {
				logger_error("load %s error", _fileName.c_str());
				return FAILURE;
			}

	        // 2.拷贝语音数据到 speech 数组中
			memcpy(speech, fileBody.c_str(), fileBody.length());

			logger("audio type-raw data_size:[%d]", fileBody.length());

			// 3.音频长度
			data_length = fileBody.length();
		}


		// 4.请求 java 收取 asr 结果
		vector<AsrSpeechTransResult> asr_trans_result_vec;

		// 4.1 init，找到当次准备写入的id，向 java 服务端发送[id, MSP_AUDIO_INIT]
		if (_mrcpClient.asrSpeechPacketSend(_id, NULL, 0, 0, MSP_AUDIO_INIT) != SUCCESS) {
			logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_INIT]", _id);
			return FAILURE;
		}
		logger("id:[%d] success to send [MSP_AUDIO_INIT] packet", _id);

		// 4.2 continue，向 java 服务端发送[id, MSP_AUDIO_CONTINUE, cur_send_speech_num]，写语音包数据，收取文本包
		int FRAME_LEN = SPEECH_PACKET_LEN;
		int num = 0;
		while (num * FRAME_LEN <= data_length) {
			if ((num + 1) * FRAME_LEN > data_length) {

				logger("id:[%d] speech from[%d] to[%d]\n", _id , num * FRAME_LEN, data_length);

				// 注意这里因为用的是音频文件最后一段肯定不能取整，但是为了满足发送的 packet 大小一致，依然使用SPEECH_PACKET_LEN
				if (_mrcpClient.asrSpeechPacketSend(_id, speech + num * FRAME_LEN, SPEECH_PACKET_LEN, num, MSP_AUDIO_CONTINUE) != SUCCESS) {
					logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_CONTINUE]", _id);
					return FAILURE;
				}

			} else {

				logger("id:[%d] speech from[%d] to[%d]\n", _id , num * FRAME_LEN, (num + 1) * FRAME_LEN);

				if (_mrcpClient.asrSpeechPacketSend(_id, speech + num * FRAME_LEN, SPEECH_PACKET_LEN, num, MSP_AUDIO_CONTINUE) != SUCCESS) {
					logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_CONTINUE]", _id);
					return FAILURE;
				}

			}
			num++;
			logger("id:[%d] cur_send_speech_num:[%d] success to send [MSP_AUDIO_CONTINUE] packet", _id, num);

			usleep(200 * 1000); // 模拟人说话时间间隙。200ms对应10帧的音频

			logger("id:[%d] status:[MSP_AUDIO_CONTINUE] start to receive text packet", _id);
            // 中间处理过程收取结果
			asr_trans_result_vec.clear();
			if (_mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_CONTINUE) == FAILURE) {
				logger_error("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_CONTINUE]", _id);
				return FAILURE;
			}
			logger("id:[%d] status:[MSP_AUDIO_CONTINUE] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

			for (int i = 0; i < asr_trans_result_vec.size(); i++) {
				logger("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
						asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
						asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
						asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
						asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);
			}
		}

		// 4.3 last，告知 java 服务端[id, MSP_AUDIO_LAST, totalSendPacketNum]
		if (_mrcpClient.asrSpeechPacketSend(_id, NULL, 0, num, MSP_AUDIO_LAST) != SUCCESS) {
			logger_error("fail to call function asrSpeechPacketSend, id:[%d] status:[MSP_AUDIO_LAST]", _id);
			return FAILURE;
		}
		logger("id:[%d] success to send [MSP_AUDIO_LAST] packet", _id);

        // 4.4 最后收取所有结果
		logger("id:[%d] status:[MSP_AUDIO_LAST] start to receive text packet", _id);

		asr_trans_result_vec.clear();
		if (_mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_LAST) == FAILURE) {
			logger_error("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", _id);
			return FAILURE;
		}
		logger("id:[%d] status:[MSP_AUDIO_LAST] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

		for (int i = 0; i < asr_trans_result_vec.size(); i++) {
			logger("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
					asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
					asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
					asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
					asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);

			if (asr_trans_result_vec[i]._cur_result._is_final == true) {
				memcpy(text, asr_trans_result_vec[i]._cur_result._transcript, strlen(asr_trans_result_vec[i]._cur_result._transcript));
			}
		}


		//////////////////////////////////////////

		finish = clock();

		logger("fileName:[%s] text is [%s] consume time:[%f]s", _fileName.c_str(), text, (float) (finish-start) / CLOCKS_PER_SEC);

		return SUCCESS;
	}

private:
	MrcpClient& _mrcpClient;
	std::string _fileName;
	int _id;                        // 当前线程写入数据的 item id
};

static void test_thread_asr_packet(MrcpClient& mrcpClient, std::string& path, std::string& fileSuffix) {
	std::vector<std::string> fileNameVec;
	getFileVec(path.c_str(), fileNameVec, fileSuffix.c_str());

	// 创建一组子线程
	std::vector<myThreadASRPacket*> threads;
	for (int i = 0; i < fileNameVec.size(); i++) {
		myThreadASRPacket* thread = new myThreadASRPacket(mrcpClient);
		threads.push_back(thread);
		thread->set_detachable(false);
		thread->setFileName(fileNameVec[i].c_str());
		thread->start();
	}

	// 等待所有子线程正常退出
	std::vector<myThreadASRPacket*>::iterator it = threads.begin();
	for (; it != threads.end(); ++it) {
		(*it)->wait();
		delete (*it);
	}
}

//////////////////////////////////////////////////////////////////////////


// asr 子线程类定义
class myThreadBlockQueue: public acl::thread {
public:
	myThreadBlockQueue(MrcpClient& mrcpClient) : _mrcpClient(mrcpClient) {}

	~myThreadBlockQueue() {}

	void setFileName(std::string fileName) {
		_fileName = fileName;
	}
	void setMode(std::string mode) {
		_mode = mode;
	}

protected:

	// 基类纯虚函数，当在主线程中调用线程实例的 start 函数时
	// 该虚函数将会被调用
	virtual void* run() {
		const char* myname = "run";

		printf("%s: thread id: %lu, %lu\r\n", myname, thread_id(), acl::thread::thread_self());

		int blockQueueId = 300;

		if (_mode == "send") {

			AsrSpeechTransResult asr_trans_result;
			std::string json_cstr = "{\"cur_predict\":{\"confidence\":0.0,\"is_final\":false,\"stability\":0.0},\"cur_result\":{\"confidence\":0.9854616522789001,\"is_final\":true,\"stability\":0.0,\"transcript\":\"how old is the Brooklyn Bridge how old is the Brooklyn Bridge\"},\"is_exception\":false}";


			memcpy(asr_trans_result._cur_result._transcript, json_cstr.c_str(), sizeof(asr_trans_result._cur_result._transcript) - 1);
			asr_trans_result._cur_result._is_final = true;

			cout << "------ send start ------" << endl;

			ShmSpeechToText shmSpeechToText = _mrcpClient.getShmSpeechToText();
			ShmSpeechToTextHead* shmSpeechToTextHead = _mrcpClient.getShmSpeechToTextHead();
			map<int, std::tr1::shared_ptr<BlockingShmQueue> > allBlockingShmQueue= _mrcpClient.getAllBlockingShmQueue();
			if (shmSpeechToText.writeTextBlockingQueue(shmSpeechToTextHead, blockQueueId, allBlockingShmQueue[blockQueueId], asr_trans_result) == FAILURE) {
				printf("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", blockQueueId);
				return NULL;
			}

			cout << "------ send success ------" << endl;
		} else if (_mode == "receive") {
				int _id = blockQueueId;
				char text[1024 + 1] = {'\0'};
				vector<AsrSpeechTransResult> asr_trans_result_vec;

				printf("id:[%d] status:[MSP_AUDIO_LAST] start to receive text packet", _id);

				asr_trans_result_vec.clear();
				if (_mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_LAST) == FAILURE) {
					printf("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", _id);
					return NULL;
				}
				printf("id:[%d] status:[MSP_AUDIO_LAST] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

				for (int i = 0; i < asr_trans_result_vec.size(); i++) {
					printf("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
							asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
							asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
							asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
							asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);

					if (asr_trans_result_vec[i]._cur_result._is_final == true) {
						memcpy(text, asr_trans_result_vec[i]._cur_result._transcript, strlen(asr_trans_result_vec[i]._cur_result._transcript));
					}
				}

				printf("text is:[%s]", text);
		}

		return NULL;
	}

private:
	MrcpClient& _mrcpClient;
	std::string _fileName;
	std::string _mode;
};

FUN_STATUS test_blockqueue(MrcpClient& mrcpClient, int blockQueueId) {

//	// 创建一组子线程
//	std::vector<myThreadBlockQueue*> threads;
//	for (int i = 0; i < 2; i++) {
//		myThreadBlockQueue* thread = new myThreadBlockQueue(mrcpClient);
//		threads.push_back(thread);
//		thread->set_detachable(false);
//
//		if ((i & 1) != 0) {
//			thread->setMode("send");
//		} else {
//			thread->setMode("receive");
//		}
//
//		thread->start();
//	}
//
//	// 等待所有子线程正常退出
//	std::vector<myThreadBlockQueue*>::iterator it = threads.begin();
//	for (; it != threads.end(); ++it) {
//		(*it)->wait();
//		delete (*it);
//	}

	int _id = blockQueueId;

	printf("step in test_blockqueue, listen id:[%d]", _id);


	char text[1024 + 1] = {'\0'};
	vector<AsrSpeechTransResult> asr_trans_result_vec;

	printf("id:[%d] status:[MSP_AUDIO_LAST] start to receive text packet", _id);

	asr_trans_result_vec.clear();
	if (mrcpClient.asrTextBlockRecv(_id, asr_trans_result_vec, MSP_AUDIO_LAST) == FAILURE) {
		printf("fail to call function asrTextBlockRecv, id:[%d] status:[MSP_AUDIO_LAST]", _id);
		return NULL;
	}
	printf("id:[%d] status:[MSP_AUDIO_LAST] success to receive text packet:[%d]", _id, asr_trans_result_vec.size());

	for (int i = 0; i < asr_trans_result_vec.size(); i++) {
		printf("revice speech trans result, cur_result:[%s %f %s %f], cur_predict:[%s %f %s %f]\n",
				asr_trans_result_vec[i]._cur_result._transcript, asr_trans_result_vec[i]._cur_result._stability,
				asr_trans_result_vec[i]._cur_result._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_result._confidence,
				asr_trans_result_vec[i]._cur_predict._transcript, asr_trans_result_vec[i]._cur_predict._stability,
				asr_trans_result_vec[i]._cur_predict._is_final ? "true" : "false", asr_trans_result_vec[i]._cur_predict._confidence);

		if (asr_trans_result_vec[i]._cur_result._is_final == true) {
			memcpy(text, asr_trans_result_vec[i]._cur_result._transcript, strlen(asr_trans_result_vec[i]._cur_result._transcript));
		}
	}

	printf("text is:[%s]", text);


	return SUCCESS;
}

static void usage(const char* procname)
{
	printf("usage: %s [-h help] [-t test_type tts/asr/asr_packet/test_blockqueue] [-n tts_client_thread_num] [-d asr_client_find_wav_path] [-x file_suffix .wav/.raw] [-i block_queue_id 999]\r\n", procname);
	printf("para options:\r\n");
	printf("\t %s -t tts -n 20\r\n", procname);
	printf("\t %s -t asr -d .\/ -x .raw\r\n", procname);
	printf("\t %s -t asr_packet -d .\/ -x .raw\r\n", procname);
	printf("\t %s -t test_blockqueue -i 999\r\n", procname);
}

int main(int argc, char* argv[]) {

	int ch;
	std::string type;
	int theadNum;
	std::string path;
	std::string fileSuffix;
	int blockQueueId;
	while ((ch = getopt(argc, argv, "ht:n:d:x:i:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 't':
			type = optarg;
			break;
		case 'n':
			theadNum = atoi(optarg);
			break;
		case 'd':
			path = optarg;
			break;
		case 'x':
			fileSuffix = optarg;
			break;
		case 'i':
			blockQueueId = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			return 0;
		}
	}

	// 0.初始化 mrcp 客户端
	MrcpClient& mrcpClient = acl::singleton2<MrcpClient>::get_instance();

	if (type == "tts") {
		mrcpClient.init(CLIENT_TYPE_TTS);

		// 1.多线程测试 tts 程序，运行若干线程请求 socket 服务端，给出文本转成语音
		test_thread_tts(mrcpClient, theadNum);
	} else if (type == "asr") {
		mrcpClient.init(CLIENT_TYPE_ASR);
		// 2.多线程测试 asr 程序，将制度目录下的 wav 音频数据取出发给 http 服务端，给出语音转成文本
		test_thread_asr(mrcpClient, path);
	} else if (type == "asr_packet") {
		mrcpClient.init(CLIENT_TYPE_ASR_PACKET);
		// 3.多线程测试 asr 程序，将制度目录下的 wav 音频数据取出[分割数据包]发给 http 服务端，给出语音转成文本
		test_thread_asr_packet(mrcpClient, path, fileSuffix);
	} else if (type == "test_blockqueue") {
		mrcpClient.init(CLIENT_TYPE_ASR_PACKET);
		test_blockqueue(mrcpClient, blockQueueId);
	} else {
		usage(argv[0]);
		return 0;
	}

	return 0;
}

// #endif



