#pragma once

#include "bluemeiLib.h"
#include "ServerSocket.h"
#include "SocketTools.h"
#include "FtpException.h"

namespace bluemei{

#define BUF_SIZE 256

//#pragma comment(lib,"bLib/bluemeiLib.lib")
//FtpUploader
class BLUEMEILIB_API FtpConnection : public Object
{
public:
	FtpConnection(const char *ip,int port);
	FtpConnection();
	~FtpConnection(void);
private:
	ClientSocket m_cmdSocket;//����socket
	//���ݷ���socket,����ģʽ
	ServerSocket dataLisenSocket;
	String m_strIp;
	int m_nPort;
	bool m_bIsPasv,m_bIsGoOn;
private:
	void initTranMode();
public:	
	void setPasv(bool isPasv);
	void setAddress(const char *ip,int port);
	bool connectServer(const char *username,const char *password);
	bool closeFtpConnection();
	bool upload(const char *filePath,const char *savePath,unsigned int* pUploadSize=NULL);
	void stop();
};


}//end of namespace bluemei