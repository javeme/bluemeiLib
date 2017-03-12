/************************************************************************
*AUTHOR: Daniel Liu
*FIRST CREATE: Dec.22 2005
*contact: liulele@ee.buaa.edu.cn
*
*This software is free to use. If there's any problem or you have enhanced
*it, please let me know.
*License Type: MIT
*你可以自由使用这部分代码，但请保留这段声明,无论你是否修改了该代码。
************************************************************************/
#include "SmartPtrManager.h"
#include "System.h"

#ifdef WIN32
#include <new.h>
#else
#include <new>
#endif


namespace blib{

///////////////////////////////////////////////////////////////////////
// new handler

#ifdef WIN32

static _PNH old_new_handler;
static int old_new_mode;

#else
	
typedef void (*_PNH)(void);
static _PNH old_new_handler;
#define _set_new_handler std::set_new_handler

#endif

//分配内存出错时调用的函数
#ifdef WIN32
int gc_new_handler(size_t sz)
#else
void gc_new_handler()
#endif
{
	System::instance().gc();
	//there's no memory collected, make the defualt handler to deal with
	if(!System::instance().gcCount())
	{
#ifdef WIN32
		return old_new_handler(sz);
#else
		old_new_handler();
#endif
	}

#ifdef WIN32
	//GC has reclaimed some memory, do allocate again
	return System::instance().gcCount();
#endif
}

static void initNewHandler()
{
	//set new handler, so gc runs when there's no sufficient memory
	old_new_handler = _set_new_handler(gc_new_handler);
#ifdef WIN32
	old_new_mode = _set_new_mode(1);
#endif
}

static void restoreNewHandler()
{
	_set_new_handler(old_new_handler); //restore new handler
#ifdef WIN32
	_set_new_mode(old_new_mode); //and new mode
#endif
}


///////////////////////////////////////////////////////////////////////
//class GlobalMutexLock
MutexLock GlobalMutexLock::s_globalMutex;

///< Lock constructor
GlobalMutexLock::GlobalMutexLock()
{
	s_globalMutex.getLock();
}

///< Lock destructor
GlobalMutexLock::~GlobalMutexLock()
{
	s_globalMutex.releaseLock();
}


///////////////////////////////////////////////////////////////////////
//class WrapperManager
ObjectWrapper* WrapperManager::nullWrapper = nullptr;
int WrapperManager::extraSize = -1;
int WrapperManager::arrayHeadSize = 0;

WrapperManager::WrapperManager()
{
	GlobalMutexLock l;

	if(nullWrapper == nullptr){
		static ObjectWrapper nullObjectWrapper(nullptr, nullptr);
		nullWrapper = &nullObjectWrapper;
	}

	if(extraSize < 0)
	{
		int *p = new int;
		extraSize = System::blockSize(p) - sizeof(int);//Windows, 0; Linux, 4.
		delete p;
	}
}

WrapperManager::~WrapperManager()
{
	destroy();
}

WrapperManager* WrapperManager::getInstance()
{
	return System::getWrapperManager();
}

void WrapperManager::remove(ObjectWrapper* pWrapper)
{
	GlobalMutexLock l;
	if(pWrapper==nullWrapper)
		return;
	if(System::instance().isCollecting())
		return;
	wrappers.erase(WrapperPointer(pWrapper));
	delete pWrapper;
}

void WrapperManager::collect(ObjectWrapper* pWrapper)
{
	GlobalMutexLock l;
	wrappers.erase(WrapperPointer(pWrapper));
	delete pWrapper; //delete the wrapper. which also make memory reclaimed

}

/**
 * return true if the ptr itself is located in the inner of other object managed by GC.
 */
bool WrapperManager::isEmbeddedPtr(void *pSmartPtr)
{
	GlobalMutexLock l;
	ObjectWrapper tempWrapper(pSmartPtr, NULL, 1);
	//SmartPtr的地址是否在某个指针里面？
	WrapperSet::const_iterator i = wrappers.find(WrapperPointer(&tempWrapper));
	return i != wrappers.end();
}

bool WrapperManager::existPtr(void* pTarget) const
{
	GlobalMutexLock l;
	if(pTarget == NULL)
		return true;

	ObjectWrapper tempWrapper(pTarget, NULL, 1);//destory=null,size=1

	WrapperSet::const_iterator i = wrappers.find(&tempWrapper);
	if(i == wrappers.end())
		return false;
	else
		return true;
}

ObjectWrapper* WrapperManager::attachWrapper(void* pTarget, DESTORY_PROC destory,
	size_t memSize)
{
	GlobalMutexLock l;
	if(pTarget == NULL)
		return nullWrapper;

	// a non zero memSize make the ObjectWrapper use this size instead of
	//calculate size itself, which save the computer resource
	ObjectWrapper tempWrapper(pTarget, NULL, 1);//destory=null,size=1

	WrapperSet::const_iterator i = wrappers.find(&tempWrapper);
	if(i == wrappers.end())
	{
		//Wrapper with addr and finalizer
		ObjectWrapper* pW = new ObjectWrapper(pTarget, destory, memSize);
		wrappers.insert(pW);
		return pW;
	}
	else
	{
		(*i).p->attach();
		return (*i).p;
	}
}

void WrapperManager::destroy()
{
	ptrTrace("WrapperManager deconstructing...\r\n");
	GlobalMutexLock l;
	if(nullWrapper==nullptr)
		return;
	System::instance().setCollecting(true);

	for(WrapperSet::iterator i=wrappers.begin(); i!=wrappers.end(); i++)
	{
		delete (*i).p;
	}
	wrappers.clear();

	System::instance().setCollecting(false);
	ptrTrace("WrapperManager deconstructed\r\n");
}


///////////////////////////////////////////////////////////////////////
//class SmartPtrManager
SmartPtrManager* SmartPtrManager::getInstance()
{
	return System::getSmartPtrManager();
}
void SmartPtrManager::add(LinkNode* ptr)
{
	GlobalMutexLock l;
	pHead->pNext->pPrev = ptr;
	ptr->pNext = pHead->pNext;
	pHead->pNext = ptr;
	ptr->pPrev = pHead;

}
void SmartPtrManager::remove(LinkNode* ptr)
{
	GlobalMutexLock l;
	ptr->pPrev->pNext = ptr->pNext;
	ptr->pNext->pPrev = ptr->pPrev;
}

void SmartPtrManager::moveToUserPtr(LinkNode* ptr)
{
	GlobalMutexLock l;
	ptr->pPrev->pNext = ptr->pNext;
	ptr->pNext->pPrev = ptr->pPrev;

	pUserPtrHead->pNext->pPrev = ptr;
	ptr->pNext = pUserPtrHead->pNext;
	pUserPtrHead->pNext = ptr;
	ptr->pPrev = pUserPtrHead;
}

void SmartPtrManager::moveToEmbeddedPtr(LinkNode* ptr)
{
	GlobalMutexLock l;
	ptr->pPrev->pNext = ptr->pNext;
	ptr->pNext->pPrev = ptr->pPrev;

	pEmbeddedPtrHead->pNext->pPrev = ptr;
	ptr->pNext = pEmbeddedPtrHead->pNext;
	pEmbeddedPtrHead->pNext = ptr;
	ptr->pPrev = pEmbeddedPtrHead;
}

SmartPtrManager::SmartPtrManager()
{
	GlobalMutexLock l;
	pHead = new LinkNode;
	pTail = new LinkNode;
	pHead->pNext = pTail;
	pTail->pPrev = pHead;

	pUserPtrHead = new LinkNode;
	pUserPtrTail = new LinkNode;
	pUserPtrHead->pNext = pUserPtrTail;
	pUserPtrTail->pPrev = pUserPtrHead;

	pEmbeddedPtrHead = new LinkNode;
	pEmbeddedPtrTail = new LinkNode;
	pEmbeddedPtrHead->pNext = pEmbeddedPtrTail;
	pEmbeddedPtrTail->pPrev = pEmbeddedPtrHead;

	//do gc initialize here
	//start a thread to collect garbage when system is in idle
	//System::getInstance().startGcThread();
	initNewHandler();
}

SmartPtrManager::~SmartPtrManager()
{
	destroy();
}

void SmartPtrManager::destroy()
{
	GlobalMutexLock l;
	//防止多次释放
	if(pHead==nullptr)
		return;

	restoreNewHandler();

	delete pHead;
	delete pTail;

	delete pUserPtrHead;
	delete pUserPtrTail;
	delete pEmbeddedPtrHead;
	delete pEmbeddedPtrTail;

	pHead=nullptr;
	pTail=nullptr;

	pUserPtrHead=nullptr;
	pUserPtrTail=nullptr;
	pEmbeddedPtrHead=nullptr;
	pEmbeddedPtrTail=nullptr;
}


///////////////////////////////////////////////////////////////////////
//class ObjectWrapper
ObjectWrapper::ObjectWrapper(void * p,DESTORY_PROC destory,size_t memSize)
{
	ptrTrace("Wrapper con %p,destory:%p, target:%p\r\n",this,destory,p);
	pTarget = p;
	if(p==NULL)
	{
		size=0;
	}
	else
	{
		if(memSize)
			size = memSize;
		else
		{
			size =  System::blockSize(p) - WrapperManager::extraSize;
			//ASSERT(size);
		}
	}
	count = 1;
	this->destory = destory;
}

ObjectWrapper::~ObjectWrapper()
{
	ptrTrace("Wrapper decon %p, target:%p\r\n",this,pTarget);
	if(pTarget && destory)
		destory(pTarget);
}

void ObjectWrapper::attach()
{
	++count;
};

void ObjectWrapper::disattach()
{
	GlobalMutexLock l;
	--count;
	if(count==0)
	{
		WrapperManager::getInstance()->remove(this);
	}
}
void* ObjectWrapper::getTarget()
{
	return pTarget;
}

void ObjectWrapper::setFinalizer(DESTORY_PROC finalize)
{
	destory = finalize;
}

bool ObjectWrapper::contain(void* point)
{
	unsigned char *address = (unsigned char*)point;
	return (pTarget <= address && address < (unsigned char*)pTarget+size);
}


}//end of namespace blib
