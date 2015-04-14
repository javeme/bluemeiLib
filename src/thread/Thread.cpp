#include "Thread.h"
#include "ErrorHandler.h"
#include <process.h>

namespace bluemei{

Thread::Thread(Runnable* pTarget,void* pUserParameter)
{
	//this->m_pThreadParameter=new ThreadParameter();
	this->m_threadParameter.pObject=pTarget;
	this->m_threadParameter.pUserParameter=pUserParameter;
	init();
}
//Thread::Thread(ThreadParameter *pThreadParameter)
Thread::Thread(const ThreadParameter& threadParameter)
{
	this->m_threadParameter=threadParameter;
	init();
}
Thread::~Thread()
{
	destroy();
}
//��ʼ��
void Thread::init()
{
	m_bDetached=false;
	m_bAutoDestroyObj=false;
	m_bRunning=false; 
	//m_hThread=::CreateThread(NULL,0,CallBackOfCreateThread,(LPVOID)this,CREATE_SUSPENDED,&m_threadId);
	m_hThread=(HANDLE)_beginthreadex(NULL,0,callBackOfStartThread,(LPVOID)this,CREATE_SUSPENDED,&m_threadId);
}
//�����Ƿ��Զ��ͷ��߳��ڴ���Դ
void Thread::setAutoDestroy(bool bAutoDestroy)
{
	m_bAutoDestroyObj=bAutoDestroy;
}
//��ʼִ��
void Thread::start()
{
	if(::ResumeThread(m_hThread)<0)
	{
		ThreadException e(GetLastError());
		e.setExceptionMsg("start thread failed");
		throwpe(e);
	}
	
	if(!m_lock.wait(1000*30)){//��ʱ wait�����쳣����δ���???
		throwpe(ThreadException("starting the thread may be failed, out of time"));
	}	
}
void Thread::stop()
{	
	if(this->m_bRunning)
		::TerminateThread(m_hThread,0);//�����߳̽������߳� (_endthreadex��ExitThreadΪ�Լ��������߳�)
	else{
		//throwpe(ThreadException("thread not running"));
	}
	this->m_bRunning=false;
}
void Thread::detach()
{
	//�رվ��,ʹ���ڷ���״̬,����ʱ�ͷ���Դ
	if(!m_bDetached)
		::CloseHandle(m_hThread);
	m_bDetached=true;
}
void Thread::wait()
{
	if(!m_bDetached)//isRunning&&!isDetached
	{
		int result=::WaitForSingleObject(m_hThread,INFINITE);
		if(result==WAIT_FAILED)
		{
			int error=::GetLastError();
			String str=String::format("wait for thread object failed : %d",error);
			throw Exception(str);
		}
		detach();
	}
	else{
		throwpe(ThreadException("thread have finished or detached"));
	}
}
void Thread::sleep(unsigned int msecond)
{
	Sleep(msecond);
}
//����
void Thread::destroy()
{
	//m_criticalLock.getLock();//Ϊ�γ���? ���߳����޷���ʼcritical?

	stop();
	detach();
	//if(m_pThreadParameter->pUserParameter!=NULL)
	//	delete m_pThreadParameter->pUserParameter;
	//delete m_pThreadParameter;

	//m_criticalLock.releaseLock();
}
//����pObject(��Ҫʵ��Runnable�ӿ�)��run����
void Thread::callBackStartThread()
{
	try{
		m_lock.signal();
		m_bRunning=true;	
	}catch(Exception& e)//�����쳣����δ���???
	{
		m_bRunning=false;
		printf("Thread: %s\n",e.toString().c_str());
		ErrorHandler::handle(e);
	}
	if(m_bRunning){
		m_threadParameter.pObject->run(&m_threadParameter);	
		if(m_threadParameter.pObject!=this)
			this->run(&m_threadParameter);
		m_bRunning=false;	
	}	

	if(m_bAutoDestroyObj)
	{
		//destroy();
		delete this;
	}
}
//ִ��
void Thread::run(ThreadParameter *pThreadParameter)
{
	;
}

bool Thread::isRunning() const
{
	return m_bRunning;
}

unsigned int Thread::getThreadId() const
{
	return m_threadId;
}

unsigned int Thread::currentThreadId()
{
	return GetCurrentThreadId();
}


//�����̺߳�ص�,��ʾ��ʼִ�����߳���
unsigned int WINAPI callBackOfStartThread(void* pVoid)
{
	((Thread*)pVoid)->callBackStartThread();
	return true;
}

}//end of namespace bluemei