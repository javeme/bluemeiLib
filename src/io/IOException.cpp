#include "IOException.h"

namespace blib{

IOException::IOException()
{
	;
}

IOException::IOException(String msg)
{
	this->setExceptionMsg(msg);
}

String IOException::name() const
{
	return "IOException";
}

}//end of namespace blib