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
		size+=socket.writeBytes((const char*)buffer+size,len);//���Ľ�:�������ʧ��,��ôע��һ����д�¼�,�ȴ��¼�����ʱ��д
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

//�жϵȴ�
bool IOCompletionPortImpl::cancelWait()
{
	/*����ʵ�ַ���: 
	1.��ʼʱ����һ��ר�ŵ�pipe,����epoll_waitʱ��pipeд��,epoll_wait���غ��ж���pipe�����;
	2.�����ź��жϵ�epoll_wait,����-1.(����ȫ!)
	*/
	static bool isRegister=false;
	if(!isRegister )
	{
		//�����źŴ�����
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