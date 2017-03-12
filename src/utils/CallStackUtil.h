#ifndef CallStackUtil_H_H
#define CallStackUtil_H_H

#include "bluemeiLib.h"
#include "CriticalLock.h"
#include "BString.h"

#ifdef WIN32
#include "StackWalker.h"

namespace blib{

class BLUEMEILIB_API CallStackUtil : public StackWalker
{
public:
	static CallStackUtil* inscance();
protected:
	CallStackUtil();
	virtual ~CallStackUtil();
public:
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry);
	virtual void OnOutput(LPCSTR szText);

	bool obtainCallStack(List<String>& list);
	//List<String> getCallStackMsgs() const { return m_listCallStackMsg; }
protected:
	List<String>* m_pListCallStackMsg;
	CriticalLock m_lock;
protected:
	static CallStackUtil staticCallStackUtil;
};

}//end of namespace blib

#else // not WIN32

namespace blib{

class BLUEMEILIB_API CallStackUtil : public Object
{
public:
    static CallStackUtil* inscance();
protected:
    CallStackUtil() {}
    virtual ~CallStackUtil() {}
public:
    bool obtainCallStack(List<String>& list);
protected:
    static CallStackUtil staticCallStackUtil;
};

}//end of namespace blib

#endif //end of #ifdef WIN32

#endif
