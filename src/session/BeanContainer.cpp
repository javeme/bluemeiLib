#include "StdAfx.h"
#include "BeanContainer.h"

namespace blib{

BeanContainer::BeanContainer(void)
{
}

BeanContainer::~BeanContainer(void)
{
	clear();
}

bool BeanContainer::put(const string& key,Object* pBean)
{
	return m_beanMap.insert(make_pair(key,pBean)).second;
}

Object* BeanContainer::get(const string& key) const
{
	auto& itor=m_beanMap.find(key);
	if(itor!=m_beanMap.end())
		return itor->second;
	return nullptr;
}

Object* BeanContainer::remove(const string& key)
{
	Object* obj=nullptr;
	auto& itor=m_beanMap.find(key);
	if(itor!=m_beanMap.end()){
		obj=itor->second;
		m_beanMap.erase(itor);
	}
	return obj;
}

bool BeanContainer::remove(Object* pBean)
{
	auto& itor=m_beanMap.begin();
	for(;itor!=m_beanMap.end();itor++){
		if(itor->second==pBean){
			m_beanMap.erase(itor);
			return true;
		}
	}
	return false;
}

void BeanContainer::clear(bool autoRelease)
{
	if(autoRelease)
	{
		for(auto iter=m_beanMap.begin();iter!=m_beanMap.end();)
		{
			Object* obj=iter->second;
			//delete obj;//delete iter->second;//跨模块删除可能引起错误
			if(obj!=nullptr)
				obj->release();
			m_beanMap.erase(iter++);
		}
	}
	else
		m_beanMap.clear();
}

Object* BeanContainer::operator[](const string& key) const
{
	return get(key);
}

Object*& BeanContainer::operator[](const string& key)
{
	return m_beanMap[key];
}

}//end of namespace blib