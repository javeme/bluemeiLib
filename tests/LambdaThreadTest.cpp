
//test
//#define finally catch(...)
#include "ClientSocket.h"
#include "File.h"
using namespace bluemeiIO;
//typedef void F(int error);
void download(String url,const function<void (int error)>& f)
{
	auto func=[url,f](void* pUserParameter){
		printf("downloading...\n");
		int pos=url.find("/",url.find('.'));
		if(pos<0)
			pos=url.length();
		String host=url.substr(0,pos);
		String para=url.substr(pos);
		pos=host.find("://");
		if(pos<0)
			pos=0;
		else
			pos+=strlen("://");
		host=host.substr(pos);

		try{
			SocketTools::initSocketContext();
			ClientSocket socket;
			PHOSTENT phe = gethostbyname(host.c_str()); 
			in_addr addr;
			if (phe != nullptr) {
				addr.s_addr = *(u_long *) phe->h_addr_list[0]; 
			}
			else
			{
				cout<<("get host by name failed")<<endl;
				return;
			}
			socket.connect(inet_ntoa(addr),80);

			if(para.length()==0)
				para="/";
			string str="GET "+para+" HTTP/1.0\r\n";
			str+="Host:localhost\r\n";
			str+="Accept:  */*\r\n";
			str+="Accept-Language: zh-CN\r\n";
			str+="User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; Trident/5.0; SLCC2; .NET CLR 2.0.50727; Tablet PC 2.0; .NET4.0C)\r\n";
			str+="Accept-Encoding: gzip, deflate\r\n";
			str+="Accept-Charset:GB2312,utf-8;q=0.7,*;q=0.7\r\n";
			str+="Proxy-Connection: Keep-Alive\r\n";
			//str+="Cookie:BAIDUID=9E11FAC9F9430D9E4709C4237FFF1CAE:FG=1\r\n";
			str+="\r\n";
			socket.writeString(str);
			char buf[1024];
			int size=0,total=0,len=0,pos=0;
			bool isData=false;
			File file("d:/test2.mp3");
			str="";
			while((size=socket.readBytes(buf+pos,1024-pos))>0)
			{
				if(!isData){
					//可能被拆分if(str.substr(str.length()-3,3)=="\n\r\n")
					str=string(buf,size);
					pos=str.find("\r\n\r\n");
					if(pos>0)
					{
						int pos1=str.find("Content-Length:");
						if(pos1>=0){
							int pos2=str.find("\r\n",pos1);
							pos1+=strlen("Content-Length:");
							str=str.substr(pos1,pos2-pos1);
							total=atoi(str.c_str());
						}
						pos+=strlen("\r\n\r\n");
						isData=true;
					}
				}
				if(isData)
				{
					len+=size-(pos);
					file.writeBytes(buf+pos,size-pos);
					pos=0;
					printf("\r%02d%%",(100*len)/total);
#if 0
					if(size%1024!=0)
					{
						cout<<"剩余:"<<total-len<<endl;
					}
#endif
					if(len>=total)
						break;
				}
			}
			printf("\ndownload finished from %s\n",url.c_str());
			f(0);
		}catch(Exception& e){
			cout<<e.toString()<<endl;
			f(-1);
		}
		SocketTools::cleanUpSocketContext();
	};
	//thread
	Thread *pThread=new LambdaThread(func,nullptr);
	pThread->start();
}
void test(){
	CDiagClientMdiApp::InitConsoleWindow();	
	String url="http://bbshtml.shangdu.com/Editor/UserFiles/File/4982/1869/7724/1220329462819.mp3";	
	download(url,[](int error){
		printf("error:%d\n",error);
	});
	printf("have start download\n");
}