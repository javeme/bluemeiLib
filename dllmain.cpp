// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "System.h"

#include "Vector.h"
#include "ArrayList.h"
#include "HashMap.h"
#include "MultiValueHashMap.h"
#include "DataBuffer.cpp"
#include "ByteBuffer.h"
#include "AccurateTimeTicker.h"
using namespace bluemei;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//������������
		//System::getInstance().init(); //ֻ����DllMain����֮��Ż������������߳�,������Tread.startʱ��ʱ
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//�˳���������
		//System::getInstance().destroy();
		break;
	}
	return TRUE;
}

