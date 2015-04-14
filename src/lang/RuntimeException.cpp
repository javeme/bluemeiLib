#include "RuntimeException.h"

namespace bluemei{

///////////////////////////////////////////////////////////////////////
//RuntimeException
RuntimeException::RuntimeException(cstring str):Exception(str)
{
	;
}

RuntimeException::~RuntimeException()
{
	;
}

String RuntimeException::name()const
{
	return "RuntimeException";
}


///////////////////////////////////////////////////////////////////////
//TypeException
TypeException::TypeException(cstring msg) : RuntimeException(msg)
{
	;
}

TypeException::TypeException(cstring instance, cstring cls) 
	: RuntimeException(String::format("object %s is not an instance of class %s", instance, cls))
{
	;
}

String TypeException::name() const
{
	return "TypeException";
}

///////////////////////////////////////////////////////////////////////
//BadCastException
BadCastException::BadCastException(cstring str):RuntimeException(str)
{
	;
}

BadCastException::BadCastException(cstring from, cstring to)
{
	String err = String::format("Unsupported conversion from type<%s> to type<%s>", from, to);
	setExceptionMsg(err);
}

String BadCastException::name()const
{
	return "BadCastException";
}


///////////////////////////////////////////////////////////////////////
//NotFoundException
NotFoundException::NotFoundException(cstring msg) : RuntimeException(msg)
{
	;
}

NotFoundException::~NotFoundException(void)
{
}

String NotFoundException::name() const
{
	return "NotFoundException";
}

///////////////////////////////////////////////////////////////////////
//ClassNotFoundException
ClassNotFoundException::ClassNotFoundException(cstring msg) : NotFoundException(msg)
{
	;
}

ClassNotFoundException::~ClassNotFoundException(void)
{
}

String ClassNotFoundException::name()const
{
	return "ClassNotFoundException";
}

///////////////////////////////////////////////////////////////////////
//AttributeNotFoundException
AttributeNotFoundException::AttributeNotFoundException(cstring cls, cstring attr) 
	: NotFoundException(String::format("Attribute '%s' not found in class %s", attr, cls))
{
	;
}

AttributeNotFoundException::~AttributeNotFoundException(void)
{
}

String AttributeNotFoundException::name()const
{
	return "AttributeNotFoundException";
}

///////////////////////////////////////////////////////////////////////
//OutOfBoundException
OutOfBoundException::~OutOfBoundException()
{
	;
}

OutOfBoundException::OutOfBoundException(int out,int size)
{
	String str=String::format("%d out of size %d",out,size);
	this->setExceptionMsg(str);
}

String OutOfBoundException::name()const
{
	return "OutOfBoundException";
}


///////////////////////////////////////////////////////////////////////
//OutOfMemoryException
OutOfMemoryException::OutOfMemoryException(long long size)
{
	this->setExceptionMsg(String::format("when alloc memory size %ld",size));
}

String OutOfMemoryException::name()const
{
	return "OutOfMemoryException";
}


///////////////////////////////////////////////////////////////////////
//TimeoutException
TimeoutException::TimeoutException(long long time)
{
	this->setExceptionMsg(String::format("%d ms",time));
}

String TimeoutException::name()const
{
	return "TimeoutException";
}


}//end of namespace bluemei