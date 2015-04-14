#include "stdafx.h"
#include "Timer.h"

namespace bluemei{

Timer::Timer()
{
	m_bGoOn=false;
	m_pTask=nullptr;
}

Timer::Timer(Class& taskClass,unsigned long delay,unsigned long period/*=0*/)
{
	m_bGoOn=false;
	m_pTask=nullptr;
	schedule(taskClass,delay,period);
}

Timer::~Timer()
{
	cancel();
}

void Timer::schedule(Class& taskClass,unsigned long delay,unsigned long period)
{
	Runnable* pTask=dynamic_cast<Runnable*>(taskClass.createObject());
	schedule(pTask,delay,period,true);
}

void Timer::schedule(Runnable* pTask,unsigned long delay,unsigned long period,bool autoDestroyTask)
{
	if(m_pTask!=nullptr)
	{
		//delete pTaskClass;
		throw Exception("don't schedule multiple tasks in a timer");
	}
	if(pTask==nullptr)
		throw Exception("task object can't be null");
	m_pTask=pTask;
	m_nextTime=GetTickCount()+delay;
	m_nPeriod=period;
	//执行线程
	Thread *pThread=new LambdaThread([&,autoDestroyTask](void* pUserParameter){
		m_bGoOn=true;
		while(m_bGoOn){
			executeTask();
		}
		if(autoDestroyTask)
			delete m_pTask;
		m_pTask=nullptr;
		m_waitFinishLock.signal();
	},nullptr);
	pThread->start();
}

void Timer::cancel()
{
	if(m_bGoOn){
		m_bGoOn=false;		
		m_lock.signal();//释放定时器
		m_waitFinishLock.wait();//等待线程结束
	}
}

void Timer::executeTask()
{
	unsigned long currentTime=GetTickCount();
	long delay=m_nextTime-currentTime;
	bool isTimeout=true;
	if(delay>0){//时间未到
		isTimeout=!m_lock.wait(delay);//是否到时间,否则表示被打断
	}
	if(isTimeout)
	{
		m_pTask->run(nullptr);
		m_nextTime=m_nextTime+m_nPeriod;
	}
	if(m_nPeriod==0)
		m_bGoOn=false;
}

void Timer::reset()//如何与定时器线程同步??? 待改进!!!
{
	unsigned long currentTime=GetTickCount();
	if(m_lock.getWaitCount()==1)
	{
		m_nextTime=currentTime+m_nPeriod;
		m_lock.notify();//但可能定时器正好超时,那么会使下次定时立马返回,下面判断: (待改进!!!)
	}
	//if(m_lock.getWaitCount()>0)//木有打断到定时器(也有可能是打断了,但进入下一次超时等待, 但几率很小,或者未来得及减1立马读取了waitCount)
	//	m_lock.getLock();//消费掉
}

long Timer::getRemainderTime()
{
	unsigned long currentTime=GetTickCount();
	return m_nextTime-currentTime;
}

bool Timer::isRunning()const
{
	return m_bGoOn;
}


}//end of namespace bluemei