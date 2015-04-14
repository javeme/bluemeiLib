#pragma once
#include "bluemeiLib.h"
#include "File.h"
#include "Object.h"

namespace bluemei{

using std::string;
#define PropertiesMap map<string,string>

class BLUEMEILIB_API SimpleCfgFile : public Object
{
public:
	SimpleCfgFile(const string& path);
	~SimpleCfgFile(void);
private:
	PropertiesMap propertiesMap;
	string filePath;	
	string content;
	bool isChanged;
public:
	void readPropertyFromFile(const string& path);
public:	
	bool getProperty(const string& key,string& value);//��ȡ��������		
	bool setProperty(const string& key,const string& value);//������������
	bool removeProperty(const string& key);
	void saveProperty();
};

}//end of namespace bluemei