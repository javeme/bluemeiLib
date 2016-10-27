#include "Class.h"
#include "blib/Object.h"
#include "blib/BString.h"
#include "blib/RuntimeException.h"
#include "blib/ClassField.h"
#include "blib/ObjectFactory.h"
#include "blib/ArrayList.h"
#include "blib/HashMap.h"
#include "blib/Ptr.h"


namespace bluemei{

Class::Class()
{
	m_name="undefined";
	m_pCreateFun=nullptr;
	m_superClassRef=nullptr;
	m_pInitPara=nullptr;
}

Class::Class(cstring name,CreateFun* pFun,const Class* superClass)
{
	this->m_name=name;
	this->m_pCreateFun=pFun;
	this->m_superClassRef=superClass;
	this->m_pInitPara=nullptr;
}

Class::~Class()
{
	FieldMap::iterator iter = m_fields.begin();
	for(; iter != m_fields.end(); ++iter)
	{
		delete iter->second;
		iter->second = null;
	}
	m_fields.clear();
}

cstring Class::getName() const
{
	return m_name.c_str();
}

Object* Class::createObject() const
{
	Object* pObj=(*m_pCreateFun)();
	if(pObj!=nullptr && m_pInitPara!=nullptr){
		pObj->init(m_pInitPara);
	}
	return pObj;
}

bool Class::operator==( const Class& other ) const
{
	return m_name.compare(other.getName())==0;
}

bool Class::isMyObject(const Object* pObj) const
{
	//return (this==pObj->thisClass());

	if(pObj==nullptr)
		throw NullPointerException("Class::isMyObject: parameter is null");

	/*const static size_t LEN=strlen("class ");

	cstring objClassName=typeid(*pObj).name();
	if(strlen(objClassName)>LEN)
	{
		if(strcmp(objClassName+LEN, getName())==0)
			return true;
	}
	return false;*/

	/*String objClassName=typeid(*pObj).name();
	objClassName=objClassName.getRightSub(objClassName.length()-LEN);
	return (objClassName==name);*/

	Object* testObj = createObject();//@Todo
	bool isSame = (typeid(*pObj) == typeid(*testObj));
	delete testObj;
	return isSame;
}

bool Class::isMyInstance(const Object* pObj) const
{
	/*Object* pNewObj=pObj->createObject();
	while(true)
	{
		if(pNewObj->thisClass()==this)//dynamic_cast<typeid(*createObject())>(pObj)
			return true;
		pObj=pNewObj;
		pNewObj=pNewObj->__super::createObject();
		delete pObj;
	}
	return false;*/
	/*
	//判断本类的实例虚函数均在传入实例的虚函数表中(单继承)
	Object* pSelfObj=createObject();
	Object* pNewObj=pObj->getThisClass()->createObject();//Object如果是dll外部创建,那么虚函的地址是不一样的
	int* objPtr=(int*)pSelfObj;
	int vTablePtr=objPtr[0];
	int* vFuncPtr=(int*)vTablePtr;
	int* vFuncPtrObj=((int*)*(int*)pNewObj);

	bool isBelong=true;
	int i=0;
	int j=0;
	//1.找到第一个匹配点
	int funcAddr=vFuncPtr[i++];
	while(isBelong)
	{
		int funcAddrObj=vFuncPtrObj[j++];
		if(funcAddrObj==NULL)
		{
			isBelong=false;
			break;
		}
		else if(funcAddr==funcAddrObj)
		{
			break;
		}
	}
	//2.匹配后继点
	while(isBelong)
	{
		int funcAddr=vFuncPtr[i++];
		int funcAddrObj=vFuncPtrObj[j++];

		if(funcAddr==NULL)
			break;
		if(funcAddrObj==NULL || funcAddr!=funcAddrObj)
		{
			isBelong=false;
			break;
		}
	}
	delete pSelfObj;
	delete pNewObj;
	return isBelong;*/

	const Class* objClass=pObj->getThisClass();
	while(objClass!=nullptr)
	{
		if(*this==*objClass)//dynamic_cast<typeid(*createObject())>(pObj)
			return true;
		objClass=objClass->superClass();
	}
	return false;
}


bool Class::putField(const FieldInfo& fld)
{
	//return m_fields.put(fld.name(), fld.clone());
	return m_fields.insert(std::make_pair(fld.name(), fld.clone())).second;
}

bool Class::removeField(cstring fldName)
{
	FieldMap::iterator iter = m_fields.find(fldName);
	if(iter == m_fields.end())
		return false;
	FieldInfo* fld = iter->second;
	m_fields.erase(iter);
	delete fld;
	return true;
}

ArrayList<const FieldInfo*> Class::allFields() const
{
	ArrayList<const FieldInfo*> list;
	FieldMap::const_iterator iter = m_fields.begin();
	for(; iter != m_fields.end(); ++iter)
	{
		list.add(iter->second);
	}
	return list;
}

const FieldInfo* Class::getField(cstring fldName) const
{
	FieldMap::const_iterator iter = m_fields.find(fldName);
	if(iter == m_fields.end())
		throw AttributeNotFoundException(getName(), fldName);
	return iter->second;
}

bool Class::hasField(cstring fldName) const
{
	return m_fields.find(fldName) != m_fields.end();
}

const Class* Class::undefined()
{
	static Class cls;
	return &cls;
}

void Class::registerClass(Class* cls)
{
	ObjectFactory::instance().registerClass(cls);
}

Class* Class::registerClass(cstring name,CreateFun* pFun,const Class* superClass)
{
	if(!ObjectFactory::instance().exist(name))
	{
		ScopePointer<Class> cls = new Class(name, pFun, superClass);
		registerClass(cls);
		return cls.detach();
	}
	else
	{
		String msg = String::format("Exists class '%s'", name);
		throwpe(ExistException(msg));
	}
}

Class* Class::registerClassIfNotExist(cstring name,CreateFun* pFun,
	const Class* superClass)
{
	Class* exist = ObjectFactory::instance().exist(name);
	if(!exist)
	{
		ScopePointer<Class> cls = new Class(name, pFun, superClass);
		registerClass(cls);
		return cls.detach();
	}
	else
	{
		return exist;
	}
}

//static method
void Class::throwRuntimeException(cstring msg) throw(RuntimeException)
{
	throw RuntimeException(msg);
}

}//end of namespace bluemei
