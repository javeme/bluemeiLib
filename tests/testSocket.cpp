#include "ServerSocket.h"

void testSocket(int argc, char* argv[])
{
	if (argc < 1)
		return;
	String arg1 = argv[1];

	String ip = "127.0.0.1";
	int port = 8080;

	if (arg1 == "server")
	{
		ServerSocket ss(port);
		while(true){
			ClientSocket* cc = ss.accept();
			auto trd = new LambdaThread([=](){
				String line = cc->readLine();
				printf("%s\n", line.c_str());
				line = "reply by server: " + line + "\n";
				cc->writeBytes(line.c_str(),line.length());
				Thread::sleep(1000*1);
				printf("closed %s\n", cc->toString().c_str());
				cc->close();
				delete cc;
			});
			trd->start();
		}
	}
	else
	{
		for (int i=0; i<1000000; i++)
		{
			String content = String::format("hello %d\n", i);
			ClientSocket cs;
			cs.connect(ip, port);
			cs.writeBytes(content.c_str(), content.length());
			String result = cs.readLine();
			printf("%s\n", result.c_str());
		}
	}
}