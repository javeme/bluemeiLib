#include "FtpConnection.h"
#include <time.h>

#include "SocketException.h"
#include "File.h"
#include "ErrorHandler.h"

namespace bluemei{


FtpConnection::FtpConnection(const char *ip,int port)
{
	m_bIsPasv=true;
	this->m_strIp=ip;
	this->m_nPort=port;
}
FtpConnection::FtpConnection()
{
	m_bIsPasv=true;
}

FtpConnection::~FtpConnection(void)
{
	try{
		dataLisenSocket.close();
	}catch(Exception& e)
	{
		ErrorHandler::handle(e);
	}
}
void FtpConnection::setAddress(const char *ip,int port)
{	
	this->m_strIp=ip;
	this->m_nPort=port;
}
void FtpConnection::setPasv(bool isPasv)
{
	this->m_bIsPasv=isPasv;
}
bool FtpConnection::connectServer(const char *username,const char *password)
{
	//���ӷ�����
	m_cmdSocket.connect(m_strIp,m_nPort);

    char buff[BUF_SIZE];
    memset(buff, 0, sizeof(buff));
	int recvedLen=0,pos=0;
	String line;

	//recvedLen=m_cmdSocket.readBytes(buff, sizeof(buff));
	line=m_cmdSocket.readLineByGbk();
	pos=line.find("220");
	if (pos<0) {
		throw FtpException(REFUSE,"connection refused by remote ftp server");
    }
	//��֤�û���,����
	m_cmdSocket.skip(m_cmdSocket.availableBytes());//��ս��ն���
    sprintf_s(buff, "USER %s\r\n", username);
	m_cmdSocket.writeBytes(buff,strlen(buff));

    line=m_cmdSocket.readLineByGbk();
	pos=line.find("331");
    if (pos<0){
		throw FtpException(REFUSE,"connection refused by remote ftp server,cause:"+line);
    }
	//����
    sprintf_s(buff, "PASS %s\r\n", password);
    m_cmdSocket.writeBytes(buff,strlen(buff));

	line=m_cmdSocket.readLineByGbk();
	pos=line.find("230");
	if (pos<0) {
		throw FtpException(PASSWORD_EEROR,"username and password not matched");
    }
	//��ȡϵͳ��Ϣ
    sprintf_s(buff, "SYST\r\n");
	m_cmdSocket.writeBytes(buff,strlen(buff));
	m_cmdSocket.readBytes(buff,sizeof(buff));    

	//���ô�������
    sprintf_s(buff, "TYPE I\r\n");
	m_cmdSocket.writeBytes(buff,strlen(buff));
    line=m_cmdSocket.readLineByGbk();
	pos=line.find("200");
	if (pos<0) {
		throw FtpException(SET_BINARY_FAILED,"set type binary failed,cause:"+line);
    }
	return true;
}
bool FtpConnection::closeFtpConnection()
{	
	m_cmdSocket.skip(m_cmdSocket.availableBytes());//��ս��ն���
	String line="QUIT\r\n";
	m_cmdSocket.writeString(line);
    line=m_cmdSocket.readLineByGbk();
	int pos=line.find("221");
	if (pos<0) {		
		line+=m_cmdSocket.readLineByGbk();
		throw FtpException(CLOSE_CON_FAILED,"close ftp connection:"+line);
    }
	m_cmdSocket.close();
	return true;
}
//��������/����ģʽ
void FtpConnection::initTranMode()
{
	;	
}
//�ϴ�,filePath�����ļ�·��,savePath����������·��
bool FtpConnection::upload(const char *filePath,const char *savePath,unsigned int* pUploadSize)
{
	if(pUploadSize==NULL||*pUploadSize<0)
		return false;
	unsigned int& uploadSize=*pUploadSize;	
    char buff[BUF_SIZE];	
    memset(buff, 0, sizeof(buff));
	int pos=0;
	String line;
	ClientSocket* pDataSocket=NULL;
	
	if(m_bIsPasv)//����ģʽ(Ĭ�Ϸ�ʽ)
	{	
		//�������һ��ʱ��,��ֹ���߳�ͬʱ����
		srand((unsigned int)time(NULL));
		int randTime= (rand() % 1000) + 10;
		Sleep(randTime);
		//��ս��ջ���
		m_cmdSocket.skip(m_cmdSocket.availableBytes());
		//���󱻶�ģʽ
		sprintf_s(buff, "PASV \r\n");
		m_cmdSocket.writeBytes(buff,strlen(buff));
		line=m_cmdSocket.readLineByGbk();
		pos=line.find("227");
		if (pos<0) {//���뱻��ģʽʧ��
			throw FtpException(SET_PASV_FAILED,"set type PASV failed,cause by:"+line);
		}		
		//�����˿ں�
		int start=0,end=0;
		start=line.rfind(",");
		end=line.rfind(")");
		start+=1;
		String strLPort=line.substring(start,end-start);
		//��λ
		end=start-1;
		start=line.rfind(",",start-2);
		start+=1;
		String strHPort=line.substring(start,end-start);
		//data_port=8236;
		int dataPort=atoi(strHPort.c_str())*256+atoi(strLPort.c_str());
	
		pDataSocket=new ClientSocket();
		try{
			pDataSocket->connect(this->m_strIp,dataPort);
		}catch(...){
			delete pDataSocket;
			pDataSocket=NULL;
			throw;
		}
		printf("create data connection success\n");	
	}
	//����ģʽ
	else
	{		
		int listenDataPort;
		srand ((unsigned int) time (NULL) * 1000000);
		/*/�������һ��ʱ��,��ֹ���߳�ͬʱ����
		int randTime= (rand() % 1000) + 10;
		_sleep(randTime);//*/
		//use random number as port for ftpdata bind
		bool isSuccess=false;
		while(!isSuccess)
		{
			//���Զ˿��Ƿ����
			try{
				listenDataPort = (rand()%20000) + 2000;
				dataLisenSocket.listen(listenDataPort);//����˿ڰ�
				isSuccess=true;
			}catch(SocketException& e){
				if(e.getError()!=WSAEADDRINUSE)
					break;
			}
		}
		
		m_cmdSocket.skip(m_cmdSocket.availableBytes());//��ս��ջ���
		sprintf_s(buff, "PORT %s,%d,%d\r\n", "127,0,0,1",listenDataPort/256, listenDataPort%256);
		//sprintf(buff, "PORT %d,%d\r\n", localport / 256, localport % 256);		
		m_cmdSocket.writeBytes(buff,strlen(buff));
		line=m_cmdSocket.readLineByGbk();
		pos=line.find("200");
		if (pos<0) {//��������ģʽʧ��
			throw FtpException(SET_PORT_FAILED,"set type PORT failed,cause:"+line);
		}		
	}
	//���Ͷϵ�����
	if(uploadSize>0)
	{
		//�����ϴ�����
		sprintf_s(buff, "REST %d\r\n", uploadSize);
		m_cmdSocket.writeBytes(buff,strlen(buff));
		line=m_cmdSocket.readLineByGbk();
		pos=line.find("350");
		if (pos<0) 
		{
			throw FtpException(SET_REST_FAILED,"unable to send the offset of break point,cause:"+line);
		}
	}
	//�����ϴ�����
    sprintf_s(buff, "STOR %s\r\n", savePath);
    m_cmdSocket.writeBytes(buff,strlen(buff));
	line=m_cmdSocket.readLineByGbk();
	pos=line.find("150");
	if (pos<0) {
		throw FtpException(SET_STOR_FAILED,"unable to STOR,cause:"+line);
	}
	//����ģʽʱ,�ȴ�����������
	if(!m_bIsPasv)
	{		
		pDataSocket=dataLisenSocket.accept();
		//dataLisenSocket.close();
	}
	//�ϴ�����
	File file(filePath,"rb");	
	int len=0;
	if(uploadSize>0)//�ϵ�����
	{
		file.setPos(uploadSize);
	}
	m_bIsGoOn=true;//�����ⲿ�ж�
    while(m_bIsGoOn && (len=file.readBytes(buff,sizeof(buff)))>0)
    {
		uploadSize+=pDataSocket->writeBytes(buff,len);
    }
	file.close();
	if(m_bIsPasv)
		delete pDataSocket;
	else
		ServerSocket::destroy(pDataSocket);
	printf("upload finish\n");
    /*
	do{
		line=m_cmdSocket.readLineByGbk();
		pos=line.find("226");
	}while(pos<0); */  
	line=m_cmdSocket.readLineByGbk();
	pos=line.find("226");
	if(pos<0)
	{
		throw FtpException(UPLOAD_FILE_FAILED,"result from server,cause:"+line);
	}
    return m_bIsGoOn;
}
void FtpConnection::stop()
{
	this->m_bIsGoOn=false;
}


void test()
{
	try{
		SocketTools::initSocketContext();
		FtpConnection uploader("219.148.23.133",8237);
		uploader.connectServer("bluemei","bluemei");
		uploader.upload("C:/Documents and Settings/Administrator/����/pic/prompt1.jpg","test.jpg");
		uploader.closeFtpConnection();
	}catch(Exception& e)
	{
		//printf(e.getErrorString().c_str());
		e.printException();
	}
	SocketTools::cleanUpSocketContext();
	system("pause");
}

}//end of namespace bluemei