#ifndef FileTemplate_H_H
#define FileTemplate_H_H

#include "StringTemplate.h"

namespace blib{

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

}//namespace blib

#endif
