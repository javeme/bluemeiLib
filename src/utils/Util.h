#ifndef Util_H_H
#define Util_H_H

#include "bluemeiLib.h"
#include "Object.h"

namespace blib{

using std::string;
using std::wstring;

#define PropertiesMap std::map<string,string>

typedef char		int8 ;
typedef short		int16;
typedef int			int32;
typedef __int64		int64;

typedef unsigned char		uint8 ;
typedef unsigned short		uint16;
typedef unsigned int		uint32;
typedef unsigned __int64	uint64;

typedef uint16		word ;
typedef uint32		dword;


class BLUEMEILIB_API Util : public Object
{
public:
	Util(void);
	~Util(void);
public:
	static bool simpleEncode(string& plaintext,const string& code);//明文->密码
	static bool simpleDecode(string& cipher,const string& code);//密码->明文
public:
	static bool getSelfPath(string& path);
	static bool open(const string& name,const string& arg="");
public:
	static void replaceString(string & strToReplace, const string & strSrc, const string &strDst);
	static void trim(string &str);
	static short getChar(int& pos,const string& str);
	static int match(const string& value,const string& regex);//正则匹配

	static int str2Int(cstring str);
	static string int2Str(int i);

	static int64 str2Int64(cstring str);
	static string int642Str(int64 i);

	static double str2Float(cstring str);
	static string float2Str(double f);

	static bool str2Boolean(cstring str);
	static string boolean2Str(bool b);

	//判断字符串是否为数字
	template<class CharType>
	static bool isIntNumber(const CharType* str,int len);
	//判断字符串是否为浮点数
	template<class CharType>
	static bool isFloatNumber(const CharType* str,int len);
public:
	static int random();

	static string uuid4();
	static string uuid1();
	static string guid();
public:
	static void dumpStack();
};

template<class CharType>
bool Util::isIntNumber(const CharType* str,int len)
{
	//int len=strlen(str);
	int i=0;
	if(len>0 && str[0]=='-')//负号
		i++;
	for(;i<len;i++)
	{
		if(!isdigit(str[i]))
			return false;
	}
	return true;
}

template<class CharType>
bool Util::isFloatNumber(const CharType* str,int len)
{
	int dotCount=0;
	//int len=strlen(str);
	int i=0;
	if(len>0 && str[0]=='-')//负号
		i++;
	for(;i<len;i++)
	{
		auto c=str[i];
		if(c=='.'){
			dotCount++;
			if(dotCount>1)
				return false;
		}
		else if(!isdigit(c))
			return false;
	}
	return true;
}

}//end of namespace blib


// http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
//#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#endif // end of _MSC_VER

#endif
