#pragma once
#include "IOCompletionPort.h"

namespace bluemei{


IOCompletionPort::IOCompletionPort()
{
	impl=new IOCompletionPortImpl();
	create();
}

IOCompletionPort::~IOCompletionPort()
{
	close();
	delete impl;
}

void IOCompletionPort::create()
{
	return impl->create();
}

void IOCompletionPort::close()
{
	return impl->close();
}

}//end of namespace bluemei