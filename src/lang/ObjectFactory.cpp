#pragma once
#include "ObjectFactory.h"

namespace blib{

ObjectFactory::ObjectFactory(void)
{
	;
}

ObjectFactory::~ObjectFactory(void)
{
	clear();
}

ObjectFactory* ObjectFactory::s_objectFactory = new ObjectFactory();

ObjectFactory& ObjectFactory::instance()
{
	checkNullPtr(s_objectFactory);
	return *s_objectFactory;
}

void ObjectFactory::destroy()
{
	ObjectFactory* objectFactory = ObjectFactory::s_objectFactory;
	ObjectFactory::s_objectFactory = null;
	delete objectFactory;
}

Object* ObjectFactory::createObject(cstring className)
{
	ClassMap::iterator iter=m_classMap.find(className);
	if(iter!=m_classMap.end())
	{
		const Class* pClass=iter->second;
		return pClass->createObject();//Invoke
	}
	String str = String::format("ObjectFactory: have not found class '%s'", className);
	throwpe(ClassNotFoundException(str));
}

void ObjectFactory::registerClass(Class* pClass)
{
	cstring className=pClass->getName();
	if(!m_classMap.insert(std::make_pair(className,pClass)).second)
	{
		String str=String::format("ObjectFactory: have registered class '%s'",className);
		throwpe(KeyExistException(str));//have register
	}
}

Class* ObjectFactory::exist(cstring className)
{
	ClassMap::iterator iter=m_classMap.find(className);
	if(iter != m_classMap.end())
		return iter->second;
	return null;
}

void ObjectFactory::clear()
{
	ClassMap::iterator iter=m_classMap.begin();
	for(;iter!=m_classMap.end();++iter)
	{
		delete iter->second;
		iter->second=null;
	}
	m_classMap.clear();
}

}//end of namespace blib