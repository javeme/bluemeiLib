#include "FtpException.h"

namespace blib{

FtpException::FtpException(int type,cstring msg):Exception(msg)
{
	this->m_nErrorType=type;
}

FtpException::FtpException(int type,const String& msg)
{
	this->m_nErrorType=type;
	this->setExceptionMsg(msg.c_str());
}

FtpException::~FtpException(void)
{
}

String FtpException::name() const
{
	return "FtpException";
}

}//end of namespace blib