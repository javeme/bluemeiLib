#include "SimpleCfgFile.h"
#include "Util.h"

namespace bluemei{
	
SimpleCfgFile::SimpleCfgFile(const string& path)
{	
	isChanged=false;
	readPropertyFromFile(path);
}
SimpleCfgFile::~SimpleCfgFile(void)
{
	this->saveProperty();
}
void SimpleCfgFile::readPropertyFromFile(const string& path)
{
	if(isChanged)
		this->saveProperty();

	this->filePath = path;
	File file(path,"r");
	content="";
	string line,key,value;
	int pos=-1;
	while(file.readLine(line)>0)
	{		
		content.append(line+"\r\n");
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
			propertiesMap.insert(make_pair(key,value)); 
		}
	}
	file.close();
}
//��ȡ��������
bool SimpleCfgFile::getProperty(const string& key,string& value)
{
	PropertiesMap::iterator it=propertiesMap.find(key);
	if(it!=propertiesMap.end()) 
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
		int start=content.find(key);
		start=content.find("=",start);
		start+=1;
		int end=content.find(oldValue,start);//��'='��ʼ����
		content.replace(end ,oldValue.length() ,value); 
		isChanged=true;
	}
	else//������,���
	{
		//string pair=key+"="+value+"\r\n";
		content.append(key+"="+value+"\r\n");
		isChanged=true;
	}
	return isChanged;
}
bool SimpleCfgFile::removeProperty(const string& key)
{
	int start=content.find(key),end;
	if(start>=0)
	{
		end=content.find("\n",start);
		if(end<0)//���һ��
			end=content.length();
		if(end>0)
		{
			content.replace(start ,end-start+1 ,"");
			isChanged=true;
		}
	}
	return isChanged;
}
//���ı�����Ա��浽�ļ���ȥ
void SimpleCfgFile::saveProperty()
{
	if(!isChanged)
		return;
	//д������
	File file(filePath,"w");
	file.writeBytes(content.c_str(),content.length());
	isChanged=false;
}

}//end of namespace bluemei