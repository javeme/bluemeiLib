
/*
//test MessageThread
#include "MessageThread.h"
static bluemei::MessageThread msgThread([](Message* msg){
	TRACE(">>>>Message: %d\n",msg->getId());
});
msgThread.start();

msgThread.addMessage(new Message(100,0,0,new Exception(e)));
msgThread.addMessage(new Message(101,1));
msgThread.addMessage(new Message(102,2));
msgThread.addMessage(new Message(1022,2));
msgThread.addMessage(new Message(1021,2));
msgThread.addMessage(new Message(1020,2));
msgThread.addMessage(new Message(108,8));
msgThread.addMessage(new Message(103,3));
msgThread.addMessage(new Message(1031,3));
msgThread.addMessage(new Message(1030,3));
*/

//test MessageThread
#include "MessageThread.h"
#include "LambdaThread.h"
#include "MessageHandler.h"

void testMessageThread(MainFrame* _this)
{
	static MessageHandler mh(_this);

	static bluemei::MessageThread msgThread([=](Message* msg){
		String str=String::format(">>>>Message: %5d [%s] (%s)\n",msg->getId(),
			Date(msg->getTimestamp()).toString().c_str(),
			msg->getObject()->toString().c_str());
		TRACE(str.c_str());
		mh.PostRunnable([=](void* para){ _this->OutputMessage((CString)CONVSTR(str)); });

		Sleep(1);
	});

	msgThread.start();

	mh.PostDelay(1000*5,[_this](void* para){ 
		msgThread.finish();		
		String str=String::format(">>>>>>>>Message Thread finish (remained message: %s)\n",
			String(String("")+msgThread.hasMessage()).c_str());
		_this->OutputMessage((CString)CONVSTR(str));
	});

	static int count=0;
	for(int i=0;i<10;i++)
	{
		Thread* thread=new LambdaThread([i](void* para){
			for(int j=0;j<1000;j++)
			{
				int pri=j;
				msgThread.addMessage(new Message(count++,pri,0,0,new String(String("test@")+pri)));
				Sleep(10);
			}
			TRACE(">>>>>>>>>>>>>>>>Work Thread %d finished\n",i);
		},nullptr);
		thread->start();
	}
}
