#pragma once
#include "StringTemplate.h"

namespace bluemei{

class BLUEMEILIB_API FileTemplate : public Template
{
public:
	FileTemplate(const String& path);
	virtual ~FileTemplate(void);
public:
	virtual String render(const TemplateCtx& ctx);
private:
	StringTemplate* m_tpl;
};

}