#include "SimpleCfgFile.h"
#include "Util.h"

namespace bluemei{

SimpleCfgFile::SimpleCfgFile(const string& path)
{
	m_isChanged=false;
	readPropertyFromFile(path);
}
SimpleCfgFile::~SimpleCfgFile(void)
{
	this->saveProperty();
}
void SimpleCfgFile::readPropertyFromFile(const string& path)
{
	if(m_isChanged)
		this->saveProperty();

	this->m_filePath = path;
	File file(path,"r");
	m_content="";
	string line,key,value;
	int pos=-1;
	while(file.readLine(line)>0)
	{
		m_content.append(line+"\r\n");
		//ȥ��ע��
		pos=line.find('#');
		if(pos>=0)
		{
			line=line.substr(0,pos);
		}
		pos=line.find('=');
		if(pos>0)// && pos<line.length()-1
		{
			key=line.substr(0,pos);
			value=line.substr(pos+1);
			Util::trim(key);
			Util::trim(value);
			m_propertiesMap.insert(make_pair(key,value));
		}
	}
	file.close();
}
//��ȡ��������
bool SimpleCfgFile::getProperty(const string& key,string& value)
{
	PropertiesMap::iterator it=m_propertiesMap.find(key);
	if(it!=m_propertiesMap.end())
	{
		value=it->second;
		return true;
	}
	else
		return false;
}
//������������
bool SimpleCfgFile::setProperty(const string& key,const string& value)
{
	/*
	��������
	File file(filePath,"wb");
	int size=file.getSize();
	char* buf=new char[size];
	try{
		size=file.readAll(buf,size);
	catch(Exception& e){
		delete []buf;
		throw e;
	}
	content=string(buf,size);
	delete []buf;//*/
	string oldValue;
	if(getProperty(key,oldValue))//�Ѵ���
	{
		//replaceString(content,oldValue,value);//�����滻�������ִ�,��θĽ�?
		int start=m_content.find(key);
		start=m_content.find("=",start);
		start+=1;
		int end=m_content.find(oldValue,start);//��'='��ʼ����
		m_content.replace(end ,oldValue.length() ,value);
		m_isChanged=true;
	}
	else//������,���
	{
		//string pair=key+"="+value+"\r\n";
		m_content.append(key+"="+value+"\r\n");
		m_isChanged=true;
	}
	return m_isChanged;
}
bool SimpleCfgFile::removeProperty(const string& key)
{
	int start=m_content.find(key),end;
	if(start>=0)
	{
		end=m_content.find("\n",start);
		if(end<0)//���һ��
			end=m_content.length();
		if(end>0)
		{
			m_content.replace(start ,end-start+1 ,"");
			m_isChanged=true;
		}
	}
	return m_isChanged;
}
//���ı�����Ա��浽�ļ���ȥ
void SimpleCfgFile::saveProperty()
{
	if(!m_isChanged)
		return;
	//д������
	File file(m_filePath,"w");
	file.writeBytes(m_content.c_str(),m_content.length());
	m_isChanged=false;
}

}//end of namespace bluemei