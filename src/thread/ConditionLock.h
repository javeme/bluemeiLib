#ifndef ConditionLock_H_H
#define ConditionLock_H_H

#include "bluemeiLib.h"
#include "Exception.h"
#include "UniqueLock.h"
#include "Atomic.h"

namespace blib{

#ifdef WIN32

typedef struct {
	HANDLE mutex;
	HANDLE semaphore;
} condition_t;

#else // not WIN32

#define INFINITE -1

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} condition_t;

#endif //end of #ifdef WIN32


/* class ConditionLock
 * usage example:
 *  ConditionLock cond;
    void wait()
    {
        cond.getLock();
        while ({condition is not true})
            cond.wait();
        {do something...}
        {reset condition to false}
        cond.releaseLock();
    }

    void wakeup()
    {
        cond.getLock();
        {set condition to true}
        cond.wakeup();
        cond.releaseLock();
    }
 */
class BLUEMEILIB_API ConditionLock : public UniqueLock
{
public:
	ConditionLock() { this->init(); }
	virtual ~ConditionLock() { this->destroy(); }
public:
	ConditionLock(ConditionLock&& other);
	ConditionLock& operator=(ConditionLock&& other);
private:
	ConditionLock(const ConditionLock& other);
	ConditionLock& operator=(const ConditionLock& other);
protected:
	virtual void init();
	virtual void destroy();
public:
	virtual bool wait(unsigned long ms=INFINITE) throw(Exception);
	virtual void wakeup() throw(Exception);

	virtual void getLock() throw(Exception);
	virtual void releaseLock() throw(Exception);

	virtual unsigned int getWaitCount() const { return m_waitCount; }
private:
	condition_t m_condition;
	std::atomic<unsigned int> m_waitCount;
};

}//end of namespace blib

#endif
