#pragma once
#include "stdafx.h"
#include "IOCPModel.h"
#include "SocketTools.h"
using namespace bluemei;


// test for server
class IOCPTester : public IOCPEventHandle
{
public:
	IOCPTester(){
		SocketTools::initSocketContext();

		model=new IOCPModel();
		model->setEventHandle(this);
	}
	virtual ~IOCPTester(){
		model->setEventHandle(nullptr);
		delete model;

		SocketTools::cleanUpSocketContext();
	}
	void start(){
		model->listen(8080);
	}
	void stop(){
		model->unlisten();
	}

public:
	virtual bool onWrite( IOEvent& ev ) 
	{
		printf("onWrite()\n");

		return true;
	}

	virtual bool onRead( IOEvent& ev ) 
	{
		printf("onRead()\n");

		return true;
	}
	
	virtual bool onAccept( ClientSocket*& client,IOEvent& ev ) 
	{
		return false;
	}

	virtual bool onError( IOEvent& ev ) 
	{
		printf("onError()\n");

		return true;
	}

	virtual bool onClosed( IOEvent& ev ) 
	{
		printf("onClosed()\n");

		return false;
	}

	virtual bool onException( Exception& e ) 
	{
		e.printException();

		return true;
	}

	virtual bool onWriteFinish( IOEvent& ev ) 
	{
		printf("onWriteFinish()\n");

		char* str=(char*)ev.data.ptr;
		str[ev.data.u32]='\0';

		printf("sent:\n");
		printf(str);

		return false;
	}

	virtual bool onReadFinish( IOEvent& ev ) 
	{
		char* str=(char*)ev.data.ptr;
		str[ev.data.u32]='\0';

		printf("receive:\n");
		printf(str);

		//byte buf[]={8,8,8,8,0,0,0,0,8,8};
		//int len=10;

		String response="HTTP/1.1 200 OK\n"
			"Server:Apache Tomcat/5.0.12\n"
			"Date:Mon,6Oct2013 13:23:42 GMT\n"
			"Content-Length:%d\n\n";
		static int count=0;
		String content=String::format("<h1> ≤‚ ‘IOCP:%d!\n </h1>",count++);
		response=String::format(response,content.length());
		response=response+content;

		byte* buf=(byte*)response.c_str();
		int len=response.length();

		model->send(buf,len,ev.data.fd);

		return true;
	}


protected:
	IOCPModel* model;
};

void main()
{
	_CrtSetBreakAlloc(284);

	try
	{
		IOCPTester t;
		t.start();
		getchar();
		t.stop();
	}
	catch (Exception& e)
	{
		printf("%s\n",e.toString().c_str());
	}
	system("pause");
}


// test for client
#include "LambdaThread.h"

void test()
{
	static int count=0;
	const static int LEN=1024;
	static char buf[LEN]={0};
	for(int i=0;i<LEN;i++)
		buf[i]='a';
	buf[LEN-1]='\n';
	
	auto pThread=new LambdaThread([&](void* pUserParameter)
	{
		ClientSocket sock;
		try
		{
			sock.connect("192.168.2.165",8080);
			count++;
		}catch (Exception& e)
		{
			printf("%s--%d\n",e.toString().c_str(),count);
		}

		for(int i=0;i<10;i++)
		{
			try
			{
				sock.writeBytes(buf,LEN);

				char receiveBuf[LEN];
				int size=sock.readBytes(receiveBuf,LEN);
				receiveBuf[size]=0;
				printf("\n\nreceive:\n");
				printf(receiveBuf);
			}catch (Exception& e)
			{
				printf("%s\n",e.toString().c_str());
			}
			Sleep(1000*1);
		}

		try
		{
			sock.close();
		}catch (Exception& e)
		{
			e.printException();
		}
		//end of thread
	},nullptr);
	pThread->start();
}
void main2()
{
	SocketTools::initSocketContext();

	for(int i=0;i<1000;i++)
	{
		test();
	}
	getchar();

	SocketTools::cleanUpSocketContext();
	system("pause");
}