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
#include "../common/results.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 4th generation getters                                                                        //
//   - These functions validate and extract in one go.                                           //
//   - They return a unique_ptr to an instance of type Result                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Results::Result > Get_VorbisHeaderPacket(const std::uint8_t * Buffer, std::uint64_t Length);

//~ std::unique_ptr< Results::Result > Get_VorbisHeaderPacket(const std::uint8_t * Buffer, std::uint64_t Length)
//~ {
	//~ auto Success{false};
	//~ auto Index{0ull};
	//~ auto Value{std::make_shared< Results::Value >()};
	//~ auto PacketTypeResult{Get_8Bit_UnsignedInteger(Buffer + Index, Length - Index)};
	
	//~ if(PacketTypeResult->GetSuccess() == true)
	//~ {
		//~ Index += PacketTypeResult->GetLength();
		//~ Value->Append("PacketType", PacketTypeResult->GetValue());
		//~ if(Length - Index >= 6ull)
		//~ {
			//~ auto VorbisIdentifierResult{Get_ASCII_AlphaStringTerminatedByLength(Buffer + Index, 6ull)};
			
			//~ if((VorbisIdentifierResult->GetSuccess() == true) && (std::experimental::any_cast< std::string >(VorbisIdentifierResult->GetAny("VorbisIdentifier")) == "vorbis"))
			//~ {
				//~ Index += VorbisIdentifierResult->GetLength();
				//~ Value->Append("VorbisIdentifier", VorbisIdentifierResult->GetValue());
				//~ Success = true;
			//~ }
		//~ }
		//~ Success = true;
	//~ }
	
	//~ return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
//~ }

std::unique_ptr< Results::Result > Get_VorbisPage(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	
	if(Length - Index >= 4ull)
	{
		auto CapturePatternResult{Get_ASCII_AlphaString_EndedByLength(Buffer + Index, 4ull)};
		
		if((CapturePatternResult->GetSuccess() == true) && (std::experimental::any_cast< std::string >(CapturePatternResult->GetAny()) == "OggS"))
		{
			Index += CapturePatternResult->GetLength();
			Value->Append("CapturePattern", CapturePatternResult->GetValue());
			
			auto StreamStructureVersionResult{Get_UnsignedInteger_8Bit(Buffer + Index, Length - Index)};
			
			if(StreamStructureVersionResult->GetSuccess() == true)
			{
				Index += StreamStructureVersionResult->GetLength();
				Value->Append("StreamStructureVersion", StreamStructureVersionResult->GetValue());
				
				auto HeaderTypeFlagResult{Get_BitSet_8Bit(Buffer + Index, Length - Index)};
				
				if(HeaderTypeFlagResult->GetSuccess() == true)
				{
					Index += HeaderTypeFlagResult->GetLength();
					Value->Append("HeaderTypeFlag", HeaderTypeFlagResult->GetValue());
					
					auto GranulePositionResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
					
					if(GranulePositionResult->GetSuccess() == true)
					{
						Index += HeaderTypeFlagResult->GetLength();
						Value->Append("GranulePosition", GranulePositionResult->GetValue());
						Success = true;
					}
				}
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
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
	if(HeaderLine == true)
	{
		std::cout << std::endl;
	}
	if(Value->GetCount() > 0)
	{
		for(auto & SubValue : Value->GetValues())
		{
			PrintValue(Indentation + "    ", SubValue);
		}
	}
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
				
				while(Index < FileSize)
				{
					auto VorbisPageResult(Get_VorbisPage(Address + Index, FileSize - Index));
					
					if(VorbisPageResult->GetSuccess() == true)
					{
						Index += VorbisPageResult->GetLength();
						PrintValue("", VorbisPageResult->GetValue());
					}
					else
					{
						Index += 1;
					}
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
