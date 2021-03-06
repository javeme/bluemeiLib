#include "CriticalLock.h"

namespace blib{

#ifdef WIN32

CriticalLock::CriticalLock()
{
	m_waitCount=0;
	//初始化临界区
	//may raise a STATUS_NO_MEMORY exception
	::InitializeCriticalSection(&m_critialSection);
}

CriticalLock::~CriticalLock(void)
{
	//释放临界区
	::DeleteCriticalSection(&m_critialSection);
}

void CriticalLock::getLock()
{
	//m_waitCount++;
	unsigned int count=InterlockedIncrement(&m_waitCount);//原子操作
	//进入临界区
	::EnterCriticalSection(&m_critialSection);//可能抛异常:EXCEPTION_POSSIBLE_DEADLOCK
	//m_waitCount--;//已经加入临界区(仅一个线程在此运行),无需原子操作, 但也可能被waitCount++打断
	InterlockedDecrement(&m_waitCount);
}

void CriticalLock::releaseLock()
{
	//离开临界区
	::LeaveCriticalSection(&m_critialSection);
}

unsigned int CriticalLock::getWaitCount() const
{
	//return m_waitCount;
    unsigned int wc=0;
	//获得操作系统类型
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	//判断是NT架构的还是旧架构的
	if(osvi.dwPlatformId==VER_PLATFORM_WIN32_NT){
		wc=((-1) - m_critialSection.LockCount) >> 2;
	}
	else if(osvi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS){
		wc=m_critialSection.LockCount- (m_critialSection.RecursionCount -1);
	}
	else
		wc=m_waitCount;
	return wc;
}

unsigned int CriticalLock::getMyThreadEnteredCount() const
{
	return (unsigned int)m_critialSection.RecursionCount;
}

#else // not WIN32

#endif //end of #ifdef WIN32

}//end of namespace blib
