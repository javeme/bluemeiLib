#include "IOException.h"

namespace bluemei{

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

}//end of namespace bluemei