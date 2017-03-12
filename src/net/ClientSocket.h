#ifndef _ClientSocket_H_H
#define _ClientSocket_H_H

#include "bluemeiLib.h"
#include "Object.h"
#include "BString.h"

#ifdef WIN32

#include <winsock2.h>

typedef SOCKET socket_t;

#define SOCKET_ERR_TIMEDOUT      WSAETIMEDOUT
#define SOCKET_ERR_ADDRINUSE     WSAEADDRINUSE
#define SOCKET_ERR_INTR          WSAEINTR
#define SOCKET_ERR_AGAIN         WSAEWOULDBLOCK
#define SOCKET_ERR_WOULDBLOCK    WSAEWOULDBLOCK
#define SOCKET_ERR_INPROGRES     WSAEINPROGRESS

inline int socketError() {
	return WSAGetLastError();
}

inline in_addr ip_to_in_addr(const char* ip) {
	in_addr addr;
	addr.s_addr=::inet_addr(ip);
	return addr;
}

#else // not WIN32

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

typedef int socket_t;

#define closesocket ::close
#define ioctlsocket ::ioctl

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#define SOCKET_ERR_TIMEDOUT      ETIMEDOUT
#define SOCKET_ERR_ADDRINUSE     EADDRINUSE
#define SOCKET_ERR_INTR          EINTR
#define SOCKET_ERR_AGAIN         EAGAIN
#define SOCKET_ERR_WOULDBLOCK    EWOULDBLOCK
#define SOCKET_ERR_INPROGRES     EINPROGRESS

inline int socketError() {
	return errno;
}

inline in_addr ip_to_in_addr(const char* ip) {
	return in_addr { ::inet_addr(ip) };
}

#endif //end of #ifdef WIN32


namespace blib{

class BLUEMEILIB_API ClientSocket : public Object
{
public:
	ClientSocket();
	ClientSocket(socket_t hSocket);
	~ClientSocket();
public:
	void connect(cstring ip,unsigned short port);

	void setPeerAddr(sockaddr_in& addr);
	virtual String toString() const;

	String getPeerHost() const;//若返回string&是否会出错?
	int getPeerPort() const;

	void setTimeout(int ms)/*超时,毫秒 */;
	void setNoDelay(bool noDelay);

	unsigned long availableBytes();
	void skip(unsigned long len);

	int readBytes(char buffer[],int maxLength,int flags=0);
	int readEnoughBytes(char buffer[],int length);
	int readInt();
	int readShort();
	unsigned char readByte();
	//String& readLineByUtf();
	String readLine();
	//String readUtfString(int wantToReadSize);
	String readUtfString();

	//int writeBytes(byte*& buffer,int length);
	int writeBytes(const char buffer[],int length,int flags=0);
	int writeEnoughBytes(const char buffer[],int length);
	int writeInt(int value);
	int writeShort(short value);
	int writeByte(unsigned char value);
	int writeUtfString(const String& str);
	int writeString(const String& str);

	void close();

public:
	void attach(socket_t s);
	socket_t detach();

protected:
	void createSocket();

protected:
	socket_t m_hSocket;
	sockaddr_in m_peerAddr;
private:
	bool m_bClose;
	int m_nTimeout;
public:
	const static int LINE_BUFFER_SIZE;
};

}//end of namespace blib

#endif
