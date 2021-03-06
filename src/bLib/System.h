#ifndef System_H_H
#define System_H_H

#include "bluemeiLib.h"
#include "Thread.h"
#include "SmartPtrManager.h"


namespace blib{

#ifdef WIN32
#include "windows.h"
#include "pdh.h"
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "pdh.lib")
#endif

template<typename T>class SmartPtr;
class WrapperManager;

class BLUEMEILIB_API System : public Object
{
private:
	System();
	virtual ~System();
protected:
	Thread* m_pGcThread;
	CriticalLock m_lock;

	volatile bool m_bCollecting;
	volatile int m_nGcCount; // how many object has freed by gc
	volatile bool m_bQuit;

	volatile bool m_bDestroy;
protected:
	void idleCollect();
	virtual unsigned int mark(SmartPtr<void>* ptr);

	void addGarbage(ObjectWrapper* pWrap, vector<WrapperPointer>& garbageList,
		vector<WrapperPointer>& garbageQueue);

	void startGcThread();
	void destroyGcThread();
public:
	static void debugInfo(const char* str , ...);
public:
	static System& instance();
	static WrapperManager* getWrapperManager();
	static SmartPtrManager* getSmartPtrManager();
	static bool isSystemIdle();
	static size_t blockSize(void *p);
protected:
	static CriticalLock s_instanceLock;
	static WrapperManager* s_instanceWrapperManager;
	static SmartPtrManager* s_instanceSmartPtrManager;
public:
	virtual void init();
	virtual void destroy();

	virtual void gc();

	int gcCount();
	bool isCollecting();
	void setCollecting(bool b);
};


}//end of namespace blib

#endif
