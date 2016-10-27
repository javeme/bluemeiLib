#include "System.h"
#include "smartptr.h"
#include "LambdaThread.h"
#include "ObjectFactory.h"
#include "Log.h"
#include <algorithm>

namespace bluemei{

//collect every 30 seconds
#define GC_INTERVAL 30

//that's to say, if the system overload is no more than 40%, it's idle
#define GC_MAXLOAD 40

CriticalLock System::s_instanceLock;
WrapperManager* System::s_instanceWrapperManager=nullptr;
SmartPtrManager* System::s_instanceSmartPtrManager=nullptr;

System::System()
{
	m_pGcThread=nullptr;
	m_bCollecting=false;
	m_nGcCount = 0;
	m_bQuit = false;

	m_bDestroy=false;
}
System::~System()
{
	destroy();
}

void System::startGcThread()
{
	ScopedLock sl(m_lock);

	if(m_pGcThread!=nullptr)
		return;
	m_pGcThread=new LambdaThread([&](){
		idleCollect();
	},nullptr);
	m_pGcThread->setAutoDestroy(false);
	m_pGcThread->start();
}


void System::destroyGcThread()
{
	ScopedLock sl(m_lock);

	if(m_pGcThread!=nullptr)
	{
		if(!m_bQuit)
		{
			m_bQuit=true;
			m_pGcThread->wait();
		}
		delete m_pGcThread;
		m_pGcThread=nullptr;
		gc();
	}
}

bool System::isSystemIdle()
{
#ifdef WIN32
	DWORD			counterType;
	HQUERY		  hQuery;
	PDH_STATUS	  pdhStatus;
	HCOUNTER		hCounter;
	static PDH_RAW_COUNTER lastrawdata;
	static bool first = true;

	TCHAR szCounterPath[] = TEXT("\\Processor(_Total)\\% Processor Time");

	/* We can use "% Idle Time" to retrieve IDL time directly, but this counter is available
	 * on WindowsXP only. Fortunatelly, we can get IDL=100-Processer
	 */
	//TCHAR szCounterPath[] = TEXT("\\Processor(0)\\% Idle Time");

	// Open a query object.
	pdhStatus = PdhOpenQuery(NULL, 0, &hQuery);
	if( pdhStatus != ERROR_SUCCESS )
	{
		return true;
	}

	// Add one counter that will provide the data.
	pdhStatus = PdhAddCounter(hQuery,
		szCounterPath,
		0,
		&hCounter);
	if(pdhStatus != ERROR_SUCCESS)
	{
		PdhCloseQuery(hQuery);
		return true;
	}


	PDH_RAW_COUNTER rawdata2;

	pdhStatus = PdhCollectQueryData(hQuery);
	if (pdhStatus != ERROR_SUCCESS)
	{
		PdhCloseQuery(hQuery);
		return true;
	}
	PdhGetRawCounterValue(hCounter, &counterType, &rawdata2);
	if(first)
	{
		lastrawdata = rawdata2;
		first = false;
		PdhCloseQuery(hQuery);
		return true;
	}

	pdhStatus = PdhCollectQueryData(hQuery);
	if (pdhStatus != ERROR_SUCCESS)
	{
		PdhCloseQuery(hQuery);
		return true;
	}
	PdhGetRawCounterValue(hCounter, &counterType, &rawdata2);


	PDH_FMT_COUNTERVALUE fmtValue;
	pdhStatus = PdhCalculateCounterFromRawValue(hCounter,
		PDH_FMT_DOUBLE,
		&rawdata2,
		&lastrawdata,
		&fmtValue);
	if (pdhStatus != ERROR_SUCCESS)
	{
		PdhCloseQuery(hQuery);
		return true;
	}
	lastrawdata = rawdata2;

	pdhStatus = PdhCloseQuery(hQuery);
	return fmtValue.doubleValue <= GC_MAXLOAD;

	/*
	//Method 2, use GetSystemTimes, this is more Simple, but
	//GetSystemTimes is not available on Window2000 OS
	static __int64 last_idle = 0,last_kernel = 0,last_user = 0;
	__int64 idle, kernel, user;
	if(! GetSystemTimes((LPFILETIME)&idle,(LPFILETIME)&kernel,(LPFILETIME)&user))
	{
		ptrTrace("fail to call GetSystemTimes\r\n");
		return false;
	}
	//PdhOpenQuery
	double rate =  (double)(idle-last_idle)/(user-last_user+kernel-last_kernel);
	last_idle=idle;
	last_kernel = kernel;
	last_user = user;
	if( rate >= 0.8 )//more than 80% of system is in idle
		return true;
	return false;
	*/
#else //Linux & UNIX
	char buf[4];
	int f=open("/proc/loadavg", O_RDONLY);
	if(f<0)
	{
		ptrTrace("fail to open file /proc/loadavg");
		return false;
	}
	if (read(f, buf, 4)!=4)
	{
		ptrTrace("fail to read file /proc/loadavg");
		close(f);
		return false;
	}
	close(f);
	return ((buf[0]=='0') && (atoi(buf+2))<GC_MAXLOAD);
#endif
}

//#define blockSize _msize
size_t System::blockSize(void *p)
{
#ifdef  _MSC_VER
	#pragma warning(push)
	#pragma warning( disable : 4311)
	if((int)p%8)//when allocate an array in windows with MSVC, address returned by new is the address returned
		//by malloc +4;, so, if pass this address to _msize will cause an error
	{
		#pragma warning(pop)
		//	return _msize_dbg(p, _NORMAL_BLOCK);
		return _msize((char*)p-4)-4;
	}
	return _msize(p);
	//return _msize_dbg(p, _NORMAL_BLOCK);
#elif defined(__GNUC__)
	return malloc_usable_size(p);
#else
	#error Only MS C++ and GCC are supported
#endif
}


/**
when system is in idle, this function will be called to collect garbage.
*/
void System::idleCollect()
{
	unsigned int count = 0;
	while(!m_bQuit)
	{
		//quit of the appclication will delay at most 1 second
#ifdef WIN32
		Sleep(1000);//there's no sleep in MSVC? how strange!
#else
		sleep(1000); //nanosleep also available on Linux platform
#endif
		++count;
		if(count == GC_INTERVAL)
		{
			count = 0;
			if(isSystemIdle())
			{
				gc();
			}
		}
	}
}

unsigned int System::mark(SmartPtr<void>* ptr)
{
	GlobalMutexLock l;

	unsigned int markCount = 1;
	ObjectWrapper* pWrapper = ptr->m_wrapper;
	pWrapper->isInUse = true;

	SmartPtrManager* ptrManager = SmartPtrManager::getInstance();
	LinkNode* p= (LinkNode*)ptrManager->pEmbeddedPtrHead->pNext;

	while(p!=ptrManager->pEmbeddedPtrTail)
	{
		//SmartPtr<void>* point = static_cast<SmartPtr<void>*>(p);
		SmartPtr<void>* point = (SmartPtr<void>*)p;
		if(!(point->m_wrapper->isInUse) && pWrapper->contain(point))
		{
			//point is in scope of ptr
			markCount += mark(point);
		}
		p= (LinkNode*)p->pNext;
	}
	return markCount;
}

//TODO: �ͷ�ѭ�����ö���(���ܴ����ͷ�˳���ϵĴ�����ա������ƣ�)
//ref: http://kaweh.candy.blog.163.com/blog/static/368187722008217111254485/
void System::gc()
{
	//LOG_FATAL("Begin mark ptr");
	unsigned int markCount = 0;

	GlobalMutexLock l;
	ptrTrace("GC: to collect garbage\r\n");
	SmartPtrManager* ptrManager = getSmartPtrManager();
	WrapperManager* wrapManager = getWrapperManager();
	LinkNode* node = (LinkNode*)ptrManager->pHead->pNext;
	//update Ptr's state that are new created
	while(node!=ptrManager->pTail)
	{
		SmartPtr<void>* ptr = static_cast<SmartPtr<void>*>(node);
		node = node->pNext;
		//SmartPtr�����ַ�Ƿ�ΪǶ�����(�����������ڲ��ı���)
		if(wrapManager->isEmbeddedPtr(ptr))
			ptrManager->moveToEmbeddedPtr(ptr);
		else
			ptrManager->moveToUserPtr(ptr);
	}

	//mark all point as nouse
	WrapperManager::WrapperSet::iterator i = wrapManager->wrappers.begin();
	for(; i != wrapManager->wrappers.end(); i++)
	{
		(*i).p->isInUse = false;
	}

	//markup all used points
	node= (LinkNode*)ptrManager->pUserPtrHead->pNext;
	while(node!=ptrManager->pUserPtrTail)
	{
		markCount += mark(static_cast<SmartPtr<void>*>(node));
		node=node->pNext;
	}

	ptrTrace("GC: finished mark, %d ptr processed, total %d objects\r\n",
		markCount, WrapperManager::getInstance()->wrappers.size());

	//clear all unused object
	m_nGcCount = 0;
	m_bCollecting = true;

#if 0 // Ϊ�˱�֤��ȷ��˳������ͷ�,��ʹ������򵥵ķ���
	/**
	 * use a vector to store all the pointer that are no used.
	 * because we can't delete the wrapper object during iterating
	 */
	std::vector<WrapperPointer> garbage;
	for(WrapperManager::WrapperSet::iterator i = wrapManager->wrappers.begin();
		i !=  wrapManager->wrappers.end(); i++)
	{
		WrapperPointer wp = *i;
		ObjectWrapper *pWrap = wp.p;
		//no other reachable object use this one
		if(!pWrap->isInUse)
		{
			m_nGcCount ++;
			garbage.push_back(wp);
		}
	}
	for(std::vector<WrapperPointer>::iterator i = garbage.begin();
		i != garbage.end(); i++)
	{
		WrapperPointer wp = *i;
		wrapManager->wrappers.erase(wp);
		delete (wp).p; //delete the wrapper. which also make memory reclaimed
		//wrapManager->collect((*i).p); //collect this object and the wrapper object
	}

#else
	//��������,ǧ��ע����յ�˳��,WrapperSet�ǰ��յ�ַ�����ŵ�
	//(�󴴽��ķ���ǰ��, ���ܲ���ȫ���մ���˳������)
	std::vector<WrapperPointer> garbageList;
	for(WrapperManager::WrapperSet::iterator i = wrapManager->wrappers.begin(); i !=  wrapManager->wrappers.end(); i++)
	{
		WrapperPointer wp = *i;
		if(!wp.p->isInUse)
		{
			m_nGcCount ++;
			//�󴴽����ͷŵ�˳��-wrappers[0]��󴴽�(��ַ���)�ŵ���ǰ��	(����ʵ��wrappers˳�������κι���)
			//garbageList.push_back(wp);
			//�ȴ������ͷ�(����ʱ��Ҫ���ó�Ա����,���Գ�Ա����Ӧ�ú��ͷ�,��Ա�����϶��󴴽�)(�����Ա������ַ��--�������������ʱ������)
			garbageList.insert(garbageList.begin(),wp);
		}
	}

	std::vector<WrapperPointer> garbageQueue;
	for(std::vector<WrapperPointer>::iterator i = garbageList.begin();
		i != garbageList.end(); i++)
	{
		ObjectWrapper *pWrap=(*i).p;
		//ɾ��˳��(����û��ʲô�ð취����֤��ȷ���ͷ�˳��,��ʹ��addGarbage,ֱ���ͷ�)
		//addGarbage(pWrap,garbageList,garbageQueue);//���Լ�����������ӵ���������
		wrapManager->collect(pWrap);
	}
#endif

	m_bCollecting = false;
	ptrTrace("GC: completed after %d objects collected\r\n", m_nGcCount);
}

//�˷�����Ŀ�����ҵ�һ����ȷ��ѭ�������ͷ�˳��,���ڸ�ָ����ܻ�������ʱ������ָ��
//�������ͷŸ�ָ������ȷ��(������ָ�������������·Ƿ��ķ���).
//�������������ͷź󴴽���ָ��(��ָ��)ò��Ҳ�Ǻ���ģ�
//����û��ʵ����ν��ȷ˳��, �������ɾ�����������û�������߼���
void System::addGarbage(ObjectWrapper* pWrap,
	vector<WrapperPointer>& garbageList, vector<WrapperPointer>& garbageQueue)
{
	auto itor=std::find(garbageQueue.begin(),garbageQueue.end(),pWrap);
	if(itor==garbageQueue.end())
	{
		//���ͷ�ָ����뵽����ͷ��(������ӵĻᱻ��һ����������ȥ)
		garbageQueue.insert(garbageQueue.begin(),pWrap);

		SmartPtrManager* ptrManager = getSmartPtrManager();

		//����������Ҫ�ͷŵ�ָ��, ����и�����(�������ͷ�ָ��)����ӵ���������ͷ
		vector<WrapperPointer>::iterator i=garbageList.begin();
		for(; i!= garbageList.end(); i++)
		{
			ObjectWrapper *pWrapOther=(*i).p;
			//TODO: ָ�뱻�����ڸ�������(���ʵ�ַ���isParentOf)
			//if(pWrapOther->isParentOf(pWrap))
			//	addGarbage(pWrapOther,garbageList,garbageQueue);
		}
	}
}


System& System::instance()
{
	static System sys;
	return sys;
}

WrapperManager* System::getWrapperManager()
{
	//static WrapperManager instance;
	//return &instance;
	ScopedLock sl(s_instanceLock);
	if(s_instanceWrapperManager==nullptr)
		s_instanceWrapperManager=new WrapperManager();
	return s_instanceWrapperManager;
}

SmartPtrManager* System::getSmartPtrManager()
{
	//static SmartPtrManager instance;
	//return &instance;
	ScopedLock sl(s_instanceLock);
	if(s_instanceSmartPtrManager==nullptr)
		s_instanceSmartPtrManager=new SmartPtrManager();
	return s_instanceSmartPtrManager;
}


int System::gcCount()
{
	return m_nGcCount;
}

bool System::isCollecting()
{
	return m_bCollecting;
}

void System::setCollecting(bool b)
{
	m_bCollecting=b;
}

void System::init()
{
	ScopedLock sl(m_lock);
	m_bDestroy=false;

	startGcThread();
	getWrapperManager();
	getSmartPtrManager();
}

void System::destroy()
{
	ScopedLock sl(m_lock);

	if(m_bDestroy)
		return;
	m_bDestroy=true;

	destroyGcThread();

	delete getWrapperManager();
	delete getSmartPtrManager();

	TypeManager::releaseInstance();

	ObjectFactory::destroy();

	LogManager::destroy();
}

void System::debugInfo(const char* str, ...)
{
	va_list vlist;
	//�ɱ������ʼλ��
	va_start(vlist, str);
	//���ݿɱ����
	char buf[1024*4];
	vsprintf_s(buf, str, vlist);
	OutputDebugStringA(buf);
	//fwrite(file, buf);
	va_end(vlist);
}


}//end of namespace bluemei