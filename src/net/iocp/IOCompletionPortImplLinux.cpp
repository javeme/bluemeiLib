#include "stdafx.h"
#include "IOCPException.h"
#include "IOCompletionPortImplLinux.h"

namespace bluemei{

IOCompletionPortImpl::IOCompletionPortImpl()
{
	m_hIOCompletionPort=NULL;
}

IOCompletionPortImpl::~IOCompletionPortImpl()
{
	if (m_hIOCompletionPort!=NULL)
	{
		close();
	}
}

void IOCompletionPortImpl::create()
{
	if(m_hIOCompletionPort==NULL)
	{
		m_hIOCompletionPort = ::epoll_create(MAX_FDS);
		if(m_hIOCompletionPort<=0)
			throw IOCPException("create iocp failed");
	}
}

void IOCompletionPortImpl::close()
{
	if(!::close(m_hIOCompletionPort))
		throw IOCPException("close iocp failed");
	m_hIOCompletionPort=NULL;
}

void IOCompletionPortImpl::registerEvents( int events,socket_t socket )
{
	IOEvent ev;
	ev.data.fd=socket;
	ev.events=events;
	::epoll_ctl(m_hIOCompletionPort,EPOLL_CTL_ADD,socket,&ev);
}

void IOCompletionPortImpl::unregisterEvents( int events,socket_t socket )
{
	IOEvent ev;
	ev.data.fd=socket;
	ev.events=events;
	::epoll_ctl(m_hIOCompletionPort,EPOLL_CTL_DEL,socket,&ev);
}

void IOCompletionPortImpl::modifyEvents( int events,socket_t socket )
{
	IOEvent ev;
	ev.data.fd=socket;
	ev.events=events;
	::epoll_ctl(m_hIOCompletionPort,EPOLL_CTL_MOD,socket,&ev);
}

void IOCompletionPortImpl::send(const byte* buffer, unsigned int len, socket_t sock)
{
	ClientSocket socket(sock);
	int size=0;
	while(size<len)
		size+=socket.writeBytes((const char*)buffer+size,len);//待改进:如果发送失败,那么注册一个可写事件,等待事件触发时再写
	socket.detach();
}

int IOCompletionPortImpl::waitEvent( IOEvent* events,int maxEvents,int timeout )
{
	return ::epoll_wait(m_hIOCompletionPort,events,maxEvents,timeout);
}

void ctrlSignal(int signalId)
{
	;
}

//中断等待
bool IOCompletionPortImpl::cancelWait()
{
	/*两种实现方法: 
	1.开始时监听一个专门的pipe,需打断epoll_wait时往pipe写入,epoll_wait返回后判断是pipe则忽略;
	2.利用信号中断掉epoll_wait,返回-1.(不安全!)
	*/
	static bool isRegister=false;
	if(!isRegister )
	{
		//关联信号处理函数
		if(::signal(SIGUSR1, ctrlSignal)!=SIG_ERR)
			isRegister=true;
	}

	if(isRegister && ::raise(SIGUSR1)==0)
		return true;

	return false;
}

void IOCompletionPortImpl::releaseEventBuffer(IOEvent* event)
{
	;
}

}//end of namespace bluemei