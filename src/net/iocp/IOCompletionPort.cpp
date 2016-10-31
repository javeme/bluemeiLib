#pragma once
#include "IOCompletionPort.h"

namespace blib{


IOCompletionPort::IOCompletionPort()
{
	m_impl=new IOCompletionPortImpl();
	create();
}

IOCompletionPort::~IOCompletionPort()
{
	close();
	delete m_impl;
}

void IOCompletionPort::create()
{
	return m_impl->create();
}

void IOCompletionPort::close()
{
	return m_impl->close();
}

}//end of namespace blib