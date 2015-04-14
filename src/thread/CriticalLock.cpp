#include "CriticalLock.h"

namespace bluemei{

CriticalLock::CriticalLock()
{
	waitCount=0;
	//��ʼ���ٽ���
	::InitializeCriticalSection(&critialSection);//can raise a STATUS_NO_MEMORY exception
}

CriticalLock::~CriticalLock(void)
{
	//�ͷ��ٽ���
	::DeleteCriticalSection(&critialSection);
}
/*
CriticalLock::CriticalLock(const CriticalLock& other)
{
	waitCount=0;
	::InitializeCriticalSection(&critialSection);
	this->operator=(other);
}

CriticalLock& CriticalLock::operator=(const CriticalLock& other)
{
	return *this;
}*/

void CriticalLock::getLock()
{
	//waitCount++;
	unsigned int count=InterlockedIncrement(&waitCount);//ԭ�Ӳ���
	//�����ٽ���
	::EnterCriticalSection(&critialSection);//�������쳣:EXCEPTION_POSSIBLE_DEADLOCK
	//waitCount--;//�Ѿ������ٽ���(��һ���߳��ڴ�����),����ԭ�Ӳ���, ��Ҳ���ܱ�waitCount++���
	InterlockedDecrement(&waitCount);
}

void CriticalLock::releaseLock()
{
	//�뿪�ٽ���
	::LeaveCriticalSection(&critialSection);
}

long CriticalLock::getWaitCount() const
{
	//return waitCount;
	long wc=0;
	//��ò���ϵͳ����
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	//�ж���NT�ܹ��Ļ��Ǿɼܹ���
	if(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT){
		wc=((-1) - critialSection.LockCount) >> 2;
	}
	else if(osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS){
		wc=critialSection.LockCount- (critialSection.RecursionCount -1);
	}
	else 
		wc=waitCount;
	return wc;
}

long CriticalLock::getMyThreadEnteredCount() const
{
	return critialSection.RecursionCount;
}

}//end of namespace bluemei