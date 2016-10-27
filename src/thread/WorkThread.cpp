#include "WorkThread.h"
#include "ThreadPool.h"
#include "ErrorHandler.h"
//#include <tchar.h>

namespace bluemei{

WorkThread::WorkThread(TaskPool *const pTaskPool)
	:/*m_pLock(pLock),*/m_pTaskPool(pTaskPool)
{
	//this->m_pLock=pLock;
	//log.openFile("d:/users/worker.log","ab");
	m_pCurrentTask=NULL;
	m_bIsIdle=true;
}

WorkThread::~WorkThread(void)
{
	;
}

void WorkThread::run()
{
	try{
		Task* pTask=NULL;
		while(m_bRunning)
		{
			//m_pLock->waitTask();
			pTask=m_pTaskPool->getTask();
			m_bIsIdle=false;
			if(pTask!=NULL)
			{
				m_pCurrentTask=pTask;
				m_pCurrentTask->taskStarted();
				m_pCurrentTask->run();
				//m_pTaskPool->onFinishTask(m_pCurrentTask);
				m_pCurrentTask->taskFinished();
				m_pCurrentTask=NULL;
			}
			m_bIsIdle=true;
		}
	}catch(Exception& e){
		ErrorHandler::handle(e);
	}catch (std::exception& e){
		ErrorHandler::handle(StdException(e));
	}catch (...){
		ErrorHandler::handle(UnknownException("ThreadPool error"));
	}
}

void WorkThread::stopAndWait()
{
	if(Thread::currentThreadId() == this->getThreadId()) {
		throwpe(ThreadException("Can't stop/wait for self thread."));
	}

	m_bRunning=false;

	//δ��������ʱ�жϵ�
	if(m_pCurrentTask!=NULL && m_pCurrentTask->isRunning())//�Ƿ�Ӧ�û������?
		m_pCurrentTask->stop();//��������

	this->wait();//��ǰ�ȴ���(this)�߳̽���
}

bool WorkThread::isIdle() const
{
	return m_bIsIdle;
}

}//end of namespace bluemei