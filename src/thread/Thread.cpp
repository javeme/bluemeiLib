#include "Thread.h"
#include "ErrorHandler.h"

namespace blib {

///////////////////////////////////////////////////////////////////////////////
// callback of new Thread
unsigned int THREAD_CALLBACK callBackOfStartThread(void* thread) {
	return ((Thread*)thread)->callBackStartThread();
}

///////////////////////////////////////////////////////////////////////////////
// class Thread
Thread::Thread(void* pUserParameter/*=NULL*/) {
	this->m_pObject = this;
	this->m_pUserParameter = pUserParameter;
	this->init();
}

Thread::Thread(Runnable* pTarget, void* pUserParameter) {
	//this->m_pThreadParameter=new ThreadParameter();
	this->m_pObject = pTarget;
	this->m_pUserParameter = pUserParameter;
	this->init();
}

Thread::~Thread() {
	try {
		this->destroy();
	} catch (Exception& e) {
		ErrorHandler::handle(e);
	} catch (std::exception& e) {
		ErrorHandler::handle(StdException(e));
	} catch (...) {
		ErrorHandler::handle(UnknownException("Thread::destroy() error"));
	}
}

void Thread::setAutoDestroy(bool bAutoDestroy) {
	m_bAutoDestroyObj = bAutoDestroy;
}

bool Thread::isAutoDestroyObj() const {
	return m_bAutoDestroyObj;
}

void* Thread::getUserParameter() const {
	return m_pUserParameter;
}

void Thread::wait() throw(Exception) {
	if (!m_bDetached) {
		if (this->getThreadId() == Thread::mainThreadId()) {
			throwpe(ThreadException("can't wait for the main thread"));
		}
		this->join();
		this->detach();
	} else {
		throwpe(ThreadException("thread has finished or detached"));
	}
}

void Thread::sleep(unsigned int msecond) {
	msleep(msecond);
}

void Thread::stop() throw(Exception) {
	if (getThreadId() == Thread::mainThreadId() && !m_bDetached) {
		throwpe(ThreadException("can't stop the main thread"));
	}

	if (isRunning()) {
		if (getThreadId() == Thread::currentThreadId()) {
			//本线程结束自己时,自然结束即可
		} else {
			//其它线程结束本线程 (_endthreadex及ExitThread为自己结束本线程)
			this->terminate();
		}
	} else {
		//throwpe(ThreadException("thread not running"));
	}
	this->m_bRunning = false;
}

void Thread::destroy() throw(Exception) {
	this->stop();
	this->detach();

	// NOTE: thread users should release these parameters
	//if(m_pThreadParameter->pUserParameter!=NULL)
	//	delete m_pThreadParameter->pUserParameter;
	//delete m_pThreadParameter;
}

// NOTE: don't raise any exception
int Thread::callBackStartThread() {
	int ret = 0;
	m_bRunning = true;
	if (this->m_threadId == 0) {
		this->m_threadId = Thread::currentThreadId();
	}

	try {
		m_lock.signal();

		if (this->m_pObject)
			this->m_pObject->run();
	} catch (Exception& e) {
		ErrorHandler::handle(e);
		ret = -1;
	} catch (std::exception& e) {
		ErrorHandler::handle(StdException(e));
		ret = -1;
	} catch (...) {
		ErrorHandler::handle(UnknownException("Thread error"));
		ret = -1;
	}
	m_bRunning = false;

	if (m_bAutoDestroyObj) {
		//destroy();
		delete this;
	}

	return ret;
}

//执行
void Thread::run() {
	// nothing
}

bool Thread::isRunning() const {
	return m_bRunning;
}

unsigned long Thread::getThreadId() const {
	return m_threadId;
}

unsigned long Thread::s_mainThreadId = Thread::currentThreadId();

unsigned long Thread::mainThreadId() {
	return s_mainThreadId;
}

#ifdef WIN32

unsigned long Thread::currentThreadId() {
	return ::GetCurrentThreadId();
}

//初始化
void Thread::init() {
	m_bDetached = false;
	m_bAutoDestroyObj = false;
	m_bRunning = false;
	//m_hThread=::CreateThread(NULL,0,CallBackOfCreateThread,(LPVOID)this,
	//	CREATE_SUSPENDED,&m_threadId);
	m_hThread = (HANDLE) _beginthreadex(NULL, 0, callBackOfStartThread,
			(LPVOID)this, CREATE_SUSPENDED, &m_threadId);
}

//开始执行
void Thread::start() throw(Exception) {
	if (::ResumeThread(m_hThread) < 0) {
		ThreadException e(GetLastError());
		e.setExceptionMsg("start thread failed");
		throwpe(e);
	}

	if (!m_lock.wait(1000 * 30)) { //超时 wait发生异常该如何处理???
		throwpe(ThreadException("timeout to start the new thread"));
	}
}

void Thread::attach() throw(Exception) {
	//分离老线程句柄
	this->destroy();

	//关联当前线程句柄
	unsigned int current = Thread::currentThreadId();
	m_hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, current);
	if (m_hThread != NULL) {
		m_threadId = current;
		m_bRunning = true;
		m_bDetached = false;
	}
}

void Thread::detach() throw(Exception) {
	//关闭句柄,使处于分离状态.(但线程结束时,调用此函数释放系统资源)
	if (!m_bDetached) {
		BOOL success = ::CloseHandle(m_hThread);
		assert(success);
		m_hThread = NULL;
	}
	m_bDetached = true;
}

void Thread::join() throw(Exception) {
	int result = ::WaitForSingleObject(m_hThread, INFINITE);
	if (result == WAIT_FAILED) {
		int error = ::GetLastError();
		String str = String::format("failed to join thread: %d", error);
		throwpe(ThreadException(str));
	}
}

void Thread::terminate() throw(Exception) {
	BOOL success = ::TerminateThread(m_hThread, 0);
	if (!success) {
		int error = ::GetLastError();
		String str = String::format("failed to terminate thread: %d", error);
		throwpe(ThreadException(str));
	}
}

#else // not WIN32

unsigned long Thread::currentThreadId() {
	// TODO: mapping std::thread::id to unsigned long
	std::thread::id id = std::this_thread::get_id();
	return (*(unsigned long*)&id); // not safe
}

void Thread::init() {
	m_bDetached = false;
	m_bAutoDestroyObj = false;
	m_bRunning = false;
	m_threadId = 0;
}

void Thread::start() throw(Exception) {
	std::thread t(blib::callBackOfStartThread, (void*)this);

	if (!m_lock.wait(1000 * 30)) {
		throwpe(ThreadException("timeout to start the new thread"));
	}
	this->m_hThread = std::move(t);
}

// attach to current thread (like main thread)
void Thread::attach() throw(Exception) {
	this->destroy();

	// FIXME: don't dependent on member std::thread.__t_
	std::thread::id current = std::this_thread::get_id();
	std::thread t;
	printf("Thread::attach() unsafe!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	(*(std::thread::id*)&t) = current; // not safe

	if (t.joinable()) {
		this->m_hThread = std::move(t);
		m_threadId = Thread::currentThreadId();
		m_bRunning = true;
		m_bDetached = false;
	}
}

void Thread::detach() throw(Exception) {
	if (!m_bDetached) {
		m_hThread.detach();
	}
	m_bDetached = true;
}

void Thread::join() throw(Exception) {
	m_hThread.join();
	m_bDetached = true;
}

void Thread::terminate() throw(Exception) {
	// TODO: terminate through pthread
	// NOTE: http://stackoverflow.com/questions/12207684/how-do-i-terminate-a-thread-in-c11
	m_hThread.detach();
	m_hThread.~thread();
}

#endif //end of #ifdef WIN32

}	//end of namespace blib
