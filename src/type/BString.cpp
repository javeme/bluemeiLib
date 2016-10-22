#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "BString.h"
#include "Object.h"
#include "ArrayList.h"
#include "RuntimeException.h"
#include "StringBuilder.h"

namespace bluemei{

const cstring NULL_STRING="";//"" or "<null>"
#define verdictNull(src)  if(src==nullptr) src=NULL_STRING; //�Ǹ�"��ֵ"�������쳣��???

////////////////////////////////////////////////////////////
// String�Ĺ��졢��������
// String()
// String(const char *src)
// String(String &src)
// ~String()

String::String()
{
	init(0);
}
/*
String::String(const char *src)
{
	m_nLength = strlen(src);
	m_charBuffer = new char[m_nLength+1);
	strcpy(m_charBuffer, src);
}*/
String::String(cstring src, int len)
{
	verdictNull(src);
	if(len<0)
		len = strlen(src);
	init(len);
	memcpy(m_charBuffer, src, m_nLength);
}
String::String(const std::string &src)
{
	init(src.length());
	memcpy(m_charBuffer, src.c_str(), m_nLength);
}
String::String(const String &src)
{
	init(0);
	*this=src;
}

String::String(String&& src)
{
	init(0);
	*this=move(src);
}

String::~String()
{
	if(m_charBuffer!=nullptr)
		delete[]m_charBuffer;
}

bool String::init(int len)
{
	m_nLength = len;
	// TODO: needs to support small string optimization?
	m_charBuffer = new char[m_nLength+1];
	if(m_charBuffer==nullptr)
		return false;
	m_charBuffer[m_nLength] = 0;
	return true;
}

void String::checkBound(unsigned int offset) const
{
	if (offset >= m_nLength)
		throwpe(OutOfBoundException(offset,m_nLength));
}

////////////////////////////////////////////////////////////
// String�����������
// String operator = (const char *src)
// String operator + (String &add2)
// operator char *()

String& String::operator= (cstring src)
{
	if(m_charBuffer!=nullptr)
		delete[]m_charBuffer;
	verdictNull(src);
	init(strlen(src));
	memcpy(m_charBuffer, src, m_nLength);
	return *this;
}
String& String::operator= (const String &src)
{
	if(this!=&src)
		*this=((const char*)src);
	return *this;
}

String& String::operator= (String&& src)
{
	if(this!=&src)
	{
		delete[] m_charBuffer;
		m_charBuffer = src.m_charBuffer;
		m_nLength = src.m_nLength;

		src.init(0);
	}
	return *this;
}

String String::operator+ (const String &add) const
{
	String tmp=*this;
	tmp+=add;
	return tmp;
}

String& String::operator+= (const String &add)
{
	return append(add);
}

String::operator cstring() const
{
	return c_str();
}

String::operator std::string() const
{
	return m_charBuffer;
}

String String::toString() const
{
	return *this;
}

cstring String::c_str() const
{
	return m_charBuffer;
}

String& String::append(const String &add)
{
	int lenAdd = add.length();
	int lenTotal = m_nLength + lenAdd;
	char *buf = new char[lenTotal+1];
	//strcpy(buf, m_charBuffer);
	//strcat(buf+m_nLength, (const char*)add);
	memcpy(buf, m_charBuffer, m_nLength);
	memcpy(buf+m_nLength, (const char*)add, lenAdd);
	buf[lenTotal] = '\0';
	delete[]m_charBuffer;
	m_charBuffer=buf;
	m_nLength=lenTotal;
	return *this;
}

//�õ��ڴ���Ϊindex���ַ�
char String::charAt(int index) const
{
	if(index < 0)
	{
		index = index + m_nLength;
	}
	checkBound(index);
	return m_charBuffer[index];
}

//�õ��ַ����ĳ���
unsigned int String::length() const
{
	return m_nLength;
}

//�õ�start�󣨺����Ӵ�substr������
int String::find(const String& substr, unsigned int start/*= 0*/) const
{
	if(start == 0 && this->empty())
		return substr.empty() ? 0 : -1;
	//else
	//	checkBound(start);

#if 0
	for(int i = start ; i <= m_nLength - substr.length() ; i++)
	{
		if(m_charBuffer[i] == substr.charAt(0))
		{
			int j=1;
			for(; j < substr.length() && (i + j < m_nLength) ; j++)
			{
				if(m_charBuffer[i+j] != substr.charAt(j))
					break;
			}
			if(j >= substr.length())
			{
				return i;
			}
		}
	}
	return -1;
#else
	/*
	char *strstr(const char *string, const char *strSearch);
	���ַ���string�в���strSearch�Ӵ�.
	�����Ӵ�strSearch��string���״γ���λ�õ�ָ��. ���û���ҵ��Ӵ�strSearch,
	�򷵻�NULL. ����Ӵ�strSearchΪ�մ�, ��������stringֵ
	*/
	if(start>=0 && start<m_nLength)
	{
		char* pos=strstr(m_charBuffer+start,substr.m_charBuffer);
		if(pos!=nullptr)
		{
			return pos-m_charBuffer;
		}
	}
	return -1;
#endif
}

int String::rfind(const String& substr, unsigned int fromIndex) const
{
	if(fromIndex == -1)
		fromIndex = m_nLength-1;
	if(fromIndex == 0 && this->empty())
		return substr.empty() ? 0 : -1;
	else
		checkBound(fromIndex);

	int index = min(fromIndex, m_nLength - substr.length());
	for (; index >= 0; index--){
		char* pos=strstr(m_charBuffer+index,substr.m_charBuffer);
		if(pos!=nullptr)
		{
			return pos-m_charBuffer;
		}
	}
	return -1;
}

//�ж��ַ���substr�Ƿ��ַ������Ӵ�
bool String::contain(const String& substr) const
{
	if(find(substr) < 0)
		return false;
	else
		return true;
}

bool String::startWith(const String& substr) const
{
	return this->find(substr) == 0;
}

bool String::endWith(const String& substr) const
{
	int offset = length() - substr.length();
	return offset >= 0 && offset == this->rfind(substr);
}

//ȥǰ�����󵼿ո�
String String::trim(int part) const
{
	unsigned int begin=0, end=m_nLength-1;

	if(part == TRIM_LEFT || part == TRIM_BOTH)
	{
		while(m_charBuffer[begin]==' ' && begin < m_nLength)
			begin++;
	}

	if(part == TRIM_RIGHT || part == TRIM_BOTH)
	{
		while(m_charBuffer[end]==' ' && end>=begin)
			end--;
	}
	return String(m_charBuffer+begin,end-begin+1);
}
//
//�ַ����󲿳���Ϊsublen���Ӵ�
String String::getLeftSub(unsigned int sublen) const
{
	if(sublen > m_nLength)
		sublen = m_nLength;
	else if(sublen<0)
		sublen=0;
	return String(m_charBuffer,sublen);
}

//�ַ����Ҳ�����Ϊsublen���Ӵ�
String String::getRightSub(unsigned int sublen) const
{
	if(sublen > m_nLength)
		sublen = m_nLength;
	else if(sublen<0)
		sublen=0;
	return String(m_charBuffer+(m_nLength-sublen),sublen);
}

//�ַ����м��start��ʼ����Ϊsublen���Ӵ�
String String::substring(unsigned int start,unsigned int sublen) const
{
	if(start == 0 && this->empty())
		return "";
	else
		checkBound(start);

	if(start >= m_nLength || sublen <= 0)
	{
		return "";
	}
	else if(start+sublen>m_nLength)
	{
		sublen=m_nLength-start;
	}
	return String(m_charBuffer+start,sublen);
}

String String::substring(unsigned int start) const
{
	return substring(start,m_nLength-start);
}

//���ַ�������������strNeedReplaced���Ӵ��滻ΪstrReplace
String String::replace(const String& strNeedReplaced,
	const String& strReplace) const
{
	String temp;
	int start = 0;
	int index = find(strNeedReplaced, start);
	while(index >= 0)
	{
		temp = temp + substring(start, index - start) + strReplace;

		start = index + strNeedReplaced.length();
		index = find(strNeedReplaced, start);
	}

	temp = temp + getRightSub(m_nLength - start);
	return temp;
}

//���ַ������ָ������Ϊ���ɶ�
ArrayList<String> String::splitWith(const String& separator,
	unsigned int max/*=-1*/) const
{
	ArrayList<String> list;

	if(separator.empty())
	{
		for(unsigned int i=0; i<length() && i < max; i++)
		{
			list.add(substring(i, 1));
		}
	}
	else
	{
		unsigned int start = 0;
		unsigned int end = 0;
		while((end = find(separator, start)) != -1  && list.size() < max-1)
		{
			if(end >= start)
				list.add(substring(start, end - start));

			start = end + separator.length();

			if(start >= m_nLength)
				break;
		}
		list.add(getRightSub(m_nLength - start));
	}

	return list;
}

//��list��Ԫ���ñ��ַ���ƴ������: "-".join(["a", "b"]) => "a-b"
String String::join(const ArrayList<String>& list,
	unsigned int from, unsigned int to) const
{
	unsigned int size = list.size();
	if (to == -1)
		to = size;
	if (from >= size)
		throwpe(OutOfBoundException(from, size));
	if (to > size)
		throwpe(OutOfBoundException(to-1, size));
	if (from > to)
		throwpe(InvalidArgException(String::format("from(%d) > to(%d)", from, to)));

	unsigned int thisLen = length();
	StringBuilder sb((thisLen + list[from].length()) * size + 8);
	for (unsigned int i = from; i < to; i++)
	{
		sb.append(list[i]);
		sb.append(*this);
	}
	//remove last "-"
	sb.deleteSub(sb.length() - thisLen, sb.length());
	return sb.toString();
}

bool String::compare(const String & other, bool caseSensitive/*=true */) const
{
	if(this->m_nLength!=other.length())
		return false;
#if 0
	if(caseSensitive)
	{
		return contain(other);
	}
	else
	{
		char tmp;
		for(int i = 0 ; i <= m_nLength ; i++)
		{
			tmp=other.charAt(i);
			if(m_charBuffer[i] !=tmp)
			{
				if(tmp>='A' && tmp<='Z')
				{
					if(m_charBuffer[i]!=tmp+('a'-'A'))
					{
						return false;
					}
				}
				if(tmp>='a' && tmp<='z')
				{
					if(m_charBuffer[i]!=tmp-('a'-'A'))
					{
						return false;
					}
				}
			}
		}
		return true;
	}
#else
	return compare(other.m_charBuffer,caseSensitive);
#endif
}

bool String::compare(cstring other, bool caseSensitive/*=true */) const
{
	verdictNull(other);
	if(caseSensitive)
	{
		return strcmp(m_charBuffer,other)==0;
	}
	else
		return _stricmp(m_charBuffer,other)==0;
}

bool String::operator<(cstring str) const
{
	verdictNull(str);
	return strcmp(m_charBuffer,str)<0;
}

String String::toUpper() const
{
	String upper("", length());
	for (unsigned int i=0; i<length(); i++)
	{
		upper.m_charBuffer[i] = toupper(charAt(i));
	}
	return upper;
}

String String::toLower() const
{
	String lower("", length());
	for (unsigned int i=0; i<length(); i++)
	{
		lower.m_charBuffer[i] = tolower(charAt(i));
	}
	return lower;
}

String String::format(cstring frmt, ...)
{
	String str;
	va_list arg_ptr;
	verdictNull(frmt);
	va_start(arg_ptr,frmt);
	int size=strlen(frmt)+32;
	while(true)
	{
		char* buf=new char[size+1];
		if(buf==nullptr){
			throwpe(OutOfMemoryException(size));
		}
		//int result=_snprintf(buf,size,frmt,*arg_ptr);
		int len=vsnprintf(buf,size,frmt,arg_ptr);
		if(len<0)/*len==size || len<0*/
		{
			size*=8;
			delete[] buf;
		}
		else
		{
			buf[len]='\0';
			delete[]str.m_charBuffer;
			str.m_charBuffer=buf;
			str.m_nLength=len;//strlen(buf);
			break;
		}
	}
	va_end(arg_ptr);
	return str;
}


/*
//���ز����� "123"+String("456")
String operator+(cstring str1,const String& str2)
{
	return String(str1)+str2;
}

bool operator<(const String& str1,const String& str2)
{
	return str1.operator<(str2);
}*/

#define IMPL_APPEND2STRING(Type)\
	BLUEMEILIB_API String operator+(const Type& str1,const String& str2){\
		return ((String)Value2String<Type>(str1))+(str2);\
	};\
	BLUEMEILIB_API String operator+(const String& str1,const Type& str2){\
		return str1+Value2String<Type>(str2);\
	};
IMPL_APPEND2STRING(bool);
IMPL_APPEND2STRING(char);
IMPL_APPEND2STRING(short);
IMPL_APPEND2STRING(int);
IMPL_APPEND2STRING(long);
IMPL_APPEND2STRING(long long);
IMPL_APPEND2STRING(float);
IMPL_APPEND2STRING(double);
IMPL_APPEND2STRING(cstring);


}//end of namespace bluemei