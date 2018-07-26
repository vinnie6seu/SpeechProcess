#include <vector>
#include <string>
#include <iostream>

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include "lib_acl.hpp"
#include "lib_acl.h"

/**
 * 核心处理程序，将files中存的.h文件转成可以序列化的
 */
static void create_files(const std::vector<std::string>& files) {
	acl::gsoner gr;
	gr.read_multi_file(files);
	gr.parse_code();
	gr.gen_gson();
}

/**
 * 将from文件中的内容拷贝到to文件中
 */
static bool copy_file(const char* from, const char* to) {
	acl::string buf;
	if (acl::ifstream::load(from, &buf) == false) {
		printf("read %s error %s\r\n", from, acl::last_serror());
		return false;
	}

	acl::ofstream out;
	if (out.open_trunc(to) == false) {
		printf("open %s error %s\r\n", to, acl::last_serror());
		return false;
	}

	if (out.write(buf) == -1) {
		printf("write to %s error %s\r\n", to, acl::last_serror());
		return false;
	}

	return true;
}

/**
 * 扫描当前目录，将后缀是.stub的文件拷贝一份名称变成.h
 */
static void scan_path(std::vector<std::string>& files) {
	acl::scan_dir scan;

	if (scan.open(".") == false) {
		printf("scan open error %s\r\n", acl::last_serror());
		return;
	}

	const char* file;

	while ((file = scan.next_file(false)) != NULL) {
		if (acl_strrncasecmp(file, ".stub", 2) == 0) {
			char buf[1024];

			// 将后缀是.stub的文件名改成后缀是.h的文件名
			snprintf(buf, sizeof(buf), "%s", file);
			char* dot = strrchr(buf, '.');
			assert(dot);
			*dot = 0;
			strcat(buf, ".h");

			// 将后缀是.stub的文件内容拷入到名为.h的文件中
			assert(copy_file(file, buf));

			files.push_back(buf);
		}
	}
}

static void usage(const char* procname) {
	printf("usage: %s -h [help] -d header_file_path\r\n", procname);
	printf("A.工具使用方法:\r\n");
	printf("1.编译生成 gson 工具；\r\n"
		   "2.在stub_entity目录中定义自己的 struct 结构体（可以定义多个）并保存在文件中（文件后缀名为 .stub）；\r\n"
		   "3.执行 %s -d path；\r\n"
		   "4.将 gson.h 和 由 .stub 转换的 .h 头文件拷贝到include/Entity中；\r\n"
		   "5.将 gson.cpp 拷贝到文件夹中 Serialize 中；\r\n"
		   "6.重编项目代码；\r\n", procname);
	printf("B.stub 文件编写注意点:\r\n");
	printf("1.struct 中的成员类型不能为 char 类型，主要是因为 Json 对象中不存在 char 类型；\r\n"
		   "2.不支持纯 C 语言的数组功能，比如，针对 int a[10]，因此推荐使用 std::vector<int> 代替它；\r\n"
		   "3.struct 中的成员前不得有 const 常量限定词，主要是在反序列化时需要对这些成员进行赋值；\r\n"
		   "4.struct 中不支持 C++ 关键词：public、private、protected；\r\n"
		   "5.存在包含关系的 struct 类型定义必须在同一个文件中；\r\n"
		   "6.struct 中的成员变量名不得为：$json、$node、$obj，因为这些关键词被 acl 序列化库生成的代码占用；\r\n");
}

int main(int argc, char* argv[]) {
	// 头文件列表
	std::vector<std::string> files;
	// 头文件所在路径
	std::string path;
	int ch;

	while ((ch = getopt(argc, argv, "hd:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'd':
			path = optarg;
			break;
		default:
			break;
		}
	}

	// 会将当前工作目录的绝对路径复制到参数buffer所指的内存空间中
	char buf[1024];
	if (getcwd(buf, sizeof(buf)) == NULL) {
		printf("getcwd error %s\r\n", acl::last_serror());
		return 1;
	}

	if (!path.empty()) {
		// 跳转到path目录
		if (chdir(path.c_str()) < 0) {
			printf("chdir to %s error %s\r\n", path.c_str(), acl::last_serror());
			return 1;
		}

		// 扫描path目录，将后缀是.stub的文件拷贝一份名称变成.h
		scan_path(files);
	}

	if (files.empty()) {
		usage(argv[0]);
		return 1;
	}

	// 核心处理程序，将files中存的.h文件转成可以序列化的
	create_files(files);

	// 返回bin文件所在工作目录
	if (chdir(buf) < 0) {
		printf("chdir to %s error %s\r\n", buf, acl::last_serror());
		return 1;
	}

	return 0;
}
