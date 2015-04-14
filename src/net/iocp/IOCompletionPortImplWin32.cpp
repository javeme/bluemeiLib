#include "stdafx.h"
#include "IOCPException.h"
#include "RuntimeException.h"
#include "IOCompletionPortImplWin32.h"
#include <mswsock.h> //΢����չ�����

namespace bluemei{

IOCompletionPortImpl::IOCompletionPortImpl()
{
	m_hIOCompletionPort=NULL;
	m_pPoolIOCPData=MemoryPoolManager::getMemoryPool<IOCPData>();
}

IOCompletionPortImpl::~IOCompletionPortImpl()
{
	if (m_hIOCompletionPort!=NULL)
	{
		close();
	}
	MemoryPoolManager::releaseMemoryPool(m_pPoolIOCPData);
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

void IOCompletionPortImpl::registerEvents( int events,socket_t socket )
{
	//�״�ע��socket�¼�
	if(events & EVENT_FIRST_SOCKET)
	{
		//SocketHandle* socketHandle=new SocketHandle(socket);
		HANDLE h=::CreateIoCompletionPort((HANDLE)socket,m_hIOCompletionPort,(ULONG_PTR)socket,0);
		if(h!=m_hIOCompletionPort)
		{
			throw IOCPException("register iocp events failed");
		}
	}

	//Ͷ��һ��������������
	if(events & EVENT_ACCEPT)
	{
		socket_t client=::WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,WSA_FLAG_OVERLAPPED);

		IOCPData* pPerIOData = (IOCPData*)m_pPoolIOCPData->get();//new IOCPData();
		pPerIOData->operationType=EVENT_ACCEPT;
		pPerIOData->para=client;

		//AcceptEx
		//��ȡAcceptEx������ַ(ֱ�ӵ���AcceptEx�Ļ�,ÿ��Service Provider����Ҫͨ��WSAIoctl()��ȡһ�θú���ָ��)
		const static int ADDR_SIZE=(sizeof(SOCKADDR_IN)+16);
		static LPFN_ACCEPTEX FuncAcceptEx=nullptr; //AcceptEx����ָ��  
		if(FuncAcceptEx==nullptr)
		{
			GUID GuidAcceptEx = WSAID_ACCEPTEX;//GUID,�����ʶ��AcceptEx���������  
			DWORD dwBytes = 0;    

			if(::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx,sizeof(GuidAcceptEx),   
				&FuncAcceptEx,sizeof(FuncAcceptEx),&dwBytes,NULL,NULL)==INVALID_SOCKET) 
			{
				m_pPoolIOCPData->release(pPerIOData);
				throw IOCPException(::WSAGetLastError());
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
			int errorCode=::WSAGetLastError();
			if(errorCode!=ERROR_IO_PENDING)
			{
				m_pPoolIOCPData->release(pPerIOData);
				throw IOCPException(errorCode);
			}
		}
	}	
	//Ͷ��һ����������
	else if(events & EVENT_READ)
	{
		IOCPData* pPerIOData = (IOCPData*)m_pPoolIOCPData->get();//new IOCPData();
		pPerIOData->operationType=EVENT_READ_FINISH;
		WSABUF buf;
		buf.buf = pPerIOData->buf;
		buf.len = pPerIOData->IOCP_BUFFER_SIZE;  
		//WSARecv�����������
		int rt=::WSARecv(socket, &buf, 1, &pPerIOData->lengthReceived, &pPerIOData->flags, &pPerIOData->ol, NULL);
		if(rt==SOCKET_ERROR)
		{
			int errorCode=::WSAGetLastError();
			if(errorCode!=ERROR_IO_PENDING){
				m_pPoolIOCPData->release(pPerIOData);
				throw IOCPException(errorCode);
			}
		}
	}
	//Ͷ��һ����������
	if(events & EVENT_WRITE)
	{
		send(nullptr,0,socket);
	}
	//Ͷ��һ����������
	if(events & EVENT_ERR)
	{
		;
	}
}

void IOCompletionPortImpl::send(const byte* buffer, unsigned int len, socket_t sock)
{
	IOCPData* pPerIOData = (IOCPData*)m_pPoolIOCPData->get();//new IOCPData();
	pPerIOData->operationType=EVENT_WRITE_FINISH;
	if(len <= pPerIOData->IOCP_BUFFER_SIZE)
	{
		memcpy(pPerIOData->buf,buffer,len);
	}
	WSABUF buf;
	buf.buf = (char*)buffer;
	buf.len = len;
	int rt=::WSASend(sock, &buf, 1, &pPerIOData->lengthSended, pPerIOData->flags, &pPerIOData->ol, NULL);
	if(rt==SOCKET_ERROR)
	{
		int errorCode=::WSAGetLastError();
		if(errorCode!=ERROR_IO_PENDING)
		{
			m_pPoolIOCPData->release(pPerIOData);
			throw IOCPException(errorCode);
		}
	}
}

void IOCompletionPortImpl::unregisterEvents( int events,socket_t socket )
{
	//throw Exception("unsupported method:IOCompletionPortImpl::unregisterEvents");
	;
}

void IOCompletionPortImpl::modifyEvents( int events,socket_t socket )
{
	events&=(~EVENT_FIRST_SOCKET);
	return registerEvents(events,socket);
}

int IOCompletionPortImpl::waitEvent( IOEvent* events,int maxEvents,int timeout )
{
	int size=0;

	if(maxEvents<=0)
		return 0;
	DWORD transLen=0;
	//SocketHandle* pPerHandle=nullptr;
	socket_t socket=0;
	IOCPData* pPerData=nullptr;
	BOOL bOK = ::GetQueuedCompletionStatus(m_hIOCompletionPort,
		&transLen, (LPDWORD)&socket, (LPOVERLAPPED*)&pPerData, timeout);//pPerDataҲ��ͨ��CONTAINING_RECORDת��

	//�ڴ��׽������д�����
	if(!bOK || pPerData==nullptr)//ʧ��                                                 
	{
		int errorCode=GetLastError();//WSAGetLastError		
		if (errorCode==WAIT_TIMEOUT){//��ʱ WSAETIMEDOUT
			throw TimeoutException(timeout);
		}
		else{
			//�ͷ�
			//::closesocket(socket);
			m_pPoolIOCPData->release(pPerData);//delete pPerData;

			if(errorCode==ERROR_NETNAME_DELETED)//�Է�ǿ�ƹر�
			{
				throw IOCPForceCloseException(socket,errorCode);
			}
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
		ev.data.ptr=pPerData->buf;
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
			//��������
			//registerEvents(EVENT_ACCEPT,listenSock);

			//ʹ��GetAcceptExSockaddrs����,��þ���ĸ�����ַ����.��listenSock������ϵ,��֮�������getpeername��ȡ��ַ��Ϣ
			if(::setsockopt(clientSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,(char*)&listenSock, sizeof(listenSock))==SOCKET_ERROR)
			{
				//��δ���???
				int errorCode=::WSAGetLastError();
				if(errorCode!=0){
					::closesocket(clientSock);
					m_pPoolIOCPData->release(pPerData);
					throw IOCPException(errorCode);
				}
			}
			else
			{
				//ev.data.u64=addr;
			}
		}
		//�Է��ر�����(�����ر�)
		else if(transLen == 0                           
			&& (pPerData->operationType == EVENT_READ_FINISH || pPerData->operationType == EVENT_WRITE_FINISH))
		{
			//::closesocket(socket);

			ev.events=EVENT_CLOSED;
		}
		
		size=1;
	}

	//�ͷ�
	m_pPoolIOCPData->release(pPerData);

	return size;
}

bool IOCompletionPortImpl::cancelWait()
{
	return ::PostQueuedCompletionStatus(m_hIOCompletionPort,0,NULL,NULL)==TRUE;//�����ȴ�
}


}//end of namespace bluemei