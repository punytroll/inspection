#ifndef COMMON_FILE_HANDLING_H
#define COMMON_FILE_HANDLING_H

#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

const std::string g_Green{"\033[32;1m"};
const std::string g_White{"\033[0m"};

void ReadDirectory(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor);
void ReadFile(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor);
void ReadItem(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor);

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

inline void ReadDirectory(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor)
{
	DIR * Directory(opendir(Path.c_str()));
	struct dirent * DirectoryEntry(0);
	
	while((DirectoryEntry = readdir(Directory)) != 0)
	{
		if((std::string(DirectoryEntry->d_name) != ".") && (std::string(DirectoryEntry->d_name) != ".."))
		{
			ReadItem(Path + '/' + DirectoryEntry->d_name, Processor);
		}
	}
}

inline void ReadItem(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor)
{
	if(FileExists(Path) == true)
	{
		if(IsDirectory(Path) == true)
		{
			ReadDirectory(Path, Processor);
		}
		else if(IsRegularFile(Path) == true)
		{
			ReadFile(Path, Processor);
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

inline void ReadFile(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor)
{
	auto FileDescriptor{open(Path.c_str(), O_RDONLY)};
	
	if(FileDescriptor == -1)
	{
		std::cerr << "Could not open the file \"" << Path << "\"." << std::endl;
	}
	else
	{
		std::int64_t FileSize{GetFileSize(Path)};
		
		if(FileSize != -1)
		{
			auto Address{reinterpret_cast< std::uint8_t * >(mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0))};
			
			if(Address == MAP_FAILED)
			{
				std::cerr << "Could not map the file \"" + Path + "\" into memory." << std::endl;
			}
			else
			{
				Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
				auto ParseResult{Processor(Buffer)};
				
				ParseResult->GetValue()->SetAny(g_Green + Path + g_White);
				PrintValue(ParseResult->GetValue());
				if(ParseResult->GetSuccess() == false)
				{
					std::cerr << "The file does not start with a " << ParseResult->GetValue()->GetName() << '.' << std::endl;
				}
				
				auto Rest{Buffer.GetLength() - Buffer.GetPosition()};
				
				if(Rest > 0ull)
				{
					std::cerr << "There are " << to_string_cast(Rest) << " bytes and bits after the data." << std::endl;
				}
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
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
