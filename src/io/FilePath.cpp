#include "FilePath.h"


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

bool FilePath::exists() const
{
	return FileUtil::exists(this->toString());
}

bool FilePath::readable() const
{
	return FileUtil::readable(this->toString());
}

bool FilePath::writable() const
{
	return FileUtil::writable(this->toString());
}

bool FilePath::executable() const
{
	return FileUtil::executable(this->toString());
}

bool FilePath::isfile() const
{
	return FileUtil::isfile(this->toString());
}

bool FilePath::isdir() const
{
	return FileUtil::isdir(this->toString());
}

ArrayList<FilePath> FilePath::list() const
{
	ArrayList<FilePath> files;
	if(this->isdir())
	{
		ArrayList<String> paths= FileUtil::list(this->toString());
		for(unsigned int i=0; i<paths.size(); i++){			 
			files.add(FilePath(*this).append(paths[i]));
		}
	}
	return files;
}

}//namespace bluemei