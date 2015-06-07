#include "File.h"

#include <stdio.h>
#include <stdlib.h>
#include "ErrorHandler.h"

namespace bluemei{

File::File(size_t bufferSize)
{
	init("","",bufferSize);
}
/*
File::File(string path,string openMode)
{
	this->File::File(path,openMode,BUFFER_SIZE);
}
*/
File::File(String path,String openMode,size_t bufferSize)
{
	init(path,openMode,bufferSize);
}
File::~File()
{
	try{
		close();
	}catch (Exception& e){
		ErrorHandler::handle(e);
	}
	if(buffer)
		delete[]buffer;
}
void File::init(String path,String openMode,size_t bufferSize)
{
	m_bCloseAble=false;
	m_nBufferSize=bufferSize;
	m_nUsedBufLength=0;
	buffer=new char[m_nBufferSize];
	m_pFile=NULL;
	if(path!=PATH_NULL)
	{
		try{
			openFile(path,openMode);
		}catch(...)
		{
			delete[] buffer;
			buffer=NULL;
			throw;
		}
	}
}
void File::openFile(String path,String openMode)
{
	//path="./root/"+path;
	if(path==PATH_NULL)
		throw IOException("can't open file: path-name is null.");
	m_pFile=fopen(path.c_str(),openMode.c_str());
	if(m_pFile==NULL)
		throw IOException("'"+path+"'file open failed.");
	m_bCloseAble=true;
	m_strPath=path;
}
size_t File::writeBytes(const char buf[],size_t length)
{
	if(m_nUsedBufLength<0)
	{
		throw IOException("m_nUsedBufLength<0 when write into "+m_strPath);
	}
	else if(length>(m_nBufferSize>>1))//大于二分之一缓冲区大小
	{
		flush();
		size_t nReturn=::fwrite(buf,sizeof(char),length,m_pFile);
		if(nReturn!=length)
		{
			String str=String::format("fwrite %d but return %d",length,nReturn);
			throw IOException("write bytes into "+m_strPath+" failed -"+str);
		}
		return length;
	}
	else//小于二分之一缓冲区大小
	{
		size_t emptyLen=0;
		if(m_nUsedBufLength+length>m_nBufferSize)//如果溢出,则先存入部分
		{
			emptyLen=m_nBufferSize-m_nUsedBufLength;
			memcpy(buffer+m_nUsedBufLength,buf,emptyLen);
			m_nUsedBufLength+=emptyLen;
			flush();//m_nUsedBufLength=0
		}
		memcpy(buffer+m_nUsedBufLength,buf+emptyLen,length-emptyLen);
		m_nUsedBufLength+=(length-emptyLen);
		return length;
	}
	/*
	for(int i=0;i<length;i++)
	{
		buffer[m_nUsedBufLength++]=buf[i];
		if(m_nUsedBufLength >= m_nBufferSize)
			this->flush();
	}*/

}
void File::writeLine(cstring value)
{
	writeBytes(value, strlen(value));
	writeBytes(CRLF,strlen(CRLF));
}
void File::writeLine(const String& value)
{
	writeBytes(value.c_str(),value.length());
	writeBytes(CRLF,strlen(CRLF));
}

void File::writeInt(int value)
{
	char buf[sizeof(int)];
	for(int i=0;i<sizeof(value);i++)
	{
		buf[i]=(value>>i*8) & 0xff;
	}
	writeBytes(buf,sizeof(int));
}

size_t File::readBytes(char buf[],size_t length)
{
	size_t nReturn=::fread(buf,sizeof(char),length,m_pFile);
	if(nReturn<0)
	{
		String str=String::format("expected %d actual %d",length,nReturn);
		throw IOException("failed to read bytes from ["+m_strPath+"]: "+str);
	}
	return nReturn;
}
size_t File::readLine(String& line)
{
	int readedLen=0,tmp;
	char buf[BUFFER_SIZE]="";
	do
	{
		tmp=readBytes(&buf[readedLen],1);
		if(tmp>0)
		{
			if(buf[readedLen]!='\r')
				readedLen+=tmp;
		}
		else if(tmp==0)//文件结束
		{
			if(readedLen==0)
				return -1;
		}
		if(readedLen>BUFFER_SIZE)
			throw IOException("read buffer full.");
	}while(tmp>0 && buf[readedLen-1]!='\0' && buf[readedLen-1]!='\n');	
	if(readedLen>0)
	{		
		if(tmp>0)//非文件结束
			readedLen-=1;
	}
	line=String(buf,readedLen);
	return readedLen+1;
}

size_t File::readLine(std::string& line)
{
	String str;
	size_t len=readLine(str);
	line=str.c_str();
	return len;
}

int File::readInt()
{
	int value=0;
	char buf[sizeof(int)];
	if(sizeof(int) != readBytes(buf,sizeof(int)))
		throw IOException("have not read enough bytes when readInt");
	for(int i=0;i<sizeof(int);i++)
	{
		value+=(buf[i] & 0xff)<<8*i;
	}
	return value;
}

size_t File::readAll(char buf[],size_t bufSize)
{
	if(bufSize<getSize())
	{
		return -1;
	}
	return this->readBytes(buf,getSize());
}

size_t File::currentPos()const
{
	//return ftell(m_pFile);
	fpos_t pos=0;
	if(::fgetpos(m_pFile,&pos)!=0)
	{
		String str=String::format("get file position failed,error:%d",errno);
		throw IOException(str+m_strPath);
	}
	return (size_t)pos;
}

void File::setPos(size_t pos)
{
	fpos_t fpos=pos;
	if(::fsetpos(m_pFile,&fpos)!=0)
	{
		String str=String::format("set file position failed,error:%d",errno);
		throw IOException(str+m_strPath);
	}
}

size_t File::getSize()
{
	if(::fseek(m_pFile,0,SEEK_END)==0) 
	{
		//int length=ftell(m_pFile);
		size_t length=currentPos();
		::rewind(m_pFile);//重定位到开头
		return length;
	}
	else
		return -1;
}
void File::seek(long offset)
{
	if((::fseek(m_pFile,offset,SEEK_CUR))!=0)//SEEK_SET
	{
		String str=String::format("seek file failed when seek the pos %d of ",offset);
		throw IOException(str+m_strPath);
	}
}

size_t File::write(const byte* buffer,size_t length)
{
	//return ::fwrite(buffer,sizeof(byte),length,m_pFile);
	size_t nReturn=::fwrite(buffer,sizeof(byte),length,m_pFile);
	if(nReturn!=length)
	{
		String str=String::format("expected %d actual %d",length,nReturn);
		throw IOException("failed to write bytes into ["+m_strPath+"]: "+str);
	}
	return nReturn;
}

void File::flush()
{
	if(m_nUsedBufLength==0)
		return;
	(void)this->write((const byte*)buffer, m_nUsedBufLength);	
	m_nUsedBufLength=0;
}

void File::close()
{
	if(!m_bCloseAble)
		return;
	flush();
	if(::fclose(this->m_pFile))
		throw IOException("close file ["+m_strPath+"] failed.");
	m_bCloseAble=false;
}

String File::filePath() const
{
	return m_strPath;
}

}//namespace bluemei