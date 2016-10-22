#include "SimpleCfgFile.h"
#include "Util.h"

namespace bluemei{

SimpleCfgFile::SimpleCfgFile(const String& path) : m_content(1024)
{
	m_isChanged=false;
	readPropertyFromFile(path);
}
SimpleCfgFile::~SimpleCfgFile(void)
{
	this->saveProperty();
}
void SimpleCfgFile::readPropertyFromFile(const String& path)
{
	if(m_isChanged)
		this->saveProperty();

	this->m_filePath = path;
	File file(path,"r");
	String line,key,value;
	int pos=-1;
	while(file.readLine(line)!=-1)
	{
		m_content.append(line+"\r\n");
		//去除注释
		pos=line.find("#");
		if(pos>=0)
		{
			line=line.substring(0,pos);
		}
		pos=line.find("=");
		if(pos>0)// && pos<line.length()-1
		{
			key=line.substring(0,pos).trim();
			value=line.substring(pos+1).trim();
			m_propertiesMap.insert(std::make_pair(key,value));
		}
	}
	file.close();
}
//获取配置属性
bool SimpleCfgFile::getProperty(const String& key,String& value)
{
	auto it=m_propertiesMap.find(key);
	if(it!=m_propertiesMap.end())
	{
		value=it->second;
		return true;
	}
	else
		return false;
}
//设置配置属性
bool SimpleCfgFile::setProperty(const String& key,const String& value)
{
	/*
	读入内容
	File file(filePath,"wb");
	int size=file.getSize();
	char* buf=new char[size];
	try{
		size=file.readAll(buf,size);
	catch(Exception& e){
		delete []buf;
		throw e;
	}
	content=String(buf,size);
	delete []buf;//*/
	String oldValue;
	if(getProperty(key,oldValue))//已存在
	{
		if(value==oldValue)
			return false;
		//replaceString(content,oldValue,value);//可能替换掉其它字串,如何改进?
		int start=m_content.indexOf(key);
		start=m_content.indexOf("=",start);
		start+=1;
		int end=m_content.indexOf(oldValue,start);//从'='后开始查找
		m_content.replace(end ,oldValue.length() ,value);
		m_isChanged=true;
	}
	else//不存在,添加
	{
		//String pair=key+"="+value+"\r\n";
		m_content.append(key+"="+value+"\r\n");
		m_isChanged=true;
	}
	return m_isChanged;
}
bool SimpleCfgFile::removeProperty(const String& key)
{
	int start=m_content.indexOf(key);
	int end=0;
	if(start>=0)
	{
		end=m_content.indexOf("\n",start);
		if(end<0)//最后一行
			end=m_content.length();
		if(end>0)
		{
			m_content.replace(start ,end-start+1 ,"");
			m_isChanged=true;
		}
	}
	return m_isChanged;
}
//将改变的属性保存到文件中去
void SimpleCfgFile::saveProperty()
{
	if(!m_isChanged)
		return;
	//写入内容
	File file(m_filePath,"w");
	file.writeBytes(m_content.toString().c_str(),m_content.length());
	m_isChanged=false;
}

}//end of namespace bluemei