#ifndef StringTemplate_H_H
#define StringTemplate_H_H

#include "Template.h"

namespace blib{

class BLUEMEILIB_API StringTemplate : public Template
{
public:
	StringTemplate(const String& tpl);
	virtual ~StringTemplate(void);
public:
	virtual String render(const TemplateCtx& ctx);
private:
	TemplateExpr* m_expr;
};

}//namespace blib

#endif
