
#include <stdio.h>
#include <winsock2.h>

#include "SocketTools.h"
#include "SocketException.h"

namespace bluemei{

//初始socket环境
int SocketTools::initSocketContext()
{
#ifndef WIN32
	return 0;
#endif
	WSADATA wsad;
	WORD ver;
	//使用2.0的库函数
	ver=MAKEWORD(2,0);
	int nRetCode=WSAStartup(ver,&wsad);
	if(nRetCode)
	{
		throw SocketException(::WSAGetLastError());
	}
	return nRetCode;
}
//清除socket环境
int SocketTools::cleanUpSocketContext()
{
#ifndef WIN32
	return 0;
#endif
	return WSACleanup();
}

//显示错误
String SocketTools::getWinsockErrorMsg(int errCode)
{
	String str;
	switch(errCode)
	{
		case WSABASEERR: str=("WSA_BASE_ERR"); break;
		case WSAEINTR: str=("WSA_E_INTR"); break;
		case WSAEBADF: str=("WSA_E_BADF"); break;
		case WSAEACCES: str=("WSA_E_ACCES"); break;
		case WSAEFAULT: str=("WSA_E_FAULT"); break;
		case WSAEINVAL: str=("WSA_E_INVAL"); break;
		case WSAEMFILE: str=("WSA_E_MFILE"); break;
		case WSAEWOULDBLOCK: str=("WSA_E_WOULDBLOCK"); break;
		case WSAEINPROGRESS: str=("WSA_E_INPROGRESS"); break;
		case WSAEALREADY: str=("WSA_E_ALREADY"); break;
		case WSAENOTSOCK: str=("WSA_E_NOTSOCK"); break;
		case WSAEDESTADDRREQ: str=("WSA_E_DESTADDRREQ"); break;
		case WSAEMSGSIZE: str=("WSA_E_MSGSIZE"); break;
		case WSAEPROTOTYPE: str=("WSA_E_PROTOTYPE"); break;
		case WSAENOPROTOOPT: str=("WSA_E_NOPROTOOPT"); break;
		case WSAEPROTONOSUPPORT: str=("WSA_E_PROTONOSUPPORT"); break;
		case WSAESOCKTNOSUPPORT: str=("WSA_E_SOCKTNOSUPPORT"); break;
		case WSAEOPNOTSUPP: str=("WSA_E_OPNOTSUPP"); break;
		case WSAEPFNOSUPPORT: str=("WSA_E_PFNOSUPPORT"); break;
		case WSAEAFNOSUPPORT: str=("WSA_E_AFNOSUPPORT"); break;
		case WSAEADDRINUSE: str=("WSA_E_ADDRINUSE"); break;
		case WSAEADDRNOTAVAIL: str=("WSA_E_ADDRNOTAVAIL"); break;
		case WSAENETDOWN: str=("WSA_E_NETDOWN"); break;
		case WSAENETUNREACH: str=("WSA_E_NETUNREACH"); break;
		case WSAENETRESET: str=("WSA_E_NETRESET"); break;
		case WSAECONNABORTED: str=("WSA_E_CONNABORTED"); break;
		case WSAECONNRESET: str=("WSA_E_CONNRESET"); break;
		case WSAENOBUFS: str=("WSA_E_NOBUFS"); break;
		case WSAEISCONN: str=("WSA_E_ISCONN"); break;
		case WSAENOTCONN: str=("WSA_E_NOTCONN"); break;
		case WSAESHUTDOWN: str=("WSA_E_SHUTDOWN"); break;
		case WSAETOOMANYREFS: str=("WSA_E_TOOMANYREFS"); break;
		case WSAETIMEDOUT: str=("WSA_E_TIMEDOUT"); break;
		case WSAECONNREFUSED: str=("WSA_E_CONNREFUSED"); break;
		case WSAELOOP: str=("WSA_E_LOOP"); break;
		case WSAENAMETOOLONG: str=("WSA_E_NAMETOOLONG"); break;
		case WSAEHOSTDOWN: str=("WSA_E_HOSTDOWN"); break;
		case WSAEHOSTUNREACH: str=("WSA_E_HOSTUNREACH"); break;
		case WSAENOTEMPTY: str=("WSA_E_NOTEMPTY"); break;
		case WSAEPROCLIM: str=("WSA_E_PROCLIM"); break;
		case WSAEUSERS: str=("WSA_E_USERS"); break;
		case WSAEDQUOT: str=("WSA_E_DQUOT"); break;
		case WSAESTALE: str=("WSA_E_STALE"); break;
		case WSAEREMOTE: str=("WSA_E_REMOTE"); break;
		case WSASYSNOTREADY: str=("WSA_SYSNOTREADY"); break;
		case WSAVERNOTSUPPORTED: str=("WSA_VERNOTSUPPORTED"); break;
		case WSANOTINITIALISED: str=("WSA_NOTINITIALISED"); break;
		case WSAEDISCON: str=("WSA_E_DISCON"); break;
		case WSAENOMORE: str=("WSA_E_NOMORE"); break;
		case WSAECANCELLED: str=("WSA_E_CANCELLED"); break;
		case WSAEINVALIDPROCTABLE: str=("WSA_E_INVALIDPROCTABLE"); break;
		case WSAEINVALIDPROVIDER: str=("WSA_E_INVALIDPROVIDER"); break;
		case WSAEPROVIDERFAILEDINIT: str=("WSA_E_PROVIDERFAILEDINIT"); break;
		case WSASYSCALLFAILURE: str=("WSA_SYSCALLFAILURE"); break;
		case WSASERVICE_NOT_FOUND: str=("WSA_SERVICE_NOT_FOUND"); break;
		case WSATYPE_NOT_FOUND: str=("WSA_TYPE_NOT_FOUND"); break;
		case WSA_E_NO_MORE: str=("WSA_E_NO_MORE"); break;
		case WSA_E_CANCELLED: str=("WSA_E_CANCELLED"); break;
		case WSAEREFUSED: str=("WSAEREFUSED"); break;
		case WSAHOST_NOT_FOUND: str=("WSAHOST_NOT_FOUND"); break;
		case WSATRY_AGAIN: str=("WSATRY_AGAIN"); break;
		case WSANO_RECOVERY: str=("WSANO_RECOVERY"); break;
		case WSANO_DATA: str=("WSANO_DATA"); break;
		//case WSANO_ADDRESS: str=("WSANO_ADDRESS"); break;
		case WSA_QOS_RECEIVERS: str=("WSA_QOS_RECEIVERS"); break;
		case WSA_QOS_SENDERS: str=("WSA_QOS_SENDERS"); break;
		case WSA_QOS_NO_SENDERS: str=("WSA_QOS_NO_SENDERS"); break;
		case WSA_QOS_NO_RECEIVERS: str=("WSA_QOS_NO_RECEIVERS"); break;
		case WSA_QOS_REQUEST_CONFIRMED: str=("WSA_QOS_REQUEST_CONFIRMED"); break;
		case WSA_QOS_ADMISSION_FAILURE: str=("WSA_QOS_ADMISSION_FAILURE"); break;
		case WSA_QOS_POLICY_FAILURE: str=("WSA_QOS_POLICY_FAILURE"); break;
		case WSA_QOS_BAD_STYLE: str=("WSA_QOS_BAD_STYLE"); break;
		case WSA_QOS_BAD_OBJECT: str=("WSA_QOS_BAD_OBJECT"); break;
		case WSA_QOS_TRAFFIC_CTRL_ERROR: str=("WSA_QOS_TRAFFIC_CTRL_ERROR"); break;
		case WSA_QOS_GENERIC_ERROR: str=("WSA_QOS_GENERIC_ERROR"); break;
		default:
		{
			LPVOID lpMsgBuf=NULL;
			int len=FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,errCode,0,(LPSTR)&lpMsgBuf,0,NULL);
			if(len>0)
				str=(cstring)lpMsgBuf;
			else
				str="no description";
			LocalFree(lpMsgBuf);
			break;//描述
		}
	}
	return str;
}


}//end of namespace bluemei