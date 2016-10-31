#include "ServerSocket.h"
#include "SocketException.h"
#include "ErrorHandler.h"

namespace blib{

ServerSocket::ServerSocket()
{
	m_bClose=true;
	m_socket=INVALID_SOCKET;

	m_sPort=0;
}

ServerSocket::ServerSocket(int nPort)
{
	m_bClose=true;
	m_socket=INVALID_SOCKET;

	listen(nPort);
}

ServerSocket::~ServerSocket()
{
	try{
		if(!m_bClose)
			close();
	}catch(Exception& e)
	{
		ErrorHandler::handle(e);
	}
}

//创建套接字
int ServerSocket::createSocket(int nPort)
{
	this->m_sPort=nPort;
	m_socket=socket(AF_INET,SOCK_STREAM,0);
	if(m_socket==INVALID_SOCKET)
	{
		return WSAGetLastError();
	}
	//填充地址
	sockaddr_in serverAddr;
	const int lenOfServerAddr=sizeof(serverAddr);
	memset(&serverAddr,0,lenOfServerAddr);
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(m_sPort);
	serverAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

	//绑定
	int nRetCode=::bind(m_socket,(sockaddr*)&serverAddr,lenOfServerAddr);
	if(nRetCode==SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	//监听
	int maxWaitCon = SOMAXCONN-1;
	nRetCode=::listen(m_socket,maxWaitCon);
	if(nRetCode==SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	m_bClose=false;
	return 0;
}

//接收连接
ClientSocket* ServerSocket::accept()
{
	//监听套接字设置为异步模式
	//ioctlsocket(m_serverSocket,FIONBIO,&arg);
	//关闭监控线程句柄
	//ResumeThread(hComm);
	//CloseHandle(hComm);

	//接受连接请求
	sockaddr_in clientAddr;
	int lenOfClientAddr=sizeof(clientAddr);
	SOCKET sockC=::accept(m_socket,(sockaddr*)&clientAddr,&lenOfClientAddr);
	if(sockC==INVALID_SOCKET)
	{
		throw SocketException(WSAGetLastError());
	}
	else
	{
		ClientSocket* pClientSocket=new ClientSocket(sockC);
		pClientSocket->setPeerAddr(clientAddr);
		return pClientSocket;
	}
}

//单独绑定套接字端口
void ServerSocket::listen(int nPort)
{
	int nReturn=createSocket(nPort);
	if(nReturn!=0)
		throw SocketException(nReturn,"failed to create socket");
}

//关闭
void ServerSocket::close()
{
	int nReturnCode=closesocket(this->m_socket);
	if(nReturnCode==SOCKET_ERROR)
	{
		throw SocketException(::WSAGetLastError());
	}
	m_bClose=true;
}

ServerSocket::operator socket_t() const{
	return m_socket;
}

void ServerSocket::destroy(ClientSocket*& pClientSocket)
{
	delete pClientSocket;
	pClientSocket=NULL;
}

}//end of namespace blib