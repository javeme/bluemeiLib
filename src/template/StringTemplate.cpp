#include "StringTemplate.h"
#include "TemplateParser.h"

namespace bluemei{

StringTemplate::StringTemplate(const String& tpl)
{
	TemplateParser parser;
	m_expr = parser.parse(tpl);
}

StringTemplate::~StringTemplate(void)
{
	delete m_expr;
}

String StringTemplate::render(const TemplateCtx& ctx)
{
	checkNullPtr(m_expr);
	return m_expr->eval(ctx);
}

}//end of namespace bluemei
