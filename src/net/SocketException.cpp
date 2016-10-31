#include "SocketException.h"
#include "SocketTools.h"

namespace blib{

SocketException::SocketException(int nError)
{
	//this->SocketException::SocketException(nError,"Socket Exception");
	this->m_nError=nError;
}

SocketException::SocketException(const String& strError)
	: Exception(strError)
{
	//this->SocketException::SocketException(0,strError);
	this->m_nError=0;
}

SocketException::SocketException(int nError,const String& strError)
	: Exception(strError)
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

//获取错误id
int SocketException::getError()
{
	return this->m_nError;
}

//打印错误休息
void SocketException::printErrorMsg()
{
	printf(SocketTools::getWinsockErrorMsg(m_nError));
}

String SocketException::toString() const
{
	return String::format("%s: %s(%s)",
		name().c_str(), m_strLastError.c_str(),
		SocketTools::getWinsockErrorMsg(m_nError).c_str());
}

}//end of namespace blib