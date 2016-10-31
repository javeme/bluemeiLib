#include "Util.h"
#include "RuntimeException.h"
#include "Date.h"
#include "HashCoder.h"
#include "CriticalLock.h"

#ifdef WIN32
#include <winsock.h>
#include <ShellApi.h>
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif

namespace blib{

Util::Util(void)
{
}

Util::~Util(void)
{
}

/*********************************字符串处理函数******************************/

//替换字符串:将strToReplace里面的strSrc字串替换成strDst
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

//替换首尾空格
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
	//中文
	if(tmpChar<0)
	{
		tmpChar<<=8;
		tmpChar+=str.at(++pos);
	}
	return tmpChar;
}

//模糊匹配(待完善...)
int Util::match(const string& value,const string& regex)
{
	int matchedPos=-1,pos1=0,pos2=0;
	short char1,char2;
	while(true)
	{
		char1=getChar(pos1,value);
		char2=getChar(pos2,regex);

		//相等
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
			//判断regex的下一个字符
			if(pos1==value.length())
			{
				break;
			}
			else if(pos2+1!=regex.length())
			{
				char1=getChar(pos1,value);
				pos2++;
				char2=getChar(pos2,regex);
				//如果下一个字符相同,则前移
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

/************************************对称加解密***********************************/
bool Util::simpleEncode(string& plaintext,const string& code)//明文->密码
{
	int plaintextLen=plaintext.length();
	int codeLen=code.length();//密码长度
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

bool Util::simpleDecode(string& cipher,const string& code)//密码->明文
{
	int cipherLen=cipher.length();
	int codeLen=code.length();//密码长度
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

/************************************系统相关*************************************/
//获取当前目录的路径
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

//打开可执行文件
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
		throw BadCastException(String::format("string '%s' can not be "
			"converted to floating point number",str));
	}
	double f=::atof(str);
	return f;
}

string Util::float2Str(double f)
{
	char buf[256];
	sprintf_s(buf,"%lf",f);
	return buf;
}

bool Util::str2Boolean(cstring str)
{
	String s(str);
	s = s.toLower();
	if(s.compare("false") || s.compare("no") || s.compare("0"))
		return false;
	else
		return true;
}

string Util::boolean2Str(bool b)
{
	if(b)
		return "true";
	else
		return "false";
}

int Util::random()
{
	//time(NULL)
	static bool notSetSeed = true;
	if(notSetSeed){
		notSetSeed = false;
		long long time = Date::getCurrentTime().getTotalMillSecond();
		srand((unsigned int)time);//set seed
	}
	return rand();
}

//generate a random uuid
string Util::uuid4()
{
	/*from:
	* http://www.ietf.org/rfc/rfc4122.txt
	* http://stackoverflow.com/questions/2174768/generating-random-uuids-in-linux
	*/

	char strUuid[40] = {0};
	sprintf_s(strUuid, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
		random(), random(),				 // Generates a 64-bit Hex number
		random(),						   // Generates a 32-bit Hex number
		((random() & 0x0fff) | 0x4000),	 // Generates a 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
		((random() & 0x3fff) | 0x8000),	 // Generates a 32-bit Hex number in the range [0x8000, 0xbfff]
		random(), random(), random());	  // Generates a 96-bit Hex number

	return strUuid;
}

////////////////////////////////////////////////////////////////
//class Uuid1Generater
class Uuid1Generater : public Object
{
public:
	typedef unsigned short   uint16;
	typedef unsigned int	 uint32;
	typedef __int64		  int64;
	typedef unsigned __int64 uint64;

	/* microsecond per second. 1s=1000000us=1000000000ns*/
	#define USec_Per_Sec		(1000*1000)
	#define USec_Per_MSec	   1000
	#define NSec100_Per_Sec	 (USec_Per_Sec*10)
	#define NSec100_Per_MSec	(USec_Per_MSec*10)
	#define NSec100_Since_1582	 ((uint64)(0x01B21DD213814000))

public:
	Uuid1Generater(){ initialize(); }
	virtual ~Uuid1Generater(){}
private:
	Uuid1Generater(const Uuid1Generater& other)
	{
		this->operator=(other);
	}
	Uuid1Generater& operator=(const Uuid1Generater& other)
	{
		return *this;
	}
public:
	/* generate uuid version 1
	 * from:
	 * http://www.ietf.org/rfc/rfc4122.txt
	 * http://www.tuicool.com/articles/MNrYVri
	 * http://www.cnblogs.com/lidabo/p/3483128.html
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                          time_low                             |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |       time_mid                |         time_hi_and_version   |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                         node (2-5)                            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	*/
	std::string generate()
	{
		lock.getLock();

		int64 time = Date::getCurrentTime().getTotalMillSecond(); // ms
		time = time * NSec100_Per_MSec + NSec100_Since_1582; // unit of 100ns

		if (time < timestamp)
		{
			timestamp = time;
			advance = 0;
			clockseq++;
		}
		else if (time == timestamp)
		{
			advance++;
			time += advance;
		}
		else
		{
			timestamp = time;
			advance = 0;
		}

		uint16 nowseq = clockseq;

		lock.releaseLock();

		uint32 timelow = (uint32) time;
		uint16 timemid = (uint16) ((time >> 32) & 0xffff);
		uint16 timehighver = (uint16) (((time >> 48) & 0x0fff) | 0x1000);
		uint16 clockseqvar = (uint16) ((nowseq & 0x3fff) | 0x8000);//Variant 10x1 1111 1111 1111

		char strUuid[40] = {0};
		sprintf_s(strUuid, "%08x-%04x-%04x-%04x-%04x%08x",
			timelow, timemid, timehighver,
			clockseqvar,
			nodehigh, nodelow);

		return strUuid;
	}
	void initialize()
	{
		/*
		#ifdef _USE_32BIT_TIME_T
			assert(0);
		#endif
		*/

		int64 time = Date::getCurrentTime().getTotalMillSecond(); // ms
		time = time * NSec100_Per_MSec + NSec100_Since_1582; // unit of 100ns

		timestamp = time;
		advance = 0;
		clockseq = (uint16)Util::random();

		char hostname[256] = {0};
		if(gethostname(hostname, sizeof(hostname)) == 0)
		{
			size_t len = strlen(hostname);
			nodehigh = (uint16)hashCode<cstring>(&hostname[len/2]);
			nodelow = (uint32)hashCode<cstring>(hostname);
		}
		else
		{
			nodehigh = Util::random() | 0x0100;
			nodelow = Util::random();
		}
	}
private:
	int64	 timestamp;
	uint32	advance;
	uint16	clockseq;
	uint16	nodehigh;
	uint32	nodelow;

	CriticalLock lock;
};

//generate a uuid version 1
string Util::uuid1()
{
	static Uuid1Generater generater;
	return generater.generate();
}

//generate a guid
blib::string Util::guid()
{
	GUID guid = {0};

#ifdef WIN32
	CoCreateGuid(&guid);//::UuidCreate(&guid);
#else
	uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#endif
	char strUuid[40] = {0};
	sprintf_s(strUuid, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	return strUuid;
}

}//end of namespace blib