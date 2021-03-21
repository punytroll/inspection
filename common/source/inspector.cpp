#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "buffer.h"
#include "colors.h"
#include "exception_printing.h"
#include "inspector.h"
#include "reader.h"
#include "value_printing.h"

using namespace std::string_literals;

Inspection::Inspector::~Inspector(void)
{
}

std::uint_fast32_t Inspection::Inspector::GetPathCount(void) const
{
	return _Paths.size();
}

void Inspection::Inspector::Process(void)
{
	for(auto Path : _Paths)
	{
		try
		{
			_ProcessPath(std::filesystem::directory_entry(Path));
		}
		catch(const std::exception & Exception)
		{
			Inspection::PrintExceptions(Exception);
		}
	}
}

void Inspection::Inspector::PushPath(const std::filesystem::path & Path)
{
	_Paths.push_back(Path);
}

void Inspection::Inspector::_ProcessPath(const std::filesystem::directory_entry & DirectoryEntry)
{
	if(DirectoryEntry.exists() == true)
	{
		if(DirectoryEntry.is_directory() == true)
		{
			for(auto & DirectoryEntry : std::filesystem::recursive_directory_iterator(DirectoryEntry))
			{
				if(DirectoryEntry.is_regular_file() == true)
				{
					_ProcessFile(DirectoryEntry);
				}
			}
		}
		else if(DirectoryEntry.is_regular_file() == true)
		{
			_ProcessFile(DirectoryEntry);
		}
		else
		{
			throw std::runtime_error('"' + DirectoryEntry.path().string() + "\" is no file or directory!");
		}
	}
	else
	{
		throw std::runtime_error('"' + DirectoryEntry.path().string() + "\" does not exist!");
	}
}

std::tuple< Inspection::Length, Inspection::Length > Inspection::Inspector::_GetStartAndEnd(const Inspection::Buffer & Buffer, const Inspection::Length & Start)
{
	return {Start, Buffer.GetLength()};
}

void Inspection::Inspector::_ProcessFile(const std::filesystem::directory_entry & DirectoryEntry)
{
	auto & Path{DirectoryEntry.path()};
	auto FileDescriptor{open(Path.c_str(), O_RDONLY)};
	
	if(FileDescriptor == -1)
	{
		throw std::runtime_error("Could not open the file \"" + Path.string() + "\".");
	}
	else
	{
		auto FileSize{DirectoryEntry.file_size()};
		auto Address{static_cast< std::uint8_t * >(mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0))};
		
		if(Address == MAP_FAILED)
		{
			throw std::runtime_error("Could not map the file \"" + Path.string() + "\" into memory.");
		}
		else
		{
			Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
			auto Result{std::make_unique< Inspection::Result >()};
			
			Result->GetValue()->SetName(Inspection::g_BrightGreen + Path.string() + Inspection::g_BrightWhite);
			
			auto LengthTag{Result->GetValue()->AddTag("length", Buffer.GetLength())};
			
			LengthTag->AddTag("unit", "bytes and bits"s);
			
			Inspection::Length RunningStart{0, 0};
			auto [Start, End]{_GetStartAndEnd(Buffer, Inspection::Length{0, 0})};
			auto Continue{true};
			
			while((Continue == true) && (Start != Buffer.GetLength()))
			{
				if(Start > RunningStart)
				{
					auto OtherDataField{Result->GetValue()->AppendField("OtherData")};
					auto LengthTag{OtherDataField->AddTag("length", Start - RunningStart)};
					
					LengthTag->AddTag("unit", "bytes and bits"s);
				}
				
				Inspection::Reader PartReader{Buffer, Start, End - Start};
				auto PartResult{_Getter(PartReader, {})};
				
				Result->GetValue()->AppendField(PartResult->GetValue());
				Continue = PartResult->GetSuccess();
				Start += PartReader.GetConsumedLength();
				RunningStart = Start;
				std::tie(Start, End) = _GetStartAndEnd(Buffer, Start);
			}
			if(RunningStart < Buffer.GetLength())
			{
				auto OtherDataField{Result->GetValue()->AppendField("OtherData")};
				auto LengthTag{OtherDataField->AddTag("length", Buffer.GetLength() - RunningStart)};
				
				LengthTag->AddTag("unit", "bytes and bits"s);
			}
			Result->SetSuccess(Continue);
			_Writer(Result);
			munmap(Address, FileSize);
		}
		close(FileDescriptor);
	}
}

void Inspection::Inspector::_Writer(std::unique_ptr< Inspection::Result > & Result)
{
	PrintValue(Result->GetValue());
	if(Result->GetSuccess() == false)
	{
		std::cout << Inspection::g_BrightRed << "Parsing does not give valid." << Inspection::g_BrightWhite << std::endl;
	}
}
