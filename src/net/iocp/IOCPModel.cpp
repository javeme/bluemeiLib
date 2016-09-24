#pragma once
#include "IOCPModel.h"
#include "LambdaThread.h"
#include "RuntimeException.h"
#include "Task.h"
#include "IOCPException.h"
#include <memory>

namespace bluemei{


class IOTask : public Task
{
public:
	IOTask(IOEvent e,IOCPEventHandle* eventHandle,IOCompletionPort* ioCompPort)
	{
		m_bRunning=false;
		ioEvent=e;
		this->eventHandle=eventHandle;
		this->ioCompPort=ioCompPort;
	}
public:
	virtual void stop() 
	{
		m_bRunning=false;
	}

	virtual void setId( int id ) 
	{
		;
	}

	virtual void taskStarted() 
	{
		m_bRunning=true;
	}

	virtual int getId() 
	{
		return 0;
	}

	virtual void taskFinished() 
	{
		m_bRunning=false;
		delete this;
	}

	virtual bool isRunning() 
	{
		return m_bRunning;
	}

	virtual void run()
	{
		socket_t sock=ioEvent.data.fd;
		int events=ioEvent.events;

		bool continueEvent=true;
			
		//else if(events & (EVENT_READ | EVENT_READ_FINISH))//�����ݿɶ������Ѿ�����
		if(events & EVENT_READ)//�����ݿɶ�
		{
			continueEvent&=eventHandle->onRead(ioEvent);
		}
		else if(events & EVENT_READ_FINISH)//�Ѿ���������
		{
			//�������Ѿ������������ݿɶ�
			if(events & EVENT_ACCEPT)
			{
				continueEvent=false;
			}
			continueEvent&=eventHandle->onReadFinish(ioEvent);
		}
		//else if(events & (EVENT_WRITE | EVENT_WRITE_FINISH))
		else if(events & EVENT_WRITE)
		{
			continueEvent&=eventHandle->onWrite(ioEvent);
		}
		else if(events & EVENT_WRITE_FINISH)
		{
			continueEvent&=eventHandle->onWriteFinish(ioEvent);
		}
		else if(events & EVENT_ACCEPT)
		{
			continueEvent&=eventHandle->onAccepted(ioEvent);
			continueEvent=false;
		}
		else if(events & EVENT_ERR)
		{
			continueEvent&=eventHandle->onError(ioEvent);
		}
		else if(events & EVENT_CLOSED)
		{
			continueEvent&=eventHandle->onClosed(ioEvent);
			continueEvent=false;
		}
		else
		{
			continueEvent=false;
		}

		//�ͷŻ�����
		ioCompPort->releaseEventBuffer(&ioEvent);

		//ע����һ���¼�
		if(continueEvent)
		{
			try
			{
				ioCompPort->modifyEvents(events,sock);
			}catch (Exception& e)
			{
				eventHandle->onException(e);
			}
		}
	}
protected:
	bool m_bRunning;

	IOEvent ioEvent;
	IOCPEventHandle* eventHandle;
	IOCompletionPort* ioCompPort;
};


IOCPModel::IOCPModel()
{
	m_pIOCPEventHandler=nullptr;

	//m_listenSocket=0;
	//clientSocket=0;

	m_pIOThreadPool=nullptr;

	m_pEventThread=nullptr;
	m_bRunning=false;
	m_nTimeout=1000*10;//10s
}

IOCPModel::~IOCPModel()
{
	m_pIOCPEventHandler=nullptr;
	stop();
}

void IOCPModel::listen(int port)
{
	m_listenSocket.listen(port);
	m_oIOCompletionPort.registerEvents(EVENT_FIRST_SOCKET|EVENT_ACCEPT|EVENT_ALL,
		m_listenSocket);//EPOLLIN|EPOLLET

	start();
}

void IOCPModel::unlisten()
{
	if(!m_bRunning)
		return;
	m_oIOCompletionPort.unregisterEvents(EVENT_ACCEPT|EVENT_ALL,
		m_listenSocket);//EPOLLIN|EPOLLET
	m_listenSocket.close();

	closeAllClients();
	stop();
}

socket_t IOCPModel::connect(cstring ip,int port)
{
	ClientSocket sock;
	sock.connect(ip,port);
	socket_t s=sock.detach();
	m_clientSockets.put(s,s);

	start();
	m_oIOCompletionPort.registerEvents(EVENT_FIRST_SOCKET|EVENT_ALL,s);

	return s;
}

void IOCPModel::disconnect()
{
	if(!m_bRunning)
		return;
	closeAllClients();
	stop();
}

void IOCPModel::setEventHandle(IOCPEventHandle *e)
{
	m_pIOCPEventHandler=e;
}

void IOCPModel::send(const byte* buf,unsigned int len,socket_t sock)
{
	if(!m_bRunning)
		throw IOCPException("Can't send any data after IOCP stopped.");
	//m_oIOCompletionPort.registerEvents(EVENT_WRITE,sock);	
	m_oIOCompletionPort.send(buf,len,sock);
}

void IOCPModel::start()
{
	if(m_bRunning)
		return;
	m_bRunning=true;
	
	/**
	 * ���Ӵ�����
	 * ע��: ���־ֲ�����(����)����㷽������ʱ�ᱻ�ͷ�,Ϊ�˱���Ƿ����ʴ���:
	 *  1.����ֵ���ݵ�����������ȥ.
	 *  2.����Ϊstatic����,���������ô���.
	 */
	static auto handleAccept=[this](IOEvent& ev) {
		//��������
		m_oIOCompletionPort.registerEvents(EVENT_ACCEPT|EVENT_ALL,m_listenSocket);

		socket_t sock=0;

		//�ѽ����µ�����
		if(ev.events & EVENT_ACCEPT)
		{
			sock=(socket_t)ev.data.u64;//�µ�����
			ev.data.fd=sock;
		}
		//��û�н����µ�����
		else
		{
			std::auto_ptr<ClientSocket> client(m_listenSocket.accept());
			sock=client->detach();
			ev.data.fd=sock;
			ev.events |= EVENT_ACCEPT;
		}

		//notify event: EVENT_ACCEPT
		IOTask* task=new IOTask(ev,m_pIOCPEventHandler,&m_oIOCompletionPort);
		m_pIOThreadPool->addTask(task);

		//ע����¼�,��client��ӵ�����������
		addClient(sock);
		m_oIOCompletionPort.registerEvents(EVENT_FIRST_SOCKET|EVENT_ALL,sock);
	};

	//�¼�������
	static auto handleEvents=[&](IOEvent* events, int size) {
		int eventNum=m_oIOCompletionPort.waitEvent(events, size, m_nTimeout);

		for(int i=0;i<eventNum;i++)
		{
			IOEvent& ev=events[i];
			//���µ�������Ҫ(����)����
			if(ev.data.fd==m_listenSocket)
			{
				try {
					handleAccept(ev);
				} catch (Exception& e) {
					m_bRunning&=notifyException(e);
				}				
			}
			//�������ݶ�д
			else
			{
				IOTask* task=new IOTask(ev,m_pIOCPEventHandler,&m_oIOCompletionPort);
				m_pIOThreadPool->addTask(task);
				if(ev.events & EVENT_CLOSED)
				{
					socket_t sock=ev.data.fd;
					removeClient(sock);
					ClientSocket s(sock);
					s.close();
				}
			}
		}
	};

	//����IO��д����
	m_pIOThreadPool=new ThreadPool(4);

	//������ѯ�¼�
	m_pEventThread=new LambdaThread([&](){
		m_bRunning=true;
		const int MAX_EV_NUM=32;
		IOEvent events[MAX_EV_NUM];
		while(m_bRunning)
		{
			try{
				handleEvents(events, MAX_EV_NUM);
			}catch(TimeoutException& e){
				notifyException(e);
			}catch(IOCPForceCloseException& e){
				removeClient(e.getSocket());
				m_bRunning&=notifyException(e);
			}catch(Exception& e){
				//֪ͨ�쳣
				m_bRunning&=notifyException(e);
			}
		}
		//end of thread
	},nullptr);
	m_pEventThread->setAutoDestroy(false);
	m_pEventThread->start();
}

void IOCPModel::stop()
{
	if(!m_bRunning)
		return;

	// stop event-pull thread
	if(m_bRunning && m_pEventThread!=nullptr)
	{
		m_bRunning=false;
		m_oIOCompletionPort.cancelWait();
		m_pEventThread->wait();
		
		delete m_pEventThread;
		m_pEventThread=nullptr;
	}

	// stop task workers
	if(m_pEventThread==nullptr && m_pIOThreadPool!=nullptr)
	{
		m_pIOThreadPool->stop();

		delete m_pIOThreadPool;
		m_pIOThreadPool=nullptr;
	}
}

bool IOCPModel::notifyException(Exception& e)
{
	if(m_pIOCPEventHandler==nullptr)
		return false;
	return m_pIOCPEventHandler->onException(e);
}

int IOCPModel::getLastError()
{
#ifdef WIN32
	return GetLastError();
#else //#elif defined()
	return errno;
#endif
}

bool IOCPModel::addClient(socket_t sock)
{
	socket_t v=sock;
	return m_clientSockets.put(sock,v);
}

bool IOCPModel::removeClient(socket_t sock)
{
	m_oIOCompletionPort.unregisterEvents(EVENT_ACCEPT|EVENT_ALL,sock);
	socket_t v;
	return m_clientSockets.remove(sock,v);
}

void IOCPModel::closeAllClients()
{
	auto iter = m_clientSockets.iterator();
	while (iter->hasNext())
	{
		socket_t sock = iter->next().key;
		try {
			ClientSocket client(sock);
			client.close();
		} catch (Exception& e) {
			notifyException(e);
		}
	}
	m_clientSockets.releaseIterator(iter);
}


}//end of namespace bluemei