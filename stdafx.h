// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN  // 从 Windows 头中排除极少使用的资料
// Windows 头文件:
#include <windows.h>


// TODO: 在此处引用程序需要的其他头文件

/************************************************************************/
/*                            屏蔽部分警告                               */
/************************************************************************/
//屏蔽忽略异常警告(http://www.myexception.cn/vc-mfc/165727.html)
#pragma warning(disable : 4290)

//屏蔽内部成员无导出警告
//(http://blog.csdn.net/sky101010ws/article/details/6780854)
#pragma warning(disable : 4251)
