#pragma once
#include "bluemeiLib.h"
#include "Object.h"
#include "Session.h"
#include "Application.h"

namespace bluemei{

typedef std::map<string,Session*> SessionMap;

class BLUEMEILIB_API Context : public Object
{
protected:
	Context(void);
	Context(const Context& other){};
	Context& operator=(const Context& other){return *this;};
public:
	virtual ~Context(void);
public:
	static Context& getInstance();
	Application& getApplication();
	Session& getSession(const string& name);
	bool removeSession(const string& name);
	Session& putNewSession(const string& name);
	SessionMap& getAllSession();
	void clear();//������ڶ��߳�, ���ȵ��øú�����������!!!
protected:
	static Context staticContext;
	Application m_application;
	SessionMap m_sessionMap;
};



}//end of namespace bluemei