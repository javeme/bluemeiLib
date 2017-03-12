#ifndef Template_H_H
#define Template_H_H

#include "TemplateExpr.h"

namespace blib{

class BLUEMEILIB_API Template : public Object
{
public:
	Template(void){};
	virtual ~Template(void){};
public:
	virtual String render(const TemplateCtx& ctx)=0;
};

}//namespace blib

#endif
