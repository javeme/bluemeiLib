#ifndef ErrorHandler_H_H
#define ErrorHandler_H_H

#include "Object.h"
#include "Exception.h"

namespace blib{

class BLUEMEILIB_API IErrorHandler : public Object
{
public:
	virtual bool handle(const Exception& e)=0;
};

class BLUEMEILIB_API ErrorHandler : public Object
{
public:
	static void handle(const Exception& e);
	static void setHandler(IErrorHandler* handler);
protected:
	static void handleErrorOfHandler(const Exception& e, const Exception&);
protected:
	static IErrorHandler* s_errorHandler;
};

}//end of namespace blib

#endif
