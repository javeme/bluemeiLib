#include "Thread.h"
#include "ErrorHandler.h"
#include <process.h>

namespace bluemei{

//创建线程后回调,表示开始执行新线程了
unsigned int WINAPI callBackOfStartThread(void* pVoid)
{
	return ((Thread*)pVoid)->callBackStartThread();
}

////////////////////////////////////////////////////////////
//Thread
Thread::Thread(void* pUserParameter/*=NULL*/)
{
	this->m_pObject=this;
	this->m_pUserParameter=pUserParameter;
	init();
}
Thread::Thread(Runnable* pTarget,void* pUserParameter)
{
	//this->m_pThreadParameter=new ThreadParameter();
	this->m_pObject=pTarget;
	this->m_pUserParameter=pUserParameter;
	init();
}
Thread::~Thread()
{
	destroy();
}
//初始化
void Thread::init()
{
	m_bDetached=false;
	m_bAutoDestroyObj=false;
	m_bRunning=false;
	//m_hThread=::CreateThread(NULL,0,CallBackOfCreateThread,(LPVOID)this,CREATE_SUSPENDED,&m_threadId);
	m_hThread=(HANDLE)_beginthreadex(NULL,0,callBackOfStartThread,(LPVOID)this,CREATE_SUSPENDED,&m_threadId);
}
//设置是否自动释放线程内存资源
void Thread::setAutoDestroy(bool bAutoDestroy)
{
	m_bAutoDestroyObj=bAutoDestroy;
}
bool Thread::isAutoDestroyObj() const
{
	return m_bAutoDestroyObj;
}
void* Thread::getUserParameter() const
{
	return m_pUserParameter;
}
//开始执行
void Thread::start()
{
	if(::ResumeThread(m_hThread)<0)
	{
		ThreadException e(GetLastError());
		e.setExceptionMsg("start thread failed");
		throwpe(e);
	}

	if(!m_lock.wait(1000*30)){//超时 wait发生异常该如何处理???
		throwpe(ThreadException("starting the thread may be failed, out of time"));
	}
}
void Thread::stop()
{
	if(this->m_bRunning)
		::TerminateThread(m_hThread,0);//其它线程结束本线程 (_endthreadex及ExitThread为自己结束本线程)
	else{
		//throwpe(ThreadException("thread not running"));
	}
	this->m_bRunning=false;
}
void Thread::detach()
{
	//关闭句柄,使处于分离状态,结束时释放资源
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
			throwpe(ThreadException(str));
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
//销毁
void Thread::destroy()
{
	//m_criticalLock.getLock();//为何出错? 多线程中无法初始critical?

	stop();
	detach();
	//if(m_pThreadParameter->pUserParameter!=NULL)
	//	delete m_pThreadParameter->pUserParameter;
	//delete m_pThreadParameter;

	//m_criticalLock.releaseLock();
}
//调用pObject(需要实现Runnable接口)的run函数
int Thread::callBackStartThread()
{
	int ret=0;
	m_bRunning=true;

	try{
		m_lock.signal();

		if(this->m_pObject)
			this->m_pObject->run();
	}catch(Exception& e)//发生异常该如何处理???
	{
		//printf("Thread: %s\n",e.toString().c_str());
		ErrorHandler::handle(e);
		ret=-1;
	}

	if(m_bAutoDestroyObj)
	{
		//destroy();
		delete this;
	}

	m_bRunning=false;
	return ret;
}
//执行
void Thread::run()
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


}//end of namespace bluemei