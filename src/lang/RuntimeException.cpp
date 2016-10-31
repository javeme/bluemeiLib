#include "RuntimeException.h"
#include "Date.h"

namespace blib{

///////////////////////////////////////////////////////////////////////
//RuntimeException
RuntimeException::RuntimeException(cstring str):Exception(str)
{
}

RuntimeException::~RuntimeException()
{
}

String RuntimeException::name() const
{
	return "RuntimeException";
}


///////////////////////////////////////////////////////////////////////
//TypeException
TypeException::TypeException(cstring msg) : RuntimeException(msg)
{
}

TypeException::TypeException(cstring instance, cstring cls)
	: RuntimeException(String::format("object %s is not an instance of class %s", instance, cls))
{
}

String TypeException::name() const
{
	return "TypeException";
}

///////////////////////////////////////////////////////////////////////
//BadCastException
BadCastException::BadCastException(cstring str):RuntimeException(str)
{
}

BadCastException::BadCastException(cstring from, cstring to)
{
	String err = String::format("Unsupported conversion from type<%s> to type<%s>", from, to);
	setExceptionMsg(err);
}

String BadCastException::name() const
{
	return "BadCastException";
}


///////////////////////////////////////////////////////////////////////
//NotFoundException
NotFoundException::NotFoundException(cstring msg) : RuntimeException(msg)
{
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
}

ClassNotFoundException::~ClassNotFoundException(void)
{
}

String ClassNotFoundException::name() const
{
	return "ClassNotFoundException";
}

///////////////////////////////////////////////////////////////////////
//AttributeNotFoundException
AttributeNotFoundException::AttributeNotFoundException(cstring cls, cstring attr)
	: NotFoundException(String::format("Attribute '%s' not found in class %s", attr, cls))
{
}

AttributeNotFoundException::~AttributeNotFoundException(void)
{
}

String AttributeNotFoundException::name() const
{
	return "AttributeNotFoundException";
}

///////////////////////////////////////////////////////////////////////
//ExistException
ExistException::ExistException(cstring msg) : RuntimeException(msg)
{
}

ExistException::~ExistException(void)
{
}

String ExistException::name() const
{
	return "ExistException";
}

///////////////////////////////////////////////////////////////////////
//KeyExistException
KeyExistException::KeyExistException(cstring msg) : ExistException(msg)
{
}

KeyExistException::~KeyExistException(void)
{
}

String KeyExistException::name() const
{
	return "KeyExistException";
}

///////////////////////////////////////////////////////////////////////
//OutOfBoundException
OutOfBoundException::~OutOfBoundException()
{
}

OutOfBoundException::OutOfBoundException(int out,int size)
{
	String str=String::format("%d out of size %d",out,size);
	this->setExceptionMsg(str);
}

String OutOfBoundException::name() const
{
	return "OutOfBoundException";
}


///////////////////////////////////////////////////////////////////////
//OutOfMemoryException
OutOfMemoryException::OutOfMemoryException(long long size)
{
	this->setExceptionMsg(String::format("when alloc memory size %ld",size));
}

String OutOfMemoryException::name() const
{
	return "OutOfMemoryException";
}


///////////////////////////////////////////////////////////////////////
//TimeoutException
TimeoutException::TimeoutException(long long time)
{
	cstring unit = "ms";
	double ftime = (double)time;
	if(time > SECOND_MSECOND){
		ftime /= SECOND_MSECOND;
		unit = "s";
	}
	else if(time > MINUTE_MSECOND){
		ftime /= MINUTE_MSECOND;
		unit = "min";
	}
	else if(time > HOUR_MSECOND){
		ftime /= HOUR_MSECOND;
		unit = "h";
	}
	else if(time > DAY_MSECOND){
		ftime /= DAY_MSECOND;
		unit = "day";
	}
	this->setExceptionMsg(String::format("%0.2f %s", ftime, unit));
}

String TimeoutException::name() const
{
	return "TimeoutException";
}

}//end of namespace blib