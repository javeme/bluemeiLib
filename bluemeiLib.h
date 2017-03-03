#ifndef BLIB_H_H
#define BLIB_H_H

#ifdef WIN32

///////////////////////////////////////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN

// Windows 头文件:
#include <windows.h>
#include <xutility>


/************************************************************************/
/*                            屏蔽部分警告                               */
/************************************************************************/
//屏蔽忽略异常警告(http://www.myexception.cn/vc-mfc/165727.html)
#pragma warning(disable : 4290)

//屏蔽内部成员无导出警告
//(http://blog.csdn.net/sky101010ws/article/details/6780854)
#pragma warning(disable : 4251)


///////////////////////////////////////////////////////////////////////////////
#ifdef BLUEMEILIB_EXPORTS
#define BLUEMEILIB_API __declspec(dllexport)
#define BLUEMEILIB_TEMPLATE BLUEMEILIB_API
#else
#define BLUEMEILIB_API __declspec(dllimport)
#define BLUEMEILIB_TEMPLATE
#endif // BLUEMEILIB_EXPORTS


///////////////////////////////////////////////////////////////////////////////
#define msleep Sleep
#define stricmp _stricmp

// get the first super type of a class
#define baseof(CLS) __super

#else // not WIN32

///////////////////////////////////////////////////////////////////////////////
#include <atomic>
#include <cstring>
#include <new>
#include <typeinfo>

#include <limits.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

///////////////////////////////////////////////////////////////////////////////
#define BLUEMEILIB_API
#define BLUEMEILIB_TEMPLATE

#define __int64 __INT64_TYPE__

#define msleep(ms) usleep((ms) * 1000)
#define stricmp strcasecmp


///////////////////////////////////////////////////////////////////////////////
#ifdef __APPLE__
#define malloc_usable_size malloc_size
#endif


///////////////////////////////////////////////////////////////////////////////
// get the first super type of a class
// http://stackoverflow.com/questions/15158458/
//   using-tr2direct-bases-get-nth-element-of-result
#include <tr2/type_traits>
#include <tuple>

template<typename T>
struct dbc_as_tuple { };

template<typename... Ts>
struct dbc_as_tuple<std::tr2::__reflection_typelist<Ts...>>
{
    typedef std::tuple<Ts...> type;
};

#define baseof(CLS) std::tuple_element<0, \
    dbc_as_tuple<std::tr2::direct_bases<CLS>::type>::type>::type

#endif // end of #ifdef WIN32


///////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
//using namespace std;

#define List vector
#define Map map


namespace blib{

using std::vector;
using std::map;
using std::move;
using std::type_info;

#define _CODE2STRING(code) #code //可以替换普通代码和宏展开代码
#define CODE2STRING(code) _CODE2STRING(code) //可以替换普通代码和宏展开代码

}//end of namespace blib

#endif
