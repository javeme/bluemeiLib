#pragma once
#include "TemplateExpr.h"

namespace bluemei{

class BLUEMEILIB_API Template : public Object
{
public:
	Template(void){};
	virtual ~Template(void){};
public:
	virtual String render(const TemplateCtx& ctx)=0;
};

}