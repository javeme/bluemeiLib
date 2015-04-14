#pragma once
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include "ClientSocket.h"
//#include "IOCompletionPortImpl.h"


namespace bluemei{

#define MAX_FDS 256

enum IOCPEventType
{
	EVENT_IN  = EPOLLIN,			//��ʾ��Ӧ���ļ����������Զ��� 
	EVENT_OUT = EPOLLOUT,			//��ʾ��Ӧ���ļ�����������д�� 
	EVENT_PRI = EPOLLPRI,			//��ʾ��Ӧ���ļ��������н��������ݿɶ� 
	EVENT_ERR = EPOLLERR,			//��ʾ��Ӧ���ļ��������������� 
	EVENT_HUP = EPOLLHUP,			//��ʾ��Ӧ���ļ����������Ҷϣ� 

	EVENT_LT  = EPOLLLT,			//��ʾ��Ӧ���ļ����������¼������� 
	EVENT_ET  = EPOLLET,			//��ʾ��Ӧ���ļ����������¼������� 

	EVENT_READ		= EVENT_IN,
	EVENT_WRITE		= EVENT_OUT,

	EVENT_READ_FINISH		= 0x0100,
	EVENT_WRITE_FINISH		= 0x0200,
	EVENT_ACCEPT			= 0x0400,
	EVENT_CLOSED			= 0x0800,


	EVENT_FIRST_SOCKET		= 0x80000000,
};

typedef int socket_t;

#ifdef WIN32
	typedef unsigned int __uint32_t;
	typedef unsigned long long __uint64_t;
	typedef union IOEventData {   
		void *ptr;   
		int fd;   
		__uint32_t u32;   
		__uint64_t u64;   
	}epoll_data;   

	typedef struct IOEvent {   
		__uint32_t events; /* Epoll events */   
		IOEventData data; /* User data variable */   
	}epoll_event;  

#else
 typedef epoll_event IOEvent;

#endif

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
	int m_hIOCompletionPort;
};

}//end of namespace bluemei