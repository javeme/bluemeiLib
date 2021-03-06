#pragma once
#include "Thread.h"
#include "Date.h"
#include <functional>
#include <queue>

namespace bluemei{

class BLUEMEILIB_API Message : public Object
{
public:
	Message(){
		init(0,0,0,0,nullptr);
	}
	Message(int id){
		init(0,id,0,0,nullptr);
	}
	Message(int id,int priority){
		init(priority,id,0,0,nullptr);
	}
	Message(int id,int arg1,int arg2){
		init(0,id,arg1,arg2,nullptr);
	}
	Message(int id,int arg1,int arg2,Object* object){
		init(0,id,arg1,arg2,object);
	}
	Message(int id,int priority,int arg1,int arg2,Object* object){
		init(priority,id,arg1,arg2,object);
	}
	virtual ~Message(){}
public:
	void init(int priority,int id,int arg1,int arg2,Object* object){
		m_priority=priority;
		m_id=id;
		m_arg1=arg1;
		m_arg2=arg2;
		m_object=object;
		//m_timestamp=Date::getCurrentTime().getTotalMillSecond();
		static decltype(m_timestamp) count=0;
		m_timestamp=count++;
	}
	virtual void release(){
		delete m_object;
		m_object=nullptr;
		delete this;
	}
public:
	int getPriority() const { return m_priority; }
	void setPriority(int val) { m_priority = val; }

	int  getId() const { return m_id; }
	void setId(int val) { m_id = val; }

	int  getArg1() const { return m_arg1; }
	void setArg1(int val) { m_arg1 = val; }

	int  getArg2() const { return m_arg2; }
	void setArg2(int val) { m_arg2 = val; }

	Object* getObject() const { return m_object; }
	void setObject(Object* val) { m_object = val; }

	unsigned long long getTimestamp() const { return m_timestamp; }
	void setTimestamp(unsigned long long val) { m_timestamp = val; }
protected:
	int m_priority;
	int m_id;
	int m_arg1,m_arg2;
	Object* m_object;
	unsigned long long m_timestamp;
};

typedef std::function<void (Message* msg)> MessageFunction;

class BLUEMEILIB_API MessageThread : public Thread
{
public:
	MessageThread();
	MessageThread(const MessageFunction& fun);
	virtual ~MessageThread();
private:
	MessageThread(const MessageThread& other);
	MessageThread& operator=(const MessageThread& other);
public:
	virtual void addMessage(Message* msg);
	virtual void clearMessage();
	virtual Message* peekMessage();
	virtual Message* nextMessage();
	virtual bool hasMessage()const;
public:
	virtual void run(ThreadParameter *pThreadParameter);
protected:
	virtual void doMessageLoop();
protected:
	virtual void onMessage(Message* msg);
protected:
	struct cmp{
		bool operator()(const Message* a,const Message* b)const{
			//最大堆
			if(a->getPriority() == b->getPriority())
				//时间大的优先级小(同一毫秒如何处理,溢出又如何处理?)
				return a->getTimestamp() > b->getTimestamp();
			else
				return a->getPriority() < b->getPriority();
		}
	};
	std::priority_queue<Message*,vector<Message*>,cmp> m_messageQueue;
	MessageFunction m_msgFun;
private:
	CriticalLock m_queueLock;
};

}//end of namespace bluemei