#pragma once
#include "Object.h"
#include "HashMap.h"
#include "CriticalLock.h"

namespace bluemei{

class Conver : public Object
{
public:
	virtual Object* cast(void* p)=0;
};

template<typename Self>
class ConvObject : public Conver
{
public:
	ConvObject():typeInfo(typeid(Self)){}
public:
	/*template<typename T>
	Self* operator()(T* v){
		return static_cast<Self*>(v);
	}*/
	Self* operator()(void* v){
		return static_cast<Self*>(v);
	}

	template<typename Target>
	Target* cast(void* v){
		//return static_cast<Target*>(this->operator()(v));//Target*Ϊchar*֮���ʱ��ᱨ��
		return (Target*)(this->operator()(v));
	}
	template<typename Target>
	Target* dcast(void* v){
		return dynamic_cast<Target*>(this->operator()(v));
	}

	virtual Object* cast(void* p){
		return cast<Object>(p);
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

	static TypeInfo getTypeInfo(void* p){
		Object* obj=(Object*)p;
		return &(typeid(*obj));
	}
	template<typename T>
	static TypeInfo getTypeInfo(){
		return &(typeid(T));
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
		if(!typeMap.contain(k))
		{
			auto v=new ConvObject<T>();
			return typeMap.put(k,v);
		}
		return false;
	}

	void destroy();

	template<typename Target>
	Target* cast(void* p){//ע��:�����������Ϊ�������͵�ָ��,����Ϊ����ָ��!(��Ϊpָ��ĵ�ַ����Ϊ�������ĵ�ַ,������,�ڶ�̳�ʱ�����)
		if (p==nullptr)
			return nullptr;
		auto conver=typeMap.get(getTypeInfo(p));
		if(conver==nullptr)
			return (Target*)p;
		//return static_cast<Target*>((*conver)->cast(p));//Target*Ϊchar*֮���ʱ��ᱨ��
		return (Target*)((*conver)->cast(p));
	}
	template<typename Target>
	Target* dcast(void* p){
		if (p==nullptr)
			return nullptr;	
		auto conver=typeMap.get(getTypeInfo(p));
		if(conver==nullptr)
			return (Target*)p;
		return dynamic_cast<Target*>((*conver)->cast(p));
	}


protected:
	HashMap<TypeInfo,Conver*> typeMap;
};


}//end of namespace bluemei