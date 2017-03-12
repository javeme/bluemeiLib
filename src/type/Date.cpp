#include "StdAfx.h"
#include "Date.h"
#include "Util.h"
#include "Exception.h"

namespace blib{

//解析时间的宏替换
#define PARSE_TIME()																				\
{																									\
	if(i==len-1)																					\
	{																								\
		tmpStr=strTime.substring(parsePos);															\
	}																								\
	else																							\
	{																								\
		int endPos=strTime.find(emptyStr+format[i+1],parsePos);										\
		if(endPos<parsePos)																			\
			throw Exception("failed to parse the date string, it does not match the format.");		\
		tmpStr=strTime.substring(parsePos,endPos-parsePos);											\
		parsePos=endPos;																			\
	}																								\
}

Date::Date(void)
{
	initDate();
}

Date& Date::operator=(long long ms)
{
	initDate();
	m_time.time=ms/1000;
	m_time.millitm=ms%1000;
	getDateInfo();
	return *this;
}

Date::~Date(void)
{
}

void Date::initDate()
{
	memset(&m_time,0,sizeof(m_time));
	//memset(&m_dateInfo,0,sizeof(m_dateInfo));

	//m_timeInfo.tm_isdst=0;
	setDay(1);
}

Date Date::getCurrentTime()
{
	Date date;
	date.setToCurrentTime();
	return date;
}

int Date::month2Int(const char* month)
{
	static const char* MONTHS[] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
	};
	for(int i=0; i<sizeof(MONTHS); i++)
	{
		if (strcmp(MONTHS[i], month) == 0)
			return i + 1;
	}
	return -1;
}

Date Date::parseDate( const char* format,const char* strDate )
{
	enum CharType{NORMAL,BEFORE_PARSE,PARSED};
	Date date;

	String strTime=strDate,tmpStr;
	int parsePos=0;
	String emptyStr;
	cstring errMsg = "failed to parse the date string: not matched the format.";

	char c;
	CharType type=NORMAL;
	int len=strlen(format);
	for(int i=0;i<len;i++)
	{
		c=format[i];
		switch(c)
		{
		case '%'://"现在是:%Y-%m-%d %H:%M:%S %i" --> "现在是:2010-10-10 12:59:59 990"
			type=BEFORE_PARSE;
			continue;
			break;
		case 'Y':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setYear(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'm':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setMonth(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'b'://"Nov"
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setMonth(month2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'd':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setDay(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'H':
		case 'I':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setHour(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'M':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setMinu(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'S':
		case 's':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setSecond(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'i':
			if(type==BEFORE_PARSE)
			{
				PARSE_TIME();
				date.setMillisecond(Util::str2Int(tmpStr));
				type=PARSED;
			}
			break;
		case 'a'://Sunday:"Sun"
			if(type==BEFORE_PARSE)
			{
				parsePos += 3;
				type=PARSED;
			}
			break;
		default:
			if(type != BEFORE_PARSE)
				type=NORMAL;
			break;
		}
		if(type == NORMAL){
			if(c!=strTime[parsePos])
				throw Exception(errMsg);
			parsePos++;
		}
		else if(type == BEFORE_PARSE)
			throw Exception(errMsg);
	}
	return date;
}

long Date::operator-(const Date& other) const
{
	//.time秒数, .timezone时区 以分钟数表示
	time_t second=(m_time.time - other.m_time.time) +
		((m_time.timezone - other.m_time.timezone)*60*1000);
	time_t span=second*1000+this->m_time.millitm-other.m_time.millitm;
	return (long)span;
}

Date Date::operator+(long ms) const
{
	return Date(this->getTotalMillSecond() + ms);
}

bool Date::operator<(const Date& other) const
{
	long long offset = 0;
	if(m_time.timezone != other.m_time.timezone)
		offset = ((m_time.timezone - other.m_time.timezone)*60*1000*1000);
	return getTotalMillSecond() + offset < other.getTotalMillSecond();
}

String Date::formatDate(const char* format) const
{
	String str=format;
	int pos=str.find("%i");//毫秒
	if(pos>=0)
	{
		char msec[5];
		snprintf(msec,sizeof(msec),"%d",m_time.millitm);
		str.replace("%i",msec);
	}
	DateInfo m_dateInfo=getDateInfo();
	int size=str.length()+64;
	char* buf=new char[size];
	strftime(buf,size,str.c_str(),&m_dateInfo);
	String strDate=buf;
	delete[] buf;
	return strDate;
}
String Date::toString() const
{
	DateInfo m_dateInfo=getDateInfo();
	//2012-05-04 15:37:59.999
	String str=String::format("%d-%02d-%02d %02d:%02d:%02d.%d",m_dateInfo.tm_year+FROM_YEAR,m_dateInfo.tm_mon+1,m_dateInfo.tm_mday,
		m_dateInfo.tm_hour,m_dateInfo.tm_min,m_dateInfo.tm_sec,m_time.millitm);
	return str;
}

void Date::setToCurrentTime()
{
	ftime(&m_time);

	//m_dateInfo=*localtime(&m_time.time);
}

DateInfo Date::getDateInfo() const
{
	DateInfo dateInfo;
	localtime_s(&dateInfo, &m_time.time);
	return dateInfo;
}

long long Date::getTotalMillSecond() const
{
	return m_time.time*1000+m_time.millitm;
}


void Date::setYear( int i )
{
	DateInfo m_dateInfo=getDateInfo();
	m_dateInfo.tm_year=i-FROM_YEAR;
	m_time.time=mktime(&m_dateInfo);//自1970年1月1日0时0分0 秒秒数
}

void Date::setMonth( int i )
{
	DateInfo m_dateInfo=getDateInfo();
	m_dateInfo.tm_mon=(i-1)%12;
	m_time.time=mktime(&m_dateInfo);
}

void Date::setDay( int i )
{
	DateInfo m_dateInfo=getDateInfo();
	i-=1;//[1-31]转到[0-30]
	m_dateInfo.tm_mday=i%31+1;
	m_time.time=mktime(&m_dateInfo);
}

void Date::setHour( int i )
{
	DateInfo m_dateInfo=getDateInfo();
	m_dateInfo.tm_hour=i%24;
	m_time.time=mktime(&m_dateInfo);
}

void Date::setMinu( int i )
{
	DateInfo m_dateInfo=getDateInfo();
	m_dateInfo.tm_min=i%60;
	m_time.time=mktime(&m_dateInfo);
}

void Date::setSecond( int i )
{
	DateInfo m_dateInfo=getDateInfo();
	m_dateInfo.tm_sec=i%60;
	m_time.time=mktime(&m_dateInfo);
}

void Date::setMillisecond( int i )
{
	m_time.millitm=i%1000;
}
/*
Date d;
//d=GetTickCount();
d.setToCurrentTime();
d=d.getTotalMillSecond();
TRACE("%s\n",d.toString().c_str());
d.setDay(6);
TRACE("%s\n",d.toString().c_str());
d.setYear(2008);
TRACE("%s\n",d.toString().c_str());
d.setSecond(28);
d.setMillisecond(995);
d.getDateInfo();
TRACE("%s\n",d.toString().c_str());
*/

/*
struct date_info{
short year;
byte month;
byte day;
byte hour;
byte minu;
byte sec;
byte reserve;
short ms;
};
int len=sizeof(date_info);
date_info di;
memset(&di,0,len);
*/

}//end of namespace blib
