#include "ThreadPool.h"

namespace bluemei{


ThreadPool::ThreadPool(int threadCount)
{
	m_nThreadCount=0;
	m_bIsStop=false;
	addThread(threadCount);
}

ThreadPool::~ThreadPool()
{
	//销毁未执行任务
	Task* pTask=NULL;
	while(!m_taskQueue.empty())
	{
		pTask=m_taskQueue.front();
		m_taskQueue.pop();
		delete pTask;
	}
	//销毁线程
	int size=m_threadList.size();
	for(int i=0;i<size;i++)
	{
		delete m_threadList[i];
	}
}

void ThreadPool::addThread(int threadCount)
{
	for(int i=0;i<threadCount;i++)
	{
		WorkThread *p=new WorkThread(this);
		m_threadList.addToLast(p);
		p->start();
	}
	m_nThreadCount+=threadCount;
}

int ThreadPool::addTask(Task *pTask)
{
	ScopedLock scopedLock(m_mutexLock);

	if(!m_bIsStop)
	{
		/*bool needNofity=m_taskQueue.empty();
		m_taskQueue.push(pTask);
		if(needNofity)//通知 taskLock.getWaitCount()>0
			m_taskLock.notify();*/
		m_taskQueue.push(pTask);
		m_taskLock.signal();
	}
	return m_taskQueue.size();
}

Task* ThreadPool::getTask()
{
	if(m_bIsStop)
		return NULL;

	//if(m_taskQueue.empty()) //如果在此时插入了一个任务,可能wait错过时机,没有通知到
	//	m_taskLock.waitTask();//等待
	m_taskLock.waitTask();

	Task* pTask=NULL;
	if(!m_bIsStop){
		m_mutexLock.getLock();

		if(!m_taskQueue.empty()){
			pTask=m_taskQueue.front();
			m_taskQueue.pop();
			//doingThreadList.addToLast(p);
		}

		m_mutexLock.releaseLock();
	}

	return pTask;
}

void ThreadPool::onFinishTask(Task* pTask)
{
	//delete pTask;
}

void ThreadPool::stop()
{
	ScopedLock scopedLock(m_mutexLock);

	if(m_bIsStop)
		return;
	//设置m_bIsStop之后，ThreadPool::getTask()将返回null
	m_bIsStop=true;

	m_taskLock.notifyAll();
	auto i=m_threadList.iterator();
	while(i->hasNext())
	{
		i->next()->stopAndWait();
	}
}

int ThreadPool::addMoreThread(int num)
{
	addThread(num);
	return m_threadList.size();
}

ThreadPool* ThreadPool::getInstance(int threadCount)//static
{
	static ThreadPool* p=new ThreadPool(threadCount);
	return p;
}
void ThreadPool::destroy(ThreadPool* pThreadPool)//static
{
	delete pThreadPool;
}

int ThreadPool::remainTask()
{
	ScopedLock scopedLock(m_mutexLock);

	return m_taskQueue.size();
}

int ThreadPool::totalThread() const
{
	return m_threadList.size();
}

int ThreadPool::idleThread()
{
	int count=0;
	for(auto i=m_threadList.iterator();i->hasNext();)
	{
		if(i->next()->isIdle())
			count++;
	}
	return count;
}

bool ThreadPool::isStop() const
{
	return m_bIsStop;
}

}//end of namespace bluemei