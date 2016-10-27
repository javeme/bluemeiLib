
void testFTP()
{
	try{
		SocketTools::initSocketContext();
		FtpConnection uploader("219.148.23.133",8237);
		uploader.connectServer("bluemei","bluemei");
		uploader.upload("C:/Documents and Settings/Administrator/×ÀÃæ/pic/prompt1.jpg","test.jpg");
		uploader.closeFtpConnection();
	}catch(Exception& e)
	{
		//printf(e.getErrorString().c_str());
		e.printException();
	}
	SocketTools::cleanUpSocketContext();
	system("pause");
}