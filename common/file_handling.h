#ifndef COMMON_FILE_HANDLING_H
#define COMMON_FILE_HANDLING_H

#include <dirent.h>
#include <sys/stat.h>

void ReadDirectory(const std::string & Path);
void ReadFile(const std::string & Path);
void ReadItem(const std::string & Path);

inline std::int64_t GetFileSize(const std::string & Path)
{
	struct stat Stat;
	
	if(stat(Path.c_str(), &Stat) == -1)
	{
		std::cerr << Path << ": Could not determine file size." << std::endl;
		
		return -1;
	}
	else
	{
		return Stat.st_size;
	}
}

inline bool FileExists(const std::string & Path)
{
	struct stat Stat;
	
	return stat(Path.c_str(), &Stat) != -1;
}

inline bool IsDirectory(const std::string & Path)
{
	struct stat Stat;
	
	stat(Path.c_str(), &Stat);
	
	return S_ISDIR(Stat.st_mode);
}

inline bool IsRegularFile(const std::string & Path)
{
	struct stat Stat;
	
	stat(Path.c_str(), &Stat);
	
	return S_ISREG(Stat.st_mode);
} 

inline void ReadDirectory(const std::string & Path)
{
	DIR * Directory(opendir(Path.c_str()));
	struct dirent * DirectoryEntry(0);
	
	while((DirectoryEntry = readdir(Directory)) != 0)
	{
		if((std::string(DirectoryEntry->d_name) != ".") && (std::string(DirectoryEntry->d_name) != ".."))
		{
			ReadItem(Path + '/' + DirectoryEntry->d_name);
		}
	}
}

inline void ReadItem(const std::string & Path)
{
	if(FileExists(Path) == true)
	{
		if(IsDirectory(Path) == true)
		{
			ReadDirectory(Path);
		}
		else if(IsRegularFile(Path) == true)
		{
			ReadFile(Path);
		}
		else
		{
			std::cerr << '"' << Path << "\" is no file or directory!" << std::endl;
		}
	}
	else
	{
		std::cerr << '"' << Path << "\" does not exist!" << std::endl;
		
		return;
	}
}

#endif
