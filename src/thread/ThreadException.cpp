#include "ThreadException.h"

namespace bluemei{

ThreadException::ThreadException(int error)
{
	this->m_nError=error;
}
ThreadException::ThreadException(cstring strError,int error):Exception(strError)
{
	this->m_nError=error;
}
ThreadException::~ThreadException(void)
{
}

String ThreadException::name() const
{
	return "ThreadException";
}

String ThreadException::getMessage() const{
	String str="";
	str.append(what());
	str.append("(error='");
	str.append(String("")+m_nError);
	str.append("')");

	str.append(getPosition());

	return str;
}

String ThreadException::toString() const
{
	return name() + ": " + getMessage();
}

}//end of namespace bluemei