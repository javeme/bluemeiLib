#include "SocketException.h"
#include "SocketTools.h"

namespace bluemei{

SocketException::SocketException(int nError)
{
	//this->SocketException::SocketException(nError,"Socket Exception");
	this->m_nError=nError;
}
SocketException::SocketException(String strError):Exception(strError)
{
	//this->SocketException::SocketException(0,strError);
	this->m_nError=0;
}
SocketException::SocketException(int nError,String strError):Exception(strError)
{
	this->m_nError=nError;
	//this->m_strLastError=strError;
}
SocketException::~SocketException()
{
	;
}
String SocketException::name() const
{
	return "SocketException";
}
//��ȡ����id
int SocketException::getError()
{
	return this->m_nError;
}
/*
//��ȡ������Ϣ
string& SocketException::getErrorString()//ָ�뻹�Ƕ������ú�?
{
	//string *pStr=new string(m_strError);
	//return *pStr;
	return m_strError;
}*/
//��ӡ������Ϣ
void SocketException::printErrorMsg()
{
	printf(SocketTools::getWinsockErrorMsg(m_nError));
}

String SocketException::toString()const
{
	return name() + ": " + m_strLastError 
		+ "(" + SocketTools::getWinsockErrorMsg(m_nError).c_str() + ")";
}

}//end of namespace bluemei