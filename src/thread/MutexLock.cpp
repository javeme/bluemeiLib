#include "MutexLock.h"

namespace bluemei{

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	waitCount=0;
	//初始化临界区
	mutex = ::CreateMutexA(NULL, initialOwner, name);
	if(mutex==0)
	{
		int error=::GetLastError();
		String str=String::format("init mutex failed : %d",error);
		throw Exception(str);
	}
}

MutexLock::~MutexLock(void)
{
	//释放临界区
	if(mutex!=0){
		BOOL success=::CloseHandle(mutex);
		mutex=0;
	}
}

void MutexLock::getLock()
{
	unsigned int count=InterlockedIncrement(&waitCount);//原子操作

	//进入临界区
	unsigned long result=::WaitForSingleObject(mutex, INFINITE);
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)//bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::getLock: wait for mutex failed : %d",error);
		throw Exception(str);
	}

	InterlockedDecrement(&waitCount);
}

void MutexLock::releaseLock()
{
	//离开临界区
	BOOL success = ::ReleaseMutex(mutex);
	if(!success)
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::releaseLock: release mutex failed (error=%d)",error);
		throw Exception(str);
	}
}

long MutexLock::getWaitCount() const
{
	return waitCount;
}

}//end of namespace bluemei