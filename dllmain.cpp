#ifdef WIN32

#include "stdafx.h"
#include "System.h"

#include "Vector.h"
#include "ArrayList.h"
#include "HashMap.h"
#include "MultiValueHashMap.h"
#include "DataBuffer.h"
#include "ByteBuffer.h"
#include "AccurateTimeTicker.h"
using namespace blib;


BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//启动垃圾回收
		//System::getInstance().init(); //只有在DllMain返回之后才会启动创建的线程,所以在Tread.start时超时
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//退出垃圾回收
		//System::getInstance().destroy();
		break;
	}
	return TRUE;
}

#endif
