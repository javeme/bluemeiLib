#pragma once
#include "stdafx.h"
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
		;
	}

	virtual int getId() 
	{
		return 0;
	}

	virtual void taskFinished() 
	{
		delete this;
	}

	virtual bool isRunning() 
	{
		return m_bRunning;
	}

	virtual void run()
	{
		m_bRunning=true;

		socket_t sock=ioEvent.data.fd;
		int events=ioEvent.events;

		bool continueEvent=true;
			
		//else if(events & (EVENT_READ | EVENT_READ_FINISH))//有数据可读或者已经读到
		if(events & EVENT_READ)//有数据可读
		{
			continueEvent&=eventHandle->onRead(ioEvent);
		}
		else if(events & EVENT_READ_FINISH)//已经读到数据
		{
			//有连接已经建立好且数据可读
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

	//listenSocket=0;
	//clientSocket=0;

	m_pIOThreadPool=nullptr;

	m_pEventThread=nullptr;
	m_bRunning=false;
}

IOCPModel::~IOCPModel()
{
	m_pIOCPEventHandler=nullptr;
}

void IOCPModel::listen( int port )
{
	listenSocket.listen(port);
	m_oIOCompletionPort.registerEvents(EVENT_FIRST_SOCKET|EVENT_ACCEPT|EVENT_ALL,listenSocket);//EPOLLIN|EPOLLET

	start();
}

void IOCPModel::unlisten()
{
	m_oIOCompletionPort.unregisterEvents(EVENT_ACCEPT|EVENT_ALL,listenSocket);//EPOLLIN|EPOLLET
	listenSocket.close();

	stop();
}

void IOCPModel::connect( cstring ip,int port )
{
	ClientSocket sock;
	sock.connect(ip,port);
	socket_t s=sock.detach();
	clientSockets.put(s,s);

	m_oIOCompletionPort.registerEvents(EVENT_ALL,s);

	start();
}

void IOCPModel::disconnect()
{
	stop();
}

void IOCPModel::setEventHandle( IOCPEventHandle *e )
{
	m_pIOCPEventHandler=e;
}

void IOCPModel::send( const byte* buf,unsigned int len,socket_t sock )
{
	//m_oIOCompletionPort.registerEvents(EVENT_WRITE,sock);	
	m_oIOCompletionPort.send(buf,len,sock);
}

void IOCPModel::start()
{
	if(m_bRunning)
		return;

	//负责IO读写操作
	m_pIOThreadPool=new ThreadPool();
	
	//负责轮询事件
	m_pEventThread=new LambdaThread([&](){
		m_bRunning=true;
		const int MAX_EV_NUM=32;
		IOEvent events[MAX_EV_NUM];
		while(m_bRunning)
		{
			try{
				int eventNum=m_oIOCompletionPort.waitEvent(events,MAX_EV_NUM,1000*4);

				for(int i=0;i<eventNum;i++)
				{
					IOEvent& ev=events[i];
					if(ev.data.fd==listenSocket)//有新的连接需要建立或已建立
					{
						/*struct sockaddr_in clientAddr;
						int addrLen=sizeof(clientAddr);
						socket_t client = accept(listenSocket,(sockaddr *)&clientAddr, &addrLen); //accept这个连接
						if(client=SOCKET_ERROR)
						{
							notifyException(SocketException(getLastError()));
							continue;
						}*/

						try
						{
							//继续监听
							m_oIOCompletionPort.registerEvents(EVENT_ACCEPT|EVENT_ALL,listenSocket);

							socket_t sock=0;
							//已建立新的连接
							if(ev.events & EVENT_ACCEPT)
							{
								sock=(socket_t)ev.data.u64;//新的连接

								ev.data.fd=sock;								
								m_pIOThreadPool->addTask(new IOTask(ev,m_pIOCPEventHandler,&m_oIOCompletionPort));
							}
							//还没有建立新的连接
							else
							{
								bool connected=false;
								std::auto_ptr<ClientSocket> client;
								if(m_pIOCPEventHandler!=nullptr)
								{
									ClientSocket* s=nullptr;
									connected=m_pIOCPEventHandler->onAccept(s,ev);
									client=std::auto_ptr<ClientSocket>(s);
								}						

								if(!connected)
								{
									client=std::auto_ptr<ClientSocket>(listenSocket.accept());
								}
																
								sock=client->detach();
							}
							//注册读事件,将client添加到监听队列中
							addClient(sock);
							m_oIOCompletionPort.registerEvents(EVENT_FIRST_SOCKET|EVENT_ALL,sock);//EPOLLIN|EPOLLET	
						}
						catch (Exception& e)
						{
							m_bRunning&=notifyException(e);
						}				
					}					
					else
					{
						m_pIOThreadPool->addTask(new IOTask(ev,m_pIOCPEventHandler,&m_oIOCompletionPort));
						if(ev.events & EVENT_CLOSED)
						{
							socket_t sock=ev.data.fd;
							removeClient(sock);
							ClientSocket s(sock);
							s.close();
						}
					}
				}
			}catch(TimeoutException& e){
				notifyException(e);
			}catch(IOCPForceCloseException& e){
				removeClient(e.getSocket());
				m_bRunning&=notifyException(e);
			}catch(Exception& e){
				//通知异常
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
	if(m_bRunning && m_pEventThread!=nullptr)
	{
		m_bRunning=false;
		m_pEventThread->wait();
		
		delete m_pEventThread;
		m_pEventThread=nullptr;
	}

	if(m_pIOThreadPool!=nullptr)
	{
		m_pIOThreadPool->stop();

		delete m_pIOThreadPool;
		m_pIOThreadPool=nullptr;
	}
}

bool IOCPModel::notifyException( Exception& e )
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


}//end of namespace bluemei