#include "MutexLock.h"

namespace bluemei{

// Windows MutexLock
#ifdef WIN32

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	waitCount.counter = 0;
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
	//自增(原子操作)
	unsigned int count=InterlockedIncrement(&waitCount.counter);

	//进入临界区
	unsigned long result=::WaitForSingleObject(mutex, INFINITE);
	//bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::getLock: wait for mutex "\
			"failed : %d",  error);
		throw Exception(str);
	}

	InterlockedDecrement(&waitCount.counter);
}

void MutexLock::releaseLock()
{
	//离开临界区
	BOOL success = ::ReleaseMutex(mutex);
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
	return waitCount.counter;
}


// Linux MutexLock
#else

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	atomic_set(&waitCount, 0);

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
		mutex = *sharedMutex;
	}

	int error=pthread_mutex_init(&mutex,&attr);
	pthread_mutexattr_destroy(&attr);
	if(error!=0)
	{
		String str=String::format("init mutex failed : %d",error);
		throw Exception(str);
	}

}

MutexLock::~MutexLock(void)
{
	//释放临界区
	int error=pthread_mutex_destroy(&mutex);
}

void MutexLock::getLock()
{
	//自增(原子操作)
	atomic_inc(&waitCount);

	//进入临界区
	int error=pthread_mutex_lock(&mutex);
	if(error!=0)
	{
		String str=String::format("MutexLock::getLock: wait for mutex "\
			"failed : %d",error);
		throw Exception(str);
	}

	atomic_dec(&waitCount);
}

void MutexLock::releaseLock()
{
	//离开临界区
	int error=pthread_mutex_unlock(&mutex);
	if(error!=0)
	{
		String str=String::format("MutexLock::releaseLock: release mutex "\
			"failed (error=%d)",error);
		throw Exception(str);
	}
}

long MutexLock::getWaitCount() const
{
	return atomic_read(&waitCount);
}

#endif

}//end of namespace bluemei