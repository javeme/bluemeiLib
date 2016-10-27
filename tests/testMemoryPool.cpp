
void testGradeMemoryPools(int maxSize=100, int times=1)
{
	GradeMemoryPools pools(1024*4000);

	std::map<void*, int> ptrs;

	// alloc
	for(int i = 0; i < times; i++) {
		for(int j = 1; j < maxSize; j++) {
			void *ptr = pools.alloc(j);
			memset(ptr, 'f', j);
			if(rand()%5==0)
				ptrs[ptr] = j;
			else
				pools.free(ptr, j);
		}
	}

	// free
	for(std::map<void*, int>::iterator i = ptrs.begin(); i != ptrs.end(); i++) {
		pools.free(i->first, i->second);
	}
	ptrs.clear();
}

void testNewDelete(int maxSize=100, int times=1)
{
	std::map<void*, int> ptrs;

	// alloc
	for(int i = 0; i < times; i++) {
		for(int j = 1; j < maxSize; j++) {
			void *ptr = new char[j];
			memset(ptr, 'f', j);
			if(rand()%5==0) {
				ptrs[ptr] = j;
			}
			else
				delete[] (char*)ptr;
		}
	}

	// free
	for(std::map<void*, int>::iterator i = ptrs.begin(); i != ptrs.end(); i++) {
		delete[] (char*)i->first;
	}
	ptrs.clear();
}

void testMemoryPools() {
	int maxSize = 16;
	int times = 1000000;

	int start1 = time((time_t*)NULL);
	testNewDelete(maxSize, times);
	int end1 = time((time_t*)NULL);

	std::cout<<"testNewDelete cost: "<<(end1 - start1)<<std::endl;

	int start2 = time((time_t*)NULL);
	testGradeMemoryPools(maxSize, times);
	int end2 = time((time_t*)NULL);

	std::cout<<"testGradeMemoryPools cost: "<<(end2 - start2)<<std::endl;
}