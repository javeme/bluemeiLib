/*
*edit by blib
*write for java-users
*/
#ifndef Thread_H_H
#define Thread_H_H

#include "bluemeiLib.h"
#include "Runnable.h"
#include "ThreadException.h"
#include "ResourceLock.h"
#include "CriticalLock.h"

#ifdef WIN32

#include <process.h>

typedef HANDLE thread_handle;

#define THREAD_CALLBACK WINAPI

#else // not WIN32

#include <thread>

//typedef pthread_t ThreadHandle;
typedef std::thread thread_handle;

#define THREAD_CALLBACK

#endif //end of #ifdef WIN32


namespace blib{

class BLUEMEILIB_API Thread : public Runnable
{
public:
	Thread(void* pUserParameter=NULL);;
	Thread(Runnable* pTarget,void* pUserParameter=NULL);
	virtual ~Thread();
private:
	Thread(const Thread& other);
	Thread& operator=(const Thread& other);
public:
	virtual void start() throw(Exception);
	virtual void stop() throw(Exception);
	virtual void attach() throw(Exception);
	virtual void detach() throw(Exception);
	virtual void wait() throw(Exception);
	static void sleep(unsigned int msecond);

	void setAutoDestroy(bool bAutoDestroy);
	bool isAutoDestroyObj() const;
	void* getUserParameter() const;
public:
	virtual void run();
	virtual bool isRunning() const;

	virtual unsigned long getThreadId() const;
	static unsigned long currentThreadId();
	static unsigned long mainThreadId();
public:
	int callBackStartThread();
protected:
	virtual void init();
	virtual void destroy() throw(Exception);
	virtual void join() throw(Exception);
	virtual void terminate() throw(Exception);
protected:
	Runnable* m_pObject;
	void* m_pUserParameter;
	unsigned long m_threadId;
	static unsigned long s_mainThreadId;

	SyncLock m_lock;
	bool m_bAutoDestroyObj;
	volatile bool m_bRunning,m_bDetached;
private:
	thread_handle m_hThread;
};

}//end of namespace blib

#endif
