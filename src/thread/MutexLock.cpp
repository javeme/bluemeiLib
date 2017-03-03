#include "MutexLock.h"
#include "RuntimeException.h"

namespace blib {

// Windows MutexLock
#ifdef WIN32

MutexLock::MutexLock(bool initialOwner,cstring name)
{
	m_waitCount.counter = 0;
	//初始化临界区
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
	//释放临界区
	if(m_mutex!=0) {
		BOOL success=::CloseHandle(m_mutex);
		m_mutex=0;
	}
}

void MutexLock::getLock()
{
	//自增(原子操作)
	unsigned int count=InterlockedIncrement(&m_waitCount.counter);

	//进入临界区
	unsigned long result=::WaitForSingleObject(m_mutex, INFINITE);
	//bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::getLock: wait for mutex "
				"failed : %d", error);
		throw Exception(str);
	}

	InterlockedDecrement(&m_waitCount.counter);
}

void MutexLock::releaseLock()
{
	//离开临界区
	BOOL success = ::ReleaseMutex(m_mutex);
	if(!success)
	{
		int error=::GetLastError();
		String str=String::format("MutexLock::releaseLock: release mutex "
				"failed (error=%d)",error);
		throw Exception(str);
	}
}

unsigned int MutexLock::getWaitCount() const
{
	return m_waitCount.counter;
}

// Linux MutexLock
#else

MutexLock::MutexLock(bool initialOwner, cstring name) :
		m_waitCount(0), m_recursiveCount(0) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	// create process shared mutex if `name` passed
	if (name) {
		pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

		int fd = open(name, O_RDWR, 0666);
		if (fd < 0) {
			throw Exception(String("MutexLock: failed to open mmap file"
					"for mutex: ") + name);
		}
		m_mutex = (mutex_t*) mmap(NULL, sizeof(mutex_t),
				PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd);
	} else {
		m_mutex = (mutex_t*) mmap(NULL, sizeof(mutex_t),
				PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	}

	checkNullPtr(m_mutex);
	int error = pthread_mutex_init(m_mutex, &attr);
	pthread_mutexattr_destroy(&attr);
	if (error != 0) {
		String str = String::format("MutexLock: init mutex failed : %d", error);
		throw Exception(str);
	}

}

MutexLock::~MutexLock(void) {
	//释放临界区
	int error = pthread_mutex_destroy(m_mutex);
	assert(error == 0);

	munmap((void*) m_mutex, sizeof(mutex_t));
	m_mutex = null;
}

void MutexLock::getLock() {
	checkNullPtr(m_mutex);
	//自增(原子操作)
	m_waitCount++;

	//进入临界区
	int error = pthread_mutex_lock(m_mutex);
	if (error != 0) {
		String str = String::format("MutexLock::getLock: wait for mutex "
				"failed : %d", error);
		throw Exception(str);
	}

	m_recursiveCount++;

	m_waitCount--;
}

void MutexLock::releaseLock() {
	checkNullPtr(m_mutex);
	m_recursiveCount--;

	//离开临界区
	int error = pthread_mutex_unlock(m_mutex);
	if (error != 0) {
		String str = String::format("MutexLock::releaseLock: release mutex "
				"failed (error=%d)", error);
		throw Exception(str);
	}
}

unsigned int MutexLock::getWaitCount() const {
	return m_waitCount.load();
}

unsigned int MutexLock::getMyThreadEnteredCount() const {
	return m_recursiveCount.load();;
}

#endif

} //end of namespace blib
