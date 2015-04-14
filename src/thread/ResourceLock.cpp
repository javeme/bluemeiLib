#include "ResourceLock.h"

namespace bluemei{

ResourceLock::ResourceLock(unsigned int count,cstring name)
{
	init(count,DEFAULT_MAX_COUNT,name);
}

ResourceLock::~ResourceLock(void)
{
	if(semaphoreHandle!=0)
	{
		BOOL success=::CloseHandle(semaphoreHandle);
		semaphoreHandle=0;
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
	semaphoreHandle=::CreateSemaphoreA(NULL,count,maxCount,name);
	if(semaphoreHandle==0)
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
	unsigned long result=::WaitForSingleObject(semaphoreHandle,mSecond);
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

bool ResourceLock::wait(MutexLock& mutex,unsigned long mSecond/*=INFINITE*/) throw(Exception)
{
	InterlockedIncrement(&waitCount);//原子操作
	//解锁mutex,等待信号semaphoreHandle
	unsigned long result=::SignalObjectAndWait(mutex.mutex,semaphoreHandle,mSecond,FALSE);//原子操作
	//重新锁定mutex
	mutex.getLock();
	//错误检测
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)//bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::wait: wait for object failed : %d",error);
		throw Exception(str);
	}
	InterlockedDecrement(&waitCount);
	return result!=WAIT_TIMEOUT;//没有超时
}

long ResourceLock::signal() throw(Exception)
{
	long previousCount=0;
	BOOL isSuccess=::ReleaseSemaphore(semaphoreHandle,1,&previousCount);//增加一个信号
	if(!isSuccess)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::notify: release semaphore failed (error=%d)",error);
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
	int count=waitCount;//此时的waitCount可能多于真实的等待线程数(比如信号量为1时,有一个线程正好将要进入或过去等待状态,waitCount等于1)
	if(count<=0)
		return;
	
	bool isSuccess=::ReleaseSemaphore(semaphoreHandle,count,&previousCount)==TRUE;//增加count个信号	
	if(!isSuccess)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::notifyAll: release semaphore failed (error=%d)",error);
		throw Exception(str);
	}
}

long ResourceLock::getWaitCount() const
{
	return waitCount;
	/*long count=0;
	bool isSuccess=::ReleaseSemaphore(semaphoreHandle,0,&count);	
	return count;*/
}

}//end of namespace bluemei