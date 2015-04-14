/*
*edit by bluemei
*write for java-users
*/

#ifndef _Thread_h_
#define _Thread_h_

#include "bluemeiLib.h"
//#include <windows.h>

#include "ThreadParameter.h"
#include "Runnable.h"
#include "ThreadException.h"
#include "ResourceLock.h"
#include "CriticalLock.h"

namespace bluemei{

class BLUEMEILIB_API Thread : public Runnable
{
public:
	//���� �ڲ���������� (c++�ڲ����޷�ֱ�ӷ����ⲿ�����ͷ���,����Ҳ�޷������ڲ���.ȷʵ��Ҫ�Ļ����Դ�����,�����Ͳ���ϲ���)
	Thread(void* pUserParameter=NULL){this->Thread::Thread(this,pUserParameter);};	
	Thread(Runnable* pTarget,void* pUserParameter=NULL);//���ڽӿ�
	//Thread(ThreadParameter *pThreadParameter);//pThreadParameter must be created with operater 'new'
	Thread(const ThreadParameter& threadParameter);
	virtual ~Thread();
private:
	Thread(const Thread& other);
	Thread& operator=(const Thread& other);
protected:
	ThreadParameter m_threadParameter;
	HANDLE m_hThread;
	volatile bool m_bRunning,m_bDetached;
	bool m_bAutoDestroyObj;
	SyncLock m_lock;
	unsigned int m_threadId;
	//CriticalLock m_criticalLock;
public:
	void setAutoDestroy(bool bAutoDestroy);
	void start() throw(Exception);
	void stop();
	void detach();
	void wait() throw(ThreadException);
	static void sleep(unsigned int msecond);
public:
	bool isRunning() const;
	unsigned int getThreadId() const;
	static unsigned int currentThreadId();
public: 
	void callBackStartThread();
	virtual void run(ThreadParameter *pThreadParameter);
protected: 
	virtual void init();
	virtual void destroy();
};
unsigned int WINAPI callBackOfStartThread(void* pVoid);


}//end of namespace bluemei
#endif