#pragma once
#include "IOCPException.h"
#include "RuntimeException.h"
#include "IOCompletionPortImplWin32.h"
#include <mswsock.h> //΢����չ�����

namespace blib{

IOCompletionPortImpl::IOCompletionPortImpl()
{
	m_hIOCompletionPort=NULL;
	m_pPoolIOCPData=MemoryPoolManager::instance().getMemoryPool<IOCPData>();
	checkNullPtr(m_pPoolIOCPData);
}

IOCompletionPortImpl::~IOCompletionPortImpl()
{
	if (m_hIOCompletionPort!=NULL)
		close();

	MemoryPoolManager::instance().releaseMemoryPool(m_pPoolIOCPData);
}

void IOCompletionPortImpl::create()
{
	if(m_hIOCompletionPort==NULL)
	{
		m_hIOCompletionPort = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		if(m_hIOCompletionPort==NULL)
			throw IOCPException("create iocp failed");
	}
}

void IOCompletionPortImpl::close()
{
	if(!::CloseHandle(m_hIOCompletionPort))
		throw IOCPException("close iocp failed");
	m_hIOCompletionPort=NULL;
}

void IOCompletionPortImpl::registerEvents(int events, socket_t socket)
{
	//�״�ע��socket�¼�
	if(events & EVENT_FIRST_SOCKET)
	{
		//SocketHandle* socketHandle=new SocketHandle(socket);
		HANDLE h=::CreateIoCompletionPort((HANDLE)socket,m_hIOCompletionPort,
			(ULONG_PTR)socket,0);
		if(h!=m_hIOCompletionPort)
		{
			throw IOCPException("Failed to register event IOCP-INIT");
		}
	}

	/*//Ͷ��һ����������
	if(events & EVENT_ERR)
	{
		throw IOCPException("unsupported event type: EVENT_ERR");
	}*/

	//Ͷ��һ��������������
	if(events & EVENT_ACCEPT)
	{
		this->accept(socket);
	}
	else
	{
		//Ͷ��һ����������
		if(events & EVENT_READ || events & EVENT_READ_FINISH)
		{
			this->receive(socket);
		}
		//Ͷ��һ����������
		if(events & EVENT_WRITE || events & EVENT_WRITE_FINISH)
		{
			this->send(nullptr,0,socket);
		}
	}
}

void IOCompletionPortImpl::accept(socket_t socket)
{
	socket_t client=::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,
								NULL,0,WSA_FLAG_OVERLAPPED);
	//new IOCPData();
	IOCPData* pPerIOData = (IOCPData*)m_pPoolIOCPData->get();
	pPerIOData->operationType=EVENT_ACCEPT;
	pPerIOData->para=client;

	//��ȡAcceptEx������ַ
	//(ֱ�ӵ���AcceptEx�Ļ�,ÿ��Service Provider����Ҫͨ��WSAIoctl()��ȡһ�θú���ָ��)
	const static int ADDR_SIZE=(sizeof(SOCKADDR_IN)+16);
	static LPFN_ACCEPTEX FuncAcceptEx=nullptr; //AcceptEx����ָ��
	if(FuncAcceptEx==nullptr)
	{
		GUID GuidAcceptEx = WSAID_ACCEPTEX;//GUID,�����ʶ��AcceptEx���������
		DWORD dwBytes = 0;

		if(::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx,sizeof(GuidAcceptEx),
			&FuncAcceptEx,sizeof(FuncAcceptEx),
			&dwBytes,NULL,NULL)==INVALID_SOCKET)
		{
			m_pPoolIOCPData->release(pPerIOData);
			throw IOCPException(socketError());
		}
	}

	//����AcceptEx
	int recvBufLen=pPerIOData->IOCP_BUFFER_SIZE-ADDR_SIZE*2;
	BOOL succes = FuncAcceptEx(socket,client,pPerIOData->buf,
		recvBufLen,//��������յ�һ������,������Ϊ0,����Ҳ�ɷ�ֹ�ܾ����񹥻�
		ADDR_SIZE,//��ű���ַ��ַ��Ϣ�Ŀռ��С
		ADDR_SIZE,//��ű�Զ�˵�ַ��Ϣ�Ŀռ��С
		&pPerIOData->lengthReceived,//���յ������ݴ�С
		&pPerIOData->ol);

	if(!succes)
	{
		int errorCode=socketError();
		if(errorCode!=ERROR_IO_PENDING)
		{
			m_pPoolIOCPData->release(pPerIOData);
			throw IOCPException("Failed to accept socket", errorCode);
		}
	}
}

void IOCompletionPortImpl::receive(socket_t socket)
{
	IOCPData* pPerIOData = (IOCPData*)m_pPoolIOCPData->get();
	pPerIOData->operationType=EVENT_READ_FINISH;
	WSABUF buf;
	buf.buf = pPerIOData->buf;
	buf.len = pPerIOData->IOCP_BUFFER_SIZE;
	//WSARecv�����������
	int rt=::WSARecv(socket, &buf, 1, &pPerIOData->lengthReceived, &pPerIOData->flags, &pPerIOData->ol, NULL);
	if(rt==SOCKET_ERROR)
	{
		int errorCode=socketError();
		if(errorCode!=ERROR_IO_PENDING){
			m_pPoolIOCPData->release(pPerIOData);
			throw IOCPException("Failed to receive data", errorCode);
		}
	}

	//printf("Receiving %d bytes\n", pPerIOData->lengthReceived);
}

void IOCompletionPortImpl::send(const byte* buffer, unsigned int len, socket_t sock)
{
	IOCPData* pPerIOData = (IOCPData*)m_pPoolIOCPData->get();
	pPerIOData->operationType=EVENT_WRITE_FINISH;
	//if(len <= pPerIOData->IOCP_BUFFER_SIZE)
	//	memcpy(pPerIOData->buf,buffer,len);

	WSABUF buf;
	buf.buf = (char*)buffer;
	buf.len = len;

	int rt=::WSASend(sock, &buf, 1, &pPerIOData->lengthSended, pPerIOData->flags, &pPerIOData->ol, NULL);
	if(rt==SOCKET_ERROR)
	{
		int errorCode=socketError();
		if(errorCode!=ERROR_IO_PENDING)
		{
			m_pPoolIOCPData->release(pPerIOData);
			throw IOCPException("Failed to send data", errorCode);
		}
	}

	//printf("Sending %d bytes\n", pPerIOData->lengthSended);
}

void IOCompletionPortImpl::unregisterEvents(int events,socket_t socket)
{
	throw Exception("unsupported method: IOCompletionPortImpl::unregisterEvents()");
}

void IOCompletionPortImpl::modifyEvents( int events,socket_t socket )
{
	events&=(~EVENT_FIRST_SOCKET);
	return registerEvents(events,socket);
}

int IOCompletionPortImpl::waitEvent(IOEvent* events,int maxEvents,int timeout)
{
	int size=0;

	if(maxEvents<=0)
		return 0;
	DWORD transLen=0;
	//SocketHandle* pPerHandle=nullptr;
	socket_t socket=0;
	IOCPData* pPerData=nullptr;
	//��ȡ�������,pPerDataҲ��ͨ��CONTAINING_RECORDת��
	BOOL bOK = ::GetQueuedCompletionStatus(m_hIOCompletionPort,
		&transLen, (LPDWORD)&socket, (LPOVERLAPPED*)&pPerData, timeout);

	//�ڴ��׽������д�����
	if(!bOK || pPerData==nullptr)//ʧ��
	{
		int errorCode=GetLastError();//socketError
		if (errorCode==WAIT_TIMEOUT){//��ʱ ETIMEDOUT
			//�ͷ�pPerData?
			throw TimeoutException(timeout);
		}
		else{
			//�ͷ�
			m_pPoolIOCPData->release(pPerData);//delete pPerData;

			if(errorCode==ERROR_NETNAME_DELETED)//�Է�ǿ�ƹر�
				throw IOCPForceCloseException(socket,errorCode);
			else
				throw IOCPException(errorCode);
		}
	}
	else if(transLen==0 && pPerData==nullptr)//����(ȡ��)�ȴ�cancelWait
	{
		size=0;
	}
	else//�ɹ�
	{
		IOEvent& ev=events[0];
		ev.events=pPerData->operationType;
		ev.data.fd=socket;
		ev.data.ptr=pPerData;
		ev.data.u32=transLen;

		//���ӳɹ�
		if(ev.events==EVENT_ACCEPT)
		{
			socket_t clientSock=pPerData->para;
			socket_t listenSock=socket;
			ev.data.u64=clientSock;
			if(transLen>0)//�Ѿ���ȡ������
			{
				ev.events|=EVENT_READ_FINISH;
			}

			//ʹ��GetAcceptExSockaddrs����,��þ���ĸ�����ַ����.
			//��listenSock������ϵ,��֮�������getpeername��ȡ��ַ��Ϣ
			if(::setsockopt(clientSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
				(char*)&listenSock, sizeof(listenSock))==SOCKET_ERROR)
			{
				//��δ���???
				int errorCode=socketError();
				if(errorCode!=0){
					::closesocket(clientSock);
					m_pPoolIOCPData->release(pPerData);
					throw IOCPException(errorCode);
				}
			}
		}
		//�Է��ر�����(�����ر�)
		else if(transLen == 0 &&
			(pPerData->operationType == EVENT_READ_FINISH ||
			 pPerData->operationType == EVENT_WRITE_FINISH))
		{
			ev.events=EVENT_CLOSED;
		}

		size=1;
	}

	//��Ҫ�ϲ㴦����Ϻ����ͷ�!
	//m_pPoolIOCPData->release(pPerData);

	return size;
}

bool IOCompletionPortImpl::cancelWait()
{
	//�����ȴ�
	return ::PostQueuedCompletionStatus(m_hIOCompletionPort,0,NULL,NULL)==TRUE;
}

void IOCompletionPortImpl::releaseEventBuffer(IOEvent* event)
{
	IOCPData* pPerData=(IOCPData*)event->data.ptr;
	m_pPoolIOCPData->release(pPerData);
}


}//end of namespace blib