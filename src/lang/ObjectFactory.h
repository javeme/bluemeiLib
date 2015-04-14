#ifndef ObjectFactory_H_H
#define ObjectFactory_H_H
#include "bluemeiLib.h"
#include "Class.h"
#include "Object.h"
#include "RuntimeException.h"

namespace bluemei{

typedef Map<String,const Class*> ClassMap;

//����
class BLUEMEILIB_API ObjectFactory : public Object
{
public:
	~ObjectFactory(void);
private:
	ObjectFactory(void);
	ObjectFactory(const ObjectFactory&);
	ObjectFactory& operator=(const ObjectFactory&);
private:
	static ObjectFactory sFactory;
public:
	static ObjectFactory& instance();
public:
	Object* createObject(const char* className)throw (ClassNotFoundException);
	void registerClass(const Class* pClass);
	void clear();
private:
	ClassMap m_classMap;
};


//ע�ᶯ̬������ ���ڸ������ִ�������
#if 0
#define REGISTER_CLASS(className)\
{\
	static bool _isRegister=false;\
	if(!_isRegister){\
		/*Ϊ��Ƕ��ʧЧ?registerClass(CLASS(className))*/ \
		Class *pClass=className::thisClass();\
		ObjectFactory::instance().registerClass(pClass);\
		_isRegister=true;\
	}\
}*/
#else
//��̬��ʼ����
typedef struct StaticRegisterClass
{
	StaticRegisterClass(const Class *pClass)
	{
		ObjectFactory::instance().registerClass(pClass);
	}
}static_reg_class_t;
#define REGISTER_CLASS(className) StaticRegisterClass register##className(className::thisClass())
#endif
//����ʵ��
#define NewInstanceOf(name) ObjectFactory::instance().createObject(name)

}//end of namespace bluemei

#endif