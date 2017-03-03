#include "BString.h"
#include "Object.h"
#include "ArrayList.h"
#include "RuntimeException.h"
#include "StringBuilder.h"

namespace blib{

// const null string
const cstring NULL_STRING = ""; // "" or "<null>"

// trans null to null string
#define verdictNull(src)  if(src == nullptr) src = NULL_STRING;


String::String()
{
	init(0);
}

String::String(cstring src, unsigned int len)
{
	verdictNull(src);
	if(len == -1)
		len = strlen(src);
	init(len);
	memcpy(data(), src, std::min(len, (unsigned int)strlen(src)));
}

String::String(const std::string &src)
{
	init(src.length());
	memcpy(data(), src.c_str(), src.length());
}

String::String(const String &src)
{
	init(0);
	*this = src;
}

String::String(String&& src)
{
	init(0);
	*this = std::move(src);
}

String::~String()
{
	// calculate how many small strings
	/*static int count=0, smallcount=0;
	if(m_nLength < STR_SMALL_SIZE*8)smallcount++;
	printf("================> String(%d) destroy: %d (smalls: %f%%%)\n",
		m_nLength, count++, 100 * smallcount / (float)count);*/

	this->destroy();
}

inline char* String::data() const
{
	if(m_nSize < STR_SMALL_SIZE)
		return (char*)m_chars.buf;
	else
		return m_chars.ptr;
}

inline void String::checkBound( unsigned int offset ) const
{
	if(offset >= m_nLength) {
		throwpe(OutOfBoundException(offset, m_nLength));
	}
}

void String::init(unsigned int len)
{
	// NOTE: to do small string optimization (70% of situations len < 8):
	//	store in stack(maybe) if it's a small string (len < STR_SMALL_SIZE)
	//	store in heap if it's a larger string (len >= STR_SMALL_SIZE)
	if(len < STR_SMALL_SIZE) {
		m_chars.buf[len] = '\0';
		m_nSize = STR_SMALL_SIZE - 1;
		m_nLength = len;
	}
	else {
		try {
			m_chars.ptr = new char[len + 1];
			//static GradeMemoryPools pools(1024*4000);
			//m_chars.ptr = (char*)pools.alloc(len + 1);
		} catch(std::exception&) {
			// Can't alloc any memory for String::init
			throw OutOfMemoryException(len);
		}
		m_chars.ptr[len] = '\0';
		m_nSize = m_nLength = len;
	}
}

void String::destroy()
{
	if(m_nSize >= STR_SMALL_SIZE && m_chars.ptr != null) {
		delete[] m_chars.ptr;
		//pools.free(m_chars.ptr, m_nSize + 1);
	}

	m_nSize = STR_SMALL_SIZE - 1;
	m_nLength = 0;
	m_chars.buf[0] = '\0';
}

void String::resize(unsigned int len)
{
	// expected size > current m_nSize, alloc new memory
	if(len > m_nSize) {
		assert(len >= STR_SMALL_SIZE);
		destroy();
		init(len);
	}
	// expected size <= current m_nSize, use current memory
	else {
		// NOTE: this is the benefit we add `m_nSize` member(4 bytes)
		//   and there are about 10% of situations that the
		//   request `len` matches: STR_SMALL_SIZE <= len <= m_nSize

		// update m_nLength, and keep m_nSize
		m_nLength = len;
		data()[len] = '\0';
	}
}

void String::steal(String& src)
{
	// free myself
	this->destroy();

	unsigned int& len = src.m_nLength;

	// copy data to small buffer
	if(len < STR_SMALL_SIZE) {
		memcpy(m_chars.buf, src.data(), len + 1);
		this->m_nSize = STR_SMALL_SIZE - 1;
		this->m_nLength = len;
	}
	// steal memory to ptr
	else {
		assert(STR_SMALL_SIZE <= src.m_nSize);
		this->m_chars.ptr = src.m_chars.ptr;
		this->m_nSize = this->m_nLength = len;
		// reset the source ptr to avoid being released
		src.m_chars.ptr = null;
	}

	// update the source string's properties
	src.destroy();
}


String& String::operator= (cstring src)
{
	verdictNull(src);
	resize(strlen(src));
	memcpy(data(), src, m_nLength);
	return *this;
}

String& String::operator= (const String &src)
{
	if(this != &src)
		*this = ((const char*)src);
	return *this;
}

String& String::operator= (String&& src)
{
	if(this!=&src)
	{
		// steal memory
		this->steal(src);
	}
	return *this;
}

String String::operator+ (const String &add) const
{
	String tmp = *this;
	tmp += add;
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
	return std::string(data(), m_nLength);
}

String String::toString() const
{
	return *this;
}

cstring String::c_str() const
{
	return data();
}

String& String::append(const String &add)
{
	unsigned int lenAdd = add.length();
	unsigned int lenTotal = m_nLength + lenAdd;

	// append empty string
	if(lenAdd == 0) {
		// ignore
	}
	// append to this buffer if the size is enough
	else if(lenTotal <= m_nSize) {
		// NOTE: copy the end char '\0' by `lenAdd + 1`
		memcpy(data() + m_nLength, add.data(), lenAdd + 1);
		m_nLength = lenTotal;
	}
	// alloc a new buffer
	else {
		String tmp("", lenTotal);
		char *buf = tmp.data();
		//strcpy(buf, data());
		memcpy(buf, data(), m_nLength);
		memcpy(buf + m_nLength, add.data(), lenAdd);
		*this = std::move(tmp);
	}
	return *this;
}

//得到内存编号为index的字符
char String::charAt(int index) const
{
	if(index < 0)
		index = index + m_nLength;

	checkBound(index);
	return data()[index];
}

//得到字符串的长度
unsigned int String::length() const
{
	return m_nLength;
}

//得到start后（含）子串substr的索引
int String::find(const String& substr, unsigned int start/*= 0*/) const
{
	if(start == 0 && this->empty())
		return substr.empty() ? 0 : -1;

	/*
	char *strstr(const char *string, const char *strSearch);
	在字符串string中查找strSearch子串.
	返回子串strSearch在string中首次出现位置的指针. 如果没有找到子串strSearch,
	则返回NULL. 如果子串strSearch为空串, 函数返回string值
	*/
	if(start >= 0 && start < m_nLength)
	{
		char* pos = strstr(data() + start, substr.data());
		if(pos != nullptr)
		{
			return pos - data();
		}
	}
	return -1;
}

int String::rfind(const String& substr, unsigned int end/*=-1*/) const
{
	unsigned int sublen = substr.length();

	// large than me
	if(sublen > m_nLength)
		return -1;
	// both empty
	else if(m_nLength == 0) {
		assert(0 == sublen);
		return 0;
	}

	// adjust the end position
	if((end > m_nLength) || (end + sublen > m_nLength))
		end = m_nLength - sublen;
	
	for(; end != -1; end--) {
		char* fromPtr = data() + end;
		// match the last `sublen` chars?
		bool matched = true;
		for(unsigned int i = 0; i < sublen; i++) {
			if(substr.data()[i] != fromPtr[i])
				matched = false;
		}
		if(matched)
			return end;
	}
	return -1;
}

//判断字符串substr是否字符串的子串
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

//去前导、后导空格
String String::trim(int part) const
{
	unsigned int begin = 0;
	unsigned int end = m_nLength - 1;

	if(part == TRIM_LEFT || part == TRIM_BOTH)
	{
		while(data()[begin] == ' ' && begin < m_nLength)
			begin++;
	}

	if(part == TRIM_RIGHT || part == TRIM_BOTH)
	{
		while(data()[end] == ' ' && end >= begin)
			end--;
	}

	return String(data() + begin, end - begin + 1);
}

//字符串左部长度为sublen的子串
String String::getLeftSub(unsigned int sublen) const
{
	if(sublen > m_nLength)
		sublen = m_nLength;
	else if(sublen < 0)
		sublen = 0;
	return String(data(), sublen);
}

//字符串右部长度为sublen的子串
String String::getRightSub(unsigned int sublen) const
{
	if(sublen > m_nLength)
		sublen = m_nLength;
	else if(sublen < 0)
		sublen = 0;
	return String(data() + (m_nLength - sublen), sublen);
}

//字符串中间从start开始长度为sublen的子串
String String::substring(unsigned int start, unsigned int sublen) const
{
	if(start == 0 && this->empty())
		return "";
	else
		checkBound(start);

	if(start >= m_nLength || sublen <= 0)
	{
		return "";
	}
	else if(start + sublen > m_nLength)
	{
		sublen = m_nLength - start;
	}
	return String(data() + start, sublen);
}

String String::substring(unsigned int start) const
{
	return substring(start, m_nLength - start);
}

//将字符串中所有形如strNeedReplaced的子串替换为strReplace
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

//将字符串按分隔符拆分为若干段
ArrayList<String> String::splitWith(const String& separator,
	unsigned int max/*=-1*/, bool allowEmpty/*=true*/) const
{
	ArrayList<String> list;

	if(separator.empty())
	{
		for(unsigned int i = 0; i < length() && i < max; i++)
		{
			list.add(substring(i, 1));
		}
	}
	else
	{
		unsigned int start = 0;
		unsigned int end = 0;
		while((end = find(separator, start)) != -1 &&
			list.size() < max - 1 &&
			start < m_nLength)
		{
			if(end > start || (end == start && allowEmpty))
				list.add(substring(start, end - start));

			start = end + separator.length();
		}
		// the left
		if(m_nLength > start || (m_nLength == start && allowEmpty))
			list.add(getRightSub(m_nLength - start));
	}

	return list;
}

//将list的元素用本字符串拼接起来: "-".join(["a", "b"]) => "a-b"
String String::join(const ArrayList<String>& list,
	unsigned int from, unsigned int to) const
{
	unsigned int size = list.size();
	if (to == -1 || to > size)
		to = size;
	if (from >= size)
		throwpe(OutOfBoundException(from, size));
	if (from > to) {
		throwpe(InvalidArgException(String::format("from(%d) > to(%d)",
			from, to)));
	}

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

bool String::compare(const String & other, bool caseSensitive/*=true*/) const
{
	if(this->m_nLength != other.length())
		return false;

	return compare(other.data(), caseSensitive);
}

bool String::compare(cstring other, bool caseSensitive/*=true*/) const
{
	verdictNull(other);
	if(caseSensitive)
	{
		return (0 == strcmp(data(), other));
	}
	else
		return (0 == stricmp(data(), other));
}

bool String::operator< (cstring str) const
{
	verdictNull(str);
	return strcmp(data(), str)<0;
}

String String::toUpper() const
{
	String upper("", length());
	for (unsigned int i = 0; i < length(); i++)
	{
		upper.data()[i] = toupper(charAt(i));
	}
	return upper;
}

String String::toLower() const
{
	String lower("", length());
	for (unsigned int i = 0; i < length(); i++)
	{
		lower.data()[i] = tolower(charAt(i));
	}
	return lower;
}

String String::format(cstring frmt, ...)
{
	String str;
	va_list arg_ptr;
	verdictNull(frmt);
	int size = strlen(frmt) + 32 + STR_SMALL_SIZE;

	while(true)
	{
		str.resize(size);
		char* buf = str.data();
		// NOTE: arg_ptr will be changed after calling vsnprintf()!
		// and the meaning of return value are different between win and linux:
		// http://china.ygw.blog.163.com/blog/static/68719746201342210148468/
		va_start(arg_ptr, frmt);
		int len = vsnprintf(buf, size ,frmt, arg_ptr);
		va_end(arg_ptr);
		if(len < 0 || len >= size) {
			//printf("format len: %d, size: %d, buf: %s", len, size, buf);
			size <<= 2;
		}
		else {
			str.resize(len);
			break;
		}
	}
	va_end(arg_ptr);

	return str;
}


///////////////////////////////////////////////////////////////////////////////
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


}//end of namespace blib
