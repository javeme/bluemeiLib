#include "stdafx.h"
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 BLUEMEILIB_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// BLUEMEILIB_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef BLUEMEILIB_EXPORTS
#define BLUEMEILIB_API __declspec(dllexport)
#define BLUEMEILIB_TEMPLATE BLUEMEILIB_API
#else
#define BLUEMEILIB_API __declspec(dllimport)
#define BLUEMEILIB_TEMPLATE 
#endif

#include <xutility>
#include <iostream>
#include <string>
#include <vector>
#include <map>
//using namespace std;

#define List vector
#define Map map

namespace bluemei{

using std::vector;
using std::map;
using std::move;

#define CODE2STRING(code) _CODE2STRING(code)//可以替换普通代码和宏展开代码
#define _CODE2STRING(code) #code//可以替换普通代码和宏展开代码

/* 此类是从 bluemeiLib.dll 导出的
class BLUEMEILIB_API CbluemeiLib {
public:
	CbluemeiLib(void);
	// TODO: 在此添加您的方法。
};

extern BLUEMEILIB_API int nbluemeiLib;

BLUEMEILIB_API int fnbluemeiLib(void);
*/

}//end of namespace bluemei