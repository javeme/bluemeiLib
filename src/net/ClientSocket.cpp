#include "ClientSocket.h"
#include "SocketException.h"
#include "SocketTools.h"
#include "RuntimeException.h"

namespace bluemei{

const int ClientSocket::LINE_BUFFER_SIZE = 4096;

ClientSocket::ClientSocket()
{
	m_bClose=true;
	createSocket();
}
ClientSocket::ClientSocket(socket_t hSocket)
{
	m_bClose=true;
	attach(hSocket);
}
ClientSocket::~ClientSocket()
{
	if(!m_bClose)
		close();
}
//�����׽���
void ClientSocket::createSocket()
{
	m_hSocket=socket(AF_INET,SOCK_STREAM,0);
	int nReturn=m_hSocket;
	if(nReturn==SOCKET_ERROR)
	{
		throw SocketException(WSAGetLastError(),"Failed to init socket");
	}
}
//���ӷ�����
void ClientSocket::connect(cstring ip,unsigned short port)
{
	//����ַ
	sockaddr_in serverAddr;
	int lenOfServerAddr=sizeof(serverAddr);
	memset(&serverAddr,0,lenOfServerAddr);
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port);
	serverAddr.sin_addr.S_un.S_addr=::inet_addr(ip);
	setPeerAddr(serverAddr);
	int nReturnCode=::connect(m_hSocket,(sockaddr*)&serverAddr,lenOfServerAddr);
	if(nReturnCode==SOCKET_ERROR)
	{
		int err=WSAGetLastError();
		if(err==0)
		{
			throw SocketException(err,"connect failed,"+toString());
		}
		else
			throw SocketException(err,toString());
	}
	m_bClose=false;
}
//���öԷ���ַ
void ClientSocket::setPeerAddr(sockaddr_in& addr)
{
	m_peerAddr=addr;
	m_bClose=false;
}
//��ȡ�Է���ַ
String ClientSocket::getPeerHost()const
{
	char *pChar=::inet_ntoa(m_peerAddr.sin_addr);
	//string *pStr=new string(pChar);
	//return *pStr;
	return String(pChar);
}
//��öԷ��˿�
int ClientSocket::getPeerPort()const
{
	return ntohs(m_peerAddr.sin_port);
}

//���ó�ʱ
void ClientSocket::setTimeout(int ms)//����
{
	m_nTimeout=ms;
#ifdef WIN32
	int timeout=ms;
#else
	struct timeval timeout = {ms/1000,ms%1000};
#endif
	//���÷��ͳ�ʱ
	int ret=setsockopt(m_hSocket,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));
	if(ret==SOCKET_ERROR)
		throw SocketException(WSAGetLastError(),toString());
	//���ý��ճ�ʱ
	ret=setsockopt(m_hSocket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));
	if(ret==SOCKET_ERROR)
		throw SocketException(WSAGetLastError(),toString());
}

void ClientSocket::setNoDelay(bool noDelay)
{
	 int flag = noDelay;
     int ret = setsockopt(m_hSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&flag,sizeof(flag));
	 if(ret==SOCKET_ERROR)
		 throw SocketException(WSAGetLastError(),toString());
}


//Э��ջ�дﵽ�ɶ��ֽ���
unsigned long ClientSocket::availableBytes()
{
	unsigned long length=0;
	int ret=ioctl(m_hSocket,FIONREAD,&length);
	if(ret==SOCKET_ERROR)
		throw SocketException(WSAGetLastError(),toString());
	return length;
}
//�����ֽ�
void ClientSocket::skip(unsigned long len)
{
	if(len==0)
		return;
	const static int BUF_LEN = 1024;
	int count=0,wantLen;
	char buffer[BUF_LEN];
	do{
		wantLen=len-count;
		if(wantLen>BUF_LEN)
			wantLen=BUF_LEN;
		count+=readBytes(buffer,wantLen);
	}while(count!=len);
}
//���ֽ�,���ض�ȡ�ֽ���
int ClientSocket::readBytes(char buffer[],int maxLength,int flags)
{
	if(maxLength<=0)
		return 0;

	int length=::recv(this->m_hSocket,buffer,maxLength,flags);
	//printf("==========ClientSocket::readBytes,len=%d\n", length);
	if(length==SOCKET_ERROR)
	{
		int errorCode=WSAGetLastError();
		if (errorCode==WSAETIMEDOUT)
		{
			throw TimeoutException(m_nTimeout);
		}
		else
			throw SocketException(errorCode,toString());
	}
	else if(length==0)
	{
		throw SocketClosedException("closed by peer,"+toString());
	}
	else if(length<0)
	{
		throw SocketException("unknow error,"+toString());
	}
	return length;
}
//��ȡָ�������ֽ�
int ClientSocket::readEnoughBytes(char buffer[],int length)
{
	int count=0;
	do{
		count+=readBytes(buffer+count,length-count);//EINTR
	}while(count!=length);
	//count=readBytes(buffer,length,MSG_WAITALL);// return error WSAEOPNOTSUPP on win7?
	if(count!=length)
		throw SocketException("have not read enough data,"+toString());
	return count;
}
//������
int ClientSocket::readInt()
{
	const int sizeOfInt=sizeof(int);
	char buf[sizeOfInt];

	this->readEnoughBytes(buf,sizeOfInt);//��ȡָ�������ֽڲŷ���

	int value=SocketTools::bytesToInt((byte*)buf);
	value=::ntohl(value);
	return value;
}
//������(2�ֽ�)
int ClientSocket::readShort()
{
	const int sizeOfShort=sizeof(short);
	char buf[sizeOfShort];

	this->readEnoughBytes(buf,sizeOfShort);//��ȡָ�������ֽڲŷ���

	int value=SocketTools::bytesToInt((byte*)buf);

	value=::ntohs(value);
	return value;
}

//���ֽ�(1�ֽ�)
unsigned char ClientSocket::readByte()
{
	const int sizeOfByte=sizeof(unsigned char);
	unsigned char buf[sizeOfByte];

	readEnoughBytes((char*)buf,sizeOfByte);
	return buf[0];
}

//��ȡһ������,��������Ϊgbk������ʽ,����gbk����
String ClientSocket::readLine()
{
	int count=0;
	const unsigned int bufLen = LINE_BUFFER_SIZE;
	char buffer[bufLen];
	char receivedChar;
	do{
		count+=readBytes(&buffer[count],1);
		receivedChar=buffer[count-1];//��ȥ1��Ϊcount�Ѿ�����1
		//printf("==%c\n",receivedChar);
	}while(receivedChar!='\n' && count<bufLen);
	buffer[count-1]='\0';
	if(count>1 && buffer[count-2]=='\r')
		buffer[count-2]='\0';
	return buffer;
}
//���ַ���,��������Ϊutf-8������ʽ,����gbk���� --��ʵ��
String ClientSocket::readUtfString()
{
	String str;
	int len=this->readShort();
	if(len>0)
	{
		char *buffer=new char[len];
		try{
			this->readEnoughBytes(buffer,len);
		}catch(...)
		{
			delete[] buffer;
			throw;
		}
		str=SocketTools::utf8ToGbk(buffer);
		delete[] buffer;
	}
	return str;
}

//д�ֽ�,����ȡ���ֽ���
int ClientSocket::writeBytes(const char buffer[],int length,int flags)
{
	int size=0;
	size=::send(this->m_hSocket,buffer,length,flags);
	if(size==SOCKET_ERROR)
	{
		int errorCode=::WSAGetLastError();
		if (errorCode==WSAETIMEDOUT)
		{
			throw TimeoutException(m_nTimeout);
		}
		else
			throw SocketException(errorCode,toString());
	}
#ifdef SEND_LOG
	FILE *pFile;
	if((pFile=fopen("sendData.log","a+")))
	{
		fwrite(buffer,1,length,pFile);
		fclose(pFile);
	}
#endif
	return size;
}
//дָ�������ֽ�
int ClientSocket::writeEnoughBytes(const char buffer[],int length)
{
	int count=0;
	do{
		count+=writeBytes(buffer+count,length-count);//EINTR
	}while(count!=length);
	//count=writeBytes(buffer,length,MSG_WAITALL);
	if(count!=length)
		throw SocketException("have not send enough data,"+toString());
	return count;
}
//д����
int ClientSocket::writeInt(int value)
{
	const int sizeOfInt=sizeof(value);
	char buf[sizeOfInt];
	value=::htonl(value);
	SocketTools::intToBytes((byte*)buf,value);

	int len=writeEnoughBytes(buf,sizeOfInt);

	//if(len!=lensizeOfInt)
	//	throw SocketException("length of the sended data not enough");
	return len;
}
//д����(2�ֽ�)
int ClientSocket::writeShort(short value)
{
	const int sizeOfInt=sizeof(value);
	char buf[sizeOfInt];
	value=::htons(value);
	SocketTools::shortToBytes((byte*)buf,value);

	int len=writeEnoughBytes(buf,sizeOfInt);

	return len;
}

int ClientSocket::writeByte(unsigned char value)
{
	return writeEnoughBytes((const char*)&value,sizeof(value));
}

//д�ַ���,Դ����Ϊgbk����,����������utf-8������ʽ����
int ClientSocket::writeUtfString(const String& str)
{
	//new char[length];
	String strUtf8 = SocketTools::gbkToUtf8(str);
	int length=	strUtf8.length();
	this->writeShort(length);
	int size=this->writeEnoughBytes(str.c_str(),length);
	return size;
}
////д�ַ���,����������Ĭ�ϱ��봫��
int ClientSocket::writeString(const String& str)
{
	int length=str.length();
	int size=this->writeEnoughBytes(str.c_str(),length);
	return size;
}
//�ر�
void ClientSocket::close()
{
	if(m_bClose)
		return;
	m_bClose=true;
	int nReturnCode=closesocket(this->m_hSocket);
	if(nReturnCode==SOCKET_ERROR)
	{
		throw SocketException(::WSAGetLastError(),toString());
	}
}
//��������
String ClientSocket::toString()const
{
	String str=String::format("Socket[peer address is %s:%d]",
		getPeerHost().c_str(),getPeerPort());
	return str;
}

void ClientSocket::attach(socket_t s)
{
	m_bClose=false;
	this->m_hSocket=s;
}

socket_t ClientSocket::detach()
{
	socket_t tmp=m_hSocket;
	m_hSocket=INVALID_SOCKET;
	m_bClose=true;
	return tmp;
}


}//end of namespace bluemei