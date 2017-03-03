#ifndef SocketException_H_H
#define SocketException_H_H

#include "Exception.h"

namespace blib{


class BLUEMEILIB_API SocketException : public Exception
{
public:
	SocketException(int nError=0);
	SocketException(const String& strError);
	SocketException(int nError,const String& strError);
	virtual ~SocketException();
public:
	virtual String name() const;
	virtual int getError() const;
	virtual String getErrorMsg() const;
	virtual String toString() const;
protected:
	int m_nError;
};

class BLUEMEILIB_API SocketClosedException : public SocketException
{
public:
	SocketClosedException(const String& strError) : SocketException(strError){}
	virtual String name() const { return "SocketClosedException"; }
};

class BLUEMEILIB_API SocketTryAgainException : public SocketException
{
public:
	SocketTryAgainException(int nError) : SocketException(nError){}
	virtual String name() const { return "SocketTryAgainException"; }
};


}//end of namespace blib

#endif
