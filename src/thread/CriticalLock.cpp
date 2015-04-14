#include "CriticalLock.h"

namespace bluemei{

CriticalLock::CriticalLock()
{
	waitCount=0;
	//初始化临界区
	::InitializeCriticalSection(&critialSection);//can raise a STATUS_NO_MEMORY exception
}

CriticalLock::~CriticalLock(void)
{
	//释放临界区
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
	unsigned int count=InterlockedIncrement(&waitCount);//原子操作
	//进入临界区
	::EnterCriticalSection(&critialSection);//可能抛异常:EXCEPTION_POSSIBLE_DEADLOCK
	//waitCount--;//已经加入临界区(仅一个线程在此运行),无需原子操作, 但也可能被waitCount++打断
	InterlockedDecrement(&waitCount);
}

void CriticalLock::releaseLock()
{
	//离开临界区
	::LeaveCriticalSection(&critialSection);
}

long CriticalLock::getWaitCount() const
{
	//return waitCount;
	long wc=0;
	//获得操作系统类型
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	//判断是NT架构的还是旧架构的
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