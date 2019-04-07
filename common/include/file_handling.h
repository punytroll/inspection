#ifndef INSPECTION_COMMON_FILE_HANDLING_H
#define INSPECTION_COMMON_FILE_HANDLING_H

#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <functional>
#include <iostream>

#include "result.h"
#include "value_printing.h"

const std::string g_DarkGray{"\033[90m"};
const std::string g_LightRed{"\033[91m"};
const std::string g_LightGreen{"\033[92m"};
const std::string g_White{"\033[97m"};
const std::string g_DarkYellow{"\033[33m"};

void ReadDirectory(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Buffer &) > Writer);
void ReadFile(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Buffer &) > Writer);
void ReadItem(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Buffer &) > Writer);

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

inline void ReadDirectory(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Buffer &) > Writer)
{
	auto Directory(opendir(Path.c_str()));
	
	if(Directory != nullptr)
	{
		while(true)
		{
			auto DirectoryEntry{readdir(Directory)};
			
			if(DirectoryEntry == nullptr)
			{
				break;
			}
			else if((std::string(DirectoryEntry->d_name) != ".") && (std::string(DirectoryEntry->d_name) != ".."))
			{
				ReadItem(Path + '/' + DirectoryEntry->d_name, Processor, Writer);
			}
		}
		closedir(Directory);
	}
}

inline void ReadItem(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Buffer &) > Writer)
{
	if(FileExists(Path) == true)
	{
		if(IsDirectory(Path) == true)
		{
			ReadDirectory(Path, Processor, Writer);
		}
		else if(IsRegularFile(Path) == true)
		{
			ReadFile(Path, Processor, Writer);
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

inline void ReadFile(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Buffer &) > Writer)
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
				std::cout << "Could not map the file \"" + Path + "\" into memory." << std::endl;
			}
			else
			{
				Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
				auto ParseResult{Processor(Buffer)};
				
				ParseResult->GetValue()->SetData(g_LightGreen + Path + g_White);
				Writer(ParseResult, Buffer);
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
}

inline void DefaultWriter(std::unique_ptr< Inspection::Result > & Result, Inspection::Buffer & Buffer)
{
	PrintValue(Result->GetValue());
	if(Result->GetSuccess() == false)
	{
		if(Result->GetValue()->GetName() == "")
		{
			std::cout << g_LightRed << "Parsing does not give valid and complete result." << g_White << std::endl;
		}
		else
		{
			std::cout << g_LightRed << "Parsing does not give valid " << Result->GetValue()->GetName() << '.' << g_White << std::endl;
		}
	}
	
	auto Rest{Buffer.GetLength() - Buffer.GetPosition()};
	
	if(Rest > Inspection::Length{0, 0})
	{
		std::cout << g_DarkGray << "There are " << g_DarkYellow << to_string_cast(Rest) << g_DarkGray << " bytes and bits after the data." << std::endl;
	}
}

#endif
