#pragma once
//#include "stdafx.h"

#ifdef WIN32

// �� Windows ͷ���ų�����ʹ�õ�����
#define WIN32_LEAN_AND_MEAN

// Windows ͷ�ļ�:
#include <windows.h>


/************************************************************************/
/*                            ���β��־���                               */
/************************************************************************/
//���κ����쳣����(http://www.myexception.cn/vc-mfc/165727.html)
#pragma warning(disable : 4290)

//�����ڲ���Ա�޵�������
//(http://blog.csdn.net/sky101010ws/article/details/6780854)
#pragma warning(disable : 4251)


// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� BLUEMEILIB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// BLUEMEILIB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef BLUEMEILIB_EXPORTS
#define BLUEMEILIB_API __declspec(dllexport)
#define BLUEMEILIB_TEMPLATE BLUEMEILIB_API
#else
#define BLUEMEILIB_API __declspec(dllimport)
#define BLUEMEILIB_TEMPLATE
#endif // BLUEMEILIB_EXPORTS

#endif // WIN32


#include <xutility>
#include <iostream>
#include <string>
#include <vector>
#include <map>
//using namespace std;

#define List vector
#define Map map


namespace blib{

using std::vector;
using std::map;
using std::move;

#define CODE2STRING(code) _CODE2STRING(code)//�����滻��ͨ����ͺ�չ������
#define _CODE2STRING(code) #code//�����滻��ͨ����ͺ�չ������

}//end of namespace blib