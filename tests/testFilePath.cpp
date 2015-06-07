#include "FilePath.h"

void printSpace(unsigned int count)
{
	for (unsigned int i=0; i<count; i++)
	{
		printf(" ");
	}
}

void travelPath(const FilePath& path, int depth=0)
{
	auto files = path.list();
	for (unsigned int i=0; i<files.size(); i++)
	{
		FilePath& sub = files[i];
		if(sub.isdir()){
			printSpace(2*depth);
			printf("+ %s\n", sub.toString().c_str());
			travelPath(sub, depth+1);
		}
		else if(sub.isfile()){
			printSpace(2*depth);
			printf("- %s\n", sub.toString().c_str());
		}
		else{
			printSpace(2*depth);
			printf("* %s\n", sub.toString().c_str());
		}
	}
}

void testFilePath()
{
	FilePath path("D:\\c++\\C++vs2010\\sln");
	printf("%s\n", path.toString().c_str());
	travelPath(path, 1);
}