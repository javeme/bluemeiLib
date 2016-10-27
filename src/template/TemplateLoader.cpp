#include "TemplateLoader.h"
#include "Exception.h"
#include "StringTemplate.h"
#include "FileTemplate.h"

namespace bluemei{

TemplateLoader::TemplateLoader(void)
{
}

TemplateLoader::~TemplateLoader(void)
{
}

Template* TemplateLoader::load(const String& val, const String& type)
{
	if(type == "" || type == "string")
	{
		return new StringTemplate(val);
	}
	else if(type == "file")
	{
		return new FileTemplate(val);
	}
	else
	{
		throwpe(UnsupportedType(type));
	}
}

}