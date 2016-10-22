#pragma once
#include "bluemeiLib.h"
#include "File.h"
#include "Object.h"
#include "StringBuilder.h"

namespace bluemei{

class BLUEMEILIB_API SimpleCfgFile : public Object
{
public:
	SimpleCfgFile(const String& path);
	~SimpleCfgFile(void);
private:
	Map<String,String> m_propertiesMap;
	String m_filePath;
	StringBuilder m_content;
	bool m_isChanged;
public:
	void readPropertyFromFile(const String& path);
public:
	bool getProperty(const String& key,String& value);
	bool setProperty(const String& key,const String& value);
	bool removeProperty(const String& key);
	void saveProperty();
};

}//end of namespace bluemei