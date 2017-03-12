#include "Exception.h"
#include "CallStackUtil.h"
#include "StringBuilder.h"
#include <algorithm>

namespace blib{

Exception::Exception()
{
	m_nLine=0;
	initCallStackTrace();
}

Exception::Exception(cstring msg)
{
	m_nLine=0;
	setExceptionMsg(msg);

	initCallStackTrace();
}

Exception::Exception(cstring msg, const Exception& e)
{
	m_nLine=0;
	setExceptionMsg(String(msg)+"\n  "+e.toString());

	initCallStackTrace();
}

Exception::Exception(const Exception& e)
{
	setExceptionMsg(e.toString());
	this->m_strDisplayText=e.m_strDisplayText;
	this->m_nLine=e.m_nLine;
	this->m_strFunc=e.m_strFunc;
	this->m_strFile=e.m_strFile;
	this->m_listStackMsg=e.m_listStackMsg;
}

Exception::~Exception()
{
}

Exception::Exception(Exception&& other)
{
	*this=std::move(other);
}

Exception& Exception::operator=(Exception&& other)
{
	this->m_strLastError=std::move(other.m_strLastError);
	this->m_strDisplayText=std::move(other.m_strDisplayText);
	this->m_nLine=std::move(other.m_nLine);
	this->m_strFunc=std::move(other.m_strFunc);
	this->m_strFile=std::move(other.m_strFile);
	this->m_listStackMsg=std::move(other.m_listStackMsg);
	return *this;
}

void Exception::initCallStackTrace()
{
	CallStackUtil *callStack=CallStackUtil::inscance();
	callStack->obtainCallStack(m_listStackMsg);
}

void Exception::setExceptionMsg(cstring msg)
{
	m_strLastError=msg;
	//exception::operator=(exception(msg)); // no exception::operator= in gcc
}

void Exception::printException() const
{
	printf("%s\n",toString().c_str());
}

void Exception::printStackTrace() const
{
	const char LINE='\n';
	auto& list=m_listStackMsg;

	StringBuilder sb(256 + list.size() * 128);

	// summary
	sb.append(this->toString());
	// position
	sb.append(this->getPosition());
	// line end
	sb.append(LINE);

	// stack
	sb.append("  Stack Trace:");
	sb.append(LINE);
	std::for_each(list.begin(),list.end(),[&](const String& msg){
		sb.append("  ");
		sb.append(msg);
		sb.append(LINE);
	});

	printf("%s", sb.toString().c_str());
}

List<String> Exception::getCallStackMsgs()
{
	return m_listStackMsg;
}

String Exception::name() const
{
	return "Exception";
}

const char* Exception::what() const noexcept
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
	const static String UNKNOWN="<unknown>";

	if(m_strFunc.empty())
		return "";

	StringBuilder sb(32 + m_strFunc.length() + m_strFile.length());

	sb.append("(method ");
	sb.append(m_strFunc.empty() ? UNKNOWN : m_strFunc);

	sb.append(", file \"");
	sb.append(m_strFile.empty() ? UNKNOWN : m_strFile);
	sb.append("\"");

	sb.append(" line ");
	sb.append(m_nLine);
	sb.append(")");

	return sb.toString();
}

String Exception::getDisplayText() const
{
	return m_strDisplayText;
}

void Exception::setDisplayText(cstring val)
{
	m_strDisplayText = val;
}

}//end of namespace blib
