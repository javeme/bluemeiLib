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

	//�Ƿ���Ҫ֪ͨ
	bool needNofity=m_messageQueue.empty();//m_messageLock.getWaitCount()>0

	//��Ϣ���
	m_messageQueue.push(msg);

	//֪ͨ����Ϣ��
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
	while(msg==nullptr) //��while���if��ֹ��ٻ���
	{
		//�������m_messageLock.getWaitCount()>0���ж��Ƿ�notify, ��ô
		//����m_queueLock�Ļ�,���ܻ��������ж���ȴ�֮�������һ������,��������notify!
		//����wait��һ�̲��ͷ�m_queueLock,��ȷ����������������(����䴦ʱ�̲�������)
		m_messageLock.wait(m_queueLock);

		//��wait���źź�, m_queueLock���ܱ������̻߳�ö�ȡ��Message, ���±���ȡ����
		//����Ҳ�п�������ٻ���
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