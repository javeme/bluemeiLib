#include "TypeManager.h"

namespace blib{

CriticalLock TypeManager::lock;
TypeManager* TypeManager::thisInstance=nullptr;

TypeManager* TypeManager::instance()
{
	//static TypeManager manager;
	//return &manager;
	ScopedLock sl(lock);
	if(thisInstance==nullptr)
		thisInstance=new TypeManager();
	checkNullPtr(thisInstance);
	return thisInstance;
}

void TypeManager::releaseInstance()
{
	ScopedLock sl(lock);
	delete thisInstance;
	thisInstance=nullptr;
}

bool TypeManager::hasRegister(Object* p)
{
	if (p==nullptr)
		return true;
	auto conver=m_typeMap.get(getTypeInfo(p));
	if(conver==nullptr)
		return false;
	else
		return true;
}

void TypeManager::destroy()
{
	auto i=m_typeMap.iterator();
	while(i->hasNext())
	{
		delete i->next().value;
	}
	m_typeMap.clear();
}

}//end of namespace blib