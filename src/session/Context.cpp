#include "StdAfx.h"
#include "Context.h"
#include "Exception.h"

namespace bluemei{

Context Context::staticContext;

Context::Context(void)
{
}

Context::~Context(void)
{
}

Context& Context::getInstance()
{
	return staticContext;
}

Application& Context::getApplication()
{
	return m_application;
}

Session& Context::getSession(const string& name)
{
	auto& itor=m_sessionMap.find(name);
	if(itor!=m_sessionMap.end())
		return *itor->second;
	String str="there is no session named :"+name;
	throwpe(Exception(str));
}

bool Context::removeSession(const string& name)
{
	auto& itor=m_sessionMap.find(name);
	if(itor!=m_sessionMap.end()){
		m_sessionMap.erase(itor);
		return true;
	}
	return false;
}

Session& Context::putNewSession(const string& name)
{
	if(!m_sessionMap.insert(make_pair(name,new Session())).second)
	{
		String str="can't put session named :"+name;
		throwpe(Exception(str));
	}
	return getSession(name);
}

SessionMap& Context::getAllSession()
{
	return m_sessionMap;
}

void Context::clear()
{
	for(auto iter=m_sessionMap.begin();iter!=m_sessionMap.end();iter++)
	{
		iter->second->clear();
	}
	m_application.clear();
}

}//end of namespace bluemei