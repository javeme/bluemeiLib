
void testMessageLooperWithMainThread()
{
	printf("testMessageLooperWithMainThread()\n");

	Looper looper;

	LambdaThread lt([&]{
		printf("post by thread: %d\n", Thread::currentThreadId());
		looper.postRunnable([]{
			printf("execute by thread: %d\n", Thread::currentThreadId());
		});

		printf("press any key to stop looper\n");
		getchar();
		try{
			looper.stop();
		}catch (Exception& e){
			e.printStackTrace();
		}

		printf("press any key to stop thread\n");
		getchar();
	});
	lt.setAutoDestroy(false);
	lt.start();

	// start loop in this thread
	looper.start(false);

	printf("press any key to exit\n");
	getchar();
}

void testMessageLooperWithNewThread()
{
	printf("testMessageLooperWithNewThread()\n");

	Looper looper;
	// start loop in a new thread
	looper.start();

	printf("post by thread: %d\n", Thread::currentThreadId());
	looper.postRunnable([=]{
		printf("execute by thread: %d\n", Thread::currentThreadId());
		//send(buf, onSuccess)
	});

	printf("press any key to stop\n");
	getchar();

	try{
		looper.stop();
	}catch (Exception& e){
		e.printStackTrace();
	}

	printf("press any key to exit\n");
	getchar();
}
