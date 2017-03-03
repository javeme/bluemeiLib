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

//�����׽���
int ServerSocket::createSocket(int nPort)
{
	this->m_sPort=nPort;
	m_socket=socket(AF_INET,SOCK_STREAM,0);
	if(m_socket==INVALID_SOCKET)
	{
		return socketError();
	}
	//����ַ
	sockaddr_in serverAddr;
	const int lenOfServerAddr=sizeof(serverAddr);
	memset(&serverAddr,0,lenOfServerAddr);
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(m_sPort);
	serverAddr.sin_addr={htonl(INADDR_ANY)};

	//��
	int nRetCode=::bind(m_socket,(sockaddr*)&serverAddr,lenOfServerAddr);
	if(nRetCode==SOCKET_ERROR)
	{
		return socketError();
	}
	//����
	int maxWaitCon = SOMAXCONN-1;
	nRetCode=::listen(m_socket,maxWaitCon);
	if(nRetCode==SOCKET_ERROR)
	{
		return socketError();
	}
	m_bClose=false;
	return 0;
}

//��������
ClientSocket* ServerSocket::accept()
{
	//�����׽�������Ϊ�첽ģʽ
	//ioctlsocket(m_serverSocket,FIONBIO,&arg);
	//�رռ���߳̾��
	//ResumeThread(hComm);
	//CloseHandle(hComm);

	//������������
	sockaddr_in clientAddr;
	unsigned int lenOfClientAddr=sizeof(clientAddr);
	socket_t sockC=::accept(m_socket,(sockaddr*)&clientAddr,&lenOfClientAddr);
	if(sockC==INVALID_SOCKET)
	{
		throw SocketException(socketError());
	}
	else
	{
		ClientSocket* pClientSocket=new ClientSocket(sockC);
		pClientSocket->setPeerAddr(clientAddr);
		return pClientSocket;
	}
}

//�������׽��ֶ˿�
void ServerSocket::listen(int nPort)
{
	int nReturn=createSocket(nPort);
	if(nReturn!=0)
		throw SocketException(nReturn,"failed to create socket");
}

//�ر�
void ServerSocket::close()
{
	int nReturnCode=closesocket(this->m_socket);
	if(nReturnCode==SOCKET_ERROR)
	{
		throw SocketException(socketError());
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
