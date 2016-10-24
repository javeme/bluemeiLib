#include "CodeUtil.h"
#include "System.h"
//TODO: why error C2011 if include "MessageThread.h" before include "System.h"
#include "MessageThread.h"
#include "ErrorHandler.h"


namespace bluemei{

MessageThread::MessageThread()
{
	;
}

MessageThread::MessageThread(const MessageFunction& fun):m_msgFun(fun)
{
	;
}

MessageThread::~MessageThread()
{
	clearMessage();
}

void MessageThread::postMessage(Message* msg)
{
	ScopedLock scopedLock(m_queueLock);

	//是否需要通知
	bool needNofity=m_messageQueue.empty();//m_messageLock.getWaitCount()>0

	//消息入队
	m_messageQueue.push(msg);

	//通知有消息了
	if(needNofity) {
		// to ensure that notify someone to get a msg if it's waiting for one
		m_messageLock.notify();
	}
}

void MessageThread::clearMessage()
{
	ScopedLock scopedLock(m_queueLock);

	while(!m_messageQueue.empty()){
		Message* msg=m_messageQueue.top();
		m_messageQueue.pop();

		if(msg!=nullptr)
			msg->release();
	}
}

Message* MessageThread::peekMessage()
{
	Message* msg=nullptr;
	ScopedLock scopedLock(m_queueLock);

	if(!m_messageQueue.empty()){
		msg=m_messageQueue.top();
	}

	return msg;
}

Message* MessageThread::nextMessage()
{
	Message* msg=nullptr;
	ScopedLock scopedLock(m_queueLock);

	if(!m_messageQueue.empty()){
		msg=m_messageQueue.top();
		m_messageQueue.pop();
	}

	return msg;
}

Message* MessageThread::waitMessage()
{
	ScopedLock scopedLock(m_queueLock);

	Message* msg=nextMessage();
	while(msg==nullptr) //用while替代if防止虚假唤醒
	{
		//如果根据m_messageLock.getWaitCount()>0来判断是否notify, 那么
		//不锁m_queueLock的话,可能会在上述判断与等待之间插入了一条数据,而不会再notify!
		//这里wait那一刻才释放m_queueLock,可确保不会出现上述情况(此语句处时刻插入数据)
		m_messageLock.wait(m_queueLock);

		//当wait到信号后, m_queueLock可能被其它线程获得而取走Message, 导致本处取不到
		//或者也有可能是虚假唤醒
		msg=nextMessage();
		if(msg==nullptr){
			if(!isRunning())
				break;
			static int count=0;
			System::debugInfo("MessageThread::waitMessage:Message is null(%d).\n",
				++count);
		}
	}

	return msg;
}

bool MessageThread::hasMessage() const
{
	return !m_messageQueue.empty();
}

void MessageThread::doMessageLoop()
{
	while(isRunning()){
		Message* msg=waitMessage();
		try{
			onMessage(msg);
		}catch (ExitLoopException&){
			m_bRunning=false;
		}catch (Exception& e){
			ErrorHandler::handle(e);
		}catch (...){
			if(msg) msg->release();
			throw;
		}
		if(msg) msg->release();
	}
}

void MessageThread::stop()
{
	if(isRunning())
	{
		m_bRunning=false;
		while(m_messageLock.getWaitCount()>0)
			m_messageLock.notifyAll();
		wait();
	}
}

void MessageThread::run()
{
	try{
		doMessageLoop();
	}catch (std::exception& e){
		ErrorHandler::handle(StdException(e));
	}catch (...){
		ErrorHandler::handle(UnknownException("MessageThread unknown error"));
	}
}

void MessageThread::onMessage(Message* msg)
{
	if(msg!=nullptr){
		m_msgFun(msg);
	}
	else{
		static int count=0;
		System::debugInfo("MessageThread::onMessage:Message is null(%d).\n",
			++count);
	}
}

}//end of namespace bluemei