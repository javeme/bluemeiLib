#include "Util.h"
#include "RuntimeException.h"
#include <ShellApi.h>
#include <time.h>

namespace bluemei{

Util::Util(void)
{
}

Util::~Util(void)
{
}

/*********************************�ַ���������******************************/
  
//�滻�ַ���:��strToReplace�����strSrc�ִ��滻��strDst
void Util::replaceString(string& strToReplace, const string& strSrc, const string& strDst)
{ 
	if(strSrc.length()==0)
		return;
	string::size_type pos=0; 
	string::size_type srcLen=strSrc.size(); 
	string::size_type dstLen=strDst.size();
	while( (pos=strToReplace.find(strSrc, pos)) != string::npos)
	{  
		strToReplace.replace(pos, srcLen, strDst);  
		pos += dstLen;
	}
} 
//�滻��β�ո�
void Util::trim(string& str)
{
	int pos;
	while((pos=str.find(" "))==0)
	{
		if(str.length()==0)
			break;
		str.erase(pos,1);
	}
	while((pos=str.rfind(" "))==str.length()-1)
	{
		if(str.length()==0)
			break;
		str.erase(pos,1);
	}
}
short Util::getChar(int& pos,const string& str)
{
	short tmpChar=0;
	tmpChar=str.at(pos);
	//����
	if(tmpChar<0)
	{
		tmpChar<<=8;
		tmpChar+=str.at(++pos);
	}
	return tmpChar;
}
//����ƥ��
int Util::match(const string& value,const string& regex)//������...
{
	int matchedPos=-1,pos1=0,pos2=0;
	short char1,char2;
	while(true)
	{
		char1=getChar(pos1,value);
		char2=getChar(pos2,regex);
	
		//���
		if(char1==char2)
		{
			matchedPos=pos1;
			pos1++;
			pos2++;
		}
		else if(char2=='*')
		{
			matchedPos=pos1;
			pos1++;
			//�ж�regex����һ���ַ�
			if(pos1==value.length())
			{
				break;
			}
			else if(pos2+1!=regex.length())
			{
				char1=getChar(pos1,value);
				pos2++;
				char2=getChar(pos2,regex);
				//�����һ���ַ���ͬ,��ǰ��	
				if(char1==char2)
				{	
					matchedPos=pos1;
					pos1++;
					pos2++;
				}
				else
				{
					if(char1<0)
						pos1--;					
					if(char2>0)
						pos2-=1;
					else 
						char2-=2;
				}
			}
		}
		else
		{
			//pos2++;
			break;
		}
		if(pos1==value.length() || pos2==regex.length())
			break;
	}
	return matchedPos;
}
/************************************�ԳƼӽ���***********************************/
bool Util::simpleEncode(string& plaintext,const string& code)//����->����
{
	int plaintextLen=plaintext.length();
	int codeLen=code.length();//���볤��
	unsigned short tmp1,tmp2;
	for(int i=0, j=0;i<plaintextLen;j++,i++)
	{
		if(j>=codeLen)
			j=0;
		tmp1=plaintext[i] & 0x00ff;
		tmp2=code[j] & 0x00ff;
		plaintext[i]=(char)(tmp1+tmp2)%256;
	}	
	return true;
}
bool Util::simpleDecode(string& cipher,const string& code)//����->����
{
	int cipherLen=cipher.length();
	int codeLen=code.length();//���볤��
	unsigned short tmp1,tmp2;
	for(int i=0, j=0;i<cipherLen;j++,i++)
	{
		if(j>=codeLen)
			j=0;
		tmp1=(cipher[i] & 0x00ff);
		tmp2=code[j] & 0x00ff;
		cipher[i]=(char)(tmp1-tmp2)%256;
	}	
	return true;
}
/************************************ϵͳ���*************************************/
//��ȡ��ǰĿ¼��·��
bool Util::getSelfPath(string& path)
{
	char strModulePath[MAX_PATH];	
	//::GetCurrentDirectory(MAX_PATH,strModulePath.GetBuffer(MAX_PATH));
	DWORD dwResult = ::GetModuleFileNameA(NULL,strModulePath,MAX_PATH);
	if(dwResult==0)
		return false;
	//strModulePath = strModulePath.Left(strModulePath.ReverseFind(_T('\\')) + 1);
	path=strModulePath;
	int pos=path.rfind("\\");
	if(pos<0)
		return false;
	path=path.substr(0,pos+1);
	return true;
}
//�򿪿�ִ���ļ�
bool Util::open(const string& name,const string& arg)
{	
	/*wstring op,uStrName,uStrArg;
	SocketTools::gbkToUnicode(op,"open");
	SocketTools::gbkToUnicode(uStrName,name.c_str());
	SocketTools::gbkToUnicode(uStrArg,arg.c_str());
	const wchar_t *pArg=uStrArg.length()==0?NULL:uStrArg.c_str();
	ShellExecute(NULL,op.c_str() ,uStrName.c_str(),pArg,NULL,SW_SHOW); */
	HINSTANCE h=ShellExecuteA(NULL,"open" ,name.c_str(),arg.c_str(),NULL,SW_SHOW); 
	return (int)h>32;
}

int Util::str2Int(cstring str)
{
	if(!isIntNumber(str,strlen(str))){
		throw BadCastException(String::format("string '%s' can not be converted to an integer",str));
	}
	return ::atoi(str);
}

string Util::int2Str(int i)
{
	char buf[64];
	sprintf_s(buf,"%d",i);
	return buf;
}

double Util::str2Float(cstring str)
{
	if(!isFloatNumber(str,strlen(str))){
		throw BadCastException(String::format("string '%s' can not be converted to floating point number",str));
	}
	double f=::atof(str);
	return f;
}

bluemei::string Util::float2Str(double f)
{
	char buf[256];
	sprintf_s(buf,"%lf",f);
	return buf;
}

int Util::random()
{
	srand((unsigned int)time(NULL));//�趨���������
	return rand();
}

}//end of namespace bluemei