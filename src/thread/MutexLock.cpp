#include "MutexLock.h"

namespace bluemei{

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	waitCount=0;
	//��ʼ���ٽ���
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
	//�ͷ��ٽ���
	if(mutex!=0){
		BOOL success=::CloseHandle(mutex);
		mutex=0;
	}
}

void MutexLock::getLock()
{
	unsigned int count=InterlockedIncrement(&waitCount);//ԭ�Ӳ���

	//�����ٽ���
	unsigned long result=::WaitForSingleObject(mutex, INFINITE);
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)//bAlertableΪTRUEʱ���ܷ�������WAIT_IO_COMPLETION
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::getLock: wait for mutex failed : %d",error);
		throw Exception(str);
	}

	InterlockedDecrement(&waitCount);
}

void MutexLock::releaseLock()
{
	//�뿪�ٽ���
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