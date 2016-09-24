#include "CriticalLock.h"

namespace bluemei{

CriticalLock::CriticalLock()
{
	m_waitCount=0;
	//��ʼ���ٽ���
	//may raise a STATUS_NO_MEMORY exception
	::InitializeCriticalSection(&m_critialSection);
}

CriticalLock::~CriticalLock(void)
{
	//�ͷ��ٽ���
	::DeleteCriticalSection(&m_critialSection);
}

/*
CriticalLock::CriticalLock(const CriticalLock& other)
{
	m_waitCount=0;
	::InitializeCriticalSection(&m_critialSection);
	this->operator=(other);
}

CriticalLock& CriticalLock::operator=(const CriticalLock& other)
{
	return *this;
}*/

void CriticalLock::getLock()
{
	//m_waitCount++;
	unsigned int count=InterlockedIncrement(&m_waitCount);//ԭ�Ӳ���
	//�����ٽ���
	::EnterCriticalSection(&m_critialSection);//�������쳣:EXCEPTION_POSSIBLE_DEADLOCK
	//m_waitCount--;//�Ѿ������ٽ���(��һ���߳��ڴ�����),����ԭ�Ӳ���, ��Ҳ���ܱ�waitCount++���
	InterlockedDecrement(&m_waitCount);
}

void CriticalLock::releaseLock()
{
	//�뿪�ٽ���
	::LeaveCriticalSection(&m_critialSection);
}

long CriticalLock::getWaitCount() const
{
	//return m_waitCount;
	long wc=0;
	//��ò���ϵͳ����
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	//�ж���NT�ܹ��Ļ��Ǿɼܹ���
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

long CriticalLock::getMyThreadEnteredCount() const
{
	return m_critialSection.RecursionCount;
}

}//end of namespace bluemei