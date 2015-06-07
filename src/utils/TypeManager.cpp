#include "TypeManager.h"

namespace bluemei{

CriticalLock TypeManager::lock;
TypeManager* TypeManager::thisInstance=nullptr;

TypeManager* TypeManager::instance()
{
	//static TypeManager manager;
	//return &manager;
	ScopedLock sl(lock);
	if(thisInstance==nullptr)
		thisInstance=new TypeManager();
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
	auto conver=typeMap.get(getTypeInfo(p));
	if(conver==nullptr)
		return false;
	else
		return true;
}

void TypeManager::destroy()
{
	auto i=typeMap.iterator();
	while(i->hasNext())
	{
		delete i->next().value;
	}
	typeMap.releaseIterator(i);
	typeMap.clear();
}

}//end of namespace bluemei