#ifndef HashCoder_H_H
#define HashCoder_H_H

#include "Object.h"
#include "BString.h"

namespace blib{

///////////////////////////////////////////////////////////////////////////////
template<class T>
class BLUEMEILIB_TEMPLATE hashCode{
public:
	hashCode(const T& t){
		hash=0;
	}
	operator unsigned int(){
		return hash;
	}
protected:
	unsigned int hash;
};


///////////////////////////////////////////////////////////////////////////////
class BLUEMEILIB_API StringHashCoder : public Object
{
public:
	virtual unsigned int hashCode(cstring str){
		//default BKDR Hash Function
		return hashStringBKDR(str);
	}
	//BKDR Hash Function
	unsigned int hashStringBKDR(const char *str)
	{
		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
		unsigned int hash = 0;

		while (*str)
		{
			hash = hash * seed + (*str++);
		}

		return (hash & 0x7FFFFFFF);
	}
};


///////////////////////////////////////////////////////////////////////////////
template<> BLUEMEILIB_API
inline hashCode<cstring>::hashCode(const cstring& str)
{
	static StringHashCoder shc;
	hash=shc.hashCode(str);
}

template<> BLUEMEILIB_API
inline hashCode<std::string>::hashCode(const std::string& str)
{
	static StringHashCoder shc;
	hash=shc.hashCode(str.c_str());
}

template<> BLUEMEILIB_API
inline hashCode<String>::hashCode(const String& str)
{
	static StringHashCoder shc;
	hash=shc.hashCode(str);
}

template<> BLUEMEILIB_API
inline hashCode<int>::hashCode(const int& i)
{
	hash=i;
}


///////////////////////////////////////////////////////////////////////////////
template<>
inline Value2String<Object*>::operator String() const{
	if(value==nullptr)
		return "<null>";
	return value->toString();
}

#ifdef WIN32
template class BLUEMEILIB_API Value2String<Object*>;
#endif

}//end of namespace blib

#endif
