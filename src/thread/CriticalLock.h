#ifndef CriticalLock_H_H
#define CriticalLock_H_H

#include "bluemeiLib.h"
#include "UniqueLock.h"


#ifdef WIN32

namespace blib{

class BLUEMEILIB_API CriticalLock : public UniqueLock
{
public:
	CriticalLock();
	virtual ~CriticalLock(void);
private:
	CRITICAL_SECTION m_critialSection;
	volatile unsigned int m_waitCount;//等待的线程数
public:
	virtual void getLock();
	virtual void releaseLock();
	void notify(){ return releaseLock();}
	virtual unsigned int getWaitCount() const;
	virtual unsigned int getMyThreadEnteredCount() const;
};

} //end of namespace blib

#else // not WIN32

// TODO: implement
#include "MutexLock.h"

namespace blib{

class BLUEMEILIB_API CriticalLock : public MutexLock
{
};

} //end of namespace blib

#endif //end of #ifdef WIN32

#endif //end of header guards
