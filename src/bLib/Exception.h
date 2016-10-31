#ifndef Exception_H_H
#define Exception_H_H

#include "bluemeiLib.h"
#include <exception>
#include "Object.h"
#include "BString.h"

namespace blib{

template class BLUEMEILIB_API std::allocator<String>;
template class BLUEMEILIB_API List<String>;

class BLUEMEILIB_API Exception : public std::exception, public Object
{
public:
	Exception();
	Exception(cstring msg);
	//Exception(const String& msg);
	Exception(Exception& e, cstring msg);
	virtual ~Exception();
public:
	virtual void setExceptionMsg(cstring msg);
	//virtual void setExceptionMsg(String msg){setExceptionMsg(msg.c_str());}
	virtual void printException() const;
	virtual void initCallStackTrace();
	virtual void printStackTrace() const;
	virtual List<String> getCallStackMsgs();

	virtual String name() const;
	virtual cstring what() const;
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

//�׳���λ����Ϣ���쳣
//�����쳣�׳�λ��,�������ò�������Ϊ����ֱ������ַ��������½�����:SET_POS(Exception("�쳣")) = Exception("�쳣").setPosition...
#define SET_POS(e) auto& __ex=e; __ex.setPosition(__LINE__ ,__FUNCTION__ ,__FILE__)//autoΪc++11��������,��Ҫ������֧��
#define throwpe(e) \
{\
	if(strcmp(#e,"_ex")==0){/*��������,�ܷ�����#if/static_assert�ж�*/\
		auto& _e=e;\
		SET_POS(_e);\
		throw(_e);\
	}\
	else{\
		auto& _ex=e;\
		SET_POS(_ex);\
		throw(_ex);\
	}\
}
//�׳��Զ��ͷ��쳣(eΪָ��)
#define throware(e) \
{\
	ARException _pe(e);\
	SET_POS(*_pe);\
	throw(_pe);\
}
//�׳��Զ��ͷ��쳣(peΪAutoReleaseException����)
#define throwpare(pe) \
{\
	if(strcmp(#pe,"_pex")==0){\
		auto& _pe=pe;\
		_pe->setPosition(__LINE__ ,__FUNCTION__ ,__FILE__);\
		throw(_pe);\
	}\
	else{\
		auto& _pex=pe;\
		_pex->setPosition(__LINE__ ,__FUNCTION__ ,__FILE__);\
		throw(_pex);\
	}\
}
/*
//#define SET_POS(e) Exception& __ex=e; __ex.setPosition(__LINE__ ,__FUNCTION__ ,__FILE__)
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
		throw(_ex);//throw����������û��������Ϣ��ʧ!!!\
	}\
}
*/

}//end of namespace blib
#endif