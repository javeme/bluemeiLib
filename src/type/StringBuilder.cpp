#include "StringBuilder.h"

namespace bluemei{

StringBuilder::StringBuilder(unsigned int capacity/*=DEFAULT_CAPACITY*/)
{
	this->buffer = new char_t[capacity];
	this->capacity = capacity;
	this->avail = 0;
}

StringBuilder::StringBuilder(const String& str)
{
	// Unfortunately, because the size is 16 larger, we cannot share.
	this->avail = str.length();
	this->capacity = avail + DEFAULT_CAPACITY;
	this->buffer = new char_t[this->capacity];
	copyData<char_t>(buffer, str.c_str(), str.length());
}

StringBuilder::~StringBuilder()
{
	delete[] buffer;
	avail=0;
}

StringBuilder::StringBuilder(const StringBuilder &src)
{
	buffer = null;
	capacity=0;
	avail=0;

	*this = src;
}

StringBuilder::StringBuilder(StringBuilder&& src)
{
	buffer = null;
	capacity=0;
	avail=0;

	*this = src;
}

StringBuilder& StringBuilder::operator=(const StringBuilder &src)
{
	if(buffer){
		delete[] buffer;
		buffer = null;
	}
	this->avail = src.avail;
	this->capacity = src.capacity;
	this->buffer = new char_t[this->capacity];
	copyData<char_t>(buffer, src.buffer, src.length());

	return *this;
}

StringBuilder& StringBuilder::operator=(StringBuilder&& src)
{
	if(buffer){
		delete[] buffer;
		buffer = null;
	}

	this->avail = src.avail;
	this->capacity = src.capacity;
	this->buffer = src.buffer;

	src.avail = 0;
	src.capacity = 0;
	src.buffer = null;

	return *this;
}

bool StringBuilder::operator==(const StringBuilder& other) const
{
	return toString() == other.toString();
}

bool StringBuilder::ensureCapacity(unsigned int minimumCapacity)
{
	if(minimumCapacity > capacity)
	{
		unsigned int max = capacity * 2 + 2;
		minimumCapacity = (minimumCapacity < max ? max : minimumCapacity);
		char_t* nb = new char_t[minimumCapacity];
		//VMSystem.arraycopy(buffer, 0, nb, 0, avail);
		copyData<char_t>(nb,buffer,avail);
		delete[] buffer;
		buffer = nb;
		capacity=minimumCapacity;
		return true;
	}
	return false;
}

void StringBuilder::setLength(unsigned int newLength)
{
	unsigned int valueLength = capacity;

	/* Always call ensureCapacity in order to preserve copy-on-write
	semantics.	*/
	ensureCapacity(newLength);

	if (newLength < valueLength)
	{
		/* If the StringBuilder's value just grew, then we know that
		value is newly allocated and the region between count and
		newLength is filled with '\0'.	*/
		avail = newLength;
	}
	else
	{
		/* The StringBuilder's value doesn't need to grow.	However,
		we should clear out any cruft that may exist.	*/
		//while (avail < newLength)
		//	buffer[avail++] = '\0';
		if((avail < newLength))
		{
			//memset(buffer+avail,'\0',(newLength-avail)*sizeof(char_t));
			resetMemory<char_t>(buffer+avail,newLength-avail);
		}
	}
}

void StringBuilder::clear()
{
	setLength(0);
}

void StringBuilder::getChars(unsigned int start, unsigned int end, char_t dst[], unsigned int dstOffset/*=0*/) const
{
	checkBound(start);

	if (end > avail)
		end = avail;

	if (start > end)
		throwpe(InvalidArgException(String::format("start(%d) > end(%d)", start, end)));

	//VMSystem.arraycopy(buffer, srcOffset, dst, dstOffset, srcEnd - srcOffset);
	copyData<char_t>(dst+dstOffset,buffer+start,end-start);
}

void StringBuilder::setCharAt(unsigned int index, char_t ch)
{
	checkBound(index);
	// Call ensureCapacity to enforce copy-on-write.
	ensureCapacity(avail);
	buffer[index] = ch;
}

StringBuilder& StringBuilder::append(const String& str)
{
	unsigned int len = str.length();
	ensureCapacity(avail + len);
	//str.getChars(0, len, buffer, avail);
	copyData<char_t>(buffer+avail,str.c_str(),len);
	avail += len;
	return *this;
}

StringBuilder& StringBuilder::append(const StringBuilder& stringBuffer)
{
	unsigned int len = stringBuffer.avail;
	ensureCapacity(avail + len);
	//VMSystem.arraycopy(stringBuffer.value, 0, buffer, avail, len);
	copyData<char_t>(buffer+avail,stringBuffer.buffer,len);
	avail += len;

	return *this;
}

StringBuilder& StringBuilder::append(const char_t data[], unsigned int offset, unsigned int count)
{
	//if (offset+count > data.length)
	//	throwpe(OutOfBoundException(index,avail));
	ensureCapacity(avail + count);

	//VMSystem.arraycopy(data, offset, buffer, avail, count);
	copyData<char_t>(buffer+avail,data+offset,count);
	avail += count;
	return *this;
}

StringBuilder& StringBuilder::append(char_t ch)
{
	ensureCapacity(avail + 1);
	buffer[avail++] = ch;
	return *this;
}

StringBuilder& StringBuilder::deleteSub(unsigned int start, unsigned int end)
{
	checkBound(start);

	if (end > avail)
		end = avail;

	if (start > end)
		throwpe(InvalidArgException(String::format("start(%d) > end(%d)", start, end)));

	// This will unshare if required.
	ensureCapacity(avail);
	if (avail - end != 0)
	{
		//VMSystem.arraycopy(buffer, end, buffer, start, avail - end);
		//copyData<char_t>();
		memmove(buffer+start,buffer+end,(avail-end)*sizeof(char_t));//重叠区域亦可复制成功
	}
	avail -= end - start;
	return *this;
}

StringBuilder& StringBuilder::replace(unsigned int start, unsigned int end, const String& str)
{
	checkBound(start);

	if (end > avail)
		end = avail;

	if (start > end)
		throwpe(InvalidArgException(String::format("start(%d) > end(%d)", start, end)));

	unsigned int len = str.length();
	int delta = start + len - end;//字符串多/少挤占的数量
	ensureCapacity(avail + delta);

	if (delta!=0)//delta可小于或大于0
	{
		//VMSystem.arraycopy(buffer, end, buffer, end + delta, avail - end);
		memmove(buffer+end+delta,buffer+end,(avail-end)*sizeof(char_t));
	}
	//拷贝字符串
	//str.getChars(0, len, buffer, start);
	copyData<char_t>(buffer+start,str.c_str(),len);
	avail += delta;
	return *this;
}

String StringBuilder::substring(unsigned int start, unsigned int end) const
{
	if(start == 0 && avail == 0)//empty
		return "";
	else
		checkBound(start);

	if (end > avail)
		end = avail;

	if (start > end)
		throwpe(InvalidArgException(String::format("start(%d) > end(%d)", start, end)));

	int len = end - start;
	if(len == 0)
		return "";
	return String(buffer+start, len);
}

StringBuilder& StringBuilder::insert(unsigned int offset, const char_t str[], unsigned int strOffset, unsigned int len)
{
	if(offset > length())
		offset = length();
	ensureCapacity(avail + len);
	//移动原始数据
	memmove(buffer+offset+len,buffer+offset,(avail-offset)*sizeof(char_t));
	//拷贝字符串
	copyData<char_t>(buffer+offset,str+strOffset,len);
	avail += len;
	return *this;
}

int StringBuilder::indexOf(const String& str, unsigned int fromIndex) const
{
	if (fromIndex < 0)
		fromIndex = 0;
	unsigned int limit = avail - str.length();
	for (; fromIndex <= limit; fromIndex++)
		if (regionMatches(fromIndex, str))
			return fromIndex;
	return -1;
}

int StringBuilder::lastIndexOf(const String& str, unsigned int fromIndex) const
{
	int index = min(fromIndex, avail - str.length());
	for (; index >= 0; index--)
		if (regionMatches(index, str))
			return index;
	return -1;
}

StringBuilder& StringBuilder::reverse()
{
	// Call ensureCapacity to enforce copy-on-write.
	ensureCapacity(avail);
	for (int i = avail >> 1, j = avail - i; --i >= 0; ++j)
	{
		char_t c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	return *this;
}

void StringBuilder::trimToSize()
{
	if(avail < capacity)
	{
		char_t* newBuf = new char_t[avail+1];
		newBuf[avail]='\0';
		copyData<char_t>(newBuf,buffer,avail);
		delete[] buffer;
		buffer = newBuf;
	}
}

bool StringBuilder::regionMatches(unsigned int offset, const String& other) const
{
	unsigned int len = other.length();
	cstring s = other.c_str();

	if(offset + len > length())
		return false;
	if(offset == 0 && length() == 0)//empty
		return len == 0;

	/*while (--len >= 0)
		if (buffer[toffset++] != *s++)
			return false;
	return true;*/

	checkBound(offset);
	int result=memcmp(buffer+offset,s,len*sizeof(char_t));
	return result==0;
}

}//end of namespace bluemei
