#include "CodeUtil.h"
#include "RuntimeException.h"

namespace blib {

//字节数组转换为整型
int CodeUtil::bytesToInt(byte *buf) {
	int value = 0;
	value += (buf[0] & 0xff) << 8 * 0;
	value += (buf[1] & 0xff) << 8 * 1;
	value += (buf[2] & 0xff) << 8 * 2;
	value += (buf[3] & 0xff) << 8 * 3;
	return value;
}

short CodeUtil::bytesToShort(byte *buf) {
	short value = ((buf[1] & 0xff) << 8) + (buf[0] & 0xff);
	return value;
}

float CodeUtil::bytesToFloat(byte *buf) {
	return *(float*) buf;
}

double CodeUtil::bytesToDouble(byte *buf) {
	return *(double*) buf;
}

//整型转换为字节数组
void CodeUtil::intToBytes(byte* buf, int value) {
	for (unsigned int i = 0; i < sizeof(value); i++) {
		buf[i] = (value >> i * 8) & 0xff;
	}
}
void CodeUtil::shortToBytes(byte* buf, short value) {
	for (unsigned int i = 0; i < sizeof(value); i++) {
		buf[i] = (value >> i * 8) & 0xff;
	}
}

void CodeUtil::floatToBytes(byte* buf, float f) {
	memcpy(buf, &f, sizeof(f));
}

void CodeUtil::doubleToBytes(byte* buf, double f) {
	memcpy(buf, &f, sizeof(f));
}

string CodeUtil::base64Encode(const unsigned char* src, int srcLen) {
	static const char encodeTable[] =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	string strEncode;

	unsigned int lineLen = 0;
	unsigned char buf[4] = { 0 };

	for (int i = 0; i < (int) (srcLen / 3); i++) {
		buf[1] = *src++;
		buf[2] = *src++;
		buf[3] = *src++;
		strEncode += encodeTable[buf[1] >> 2];
		strEncode += encodeTable[((buf[1] << 4) | (buf[2] >> 4)) & 0x3F];
		strEncode += encodeTable[((buf[2] << 2) | (buf[3] >> 6)) & 0x3F];
		strEncode += encodeTable[buf[3] & 0x3F];

		lineLen += 4;
		if (lineLen == 76) {
			strEncode += "\r\n";
			lineLen = 0;
		}
	}

	//对剩余数据进行编码
	int mod = srcLen % 3;
	if (mod == 1) {
		buf[1] = *src++;
		strEncode += encodeTable[(buf[1] & 0xFC) >> 2];
		strEncode += encodeTable[((buf[1] & 0x03) << 4)];
		strEncode += "==";
	} else if (mod == 2) {
		buf[1] = *src++;
		buf[2] = *src++;
		strEncode += encodeTable[(buf[1] & 0xFC) >> 2];
		strEncode +=
				encodeTable[((buf[1] & 0x03) << 4) | ((buf[2] & 0xF0) >> 4)];
		strEncode += encodeTable[((buf[2] & 0x0F) << 2)];
		strEncode += "=";
	}

	return strEncode;
}

string CodeUtil::base64Decode(const char* src) {
	static const char decodeTable[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0,
			62, // '+'
			0, 0, 0,
			63, // '/'
			52, 53, 54, 55, 56, 57, 58, 59, 60,
			61, // '0'-'9'
			0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
			14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
			25, // 'A'-'Z'
			0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
			38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
			};

	string strDecode;
	unsigned int decodeLen = 0;

	unsigned int encodeLen = strlen(src);
	int nValue = 0;

	for (unsigned int i = 0; i < encodeLen;) {
		if (*src != '\r' && *src != '\n') {
			nValue = decodeTable[(unsigned int) *src++] << 18;
			nValue += decodeTable[(unsigned int) *src++] << 12;
			strDecode += (nValue & 0x00FF0000) >> 16;
			decodeLen++;
			if (*src != '=') {
				nValue += decodeTable[(unsigned int) *src++] << 6;
				strDecode += (nValue & 0x0000FF00) >> 8;
				decodeLen++;
				if (*src != '=') {
					nValue += decodeTable[(unsigned int) *src++];
					strDecode += nValue & 0x000000FF;
					decodeLen++;
				}
			}
			i += 4;
		} else //'\r' or '\n'
		{
			src++;
			i++;
		}
	}
	return strDecode;
}

unsigned char CodeUtil::char2hexChar(unsigned char x) {
	return (unsigned char) (x > 9 ? x + 55 : x + 48);
}

bool CodeUtil::isAlpha(int c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return true;
	return false;
}

bool CodeUtil::isNumber(int c) {
	if (c >= '0' && c <= '9')
		return true;
	return false;
}

bool CodeUtil::isAlphaOrNumber(int c) {
	if (isAlpha(c) || isNumber(c))
		return true;
	return false;
}

//url encode component
string CodeUtil::urlEncodeComponent(const char* src) {
	string dest;
	dest.reserve(strlen(src));

	unsigned char ch;
	unsigned int len = 0;

	while (*src) {
		ch = (unsigned char) *src;
		if (isAlphaOrNumber(ch) || strchr("!'()*-._~", ch)) {
			dest += *src;
		}
		/*else if (*src == ' ')
		 {
		 dest += '+';
		 }*/
		else {
			dest += '%';
			dest += char2hexChar((unsigned char) (ch >> 4));
			dest += char2hexChar((unsigned char) (ch % 16));
		}
		++src;
		++len;
	}

	return dest;
}

//url decode component
string CodeUtil::urlDecodeComponent(const char* encoded) {
	string decoded;
	decoded.reserve(strlen(encoded));

	unsigned int i = 0;
	char buf[2];

	for (i = 0; i < strlen(encoded); i++) {
		memset(buf, '\0', 2);
		if (encoded[i] != '%') {
			if (encoded[i] == '+')
				decoded += ' ';
			else
				decoded += encoded[i];

			continue;
		}

		buf[0] = encoded[++i];
		buf[1] = encoded[++i];

		buf[0] = buf[0] - 48 - ((buf[0] >= 'A') ? 7 : 0)
				- ((buf[0] >= 'a') ? 32 : 0);
		buf[1] = buf[1] - 48 - ((buf[1] >= 'A') ? 7 : 0)
				- ((buf[1] >= 'a') ? 32 : 0);
		decoded += (char) (buf[0] * 16 + buf[1]);
	}

	return decoded;
}

//url encode
string CodeUtil::urlEncode(const char* src) {
	string dest;
	dest.reserve(strlen(src));

	unsigned char ch;
	unsigned int len = 0;

	while (*src) {
		ch = (unsigned char) *src;
		if (isAlphaOrNumber(ch) || strchr("!#$&'()*+,/:;=?@-._~", ch)) {
			dest += *src;
		} else {
			dest += '%';
			dest += char2hexChar((unsigned char) (ch >> 4));
			dest += char2hexChar((unsigned char) (ch % 16));
		}
		++src;
		++len;
	}

	return dest;
}

//url decode
string CodeUtil::urlDecode(const char* encoded) {
	string decoded;
	decoded.reserve(strlen(encoded));

	unsigned int i = 0;
	char buf[2];

	for (i = 0; i < strlen(encoded); i++) {
		memset(buf, '\0', 2);
		if (encoded[i] != '%') {
			decoded += encoded[i];
			continue;
		}

		buf[0] = encoded[++i];
		buf[1] = encoded[++i];

		buf[0] = buf[0] - 48 - ((buf[0] >= 'A') ? 7 : 0)
				- ((buf[0] >= 'a') ? 32 : 0);
		buf[1] = buf[1] - 48 - ((buf[1] >= 'A') ? 7 : 0)
				- ((buf[1] >= 'a') ? 32 : 0);
		decoded += (char) (buf[0] * 16 + buf[1]);
	}

	return decoded;
}

/*
 string CodeUtil::urlDecode(const char* str){
 string resault;
 resault.reserve(strlen(str));
 short tmp=0;
 char ch, hex[2] = "";
 int n = 0;

 while (ch = *(str + n))
 {
 if (ch == '%')
 {
 hex[0] = *(str + n + 1);
 hex[1] = *(str + n + 2);
 sscanf(hex, "%x", &tmp);
 resault += (char)tmp;
 n = n + 3;
 continue ;
 }
 else if(ch=='-'||ch=='.'||ch=='!'||ch=='~'||ch=='*'||ch=='\''||ch=='('||ch==')'
 || ch=='='|| ch=='&')//strchr("-.=&!~*'()", ch)
 {
 resault+=ch;
 }
 else if(ch=='+')
 {
 resault+=' ';
 }
 else if(ch>='0'&&ch<='9')
 {
 resault+=ch;
 }
 else if(ch>='A'&&ch<='z')
 {
 resault+=ch;
 }
 else
 {
 ;
 }
 n ++;
 }
 //utf8ToGbk(szResault);
 }*/

String CodeUtil::bytesToHexString(const byte buffer[], int len) {
	String str;
	char tmp[4];
	for (int j = 0; j < len; j++) {
		snprintf(tmp, sizeof(tmp), "%02X ", buffer[j]);
		str += tmp;
	}
	return str.trim();
}

} //end of namespace blib


#ifdef WIN32

namespace blib {

//utf8转unicode
int CodeUtil::utf8ToUnicode(wstring& strUnicode, const char *strUtf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, NULL, 0);
	wchar_t * wstr = new wchar_t[len + 1];
	memset(wstr, 0, len * 2 + 2);
	len = MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, wstr, len);
	strUnicode = wstring(wstr, len);
	delete[] wstr;
	return len * 2;
}

//unicode转utf8
int CodeUtil::unicodeToUtf8(string& strUtf8, const wstring& strUnicode) {
	const wchar_t* wstr = strUnicode.c_str();
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	strUtf8 = string(str, len);
	delete[] str;
	return len;
}

//gbk转换unicode
int CodeUtil::gbkToUnicode(wstring& strUnicode, const char *strGBK) {
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len];
	len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);
	strUnicode = wstring(wstr, len);
	delete[] wstr;
	return len * 2;
}

int CodeUtil::unicodeToGbk(string& result, const wstring& uStr) {
	int len = WideCharToMultiByte(CP_ACP, 0, uStr.c_str(),
			-1, NULL, 0, NULL, NULL);
	char* buf = new char[len];
	len = WideCharToMultiByte(CP_ACP, 0, uStr.c_str(), -1, buf, len, 0, 0);
	result = string(buf, len);;
	delete[] buf;
	return len;
}

//gbk转换为utf-8
int CodeUtil::gbkToUtf8(string& strUtf8, const char *strGBK) {
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK, -1, wstr, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	strUtf8 = string(str, len);
	delete[] wstr;
	delete[] str;
	return len;
}

//utf8转换为gbk编码
int CodeUtil::utf8ToGbk(string& strGbk, const char* strUtf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8, -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char *str = new char[len + 1];
	memset(str, 0, len + 1);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	strGbk = string(str, len);
	delete[] str;
	delete[] wstr;
	return len;
}

///////////////////////////////////////////////////////////////////////////////
// 返回对象形式

//utf8转unicode
wstring CodeUtil::utf8ToUnicode(const String& strUtf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
	wchar_t * wstr = new wchar_t[len + 1];
	memset(wstr, 0, len * 2 + 2);
	len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wstr, len);
	wstring strUnicode(wstr, len);
	delete[] wstr;
	return strUnicode;
}

//unicode转utf8
String CodeUtil::unicodeToUtf8(const wstring& strUnicode) {
	const wchar_t* wstr = strUnicode.c_str();
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	String strUtf8("", len);
	char* buf = (char*) strUtf8.c_str();
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, len, NULL, NULL);
	return strUtf8;
}

//gbk转换unicode
wstring CodeUtil::gbkToUnicode(const String& strGBK) {
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len];
	len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wstr, len);
	wstring strUnicode(wstr, len);
	delete[] wstr;
	return strUnicode;
}

String CodeUtil::unicodeToGbk(const wstring& uStr) {
	int len = WideCharToMultiByte(CP_ACP, 0, uStr.c_str(),
			-1, NULL, 0, NULL, NULL);
	String strGbk("", len);
	char* buf = (char*) strGbk.c_str();
	WideCharToMultiByte(CP_ACP, 0, uStr.c_str(), -1, buf, len, 0, 0);
	return strGbk;
}

//gbk转换为utf-8
String CodeUtil::gbkToUtf8(const String& strGBK) {
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wstr, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	String strUtf8("", len);
	char* buf = (char*) strUtf8.c_str();
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buf, len, NULL, NULL);
	delete[] wstr;
	return strUtf8;
}

//utf8转换为gbk编码
String CodeUtil::utf8ToGbk(const String& strUtf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
	wchar_t * wstr = new wchar_t[len + 1];
	memset(wstr, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, wstr, len);

	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	String strGbk("", len);
	char* buf = (char*) strGbk.c_str();
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, buf, len, NULL, NULL);
	delete[] wstr;
	return strGbk;
}

} //end of namespace blib

#else // not WIN32

#include <iconv.h>

namespace blib {

class CodeConverter {
public:
	CodeConverter(const char *fromCharset, const char *toCharset) {
		this->fromCharset = fromCharset;
		this->toCharset = toCharset;
		cd = iconv_open(fromCharset, toCharset);
	}

	virtual ~CodeConverter() {
		int status = iconv_close(cd);
		assert(status == 0);
	}

	size_t convert(const char *inbuf, size_t inlen, char *outbuf,
			size_t& outleft) {
		char **pin = (char **) &inbuf;
		char **pout = &outbuf;

		memset(outbuf, 0, outleft);
		// return 0 if success
		size_t left = iconv(cd, pin, &inlen, pout, &outleft);
		return left;
	}

	String convert(const char* str, size_t len) {
		size_t inlen = len;
		size_t outlen = inlen;
		size_t left = 0;

		String result("", outlen);
		while (true) {
			size_t outleft = outlen; // out buffer len & left
			// TODO: convert from previous position
			left = convert(str, inlen, (char*)result.c_str(), outleft);
			if (left == 0) {
				// keep the real length of string
				result = std::move(String(result.c_str(), outlen - outleft));
				break;
			}
			else if(left != -1) {
				if(outlen < left)
					outlen += left * 2;
				else
					outlen *= 2;
				result = std::move(String("", outlen));
			}
			else {
				throw CharsetConvertionException(str, fromCharset, toCharset);
			}
		}

		return result;
	}

private:
	iconv_t cd;
	const char* fromCharset;
	const char* toCharset;
};


String CodeUtil::unicodeToUtf8(const wstring& wstr) {
	static CodeConverter cc("unicode", "utf-8");

	size_t inlen = wstr.length() * sizeof(wchar_t);
	return cc.convert((const char*)wstr.c_str(), inlen);
}

wstring CodeUtil::utf8ToUnicode(const String& strUtf8) {
	static CodeConverter cc("utf-8", "unicode");

	return wstring((wchar_t*) cc.convert(strUtf8, strUtf8.length()).c_str());
}

String CodeUtil::unicodeToGbk(const wstring& wstr) {
	static CodeConverter cc("unicode", "gbk");

	size_t inlen = wstr.length() * sizeof(wchar_t);
	return cc.convert((const char*)wstr.c_str(), inlen);
}

wstring CodeUtil::gbkToUnicode(const String& strGBK) {
	static CodeConverter cc("gbk", "unicode");
	return wstring((wchar_t*) cc.convert(strGBK, strGBK.length()).c_str());
}

String CodeUtil::utf8ToGbk(const String& strUtf8) {
	static CodeConverter cc("utf-8", "gbk");

	return cc.convert(strUtf8, strUtf8.length());
}

String CodeUtil::gbkToUtf8(const String& strGBK) {
	static CodeConverter cc("gbk", "utf-8");

	return cc.convert(strGBK, strGBK.length());
}

} //end of namespace blib

#endif //end of #ifdef WIN32
