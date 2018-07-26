
#include "lib_acl.hpp"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>


using namespace std;

//////////////////////////////////////////////////////////////////////////

// ���߳��ඨ��
class mythread : public acl::thread
{
public:
	mythread() {}
	~mythread() {}
protected:
	// ���ി�麯�����������߳��е����߳�ʵ���� start ����ʱ
	// ���麯�����ᱻ����
	virtual void* run()
	{
		const char* myname = "run";
		printf("%s: thread id: %lu, %lu\r\n",
			myname, thread_id(), acl::thread::thread_self());
		return NULL;
	}
};

//////////////////////////////////////////////////////////////////////////

static void test_thread(void)
{
	const char* myname = "test_thread";
	mythread thr;  // ���̶߳���ʵ��

	// �����̵߳�����Ϊ�Ƿ��뷽ʽ���Ա���������Ե��� wait
	// �ȴ��߳̽���
	thr.set_detachable(false);

	// ����һ�����߳�
	if (thr.start() == false)
	{
		printf("start thread failed\r\n");
		return;
	}

	printf("%s: thread id is %lu, main thread id: %lu\r\n",
		myname, thr.thread_id(), acl::thread::thread_self());

	// �ȴ����߳����н���
	if (thr.wait(NULL) == false)
		printf("wait thread failed\r\n");
	else
		printf("wait thread ok\r\n");
}

// g++ test_multi_thread.cpp -o test_multi_thread ${ACLINC} ${ACLLD} -lacl_all -lz -lpthread -ldl
int main(void)
{
	// ��ʼ�� acl ��
	acl::acl_cpp_init();
	test_thread();
	return 0;
}
