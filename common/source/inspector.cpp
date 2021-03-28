#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "buffer.h"
#include "colors.h"
#include "exception_printing.h"
#include "inspector.h"
#include "query.h"
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

void Inspection::Inspector::_QueryWriter(std::shared_ptr< Inspection::Value > Value, const std::string & Query)
{
	auto QueryParts{Inspection::SplitString(Query.substr(1), '/')};
	
	for(auto Index = 0ul; Index < QueryParts.size(); ++Index)
	{
		auto QueryPart{QueryParts[Index]};
		auto QueryPartSpecifications{Inspection::SplitString(QueryPart, ':')};
		
		if(QueryPartSpecifications[0] == "field")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetField(QueryPartSpecifications[1]);
			}
			else if(QueryPartSpecifications.size() == 3)
			{
				std::shared_ptr< Inspection::Value > MatchingField;
				
				if((QueryPartSpecifications[2][0] == '[') && (QueryPartSpecifications[2][QueryPartSpecifications[2].size() - 1] == ']'))
				{
					auto TestQuery{QueryPartSpecifications[2].substr(1, QueryPartSpecifications[2].size() - 2)};
					
					for(auto Field : Value->GetFields())
					{
						if((Field->GetName() == QueryPartSpecifications[1]) && (Inspection::EvaluateTestQuery(Field, TestQuery) == true))
						{
							MatchingField = Field;
							
							break;
						}
					}
				}
				else
				{
					auto WantedIndex{from_string_cast< std::uint64_t >(QueryPartSpecifications[2])};
					std::uint64_t Index{0};
					
					for(auto Field : Value->GetFields())
					{
						if(Field->GetName() == QueryPartSpecifications[1])
						{
							if(WantedIndex == Index)
							{
								MatchingField = Field;
								
								break;
							}
							else
							{
								++Index;
							}
						}
					}
				}
				if(MatchingField == nullptr)
				{
					throw std::invalid_argument("The test \"" + QueryPartSpecifications[2] + "\" could not be satisfied by any field.");
				}
				else
				{
					Value = MatchingField;
				}
			}
			if(Index + 1 == QueryParts.size())
			{
				PrintValue(Value);
			}
		}
		else if(QueryPartSpecifications[0] == "data")
		{
			if(QueryPartSpecifications.size() == 1)
			{
				std::cout << Value->GetData();
			}
			else
			{
				throw std::invalid_argument("The \"data\" query part specification does not accept any arguments.");
			}
		}
		else if(QueryPartSpecifications[0] == "tag")
		{
			Value = Value->GetTag(QueryPartSpecifications[1]);
			if(Index + 1 == QueryParts.size())
			{
				PrintValue(Value);
			}
		}
		else if(QueryPartSpecifications[0] == "has-tag")
		{
			if(Value->HasTag(QueryPartSpecifications[1]) == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "has-field")
		{
			if(Value->HasField(QueryPartSpecifications[1]) == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "has-data")
		{
			if(Value->GetData().has_value() == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "is-value")
		{
			std::stringstream Output;
			
			Output << Value->GetData();
			if(Output.str() == QueryPartSpecifications[1])
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "type")
		{
			assert(QueryPartSpecifications.size() == 1);
			std::cout << GetTypeName(Value->GetData().type());
		}
		else
		{
			assert(false);
		}
	}
}