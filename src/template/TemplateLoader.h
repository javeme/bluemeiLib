#pragma once
#include "Template.h"

namespace bluemei{

class BLUEMEILIB_API TemplateLoader
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

}