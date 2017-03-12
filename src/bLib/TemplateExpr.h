#ifndef TemplateExpr_H_H
#define TemplateExpr_H_H

#include "HashMap.h"

namespace blib{

typedef HashMap<String, String> TemplateCtx;

class BLUEMEILIB_API TemplateExpr : public Object
{
public:
	TemplateExpr(){}
	virtual ~TemplateExpr(){}
public:
	virtual String eval(const TemplateCtx& ctx)=0;
};

class BLUEMEILIB_API StringExpr : public TemplateExpr
{
public:
	StringExpr(const String& val):m_val(val){}
	virtual ~StringExpr(){}
public:
	String getValue() const { return m_val; }
	//void setValue(const String& val) { m_val = val; }

	virtual String eval(const TemplateCtx& ctx) { return m_val; }
private:
	String m_val;
};

class BLUEMEILIB_API EmptyExpr : public StringExpr
{
public:
	EmptyExpr() : StringExpr(""){}
	virtual ~EmptyExpr(){}
};

class BLUEMEILIB_API VarExpr : public TemplateExpr
{
public:
	VarExpr(const String& name):m_varName(name){}
	virtual ~VarExpr(){}
public:
	String getName() const { return m_varName; }
	//void setName(const String& name) { m_varName = name; }

	virtual String eval(const TemplateCtx& ctx) {
		String undefindVar = "<undefind " + m_varName + ">";
		return ctx.getDefault(m_varName, undefindVar);
	}
private:
	String m_varName;
};

class BLUEMEILIB_API JoinerExpr : public TemplateExpr
{
public:
	JoinerExpr(TemplateExpr* left, TemplateExpr* right)
		: m_leftExpr(left), m_rightExpr(right)
	{
	}
	virtual ~JoinerExpr()
	{
		delete m_leftExpr;
		delete m_rightExpr;
	}
public:
	TemplateExpr* getLeftExpr() const { return m_leftExpr; }
	//void setLeftExpr(Expression* val) { m_leftExpr = val; }

	TemplateExpr* getRightExpr() const { return m_rightExpr; }
	//void setRightExpr(Expression* val) { m_rightExpr = val; }

	virtual String eval(const TemplateCtx& ctx) {
		checkNullPtr(m_leftExpr);
		checkNullPtr(m_rightExpr);
		return m_leftExpr->eval(ctx) + m_rightExpr->eval(ctx);
	}
private:
	TemplateExpr* m_leftExpr;
	TemplateExpr* m_rightExpr;
};

}//namespace blib

#endif
