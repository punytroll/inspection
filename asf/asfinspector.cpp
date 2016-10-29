#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/any_printing.h"
#include "../common/file_handling.h"
#include "../common/getters/4th.h"
#include "../common/guid.h"
#include "../common/results.h"

GUID g_ASFHeaderObjectGUID{"75b22630-668e-11cf-a6d9-00aa0062ce6c"};
GUID g_ASFDataObjectGUID{"75b22636-668e-11cf-a6d9-00aa0062ce6c"};
GUID g_ASFSimpleIndexObjectGUID{"33000890-e5b1-11cf-89f4-00a0c90349cb"};
GUID g_ASFIndexObjectGUID{"D6E229D3-35DA-11D1-9034-00A0C90349BE"};
GUID g_ASFMediaObjectIndexObjectGUID{"feb103f8-12ad-4c64-840f-2a1d2f7ad48c"};
GUID g_ASFTimecodeIndexObject{"3cb73fd0-0c4a-4803-953d-edf7b6228f0c"};

///////////////////////////////////////////////////////////////////////////////////////////////////
// 4th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Results::Result > Get_ASF_GUID(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_Object(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_HexadecimalString_TerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);

std::unique_ptr< Results::Result > Get_ASF_GUID(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer + Index, Length - Index)};
	
	if(GUIDResult->GetSuccess() == true)
	{
		Success = true;
		Index += GUIDResult->GetLength();
		
		auto GUIDValue{std::experimental::any_cast< GUID >(GUIDResult->GetAny())};
		
		Value->Append("Nominal value", GUIDValue);
		if(GUIDValue == g_ASFHeaderObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Header_Object"));
		}
		else if(GUIDValue == g_ASFDataObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Data_Object"));
		}
		else if(GUIDValue == g_ASFSimpleIndexObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Simple_Index_Object"));
		}
		else if(GUIDValue == g_ASFIndexObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Index_Object"));
		}
		else if(GUIDValue == g_ASFMediaObjectIndexObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Media_Object_Index_Object"));
		}
		else if(GUIDValue == g_ASFTimecodeIndexObject)
		{
			Value->Append("Interpretation", std::string("ASF_Timecode_Index_Object"));
		}
		else
		{
			Value->Append("Interpretation", std::string("<unknown GUID value>"));
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_Object(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto ObjectGUID{Get_ASF_GUID(Buffer + Index, Length - Index)};
	
	if(ObjectGUID->GetSuccess() == true)
	{
		Value->SetName("ASF Object");
		Index += ObjectGUID->GetLength();
		Value->Append("GUID", ObjectGUID->GetValue());
		
		auto ObjectSize{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
		
		if(ObjectSize->GetSuccess() == true)
		{
			Index += ObjectSize->GetLength();
			Value->Append("Size", ObjectSize->GetValue());
			
			auto DataSize{std::experimental::any_cast< std::uint64_t >(ObjectSize->GetAny())};
			
			if(DataSize <= Length)
			{
				DataSize -= Index;
				
				auto ObjectData{Get_HexadecimalString_TerminatedByLength(Buffer + Index, DataSize)};
				
				if(ObjectData->GetSuccess() == true)
				{
					Index += ObjectData->GetLength();
					Value->Append("Data", ObjectData->GetValue());
					Success = true;
				}
			}
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_HexadecimalString_TerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{true};
	auto Index{0ull};
	std::stringstream StringStream;
	
	StringStream << std::hex << std::setfill('0');
	while(Index < Length)
	{
		if(Index > 0)
		{
			StringStream << ' ';
		}
		StringStream << std::setw(2) << std::right << static_cast< std::uint32_t >(Buffer[Index]);
		Index += 1;
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >("", StringStream.str())));
}

void PrintValue(const std::string & Indentation, std::shared_ptr< Results::Value > Value)
{
	auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetAny().empty() == false)};
	
	if(HeaderLine == true)
	{
		std::cout << Indentation;
	}
	if(Value->GetName().empty() == false)
	{
		std::cout << Value->GetName() << ": ";
	}
	if(Value->GetAny().empty() == false)
	{
		std::cout << Value->GetAny();
	}
	
	auto SubIndentation{Indentation};
	
	if(HeaderLine == true)
	{
		std::cout << std::endl;
		SubIndentation += "    ";
	}
	if(Value->GetCount() > 0)
	{
		for(auto & SubValue : Value->GetValues())
		{
			PrintValue(SubIndentation, SubValue);
		}
	}
}

void PrintValue(std::shared_ptr< Results::Value > Value)
{
	PrintValue("", Value);
}

void ReadFile(const std::string & Path)
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
				std::int64_t Index{0};
				std::int64_t Skipping{0};
				
				while(Index < FileSize)
				{
					auto ASFObject{Get_ASF_Object(Address + Index, FileSize - Index)};
					
					if(ASFObject->GetSuccess() == true)
					{
						if(Skipping > 0)
						{
							std::cerr << "Skipping " << Skipping << " bytes of unrecognized data." << std::endl;
							Skipping = 0;
						}
						Index += ASFObject->GetLength();
						PrintValue(ASFObject->GetValue());
					}
					else
					{
						Skipping += 1;
						Index += 1;
					}
				}
				if(Skipping > 0)
				{
					std::cerr << "Skipping " << Skipping << " bytes of unrecognized data." << std::endl;
					Skipping = 0;
				}
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
}

int main(int argc, char ** argv)
{
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto Argument{0};
	
	while(++Argument < Arguments)
	{
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front());
		Paths.pop_front();
	}
	
	return 0;
}
