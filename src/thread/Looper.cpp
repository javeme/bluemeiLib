#include "Looper.h"

namespace bluemei{

////////////////////////////////////////////////////////////////////////
// class RunnableMessage
class BLUEMEILIB_API RunnableMessage : public Message
{
public:
	RunnableMessage(int type, const ThreadFunction& f, int priority=0) :
		m_msgType(type),
		m_msgPriority(priority),
		m_func(f) {
	}

	virtual void run() { m_func(); }

	virtual int getPriority() const { return 0; }
	virtual void setPriority(int val) { m_msgPriority = val; }

	virtual int getType() const { return m_msgType; }
	virtual void setType(int val) { m_msgType = val; }
private:
	int m_msgType;
	int m_msgPriority;		
	ThreadFunction m_func;
};


////////////////////////////////////////////////////////////////////////
// class Looper
void Looper::start(bool newThread/*=true*/)
{
	if (m_msgThread != null) {
		return;
	}

	m_msgThread = new MessageThread([&](Message* msg) {
		return this->onMessage(msg);
	});

	if (newThread) {
		m_msgThread->start();
	}
	else {
		// attach to current thread (like main thread)
		m_msgThread->attach();
		// loop until stop() has been called
		m_msgThread->run();
		// detach from current thread, otherwise the current thread
		// may be stoped and destroyed when Thread class destructing
		m_msgThread->detach();
	}
}

void Looper::stop()
{
	checkNullPtr(m_msgThread);

	// NOTE: don't use "m_msgThread->stop()" -- it may lead to some
	// of the messages left in the queue.
	ThreadFunction empty;
	m_msgThread->postMessage(new RunnableMessage(MSG_EXIT_LOOP, empty));
	m_msgThread->wait();

	delete m_msgThread;
	m_msgThread = null;
}

void Looper::postRunnable(const ThreadFunction& f, int priority/*=0*/)
{
	checkNullPtr(m_msgThread);

	Message* msg = new RunnableMessage(MSG_EXE_RUNNABLE, f, priority);
	this->m_msgThread->postMessage(msg);
}

void Looper::onMessage(Message* msg)
{
	checkNullPtr(msg);

	switch(msg->getType()) {
	case MSG_EXIT_LOOP:
		throw MessageThread::ExitLoopException();
		break;
	case MSG_EXE_RUNNABLE:
		{
			RunnableMessage* runnable = dynamic_cast<RunnableMessage*>(msg);
			if(runnable != null) {
				runnable->run();
			}
			break;
		}
	default:
		// ignore
		break;
	}
}

}//end of namespace bluemei