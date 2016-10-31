#include "CallStackUtil.h"

namespace blib{

CallStackUtil::CallStackUtil() : StackWalker()
{
	LoadModules();
}

CallStackUtil::~CallStackUtil()
{
}

CallStackUtil CallStackUtil::staticCallStackUtil;

CallStackUtil* CallStackUtil::inscance()
{
	//static CallStackUtil csu;
	return &staticCallStackUtil;
}

void CallStackUtil::OnCallstackEntry( CallstackEntryType eType, CallstackEntry &entry )
{
	//__super::OnCallstackEntry(eType,entry);

	char buffer[STACKWALK_MAX_NAMELEN];
	if ( (eType != lastEntry) && (entry.offset != 0) )
	{
		if (entry.name[0] == 0)
			strcpy_s(entry.name, "(function-name not available)");
		if (entry.undName[0] != 0)
			strcpy_s(entry.name, entry.undName);
		if (entry.undFullName[0] != 0)
			strcpy_s(entry.name, entry.undFullName);
		if (entry.lineFileName[0] == 0)
		{
			strcpy_s(entry.lineFileName, "(filename not available)");
			if (entry.moduleName[0] == 0)
				strcpy_s(entry.moduleName, "(module-name not available)");
			_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s (%s) %s %p",
				entry.name, entry.moduleName, entry.lineFileName,(LPVOID)entry.offset);
		}
		else
			_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s %s (line:%d)",
			entry.name, entry.lineFileName, entry.lineNumber);

		//printf(buffer);
		//m_listCallStackMsg.push_back(buffer);
		if(m_pListCallStackMsg!=nullptr)
		{
			m_pListCallStackMsg->push_back(buffer);
		}
	}
}

void CallStackUtil::OnOutput( LPCSTR szText )
{
	//StackWalker::OnOutput(szText);
}

/*
List<String> CallStackUtil::obtainCallStack()
{
	m_lock.getLock();
	m_listCallStackMsg.clear();
	__super::ShowCallstack();
	List<String> list=move(m_listCallStackMsg);//m_listCallStackMsg会被抽取内容后新建8字节的空间,由于在静态成员里晚释放,导致报内存泄露
	//List<String> list;
	//list.swap(m_listCallStackMsg);
	m_lock.releaseLock();
	return list;
}*/

bool CallStackUtil::obtainCallStack( List<String>& list )
{
	m_lock.getLock();
	m_pListCallStackMsg=&list;
	bool success=__super::ShowCallstack()==TRUE;
	m_lock.releaseLock();
	for(int i=0; i < 3 && list.size() > 0; i++)
		list.erase(list.begin());
	return success;
}

}//end of namespace blib