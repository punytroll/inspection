#ifndef INSPECTION_COMMON_FILE_HANDLING_H
#define INSPECTION_COMMON_FILE_HANDLING_H

#include <dirent.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <functional>
#include <iostream>

#include "colors.h"
#include "result.h"
#include "value_printing.h"

void ReadDirectory(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer);
void ReadFile(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer);
void ReadItem(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer);

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

inline void PrintException(const std::exception & Exception)
{
	std::cerr << Inspection::g_Yellow << Exception.what() << Inspection::g_Reset << std::endl;
	try
	{
		std::rethrow_if_nested(Exception);
	}
	catch(const std::exception & NestedException)
	{
		std::cerr << Inspection::g_Red << "Nested exception" << Inspection::g_Reset << ":" << std::endl;
		PrintException(NestedException);
	}
}

inline void PrintExceptions(const std::exception & Exception)
{
	std::cerr << Inspection::g_Red << "Caught an exception while processing" << Inspection::g_Reset << ":" << std::endl;
	PrintException(Exception);
	std::cerr << std::endl;
}

inline void ReadDirectory(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer)
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

inline void ReadItem(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer)
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

inline void ReadFile(const std::string & Path, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor, std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer)
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
				try
				{
					Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
					Inspection::Reader Reader{Buffer};
					auto ParseResult{Processor(Reader)};
					
					ParseResult->GetValue()->SetData(Inspection::g_BrightGreen + Path + Inspection::g_BrightWhite);
					Writer(ParseResult, Reader);
				}
				catch(const std::exception & Exception)
				{
					PrintExceptions(Exception);
				}
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
}

inline void DefaultWriter(std::unique_ptr< Inspection::Result > & Result, Inspection::Reader & Reader)
{
	PrintValue(Result->GetValue());
	if(Result->GetSuccess() == false)
	{
		if(Result->GetValue()->GetName() == "")
		{
			std::cout << Inspection::g_BrightRed << "Parsing does not give valid and complete result." << Inspection::g_BrightWhite << std::endl;
		}
		else
		{
			std::cout << Inspection::g_BrightRed << "Parsing does not give valid " << Result->GetValue()->GetName() << '.' << Inspection::g_BrightWhite << std::endl;
		}
	}
	if(Reader.HasRemaining() == true)
	{
		std::cout << Inspection::g_BrightBlack << "There are " << Inspection::g_Yellow << to_string_cast(Reader.GetRemainingLength()) << Inspection::g_BrightBlack << " bytes and bits after the data." << std::endl;
	}
}

#endif
