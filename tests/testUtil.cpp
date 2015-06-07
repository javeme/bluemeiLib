#include "Util.h"
#include "HashMap.h"

void testUuidRepeat()
{
	HashMap<String, bool> uuidMap; 
	for (int i = 0; i < 1000*1000*10; i++)
	{
		String u4 = Util::uuid4();
		String u1 = Util::uuid1();
		String ug = Util::guid();

		if(uuidMap.contain(u4)){
			uuidMap[u4] = true;
			uuidMap[u1] = false;
			uuidMap[ug] = false;
			printf("==============exist uuid: %s\n", u4.c_str());
		}
		else if(uuidMap.contain(u1)){
			uuidMap[u4] = false;
			uuidMap[u1] = true;
			uuidMap[ug] = false;
			printf("==============exist uuid: %s\n", u1.c_str());
		}
		else if(uuidMap.contain(ug)){
			uuidMap[u4] = false;
			uuidMap[u1] = false;
			uuidMap[ug] = true;
			printf("==============exist uuid: %s\n", ug.c_str());
		}
		else{
			uuidMap[u4] = false;
			uuidMap[u1] = false;
			uuidMap[ug] = false;
		}
	}
}
