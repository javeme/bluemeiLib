#include "ErrorHandler.h"
#include "System.h"

namespace blib{

IErrorHandler* ErrorHandler::s_errorHandler=nullptr;

// handle error of ErrorHandler 
void ErrorHandler::handleErrorOfHandler(const Exception& e,
	const Exception& newe)
{
	System::instance().debugInfo("Failed to handle exception: %s, "
		"a new exception raised: %s\n",
		e.toString().c_str(), newe.toString().c_str());
}

// handle error (don't throw any Exception)
void ErrorHandler::handle(Exception& e)
{
	try{
		if(!(s_errorHandler != nullptr && s_errorHandler->handle(e))){
			System::instance().debugInfo("%s\n", e.toString().c_str());
		}
	}catch(Exception& newe){
		handleErrorOfHandler(e, newe);
	}catch (std::exception& newe){
		handleErrorOfHandler(e, StdException(newe));
	}catch (...){
		handleErrorOfHandler(e, UnknownException("ErrorHandler"));
	}
}

void ErrorHandler::setHandler(IErrorHandler* handler)
{
	s_errorHandler=handler;
}


}//end of namespace blib