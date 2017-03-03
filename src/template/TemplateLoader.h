#ifndef TemplateLoader_H_H
#define TemplateLoader_H_H

#include "Template.h"

namespace blib{

class BLUEMEILIB_API TemplateLoader : public Object
{
public:
	TemplateLoader(void);
	virtual ~TemplateLoader(void);
public:
	virtual Template* load(const String& val, const String& type="");
};

class BLUEMEILIB_API UnsupportedType : public Exception
{
public:
	UnsupportedType(const String& type):Exception(type){}
	virtual String name() const {
		return "UnsupportedType";
	}
};

}//namespace blib

#endif
