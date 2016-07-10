#include "ErrorHandler.h"
#include "System.h"

namespace bluemei{
	
IErrorHandler* ErrorHandler::s_errorHandler=nullptr;

void ErrorHandler::handle(Exception& e)
{
	if(s_errorHandler!=nullptr && s_errorHandler->handle(e)){
		;
	}
	else
		System::instance().debugInfo("%s\n",e.toString().c_str());
}

void ErrorHandler::setHandler(IErrorHandler* handler)
{
	s_errorHandler=handler;
}


}//end of namespace bluemei