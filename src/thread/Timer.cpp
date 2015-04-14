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
	//ִ���߳�
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
		m_lock.signal();//�ͷŶ�ʱ��
		m_waitFinishLock.wait();//�ȴ��߳̽���
	}
}

void Timer::executeTask()
{
	unsigned long currentTime=GetTickCount();
	long delay=m_nextTime-currentTime;
	bool isTimeout=true;
	if(delay>0){//ʱ��δ��
		isTimeout=!m_lock.wait(delay);//�Ƿ�ʱ��,�����ʾ�����
	}
	if(isTimeout)
	{
		m_pTask->run(nullptr);
		m_nextTime=m_nextTime+m_nPeriod;
	}
	if(m_nPeriod==0)
		m_bGoOn=false;
}

void Timer::reset()//����붨ʱ���߳�ͬ��??? ���Ľ�!!!
{
	unsigned long currentTime=GetTickCount();
	if(m_lock.getWaitCount()==1)
	{
		m_nextTime=currentTime+m_nPeriod;
		m_lock.notify();//�����ܶ�ʱ�����ó�ʱ,��ô��ʹ�´ζ�ʱ������,�����ж�: (���Ľ�!!!)
	}
	//if(m_lock.getWaitCount()>0)//ľ�д�ϵ���ʱ��(Ҳ�п����Ǵ����,��������һ�γ�ʱ�ȴ�, �����ʺ�С,����δ���ü���1�����ȡ��waitCount)
	//	m_lock.getLock();//���ѵ�
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