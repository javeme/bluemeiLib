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
//创建套接字
void ClientSocket::createSocket()
{
	m_hSocket=socket(AF_INET,SOCK_STREAM,0);
	int nReturn=m_hSocket;
	if(nReturn==SOCKET_ERROR)
	{
		throw SocketException(WSAGetLastError(),"Failed to init socket");
	}
}
//连接服务器
void ClientSocket::connect(cstring ip,unsigned short port)
{
	//填充地址
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
//设置对方地址
void ClientSocket::setPeerAddr(sockaddr_in& addr)
{
	m_peerAddr=addr;
	m_bClose=false;
}
//获取对方地址
String ClientSocket::getPeerHost()const
{
	char *pChar=::inet_ntoa(m_peerAddr.sin_addr);
	//string *pStr=new string(pChar);
	//return *pStr;
	return String(pChar);
}
//获得对方端口
int ClientSocket::getPeerPort()const
{
	return ntohs(m_peerAddr.sin_port);
}

//设置超时
void ClientSocket::setTimeout(int ms)//毫秒
{
	m_nTimeout=ms;
#ifdef WIN32
	int timeout=ms;
#else
	struct timeval timeout = {ms/1000,ms%1000};
#endif
	//设置发送超时
	int ret=setsockopt(m_hSocket,SOL_SOCKET,SO_SNDTIMEO,(char*)&timeout,sizeof(timeout));
	if(ret==SOCKET_ERROR)
		throw SocketException(WSAGetLastError(),toString());
	//设置接收超时
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


//协议栈中达到可读字节数
unsigned long ClientSocket::availableBytes()
{
	unsigned long length=0;
	int ret=ioctl(m_hSocket,FIONREAD,&length);
	if(ret==SOCKET_ERROR)
		throw SocketException(WSAGetLastError(),toString());
	return length;
}
//跳过字节
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
//读字节,返回读取字节数
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
//读取指定长度字节
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
//读整型
int ClientSocket::readInt()
{
	const int sizeOfInt=sizeof(int);
	char buf[sizeOfInt];

	this->readEnoughBytes(buf,sizeOfInt);//读取指定长的字节才返回

	int value=SocketTools::bytesToInt((byte*)buf);
	value=::ntohl(value);
	return value;
}
//读整型(2字节)
int ClientSocket::readShort()
{
	const int sizeOfShort=sizeof(short);
	char buf[sizeOfShort];

	this->readEnoughBytes(buf,sizeOfShort);//读取指定长的字节才返回

	int value=SocketTools::bytesToInt((byte*)buf);

	value=::ntohs(value);
	return value;
}

//读字节(1字节)
unsigned char ClientSocket::readByte()
{
	const int sizeOfByte=sizeof(unsigned char);
	unsigned char buf[sizeOfByte];

	readEnoughBytes((char*)buf,sizeOfByte);
	return buf[0];
}

//读取一行数据,网络数据为gbk编码形式,返回gbk编码
String ClientSocket::readLine()
{
	int count=0;
	const unsigned int bufLen = LINE_BUFFER_SIZE;
	char buffer[bufLen];
	char receivedChar;
	do{
		count+=readBytes(&buffer[count],1);
		receivedChar=buffer[count-1];//减去1因为count已经加了1
		//printf("==%c\n",receivedChar);
	}while(receivedChar!='\n' && count<bufLen);
	buffer[count-1]='\0';
	if(count>1 && buffer[count-2]=='\r')
		buffer[count-2]='\0';
	return buffer;
}
//读字符串,网络数据为utf-8编码形式,返回gbk编码 --待实现
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

//写字节,返读取回字节数
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
//写指定长度字节
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
//写整型
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
//写整型(2字节)
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

//写字符串,源数据为gbk编码,网络数据以utf-8编码形式传送
int ClientSocket::writeUtfString(const String& str)
{
	//new char[length];
	String strUtf8 = SocketTools::gbkToUtf8(str);
	int length=	strUtf8.length();
	this->writeShort(length);
	int size=this->writeEnoughBytes(str.c_str(),length);
	return size;
}
////写字符串,网络数据以默认编码传送
int ClientSocket::writeString(const String& str)
{
	int length=str.length();
	int size=this->writeEnoughBytes(str.c_str(),length);
	return size;
}
//关闭
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
//返回描述
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