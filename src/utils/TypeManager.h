#pragma once
#include "Object.h"
#include "HashMap.h"
#include "MultiValueHashMap.h"
#include "CriticalLock.h"
#include "RuntimeException.h"
#include "Converter.h"


namespace blib{

class BLUEMEILIB_API Conver : public Object
{
public:
	virtual Object* cast(void*)=0;
	virtual bool isItselfAddr(Object*)=0;
public:
	static inline void* ptrOfType(void* v){
		return v;
	}
	static inline void* ptrOfType(const void* v){
		return const_cast<void*>(v);
	}
};

template<typename SelfType>
class BLUEMEILIB_TEMPLATE ConvObject : public Conver
{
public:
	ConvObject():typeInfo(typeid(SelfType)){}
public:
	/*template<typename T>
	Self* operator()(T* v){
		return static_cast<Self*>(v);
	}*/
	SelfType* operator()(void* v){
		return static_cast<SelfType*>(v);
	}

	/*
	template<typename Target>
	Target* cast(void* v){
		//return static_cast<Target*>(this->operator()(v));//Target*Ϊchar*֮���ʱ��ᱨ��
		return (Target*)(this->operator()(v));//��̳�ʱ����
	}*/

	template<typename Target>
	Target* dcast(void* p){
		return dynamic_cast<Target*>(this->operator()(p));
	}

	Object* cast(void* p){
		const bool isSubObject = is_convertible<SelfType*, Object*>::value;
		return static_caster<SelfType*, isSubObject>::toObject(this->operator()(p));
	}

	bool isItselfAddr(Object* obj){
		void* self = ptrOfType(dynamic_cast<SelfType*>(obj));
		return self == (void*)obj;
	}

private:
	const type_info& typeInfo;
};

//1.�ҵ�����Ķ�������
//2.static_castΪ��������
//3.dynamic_castΪĿ������
class BLUEMEILIB_API TypeManager : public Object
{
public:
	typedef const type_info* TypeInfo;

	static TypeInfo getObjectTypeInfo(Object* p){
		Object* obj=(Object*)p;
		return &(typeid(*obj));
	}

	template<typename T>
	static TypeInfo getTypeInfo(){
		return &(typeid(T));
	}

	static TypeInfo getTypeInfo(void* p){
		try{
			return getObjectTypeInfo((Object*)(void*)p);
		}catch (std::exception&){
			//p is not an Object*, return nullptr(We think it's a simple type).
			return nullptr;
		}
	}
public:
	static TypeManager* instance();
	static void releaseInstance();
protected:
	static CriticalLock lock;
	static TypeManager *thisInstance;

private:
	TypeManager(){}
	virtual ~TypeManager(){
		destroy();
	}
public:
	template<typename T>
	bool registerType(){
		auto k=getTypeInfo<T>();
		if(!m_typeMap.contain(k))
		{
			auto v=new ConvObject<T>();
			return m_typeMap.put(k,v);
		}
		return false;
	}

	bool hasRegister(Object* p);

	void destroy();

	//ע��:�����������Ϊ�������͵�ָ��,����Ϊ����ָ��!(����,�ڶ�̳�ʱ���ܻ����)
	//(��Ϊ��̳�ʱpָ��ĵ�ַ����ƫ����,���»ָ�������ʱ��ַ��һ��, ���Ա���Ϊ�������ĵ�ַ)
	template<typename Target>
	Target* cast(Object* p){
		if (p==nullptr)
			return nullptr;
		auto conver=m_typeMap.get(getObjectTypeInfo(p));
		if(conver==nullptr)
			return nullptr;
		//return static_cast<Target*>((*conver)->cast(p));//Target*Ϊchar*֮���ʱ��ᱨ��
		return (Target*)((*conver)->cast(p));
	}

	//ע��:�����������Ϊ�������͵�ָ��,����Ϊ����ָ��!
	template<typename Target>
	Target* dcast(Object* p){
		if (p==nullptr)
			return nullptr;
		auto conver=m_typeMap.get(getObjectTypeInfo(p));
		if(conver==nullptr)
			return nullptr;
		return dynamic_cast<Target*>((*conver)->cast(p));
	}

	//void* p��ǰ�����Ƿ�Ϊ��ָ�뱾��ĵ�ַ(ֻ�ж�̳��е�2�����༰��������Ż���ֵ�ַƫ��)
	bool objectIsItselfAddr(void* p){
		if (p==nullptr)
			return true;
		TypeInfo t = getTypeInfo(p);
		if (t==nullptr)//p is not an Object*, return true(We think it's a simple type)
			return true;
		auto conver=m_typeMap.get(t);
		if (conver==nullptr)
			throwpe(NotFoundException(String("not registered type: ") + t->name()));
		return (*conver)->isItselfAddr((Object*)p);
	}
protected:
	HashMap<TypeInfo,Conver*> m_typeMap;
};

template<bool T>
struct typeinfo;

template<>
struct typeinfo<true>{
	template<typename T1, typename T2>
	static bool is_same(T1* obj){
		return TypeManager::getTypeInfo(obj) == TypeManager::getTypeInfo<T2>();
	}
};

template<>
struct typeinfo<false>{
	template<typename T1, typename T2>
	static bool is_same(T1* obj){
		return std::is_same<T1, T2>::value;
	}
};

//p����������Ƿ�ΪT2����
template<typename T1, typename T2>
static bool isItselfType(T1* p){
	if (p==nullptr)
		return false;
	const bool convertible = is_convertible<T1*, Object*>::value;
	typeinfo<convertible>::is_same<T1, T2>(p);
}

//p��ǰ�����Ƿ�Ϊ��ָ�뱾�������
template<typename T>
static bool objectIsItselfType(T* p){
	if (p==nullptr)
		return false;

	return TypeManager::getTypeInfo(p) == TypeManager::getTypeInfo<T>();
}

}//end of namespace blib