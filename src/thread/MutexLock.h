#ifndef MutexLock_H_H
#define MutexLock_H_H

#include "bluemeiLib.h"
#include "Exception.h"
#include "UniqueLock.h"
#include "Atomic.h"

#ifdef WIN32

typedef HANDLE mutex_t;

#else

typedef pthread_mutex_t* mutex_t;

#endif


namespace blib{

class BLUEMEILIB_API MutexLock : public UniqueLock
{
public:
	MutexLock(bool initialOwner=false,cstring name=nullptr);
	virtual ~MutexLock(void);
private:
	MutexLock(const MutexLock& other);
	MutexLock& operator=(const MutexLock& other);
private:
	friend class ResourceLock;
	mutex_t m_mutex;
	std::atomic<unsigned int> m_waitCount;//等待的线程数(跨进程时无效)
	volatile unsigned int m_recursiveCount;
public:
	virtual void getLock();
	virtual void releaseLock();
	void notify(){ return releaseLock();}
	virtual unsigned int getWaitCount() const;
	virtual unsigned int getMyThreadEnteredCount() const;
};

}//end of namespace blib

#endif
