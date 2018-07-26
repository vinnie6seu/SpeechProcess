/*
 * test_struct.cpp
 *
 *  Created on: 2018年6月11日
 *      Author: yaoqiaobing
 */


#include <iostream>
#include <string.h>
#include <stdio.h>
#include "SpeechSynthesis/tts_sdk.hpp"
extern "C" {
	#include "IflyTek/msp_errors.h"
}

using namespace std;

typedef struct _wave_pcm_hdr {
	char            riff[4];                // = "RIFF"
	int		        size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int		        fmt_size;		        // = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int		        samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int		        avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int		        data_size;              // = 纯数据长度 : FileSize - 44
} wave_pcm_hdr;

// g++ test_tts_sdk.cpp -o test_tts_sdk ${MKHOME}/src/SpeechSynthesis/tts_sdk.o -I${MKHOME}/include -L${MKHOME}/lib ${ACLINC} ${ACLLD} -lmsc_x64 -lShm -lUtil -lacl_all -lz -lpthread -ldl
int main() {

	cout << "wav文件头部大小是:" << sizeof(wave_pcm_hdr) << endl;

	cout << "开始文本转语音....." << endl;

	int ret = -1;

	// 1.分配文本和语音空间
	char _text[256 + 1] = {'\0'}; // 输入文本
	strcpy(_text, "赶紧的，我们一起尝试进行文本转语音的测试");
	char _speech[251552 + 1] = {'\0'}; // tts转成的目标语音

	// 2.调用转换函数
	if (tts_sdk_all(_text, sizeof(_text), _speech, sizeof(_speech)) != MSP_SUCCESS) {
		cout << "fail to call function tts_sdk" << endl;
		return ret;
	}

	// 3.写出语音wav
	const char* filename = "test_tts_sdk.wav"; // 合成的语音文件名称
	FILE* fp = NULL;
	fp = fopen(filename, "wb");
	if (NULL == fp) {
		printf("open %s error.\n", filename);
		return ret;
	}

	cout << "_speech中有字节:" << sizeof(_speech) << endl;
	fwrite(_speech, sizeof(_speech), 1, fp);

	fclose(fp);
	fp = NULL;

	cout << "结束文本转语音....." << endl;

	return 0;
}
