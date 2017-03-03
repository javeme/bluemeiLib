#include "SocketException.h"
#include "SocketTools.h"

namespace blib{

SocketException::SocketException(int nError)
	: Exception(Util::int2Str(nError).c_str())
{
	//this->SocketException::SocketException(nError,"Socket Exception");
	this->m_nError=nError;
}

SocketException::SocketException(const String& strError)
	: Exception(strError)
{
	this->m_nError=0;
}

SocketException::SocketException(int nError,const String& strError)
	: Exception(strError)
{
	this->m_nError=nError;
}

SocketException::~SocketException()
{
	;
}

String SocketException::name() const
{
	return "SocketException";
}

int SocketException::getError() const
{
	return this->m_nError;
}

String SocketException::getErrorMsg() const
{
	return SocketTools::getWinsockErrorMsg(m_nError);
}

String SocketException::toString() const
{
	return String::format("%s: %s(%s)",
		this->name().c_str(),
		this->m_strLastError.c_str(),
		this->getErrorMsg().c_str());
}

}//end of namespace blib
