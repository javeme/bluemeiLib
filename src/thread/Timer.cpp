#include "Timer.h"
#include "RuntimeException.h"

#ifdef WIN32

inline unsigned int getTimeTick() { return GetTickCount(); }

#else

#include "AccurateTimeTicker.h"

inline unsigned int getTimeTick()
{
	static blib::AccurateTimeTicker ticker;
	static bool started = false;
	if(!started) {
		ticker.start();
		started=true;
	}
	return (ticker.getIntevalUSecond() / 1000); // ms
}

#endif


namespace blib{

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

void Timer::schedule(Runnable* pTask,unsigned long delay,
	unsigned long period,bool autoDestroyTask)
{
	if(m_pTask!=nullptr)
	{
		//delete pTaskClass;
		throw Exception("don't schedule multiple tasks in a timer");
	}
	if(pTask==nullptr)
		throw Exception("task object can't be null");
	m_pTask=pTask;
	m_nextTime=getTimeTick()+delay;
	m_nPeriod=period;
	//执行线程
	Thread *pThread=new LambdaThread([&,autoDestroyTask](){
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
	unsigned long currentTime=getTimeTick();
	long delay=m_nextTime-currentTime;
	bool isTimeout=true;
	if(delay>0){//时间未到
		isTimeout=!m_lock.wait(delay);//是否到时间,否则表示被打断
	}
	if(isTimeout)
	{
		m_pTask->run();
		m_nextTime=m_nextTime+m_nPeriod;
	}
	if(m_nPeriod==0)
		m_bGoOn=false;
}

void Timer::reset()//如何与定时器线程同步??? 待改进!!!
{
	unsigned long currentTime=getTimeTick();
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
	unsigned long currentTime=getTimeTick();
	return m_nextTime-currentTime;
}

bool Timer::isRunning() const
{
	return m_bGoOn;
}

bool Waiter::wait(unsigned int interval/*=1000*/)
{
	if(interval == 0)
		interval = 1;
	unsigned int times = this->m_timeout / interval;
	for(unsigned int i = 0; !this->m_condition(); i++)
	{
		Thread::sleep(interval);
		if(this->m_timeout > 0 && i >= times)
			throw TimeoutException(this->m_timeout);
	}
	return true;
}

}//end of namespace blib
