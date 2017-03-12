#include "ConditionLock.h"
#include "AccurateTimeTicker.h"

namespace blib {

#ifdef WIN32

ConditionLock::ConditionLock(ConditionLock&& other) {
	*this = std::move(other);
}

ConditionLock& ConditionLock::operator=(ConditionLock&& other) {
	this->destroy();
	this->m_condition = other.m_condition;
	other.init();

	return *this;
}

void ConditionLock::init() {
	m_condition.semaphore = ::CreateSemaphoreA(NULL, 0, 1, NULL);
	if (m_condition.semaphore == 0) {
		int error = ::GetLastError();
		String str = String::format(
				"ConditionLock: failed to init semaphore: %d", error);
		throw Exception(str);
	}

	m_condition.mutex = ::CreateMutexA(NULL, FALSE, NULL);
	if (m_condition.mutex == 0) {
		int error = ::GetLastError();
		String str = String::format("ConditionLock: failed to init mutex: %d",
				error);
		throw Exception(str);
	}
}

void ConditionLock::destroy() {
	if (m_condition.semaphore != NULL) {
		BOOL success = ::CloseHandle(m_condition.semaphore);
		assert(success);
		m_condition.semaphore = NULL;
	}

	if (m_condition.mutex != NULL) {
		BOOL success = ::CloseHandle(m_condition.mutex);
		assert(success);
		m_condition.mutex = NULL;
	}
}

//类似条件变量的等待： http://blog.csdn.net/metasearch/article/details/18677193
bool ConditionLock::wait(unsigned long ms) throw (Exception) {
	++m_waitCount;

	//解锁mutex,等待信号semaphoreHandle（原子操作）
	unsigned long result = ::SignalObjectAndWait(m_condition.mutex,
			m_condition.semaphore, ms, FALSE);
	//重新锁定mutex
	this->getLock();

	--m_waitCount;

	//错误检测
	//SignalObjectAndWait bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	if (result == WAIT_FAILED || result == WAIT_ABANDONED) {
		int error = ::GetLastError();
		String str = String::format(
				"ConditionLock: failed to wait for semaphore: %d", error);
		throw Exception(str);
	}

	return result != WAIT_TIMEOUT; //没有超时
}

void ConditionLock::wakeup() throw (Exception) {
	long previousCount = 0;
	//增加一个信号
	BOOL success = ::ReleaseSemaphore(m_condition.semaphore, 1,
		&previousCount);
	if (!success) {
		int error = ::GetLastError();
		String str = String::format(
				"ConditionLock: failed to release semaphore: %d", error);
		throw Exception(str);
	}
}

void ConditionLock::getLock() throw (Exception) {
	unsigned long result = ::WaitForSingleObject(m_condition.mutex, INFINITE);
	//bAlertable为TRUE时可能发生错误WAIT_IO_COMPLETION
	if (result == WAIT_FAILED || result == WAIT_ABANDONED) {
		int error = ::GetLastError();
		String str = String::format(
				"ConditionLock: failed to wait for mutex: %d", error);
		throw Exception(str);
	}
}

void ConditionLock::releaseLock() throw (Exception) {
	//离开临界区
	BOOL success = ::ReleaseMutex(m_condition.mutex);
	if (!success) {
		int error = ::GetLastError();
		String str = String::format(
				"ConditionLock: failed to release mutex: %d", error);
		throw Exception(str);
	}
}

#else // not WIN32

ConditionLock::ConditionLock(ConditionLock&& other) {
	*this = std::move(other);
}

ConditionLock& ConditionLock::operator=(ConditionLock&& other) {
	this->destroy();
	this->m_condition = other.m_condition;
	other.init();

	return *this;
}

void ConditionLock::init() {
	pthread_mutexattr_t psharedm;
	pthread_condattr_t psharedc;

	(void) pthread_mutexattr_init(&psharedm);
	(void) pthread_mutexattr_setpshared(&psharedm, PTHREAD_PROCESS_PRIVATE);
	(void) pthread_mutexattr_settype(&psharedm, PTHREAD_MUTEX_RECURSIVE);

	(void) pthread_condattr_init(&psharedc);
	(void) pthread_condattr_setpshared(&psharedc, PTHREAD_PROCESS_PRIVATE);

	(void) pthread_mutex_init(&m_condition.mutex, &psharedm);
	(void) pthread_cond_init(&m_condition.cond, &psharedc);
}

void ConditionLock::destroy(void) {
	;
}

bool ConditionLock::wait(unsigned long ms/*=INFINITE*/) throw (Exception) {
	// m_condition.mutex must be locked before call wait()
	int status = 0;

	m_waitCount++;
	if (ms == INFINITE) {
		status = pthread_cond_wait(&m_condition.cond, &m_condition.mutex);
	} else {
		timeval offset = { ms / 1000, (ms % 1000) * 1000 };
		timeval now; (void) gettimeofday(&now, NULL);
		timeradd(&now, &offset, &offset);
		timespec timeout = { offset.tv_sec, offset.tv_usec * 1000 };
		status = pthread_cond_timedwait(&m_condition.cond, &m_condition.mutex,
				&timeout);
	}
	m_waitCount--;

	if (status == 0) {
		return true; // it's not timeout
	}
	else if (status == ETIMEDOUT) {
		return false; // timeout
	}
	else if (status == EINVAL || status == EPERM) {
		throw Exception(String::format(
						"ConditionLock: failed to wait for condition: %s",
						strerror(errno)));
	}
	else {
		assert(status == 0);
		return true;
	}
}

void ConditionLock::wakeup() throw (Exception) {
	int status = pthread_cond_signal(&m_condition.cond);
	if (status != 0) {
		throw Exception(String::format("ConditionLock: failed to signal "
				"condition: %s", strerror(errno)));
	}
}

void ConditionLock::getLock() throw (Exception) {
	int status = pthread_mutex_lock(&m_condition.mutex);
	if (status != 0) {
		throw Exception(String::format("ConditionLock: failed to lock "
				"mutex: %s", strerror(errno)));
	}
}

void ConditionLock::releaseLock() throw (Exception) {
	int status = pthread_mutex_unlock(&m_condition.mutex);
	if (status != 0) {
		throw Exception(String::format("ConditionLock: failed to unlock "
				"mutex: %s", strerror(errno)));
	}
}

#endif

} //end of namespace blib
