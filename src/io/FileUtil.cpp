#include "FileUtil.h"
#include "FilePath.h"
#include <stdlib.h>
#include <assert.h>

namespace blib{

#ifdef _WIN32
#	include <direct.h>
#	include <io.h>
#	include <sys/stat.h>
#	define R_OK 4 /* Test for read permission. */
#	define W_OK 2 /* Test for write permission. */
#	define X_OK 1 /* Test for execute permission. */
#	define F_OK 0 /* Test for existence. */
#	define os_access _access
#	define os_chmod _chmod
#	define os_mkdir(a) _mkdir((a))
#	define os_getcwd _getcwd
#	define os_chdir _chdir
#	define S_ISREG(mode) ((mode) & S_IFREG) != 0
#	define S_ISDIR(mode) ((mode) & S_IFDIR) != 0

	int os_stat(cstring path, FileInfo& fi)
	{
		/*struct _finddata_t findInfo;
		if(_findfirst(path, &findInfo) == -1)
			return -1;

		fi.name = findInfo.name;
		fi.mode = findInfo.attrib;
		fi.size = findInfo.size;
		fi.createTime = findInfo.time_create;
		fi.updateTime = findInfo.time_write;
		fi.accessTime = findInfo.time_access;
		fi.statChangeTime = -1;

		return 0;*/
		struct _stat findInfo;
		if(_stat(path, &findInfo) == -1)
			return -1;

		fi.name = path;
		fi.mode = findInfo.st_mode;
		fi.size = findInfo.st_size;
		fi.createTime = -1;
		fi.updateTime = findInfo.st_mtime;
		fi.accessTime = findInfo.st_atime;
		fi.statChangeTime = findInfo.st_ctime;

		return 0;
	}

	ArrayList<String> listFiles(cstring dirname)
	{
		assert(dirname != NULL);
		ArrayList<String> files;

		struct _finddata_t fileInfo;

		String dir = String(dirname) + "\\*";
		long fd = _findfirst(dir.c_str(), &fileInfo);
		if(fd != -1)
		{
			do{
				//if((fileInfo.attrib &  _A_SUBDIR))
				if(strcmp(fileInfo.name, ".") == 0
					|| strcmp(fileInfo.name, "..") == 0)
					continue;
				files.add(fileInfo.name);
			}while(_findnext(fd, &fileInfo) == 0);
			_findclose(fd);
		}
		return files;
	}
#elif _LINUX
#	include <stdarg.h>
#	include <sys/stat.h>
#	include <unistd.h>
#	include <dirent.h>
#	define os_access access
#	define os_chmod chmod
#	define os_mkdir(a) mkdir((a),0755)
#	define os_getcwd getcwd
#	define os_chdir chdir

	int os_stat(cstring path, FileInfo& fi)
	{
		struct stat findInfo;
		if(stat(path, &findInfo) == -1)
			return -1;

		fi.name = path;
		fi.mode = findInfo.st_mode;
		fi.size = findInfo.st_size;
		fi.createTime = -1;
		fi.updateTime = findInfo.st_mtime;
		fi.accessTime = findInfo.st_atime;
		fi.statChangeTime = findInfo.st_ctime;

		return 0;
	}

	ArrayList<String> listFiles(cstring dirname)
	{
		assert(dirname != NULL);
		ArrayList<String> files;

		struct dirent *fileInfo;
		DIR *dir = opendir(dirname);
		while(dir && (file=readdir(dir)))
		{
			if(strcmp(fileInfo->d_name, ".") == 0
				|| strcmp(fileInfo->d_name, "..") == 0)
				continue;

			files.add(fileInfo->d_name);
		}
		closedir(dir);
		return files;
	}
#endif

///////////////////////////////////////////////////
//FileUtil
bool FileUtil::exists(const String& path)
{
	return os_access(path.c_str(), F_OK) == 0;
}

bool FileUtil::readable(const String& path)
{
	return os_access(path.c_str(), R_OK) == 0;
}

bool FileUtil::writable(const String& path)
{
	return os_access(path.c_str(), W_OK) == 0;
}

bool FileUtil::executable(const String& path)
{
	return os_access(path.c_str(), X_OK) == 0;
}

bool FileUtil::isfile(const String& path)
{
	return S_ISREG(FileUtil::info(path).mode);
}

bool FileUtil::isdir(const String& path)
{
	return S_ISDIR(FileUtil::info(path).mode);
}

ArrayList<String> FileUtil::list(const String& path)
{
	return listFiles(path.c_str());
}

bool FileUtil::remove(const String& path)
{
	return ::remove(path.c_str()) == 0;
}

bool FileUtil::rename(const String& from, const String& to)
{
	return ::rename(from.c_str(), to.c_str()) == 0;
}

bool FileUtil::chmod(const String& path, int mode)
{
	return os_chmod(path.c_str(), mode) == 0;
}

FileInfo FileUtil::info(const String& path)
{
	FileInfo fi;
	if(os_stat(path.c_str(), fi) == 0)
		return fi;
	throwpe(NotFoundException(path));
}

bool FileUtil::mkdir(const String& dir)
{
	return os_mkdir(dir) == 0;
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

String FileUtil::getcwd()
{
	int len = 256;
	while(true){
		String buf("", len);
		if(os_getcwd((char*)buf.c_str(), len))
			return buf;
		len *= 2;
	}
	return "";
}

bool FileUtil::chdir(const String& path)
{
	return os_chdir(path.c_str()) == 0;
}

}//namespace blib