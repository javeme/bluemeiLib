#pragma once
#include "bluemeiLib.h"
#include <functional>
#include "Thread.h"

namespace bluemei{

typedef std::function<void (void* pUserParameter)> ThreadFunction;
class BLUEMEILIB_API LambdaThread : public Thread {
public:
	LambdaThread(const ThreadFunction& f,void* pUserParameter);
	virtual ~LambdaThread();
	virtual void run(ThreadParameter *pThreadParameter);
private:
	ThreadFunction m_lambdaFun;
};

#if 0
//test
void download(string url,const function<void (int error)>& f)//fΪ������ɺ�Ļص�����
{
	auto func=[url,f](void* pUserParameter){
		printf("downloading...\n");
		Sleep(1000*5);
		printf("download finished from %s\n",url.c_str());
		f(0);
	};
	//thread
	Thread *pThread=new LambdaThread(func,nullptr);//�����߳�
	pThread->start();
}
#endif

}//end of namespace bluemei