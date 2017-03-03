#ifndef Exception_H_H
#define Exception_H_H

#include "bluemeiLib.h"
#include <exception>
#include <memory>
#include "Object.h"
#include "BString.h"

#ifdef WIN32
namespace std{

template class BLUEMEILIB_API std::allocator<blib::String>;
template class BLUEMEILIB_API List<blib::String>;

}
#endif

namespace blib{

class BLUEMEILIB_API Exception : public std::exception, public Object
{
public:
	Exception();
	Exception(cstring msg);
	Exception(cstring msg, const Exception& e);
	Exception(const Exception& e);
	virtual ~Exception();
public:
	Exception(Exception&& other);
	Exception& operator=(Exception&& other);
public:
	virtual void setExceptionMsg(cstring msg);
	virtual void printException() const;
	virtual void initCallStackTrace();
	virtual void printStackTrace() const;
	virtual List<String> getCallStackMsgs();

	virtual String name() const;
	virtual const char* what() const noexcept;
	virtual String toString() const;

	virtual void setPosition(int line, cstring func, cstring file);
	virtual String getPosition() const;

	virtual String getDisplayText() const;
	virtual void setDisplayText(cstring val);
protected:
	String m_strLastError;
	String m_strDisplayText;

	int m_nLine;
	String m_strFunc,m_strFile;
	List<String> m_listStackMsg;
};

class BLUEMEILIB_API StdException : public Exception
{
public:
	StdException(const std::exception& e) : Exception(e.what()) {}
public:
	virtual String name() const {
		return "StdException";
	}
};

class BLUEMEILIB_API UnknownException : public Exception
{
public:
	UnknownException(cstring msg) : Exception(msg) {}
	UnknownException() : Exception("unknown exception") {}
public:
	virtual String name() const {
		return "UnknownException";
	}
};


typedef std::auto_ptr<Exception> AutoReleaseException;
typedef AutoReleaseException ARException;

//设置异常抛出位置,利用引用操作是因为可能直接替代字符串后又新建对象:
//SET_POS(Exception("...")) => Exception("...").setPosition...
#define SET_POS(e)                                                            \
    auto&& ___ex_=e;                                                          \
    ___ex_.setPosition(__LINE__, __FUNCTION__, __FILE__)                      \
// end of SET_POS

//抛出含位置信息的异常
#define throwpe(e)                                                            \
{                                                                             \
    auto&& __ex_=e;                                                           \
    SET_POS(__ex_);                                                           \
    throw(__ex_);                                                             \
}
// end of throwpe

//抛出自动释放异常(e为指针)
#define throware(e)                                                           \
{                                                                             \
    ARException _pe(e);                                                       \
    SET_POS(*_pe);                                                            \
    throw(_pe);                                                               \
}
// end of throware

//抛出自动释放异常(pe为AutoReleaseException对象)
#define throwpare(pe)                                                         \
{                                                                             \
    if(strcmp(#pe,"_pex")==0){                                                \
        auto&& _pe=pe;                                                        \
        _pe->setPosition(__LINE__ ,__FUNCTION__ ,__FILE__);                   \
        throw(_pe);                                                           \
    }                                                                         \
    else{                                                                     \
        auto&& _pex=pe;                                                       \
        _pex->setPosition(__LINE__ ,__FUNCTION__ ,__FILE__);                  \
        throw(_pex);                                                          \
    }                                                                         \
}
// end of throwpare

/*
#define throwpe(e) \
{\
    if(strcmp(#e,"_ex")==0){\
        Exception& _e=e;\
        SET_POS(_e);\
        throw(_e);\
    }\
    else{\
        Exception& _ex=e;\
        SET_POS(_ex);\
        throw(_ex);//throw基类对象引用会把子类信息丢失!!!\
    }\
}
*/

}//end of namespace blib

#endif
