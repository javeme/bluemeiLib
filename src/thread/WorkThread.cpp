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
void WorkThread::run(ThreadParameter *pThreadParameter)
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
				m_pCurrentTask->beforeTask();
				m_pCurrentTask->run(NULL);
				//m_pTaskPool->onFinishTask(m_pCurrentTask);
				m_pCurrentTask->taskFinish();
				m_pCurrentTask=NULL;
			}
			m_bIsIdle=true;
		}
	}catch(Exception& e)
	{
		ErrorHandler::handle(e);
	}
}
void WorkThread::stopAndWait()
{
	m_bRunning=false;
		
	//未结束任务时中断掉
	if(m_pCurrentTask!=NULL && m_pCurrentTask->isRunning())//是否应该互斥访问?
		m_pCurrentTask->stop();//结束任务 
		
	this->wait();//等待本线程结束	
}

bool WorkThread::isIdle() const
{
	return m_bIsIdle;
}

}//end of namespace bluemei