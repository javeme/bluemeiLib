#ifndef ObjectFactory_H_H
#define ObjectFactory_H_H

#include "bluemeiLib.h"
#include "Class.h"
#include "Object.h"
#include "RuntimeException.h"

namespace blib{

typedef Map<String, Class*> ClassMap;

//Object Factory (there saved class info)
class BLUEMEILIB_API ObjectFactory : public Object
{
public:
	~ObjectFactory(void);
private:
	ObjectFactory(void);
	ObjectFactory(const ObjectFactory&);
	ObjectFactory& operator=(const ObjectFactory&);
public:
	static ObjectFactory& instance();
	static void destroy();
public:
	Object* createObject(cstring className) throw(ClassNotFoundException);
	void registerClass(Class* pClass);
	Class* exist(cstring className);
	void clear();
private:
	ClassMap m_classMap;
private:
	static ObjectFactory* s_objectFactory;
};


//��ע���� ���ڸ������ִ�������
typedef struct StaticRegisterClass
{
	StaticRegisterClass(Class *pClass)
	{
		if(!ObjectFactory::instance().exist(pClass->getName()))
			ObjectFactory::instance().registerClass(pClass);
	}
}static_reg_class_t;

//��ע���
#define REGISTER_CLASS(className) \
	static StaticRegisterClass register##className(className::thisClass())

//����ʵ��
#define newInstanceOf(name) ObjectFactory::instance().createObject(name)

}//end of namespace blib

#endif
