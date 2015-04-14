#pragma once
#include "ClientSocket.h"
//#include "IOCompletionPortImpl.h"
#include "MemoryPool.cpp"


namespace bluemei{

enum IOCPEventType
{
	EVENT_IN  = 0x0001,			//��ʾ��Ӧ���ļ����������Զ��� 
	EVENT_OUT = 0x0002,			//��ʾ��Ӧ���ļ�����������д�� 
	EVENT_PRI = 0x0008,			//��ʾ��Ӧ���ļ��������н��������ݿɶ� 
	EVENT_ERR = 0x0010,			//��ʾ��Ӧ���ļ��������������� 
	EVENT_HUP = 0x0020,			//��ʾ��Ӧ���ļ����������Ҷϣ� 
	
	EVENT_LT  = 0x0040,			//��ʾ��Ӧ���ļ����������¼������� 
	EVENT_ET  = 0x0080,			//��ʾ��Ӧ���ļ����������¼������� 

	EVENT_READ		= EVENT_IN,
	EVENT_WRITE		= EVENT_OUT,

	EVENT_READ_FINISH		= 0x0100,
	EVENT_WRITE_FINISH		= 0x0200,
	EVENT_ACCEPT			= 0x0400,
	EVENT_CLOSED			= 0x0800,


	EVENT_FIRST_SOCKET		= 0x80000000,
};


typedef unsigned int __uint32_t;
typedef unsigned long long __uint64_t;
typedef struct IOEventData { //union  
	void *ptr;   
	int fd;   
	__uint32_t u32;   
	__uint64_t u64;   
}epoll_data;   

typedef struct IOEvent {   
	__uint32_t events; /* Epoll events */   
	IOEventData data; /* User data variable */   
}epoll_event;  

/*
struct SocketHandle
{
	socket_t socket;
	//sockaddr_in addr;
	SocketHandle(socket_t s):socket(s){}
};*/

struct IOCPData
{
	const static int IOCP_BUFFER_SIZE=1024;

	OVERLAPPED ol;				   // �ص��ṹ
	//WSABUF dataBuffer;
	char buf[IOCP_BUFFER_SIZE];    // ���ݻ�����
	int operationType;             // ��������
	int para;					   // ��������

	unsigned long lengthReceived;
	unsigned long lengthSended;
	unsigned long flags;
};

//class IOCompletionPortImplWin32 : public IOCompletionPortImpl
class BLUEMEILIB_API IOCompletionPortImpl : public Object
{
public:
	IOCompletionPortImpl();
	virtual ~IOCompletionPortImpl();

public:
	void registerEvents(int events,socket_t socket);
	void unregisterEvents(int events,socket_t socket);
	void modifyEvents(int events,socket_t socket);

	int waitEvent(IOEvent* events,int maxEvents,int timeout);
	bool cancelWait();

	void send(const byte* buffer, unsigned int len, socket_t sock);
public:
	void create();
	void close();
private:
	 HANDLE m_hIOCompletionPort;
	 
	 MemoryPool* m_pPoolIOCPData;
};

}//end of namespace bluemei
