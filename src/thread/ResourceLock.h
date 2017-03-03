#ifndef ResourceLock_H_H
#define ResourceLock_H_H

#include "bluemeiLib.h"
#include "Exception.h"
#include "Object.h"

namespace blib{

#ifdef WIN32

typedef struct {
	HANDLE handle;
	std::atomic<unsigned int> waitcount;
} semaphore_t, semaphore_h;

#else // not WIN32

#define INFINITE -1

typedef struct {
	pthread_mutex_t lock;
	pthread_cond_t nonzero;
	volatile unsigned int count;
	std::atomic<unsigned int> waitcount;
} semaphore_t, *semaphore_h;

#endif //end of #ifdef WIN32


#define DEFAULT_MAX_COUNT INT_MAX

// TODO: rename to SemaphoreLock
class BLUEMEILIB_API ResourceLock : public Object
{
public:
	ResourceLock(unsigned int count=0, cstring name=nullptr);
	virtual ~ResourceLock(void);
public:
	ResourceLock(ResourceLock&& other);
	ResourceLock& operator=(ResourceLock&& other);
private:
	ResourceLock(const ResourceLock& other);
	ResourceLock& operator=(const ResourceLock& other);
private:
	semaphore_h m_semaphore;
protected:
	virtual void init(unsigned int count,unsigned int maxCount,
		cstring name=null);
	virtual void destroy();
public:
	virtual bool wait(unsigned long ms=INFINITE) throw(Exception);
	virtual unsigned int signal(unsigned int count=1) throw(Exception);
	virtual void notify() throw(Exception);
	virtual void notifyAll() throw(Exception);
	virtual unsigned int getWaitCount() const;
};

typedef ResourceLock SyncLock;

}//end of namespace blib

#endif
