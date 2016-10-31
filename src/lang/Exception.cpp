#include "Exception.h"
#include "CallStackUtil.h"
#include <algorithm>

namespace blib{

Exception::Exception()
{
	m_nLine=0;

	initCallStackTrace();
}

Exception::Exception(cstring msg)//exception(msg)»á¿½±´×Ö·û´®,exception(m_strLastError.c_str(),0)
{
	m_nLine=0;
	setExceptionMsg(msg);

	initCallStackTrace();
}

Exception::Exception(Exception& e,cstring msg)
{
	m_nLine=0;
	setExceptionMsg(String(msg)+"\n  "+e.toString());

	initCallStackTrace();
}

Exception::~Exception()
{
}

void Exception::initCallStackTrace()
{
	CallStackUtil *callStack=CallStackUtil::inscance();
	callStack->obtainCallStack(m_listStackMsg);
}

void Exception::setExceptionMsg(cstring msg)
{
	m_strLastError=msg;
	exception::operator=(exception(msg));
}

void Exception::printException() const
{
	//cout<<"Òì³£:"<<m_strLastError<<endl;
	printf("%s\n",toString().c_str());
}

void Exception::printStackTrace() const
{
	printException();

	auto& list=m_listStackMsg;
	std::for_each(list.begin(),list.end(),[&](const String& msg){
		printf("  %s\n", msg.c_str());
	});
}

List<String> Exception::getCallStackMsgs()
{
	return m_listStackMsg;
}

String Exception::name() const
{
	return "Exception";
}

cstring Exception::what() const
{
	//__super::what();
	return m_strLastError.c_str();
}

String Exception::toString() const
{
	return name() + ": " + m_strLastError;
}

void Exception::setPosition(int line, cstring func, cstring file)
{
	this->m_nLine=line;
	this->m_strFunc=func;
	this->m_strFile=file;
}

String Exception::getPosition() const
{
	String str=String::format("(line: %d , ",m_nLine);
	str.append("method: "+m_strFunc+" , ");
	str.append("file: "+m_strFile+")");
	return str;
}

String Exception::getDisplayText() const
{
	return m_strDisplayText;
}

void Exception::setDisplayText( cstring val )
{
	m_strDisplayText = val;
}

}//end of namespace blib