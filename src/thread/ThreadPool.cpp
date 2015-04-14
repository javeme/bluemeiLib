#pragma once
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
	while(!taskQueue.empty())
	{
		pTask=taskQueue.front();
		taskQueue.pop();
		delete pTask;		
	}	
	//销毁线程
	int size=threadList.size();
	for(int i=0;i<size;i++)
	{
		delete threadList[i];
	}
}

void ThreadPool::addThread(int threadCount)
{
	for(int i=0;i<threadCount;i++)
	{
		WorkThread *p=new WorkThread(this);
		threadList.addToLast(p);
		p->start();
	}
	m_nThreadCount+=threadCount;
}

int ThreadPool::addTask(Task *pTask)
{
	ScopedLock scopedLock(mutexLock);

	if(!m_bIsStop)
	{
		/*bool needNofity=taskQueue.empty();
		taskQueue.push(pTask);
		if(needNofity)//通知 taskLock.getWaitCount()>0
			taskLock.notify();*/
		taskQueue.push(pTask);
		taskLock.signal();
	}
	return taskQueue.size();
}

Task* ThreadPool::getTask()
{
	if(m_bIsStop)
		return NULL;
	
	//if(taskQueue.empty()) //如果在此时插入了一个任务,可能wait错过时机,没有通知到
	//	taskLock.waitTask();//等待
	taskLock.waitTask();
	
	Task* pTask=NULL;
	if(!m_bIsStop){
		mutexLock.getLock();

		if(!taskQueue.empty()){
			pTask=taskQueue.front();
			taskQueue.pop();
			//doingThreadList.addToLast(p);
		}

		mutexLock.releaseLock();
	}

	return pTask;
}

void ThreadPool::onFinishTask(Task* pTask)
{	
	//delete pTask;
}

void ThreadPool::stop()
{
	ScopedLock scopedLock(mutexLock);

	//mutexLock.getLock();
	m_bIsStop=true;
	//mutexLock.releaseLock();

	taskLock.notifyAll();
	auto i=threadList.iterator();
	while(i->hasNext())
	{
		i->next()->stopAndWait();
	}
	threadList.releaseIterator(i);
}

int ThreadPool::addMoreThread(int num)
{
	addThread(num);
	return threadList.size();
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
	ScopedLock scopedLock(mutexLock);

	return taskQueue.size();
}

int ThreadPool::totalThread() const
{
	return threadList.size();
}

int ThreadPool::idleThread()
{
	int count=0;
	for(auto i=threadList.iterator();i->hasNext();)
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