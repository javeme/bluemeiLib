#ifndef CodeUtil_H_H
#define CodeUtil_H_H

#include "Util.h"
#include "BString.h"

namespace blib{


/*
* 字符及其编码工具类
* @author Javeme
* @create 2012/3/30
*/
class BLUEMEILIB_API CodeUtil : public Util
{
public:
	static int bytesToInt(byte *buf);
	static short bytesToShort(byte *buf);
	static float bytesToFloat(byte *buf);
	static double bytesToDouble(byte *buf);

	static void intToBytes(byte* buf, int value);
	static void shortToBytes(byte* buf, short value);
	static void floatToBytes(byte* buf, float f);
	static void doubleToBytes(byte* buf, double f);

	template<typename Type>
	static Type bytesToBigInt(byte *buf, bool bigEndian=false);
	template<typename Type>
	static void bigIntToBytes(byte *buf, Type value, bool bigEndian=false);

public:
#ifdef WIN32
	static int utf8ToUnicode(wstring& strUnicode,const char *strUtf8);
	static int unicodeToUtf8(string& strUtf8,const wstring& strUnicode);

	static int gbkToUnicode(wstring& strUnicode,const char *strGBK);
	static int unicodeToGbk(string& result,const wstring& uStr);

	static int gbkToUtf8(string& strUtf8,const char *strGBK);
	static int utf8ToGbk(string& strGbk,const char* strUtf8);
#endif

	static String unicodeToUtf8(const wstring& uStr);
	static wstring utf8ToUnicode(const String& strUtf8);

	static String unicodeToGbk(const wstring& uStr);
	static wstring gbkToUnicode(const String& strGBK);

	static String utf8ToGbk(const String& strUtf8);
	static String gbkToUtf8(const String& strGBK);

public:
	static string base64Encode(const unsigned char* src, int srcLen);
	static string base64Decode(const char* str);

public:
	static unsigned char char2hexChar(unsigned char x);
	static bool isAlpha(int c);
	static bool isNumber(int c);
	static bool isAlphaOrNumber(int c);

	static string urlEncodeComponent(const char* src);
	static string urlDecodeComponent(const char* src);

	static string urlEncode(const char* src);
	static string urlDecode(const char* src);

public:
	static String bytesToHexString(const byte buffer[], int len);
};

template<typename Type>
Type blib::CodeUtil::bytesToBigInt(byte *buf, bool bigEndian)
{
	Type value = 0;
	const int size = sizeof(value);
	const int64 BIG_INT_MASK = 0xff;

	int shift = 0;
	for(int i=0; i<size; i++)
	{
		shift = bigEndian ? (size-1-i) : i;
		value += (buf[i] & BIG_INT_MASK) << 8*shift;
	}
	return value;
}

template<typename Type>
void blib::CodeUtil::bigIntToBytes(byte *buf, Type value, bool bigEndian)
{
	const int size = sizeof(value);

	int shift = 0;
	for(int i=0; i<size; i++)
	{
		shift = bigEndian ? (size-1-i) : i;
		buf[i] = (value >> 8*shift) & 0xff;
	}
}

#define invert_int32(x)\
    ( (((x) & (0xffU << 24)) >> 24) \
    | (((x) & (0xffU << 16)) >> 8)  \
    | (((x) & (0xffU <<  8)) << 8)  \
    | (((x) & (0xffU      )) << 24) \
    )


struct BLUEMEILIB_API CONVWSTR{
	String str;
	CONVWSTR(const wstring& cstr)
	{
		str = CodeUtil::unicodeToGbk(cstr);
	}
	operator const char*(){
		return str.c_str();
	}
};
struct BLUEMEILIB_API CONVSTR{
	wstring wstr;
	CONVSTR(const String& str)
	{
		wstr = CodeUtil::gbkToUnicode(str);
	}
	operator const wchar_t*(){
		return wstr.c_str();
	}
};

}//end of namespace blib

#endif
