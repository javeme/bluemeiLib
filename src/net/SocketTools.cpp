
#include <stdio.h>
#include <winsock2.h>

#include "SocketTools.h"
#include "SocketException.h"

namespace bluemei{

//初始socket环境
int SocketTools::initSocketContext()
{
#ifndef WIN32
	return 1;
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
	return 1;
#endif
	return WSACleanup();
}

//显示错误
String SocketTools::getWinsockErrorMsg(int errCode)
{
	String str;
	switch(errCode)
	{
		case WSABASEERR: str=("WSABASEERR"); break;
		case WSAEINTR: str=("WSAEINTR"); break;
		case WSAEBADF: str=("WSAEBADF"); break;
		case WSAEACCES: str=("WSAEACCES"); break;
		case WSAEFAULT: str=("WSAEFAULT"); break;
		case WSAEINVAL: str=("WSAEINVAL"); break;
		case WSAEMFILE: str=("WSAEMFILE"); break;
		case WSAEWOULDBLOCK: str=("WSAEWOULDBLOCK"); break;
		case WSAEINPROGRESS: str=("WSAEINPROGRESS"); break;
		case WSAEALREADY: str=("WSAEALREADY"); break;
		case WSAENOTSOCK: str=("WSAENOTSOCK"); break;
		case WSAEDESTADDRREQ: str=("WSAEDESTADDRREQ"); break;
		case WSAEMSGSIZE: str=("WSAEMSGSIZE"); break;
		case WSAEPROTOTYPE: str=("WSAEPROTOTYPE"); break;
		case WSAENOPROTOOPT: str=("WSAENOPROTOOPT"); break;
		case WSAEPROTONOSUPPORT: str=("WSAEPROTONOSUPPORT"); break;
		case WSAESOCKTNOSUPPORT: str=("WSAESOCKTNOSUPPORT"); break;
		case WSAEOPNOTSUPP: str=("WSAEOPNOTSUPP"); break;
		case WSAEPFNOSUPPORT: str=("WSAEPFNOSUPPORT"); break;
		case WSAEAFNOSUPPORT: str=("WSAEAFNOSUPPORT"); break;
		case WSAEADDRINUSE: str=("WSAEADDRINUSE"); break;
		case WSAEADDRNOTAVAIL: str=("WSAEADDRNOTAVAIL"); break;
		case WSAENETDOWN: str=("WSAENETDOWN"); break;
		case WSAENETUNREACH: str=("WSAENETUNREACH"); break;
		case WSAENETRESET: str=("WSAENETRESET"); break;
		case WSAECONNABORTED: str=("WSAECONNABORTED"); break;
		case WSAECONNRESET: str=("WSAECONNRESET"); break;
		case WSAENOBUFS: str=("WSAENOBUFS"); break;
		case WSAEISCONN: str=("WSAEISCONN"); break;
		case WSAENOTCONN: str=("WSAENOTCONN"); break;
		case WSAESHUTDOWN: str=("WSAESHUTDOWN"); break;
		case WSAETOOMANYREFS: str=("WSAETOOMANYREFS"); break;
		case WSAETIMEDOUT: str=("WSAETIMEDOUT"); break;
		case WSAECONNREFUSED: str=("WSAECONNREFUSED"); break;
		case WSAELOOP: str=("WSAELOOP"); break;
		case WSAENAMETOOLONG: str=("WSAENAMETOOLONG"); break;
		case WSAEHOSTDOWN: str=("WSAEHOSTDOWN"); break;
		case WSAEHOSTUNREACH: str=("WSAEHOSTUNREACH"); break;
		case WSAENOTEMPTY: str=("WSAENOTEMPTY"); break;
		case WSAEPROCLIM: str=("WSAEPROCLIM"); break;
		case WSAEUSERS: str=("WSAEUSERS"); break;
		case WSAEDQUOT: str=("WSAEDQUOT"); break;
		case WSAESTALE: str=("WSAESTALE"); break;
		case WSAEREMOTE: str=("WSAEREMOTE"); break;
		case WSASYSNOTREADY: str=("WSASYSNOTREADY"); break;
		case WSAVERNOTSUPPORTED: str=("WSAVERNOTSUPPORTED"); break;
		case WSANOTINITIALISED: str=("WSANOTINITIALISED"); break;
		case WSAEDISCON: str=("WSAEDISCON"); break;
		case WSAENOMORE: str=("WSAENOMORE"); break;
		case WSAECANCELLED: str=("WSAECANCELLED"); break;
		case WSAEINVALIDPROCTABLE: str=("WSAEINVALIDPROCTABLE"); break;
		case WSAEINVALIDPROVIDER: str=("WSAEINVALIDPROVIDER"); break;
		case WSAEPROVIDERFAILEDINIT: str=("WSAEPROVIDERFAILEDINIT"); break;
		case WSASYSCALLFAILURE: str=("WSASYSCALLFAILURE"); break;
		case WSASERVICE_NOT_FOUND: str=("WSASERVICE_NOT_FOUND"); break;
		case WSATYPE_NOT_FOUND: str=("WSATYPE_NOT_FOUND"); break;
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
			HLOCAL hLocalAddress=NULL;  
			int len=FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,errCode,0,(PSTR)&hLocalAddress,0,NULL);  
			if(len>0)
				str=(char*)hLocalAddress; 
			else
				str="no description";
			break;//描述
		}
	}
	return str;
}


}//end of namespace bluemei