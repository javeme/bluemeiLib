#include "MutexLock.h"

namespace bluemei{

// Windows MutexLock
#ifdef WIN32

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	m_waitCount.counter = 0;
	//��ʼ���ٽ���
	m_mutex = ::CreateMutexA(NULL, initialOwner, name);
	if(m_mutex==0)
	{
		int error=::GetLastError();
		String str=String::format("init mutex failed : %d",error);
		throw Exception(str);
	}
}

MutexLock::~MutexLock(void)
{
	//�ͷ��ٽ���
	if(m_mutex!=0){
		BOOL success=::CloseHandle(m_mutex);
		m_mutex=0;
	}
}

void MutexLock::getLock()
{
	//����(ԭ�Ӳ���)
	unsigned int count=InterlockedIncrement(&m_waitCount.counter);

	//�����ٽ���
	unsigned long result=::WaitForSingleObject(m_mutex, INFINITE);
	//bAlertableΪTRUEʱ���ܷ�������WAIT_IO_COMPLETION
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::getLock: wait for mutex "\
			"failed : %d",  error);
		throw Exception(str);
	}

	InterlockedDecrement(&m_waitCount.counter);
}

void MutexLock::releaseLock()
{
	//�뿪�ٽ���
	BOOL success = ::ReleaseMutex(m_mutex);
	if(!success)
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::releaseLock: release mutex "\
			"failed (error=%d)",error);
		throw Exception(str);
	}
}

long MutexLock::getWaitCount() const
{
	return m_waitCount.counter;
}


// Linux MutexLock
#else

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	atomic_set(&m_waitCount, 0);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	// create process shared mutex if `name` passed
	if (name)
	{
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

		int fd = open(name, O_RDWR, 0666);
		if (fd < 0)
			throw Exception("");
		mutex_t* sharedMutex = (mutex_t*)mmap(NULL, sizeof(mutex_t),
			PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, 0);
		close(fd);
		m_mutex = *sharedMutex;
	}

	int error=pthread_mutex_init(&m_mutex,&attr);
	pthread_mutexattr_destroy(&attr);
	if(error!=0)
	{
		String str=String::format("init mutex failed : %d",error);
		throw Exception(str);
	}

}

MutexLock::~MutexLock(void)
{
	//�ͷ��ٽ���
	int error=pthread_mutex_destroy(&m_mutex);
}

void MutexLock::getLock()
{
	//����(ԭ�Ӳ���)
	atomic_inc(&m_waitCount);

	//�����ٽ���
	int error=pthread_mutex_lock(&m_mutex);
	if(error!=0)
	{
		String str=String::format("MutexLock::getLock: wait for mutex "\
			"failed : %d",error);
		throw Exception(str);
	}

	atomic_dec(&m_waitCount);
}

void MutexLock::releaseLock()
{
	//�뿪�ٽ���
	int error=pthread_mutex_unlock(&m_mutex);
	if(error!=0)
	{
		String str=String::format("MutexLock::releaseLock: release mutex "\
			"failed (error=%d)",error);
		throw Exception(str);
	}
}

long MutexLock::getWaitCount() const
{
	return atomic_read(&m_waitCount);
}

#endif

}//end of namespace bluemei