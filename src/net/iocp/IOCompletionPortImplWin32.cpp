#pragma once
#include "IOCPException.h"
#include "RuntimeException.h"
#include "IOCompletionPortImplWin32.h"
#include <mswsock.h> //微软扩展的类库

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
	//首次注册socket事件
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

	/*//投递一个错误请求
	if(events & EVENT_ERR)
	{
		throw IOCPException("unsupported event type: EVENT_ERR");
	}*/

	//投递一个接受连接请求
	if(events & EVENT_ACCEPT)
	{
		this->accept(socket);
	}
	else
	{
		//投递一个接收请求
		if(events & EVENT_READ || events & EVENT_READ_FINISH)
		{
			this->receive(socket);
		}
		//投递一个发送请求
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

	//获取AcceptEx函数地址
	//(直接调用AcceptEx的话,每次Service Provider都得要通过WSAIoctl()获取一次该函数指针)
	const static int ADDR_SIZE=(sizeof(SOCKADDR_IN)+16);
	static LPFN_ACCEPTEX FuncAcceptEx=nullptr; //AcceptEx函数指针
	if(FuncAcceptEx==nullptr)
	{
		GUID GuidAcceptEx = WSAID_ACCEPTEX;//GUID,这个是识别AcceptEx函数必须的
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

	//调用AcceptEx
	int recvBufLen=pPerIOData->IOCP_BUFFER_SIZE-ADDR_SIZE*2;
	BOOL succes = FuncAcceptEx(socket,client,pPerIOData->buf,
		recvBufLen,//若不想接收第一次数据,可以设为0,此外也可防止拒绝服务攻击
		ADDR_SIZE,//存放本地址地址信息的空间大小
		ADDR_SIZE,//存放本远端地址信息的空间大小
		&pPerIOData->lengthReceived,//接收到的数据大小
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
	//WSARecv待加入错误处理
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
	//获取操作结果,pPerData也可通过CONTAINING_RECORD转换
	BOOL bOK = ::GetQueuedCompletionStatus(m_hIOCompletionPort,
		&transLen, (LPDWORD)&socket, (LPOVERLAPPED*)&pPerData, timeout);

	//在此套接字上有错误发生
	if(!bOK || pPerData==nullptr)//失败
	{
		int errorCode=GetLastError();//socketError
		if (errorCode==WAIT_TIMEOUT){//超时 ETIMEDOUT
			//释放pPerData?
			throw TimeoutException(timeout);
		}
		else{
			//释放
			m_pPoolIOCPData->release(pPerData);//delete pPerData;

			if(errorCode==ERROR_NETNAME_DELETED)//对方强制关闭
				throw IOCPForceCloseException(socket,errorCode);
			else
				throw IOCPException(errorCode);
		}
	}
	else if(transLen==0 && pPerData==nullptr)//结束(取消)等待cancelWait
	{
		size=0;
	}
	else//成功
	{
		IOEvent& ev=events[0];
		ev.events=pPerData->operationType;
		ev.data.fd=socket;
		ev.data.ptr=pPerData;
		ev.data.u32=transLen;

		//连接成功
		if(ev.events==EVENT_ACCEPT)
		{
			socket_t clientSock=pPerData->para;
			socket_t listenSock=socket;
			ev.data.u64=clientSock;
			if(transLen>0)//已经读取到数据
			{
				ev.events|=EVENT_READ_FINISH;
			}

			//使用GetAcceptExSockaddrs函数,获得具体的各个地址参数.
			//与listenSock建立关系,且之后可以用getpeername获取地址信息
			if(::setsockopt(clientSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
				(char*)&listenSock, sizeof(listenSock))==SOCKET_ERROR)
			{
				//如何处理???
				int errorCode=socketError();
				if(errorCode!=0){
					::closesocket(clientSock);
					m_pPoolIOCPData->release(pPerData);
					throw IOCPException(errorCode);
				}
			}
		}
		//对方关闭连接(主动关闭)
		else if(transLen == 0 &&
			(pPerData->operationType == EVENT_READ_FINISH ||
			 pPerData->operationType == EVENT_WRITE_FINISH))
		{
			ev.events=EVENT_CLOSED;
		}

		size=1;
	}

	//需要上层处理完毕后再释放!
	//m_pPoolIOCPData->release(pPerData);

	return size;
}

bool IOCompletionPortImpl::cancelWait()
{
	//结束等待
	return ::PostQueuedCompletionStatus(m_hIOCompletionPort,0,NULL,NULL)==TRUE;
}

void IOCompletionPortImpl::releaseEventBuffer(IOEvent* event)
{
	IOCPData* pPerData=(IOCPData*)event->data.ptr;
	m_pPoolIOCPData->release(pPerData);
}


}//end of namespace blib