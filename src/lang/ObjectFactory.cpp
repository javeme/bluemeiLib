#pragma once
#include "ObjectFactory.h"

namespace bluemei{

ObjectFactory ObjectFactory::sFactory;

ObjectFactory::ObjectFactory(void)
{
	;
}

ObjectFactory::~ObjectFactory(void)
{
	clear();
}

ObjectFactory& ObjectFactory::instance()
{
	return sFactory;
}

Object* ObjectFactory::createObject(const char* className)
{
	ClassMap::iterator iter=m_classMap.find(className);
	if(iter!=m_classMap.end())
	{
		const Class* pClass=iter->second;
		return pClass->createObject();//Invoke
	}
	String str="ObjectFactory: have not found class '";
	str+=className;
	str+="'";
	throw ClassNotFoundException(str.c_str());
}

void ObjectFactory::registerClass(const Class* pClass)
{
	cstring className=pClass->getName();
	if(!m_classMap.insert( ClassMap::value_type(className,pClass)).second)
	{
		String str=String::format("ObjectFactory: have registered class '%s'",className);
		throwpe(Exception(str));//have register
	}
}

void ObjectFactory::clear()
{
	ClassMap::iterator iter=m_classMap.begin();
	for(;iter!=m_classMap.end();++iter)
	{
		if(iter->second!=NULL)
		{
			//delete iter->second;
			iter->second=NULL;
		}
	}
	m_classMap.clear();
}

}//end of namespace bluemei