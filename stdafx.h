#ifndef BLIB_STDAFX_H_H
#define BLIB_STDAFX_H_H

#ifdef WIN32

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN  // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�:
#include <windows.h>


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�

/************************************************************************/
/*                            ���β��־���                               */
/************************************************************************/
//���κ����쳣����(http://www.myexception.cn/vc-mfc/165727.html)
#pragma warning(disable : 4290)

//�����ڲ���Ա�޵�������
//(http://blog.csdn.net/sky101010ws/article/details/6780854)
#pragma warning(disable : 4251)

#endif

#endif
