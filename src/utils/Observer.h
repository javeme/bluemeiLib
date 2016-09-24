#ifndef Observer_H_H
#define Observer_H_H
#include "Object.h"
#include "Exception.h"
#include "LinkedList.cpp"

namespace bluemei{

//�۲��ߣ�Observer����ɫ
template <class MessageType>
class Observer : public Object
{
public:
	virtual void onNotifyException(Exception& e, Object* pFrom)=0;
	virtual void onNotify(MessageType* pMessage, Object* pFrom)=0;
};

//���⣨Subject����ɫ
template <class MessageType>
class Observable : public Object
{
public:
	Observable(){
		this->m_changed=false;
	}
	virtual ~Observable(){
		unregisterAllObserver();
	}
public:
	virtual void registerObserver(Observer<MessageType>* pObserver){
		if(m_obs.getLocate(pObserver)<0)
			m_obs.addToLast(pObserver);
	}
	virtual void unregisterObserver(Observer<MessageType>* pObserver){
		m_obs.remove(pObserver);
	}
	virtual void unregisterAllObserver()
	{
		m_obs.clear();
	}

	virtual void notifyObservers(Exception& e, Object* pFrom)
	{
		if(!m_changed)
			return;
		clearChanged();

		for(int i=0; i<m_obs.size(); i++)
			m_obs.get(i)->onNotifyException(e,pFrom);
	}
	virtual void notifyObservers(MessageType* pMessage, Object* pFrom)
	{
		if(!m_changed)
			return;
		clearChanged();

		for(int i=0; i<m_obs.size(); i++)
			m_obs.get(i)->onNotify(pMessage,pFrom);
	}

	virtual void setChanged()
	{
		m_changed = true;
	}
	virtual void clearChanged()
	{
		m_changed = false;
	}
	virtual bool hasChanged()
	{
		return m_changed;
	}

	virtual int countObservers()
	{
		return m_obs.size();
	}
protected:
	LinkedList<Observer<MessageType>*> m_obs;
	bool m_changed;
};

}//end of namespace bluemei
#endif