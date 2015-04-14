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