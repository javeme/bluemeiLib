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
	//Ĭ�Ͻ���û���ź���,�����߳̽��޷����,ֻ�ܵȴ�
	wait();//INFINITE
}
bool ResourceLock::wait(unsigned long mSecond/*=INFINITE*/)
{
	//waitCount++;
	InterlockedIncrement(&waitCount);//ԭ�Ӳ���
	unsigned long result=::WaitForSingleObject(semaphoreHandle,mSecond);
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("wait for object failed : %d",error);
		throw Exception(str);
	}
	
	//waitCount--;
	InterlockedDecrement(&waitCount);
	return result!=WAIT_TIMEOUT;//û�г�ʱ
}

bool ResourceLock::wait(MutexLock& mutex,unsigned long mSecond/*=INFINITE*/) throw(Exception)
{
	InterlockedIncrement(&waitCount);//ԭ�Ӳ���
	//����mutex,�ȴ��ź�semaphoreHandle
	unsigned long result=::SignalObjectAndWait(mutex.mutex,semaphoreHandle,mSecond,FALSE);//ԭ�Ӳ���
	//��������mutex
	mutex.getLock();
	//������
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)//bAlertableΪTRUEʱ���ܷ�������WAIT_IO_COMPLETION
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock::wait: wait for object failed : %d",error);
		throw Exception(str);
	}
	InterlockedDecrement(&waitCount);
	return result!=WAIT_TIMEOUT;//û�г�ʱ
}

long ResourceLock::signal() throw(Exception)
{
	long previousCount=0;
	BOOL isSuccess=::ReleaseSemaphore(semaphoreHandle,1,&previousCount);//����һ���ź�
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

//�ͷ����еĵȴ����� (��ͬ��java��notifyAll,����broadcast)
void ResourceLock::notifyAll()
{
	long previousCount=0;
	int count=waitCount;//��ʱ��waitCount���ܶ�����ʵ�ĵȴ��߳���(�����ź���Ϊ1ʱ,��һ���߳����ý�Ҫ������ȥ�ȴ�״̬,waitCount����1)
	if(count<=0)
		return;
	
	bool isSuccess=::ReleaseSemaphore(semaphoreHandle,count,&previousCount)==TRUE;//����count���ź�	
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