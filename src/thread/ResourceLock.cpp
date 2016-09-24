#include "ResourceLock.h"

namespace bluemei{

ResourceLock::ResourceLock(unsigned int count,cstring name)
{
	init(count,DEFAULT_MAX_COUNT,name);
}

ResourceLock::~ResourceLock(void)
{
	if(m_semaphore!=0)
	{
		BOOL success=::CloseHandle(m_semaphore);
		m_semaphore=0;
	}
}

ResourceLock::ResourceLock(const ResourceLock& other)
{
	init(0,DEFAULT_MAX_COUNT);
}

ResourceLock& ResourceLock::operator=(const ResourceLock& other)
{
	return *this;
}

void ResourceLock::init(unsigned int count,unsigned int maxCount,cstring name)
{
	waitCount=0;
	m_semaphore=::CreateSemaphoreA(NULL,count,maxCount,name);
	if(m_semaphore==0)
	{
		int error=::GetLastError();
		String str=String::format("init semaphore failed : %d",error);
		throw Exception(str);
	}
}
void ResourceLock::getLock()
{
	//默认仅有没有信号量,其它线程将无法获得,只能等待
	wait();//INFINITE
}
bool ResourceLock::wait(unsigned long mSecond/*=INFINITE*/)
{
	//waitCount++;
	InterlockedIncrement(&waitCount);//原子操作
	unsigned long result=::WaitForSingleObject(m_semaphore,mSecond);
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("wait for object failed : %d",error);
		throw Exception(str);
	}

	//waitCount--;
	InterlockedDecrement(&waitCount);
	return result!=WAIT_TIMEOUT;//没有超时
}

//类似条件变量的等待： http://blog.csdn.net/metasearch/article/details/18677193
bool ResourceLock::wait(MutexLock& mutex,unsigned long mSecond/*=INFINITE*/)
	throw(Exception)
{
	InterlockedIncrement(&waitCount);//原子操作
	//解锁mutex,等待信号semaphoreHandle（原子操作）
	unsigned long result=::SignalObjectAndWait(mutex.m_mutex,m_semaphore,
		mSecond,FALSE);
	//重新锁定mutex
	mutex.getLock();
	//错误检测
	//SignalObjectAndWait bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::wait: wait for object failed"\
			": %d", error);
		throw Exception(str);
	}
	InterlockedDecrement(&waitCount);
	return result!=WAIT_TIMEOUT;//没有超时
}

long ResourceLock::signal() throw(Exception)
{
	long previousCount=0;
	//增加一个信号
	BOOL isSuccess=::ReleaseSemaphore(m_semaphore,1,&previousCount);
	if(!isSuccess)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::notify: release semaphore "\
			"failed (error=%d)",error);
		throw Exception(str);
	}
	return previousCount+1;
}

void ResourceLock::notify() throw(Exception)
{
	int count=waitCount;
	if(count>0)
		signal();
}

//释放所有的等待对象 (不同于java的notifyAll,类似broadcast)
void ResourceLock::notifyAll()
{
	long previousCount=0;
	//此时的waitCount可能多于真实的等待线程数(比如信号量为1时,
	//有一个线程正好将要进入或过去等待状态,waitCount等于1)
	int count=waitCount;
	if(count<=0)
		return;

	//增加count个信号
	bool isSuccess=::ReleaseSemaphore(m_semaphore,count,&previousCount)==TRUE;
	if(!isSuccess)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::notifyAll: release semaphore "\
			"failed (error=%d)",error);
		throw Exception(str);
	}
}

long ResourceLock::getWaitCount() const
{
	return waitCount;
	/*long count=0;
	bool isSuccess=::ReleaseSemaphore(m_semaphore,0,&count);
	return count;*/
}

//linux implement: http://linux.die.net/man/3/pthread_mutexattr_init
//TODO

}//end of namespace bluemei