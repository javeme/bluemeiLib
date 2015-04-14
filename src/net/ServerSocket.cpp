#include "ServerSocket.h"
#include "SocketException.h"
#include "ErrorHandler.h"

namespace bluemei{

ServerSocket::ServerSocket()
{
	m_bClose=true;
	sockS=INVALID_SOCKET;

	m_sPort=0;
}

ServerSocket::ServerSocket(int nPort)
{
	m_bClose=true;
	sockS=INVALID_SOCKET;

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
	sockS=socket(AF_INET,SOCK_STREAM,0);
	if(sockS==INVALID_SOCKET)
	{
		return WSAGetLastError();
	}
	//����ַ
	sockaddr_in serverAddr;
	int lenOfServerAddr=sizeof(serverAddr);
	memset(&serverAddr,0,lenOfServerAddr);
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(m_sPort);
	serverAddr.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

	//��
	int nRetCode=::bind(sockS,(sockaddr*)&serverAddr,lenOfServerAddr);
	if(nRetCode==SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	//����
	nRetCode=::listen(sockS,SOMAXCONN);
	if(nRetCode==SOCKET_ERROR)
	{
		return WSAGetLastError();
	}
	m_bClose=false;
	return 0;
}

//��������
ClientSocket* ServerSocket::accept()
{
	//�����׽�������Ϊ�첽ģʽ
	//ioctlsocket(sockS,FIONBIO,&arg);
	//�رռ���߳̾��
	//ResumeThread(hComm);
	//CloseHandle(hComm);

	//������������
	sockaddr_in clientAddr;
	int lenOfClientAddr=sizeof(clientAddr);
	SOCKET sockC=::accept(sockS,(sockaddr*)&clientAddr,&lenOfClientAddr);
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

//�������׽��ֶ˿�
void ServerSocket::listen(int nPort)
{
	int nReturn=createSocket(nPort);
	if(nReturn!=0)
		throw SocketException(nReturn,"create socket failed");
}

//�ر�
void ServerSocket::close()
{
	int nReturnCode=closesocket(this->sockS);
	if(nReturnCode==SOCKET_ERROR)
	{
		throw SocketException(::WSAGetLastError());
	}
	m_bClose=true;
}

ServerSocket::operator socket_t()const{
	return sockS;
}

void ServerSocket::destroy(ClientSocket*& pClientSocket)
{
	delete pClientSocket;
	pClientSocket=NULL;
}

}//end of namespace bluemei