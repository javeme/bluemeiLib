#include "FilePath.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#elif _LINUX
#include <stdarg.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#elif _LINUX
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif

namespace bluemei{


FilePath::FilePath(const String& path/*=""*/) : m_path(path.length() + 64)
{
	m_path.append(path.replace("\\", PATH_SEPARATOR));
}

FilePath::~FilePath()
{
	;
}

FilePath& FilePath::append(const String& path)
{
	String sepl = PATH_SEPARATOR;
	int pos = m_path.lastIndexOf(sepl);
	bool lastSepl = (pos >= 0) && (pos == m_path.length() - sepl.length());
	if(m_path.length() > 0 && !lastSepl)
		m_path.append(sepl);
	m_path.append(path);

	return *this;
}

String FilePath::dirName() const
{
	int index = m_path.toString().rfind(PATH_SEPARATOR);
	if(index < 0)
		return "";
	else
		return m_path.substring(0, index);
}

String FilePath::fileName() const
{
	String path = m_path.toString();
	int index = path.rfind(PATH_SEPARATOR);
	if(index + 1 == path.length())
		return "";
	else
		return m_path.substring(index + 1);
}

String FilePath::toString() const
{
	return m_path.toString();
}


///////////////////////////////////////////////////
//FileUtil
bool FileUtil::exists(const String& path)
{
	return ACCESS(path.c_str(), 0) == 0;
}

bool FileUtil::mkdir(const String& dir)
{
	return MKDIR(dir) == 0;
}

bool FileUtil::mkdirs(const String& dir)
{
	if(exists(dir))
		return true;

	String dirSeprt = dir.replace("\\", PATH_SEPARATOR);
	auto list = dirSeprt.splitWith(PATH_SEPARATOR);
	//String ssss=list.toString();

	FilePath path;
	if(dirSeprt.startWith(PATH_SEPARATOR))
		path.append(PATH_SEPARATOR);
	for(unsigned int i=0; i<list.size(); i++)
	{
		//remove empty string
		if(list[i].empty())
			continue;
		//append to parent path
		path.append(list[i]);
		if(!exists(path))
		{
			if(!mkdir(path))
				return false;
		}
	}
	return true;
}

}//namespace bluemei