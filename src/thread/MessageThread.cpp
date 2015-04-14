#include "stdafx.h"
#include "MessageThread.h"
#include "CodeUtil.h"
#include "utils\System.h"

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

void MessageThread::addMessage( Message* msg )
{
	ScopedLock scopedLock(m_queueLock);

	//是否需要通知
	bool needNofity=m_messageQueue.empty();//m_messageLock.getWaitCount()>0

	//消息入队
	m_messageQueue.push(msg);
	
	//通知有消息了
	if(needNofity)
		m_messageLock.notify();
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
	if(msg==nullptr)//while
	{
		//不锁m_queueLock的话,可能会在上述判断与等待之间插入了一条数据,而不会再notify!!!
		m_messageLock.wait(m_queueLock);
		msg=nextMessage();
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
		onMessage(msg);
		/*if (hasMessage()){
			Message* msg=nextMessage();
			onMessage(msg);
		}
		else
			sleep(1);//如何处理好??? */
	}
}

void MessageThread::finish()
{
	if(isRunning())
	{
		m_bRunning=false;
		while(m_messageLock.getWaitCount()>0)
			m_messageLock.notifyAll();
		wait();
	}
}

void MessageThread::run( ThreadParameter *pThreadParameter )
{
	doMessageLoop();
}

void MessageThread::onMessage( Message* msg )
{
	if(msg!=nullptr){
		m_msgFun(msg);
		msg->release();
	}
	else{
		static int count=0;
		System::debugInfo("MessageThread::onMessage:Message is null(%d).\n",++count);
	}
}

}//end of namespace bluemei

/*
//test MessageThread
#include "MessageThread.h"
static bluemei::MessageThread msgThread([](Message* msg){
	TRACE(">>>>Message: %d\n",msg->getId());
});
msgThread.start();

msgThread.addMessage(new Message(100,0,0,new Exception(e)));
msgThread.addMessage(new Message(101,1));
msgThread.addMessage(new Message(102,2));
msgThread.addMessage(new Message(1022,2));
msgThread.addMessage(new Message(1021,2));
msgThread.addMessage(new Message(1020,2));
msgThread.addMessage(new Message(108,8));
msgThread.addMessage(new Message(103,3));
msgThread.addMessage(new Message(1031,3));
msgThread.addMessage(new Message(1030,3));
*/
/*
//test MessageThread
#include "MessageThread.h"
#include "LambdaThread.h"
#include "MessageHandler.h"

void testMessageThread(MainFrame* _this)
{
	static MessageHandler mh(_this);

	static bluemei::MessageThread msgThread([=](Message* msg){
		String str=String::format(">>>>Message: %5d [%s] (%s)\n",msg->getId(),
			Date(msg->getTimestamp()).toString().c_str(),
			msg->getObject()->toString().c_str());
		TRACE(str.c_str());
		mh.PostRunnable([=](void* para){ _this->OutputMessage((CString)CONVSTR(str)); });

		Sleep(1);
	});

	msgThread.start();

	mh.PostDelay(1000*5,[_this](void* para){ 
		msgThread.finish();		
		String str=String::format(">>>>>>>>Message Thread finish (remained message: %s)\n",
			String(String("")+msgThread.hasMessage()).c_str());
		_this->OutputMessage((CString)CONVSTR(str));
	});

	static int count=0;
	for(int i=0;i<10;i++)
	{
		Thread* thread=new LambdaThread([i](void* para){
			for(int j=0;j<1000;j++)
			{
				int pri=j;
				msgThread.addMessage(new Message(count++,pri,0,0,new String(String("test@")+pri)));
				Sleep(10);
			}
			TRACE(">>>>>>>>>>>>>>>>Work Thread %d finished\n",i);
		},nullptr);
		thread->start();
	}
}
*/