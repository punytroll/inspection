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
#include "../common/5th/buffer.h"
#include "../common/5th/getters.h"
#include "../common/5th/result.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t Length);
std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_VorbisHeaderPacket(Inspection::Buffer & Buffer);

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

std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::shared_ptr< Inspection::Value > Value;
	auto HeaderTypeResult{Get_BitSet_8Bit(Buffer)};
	
	if(HeaderTypeResult->GetSuccess() == true)
	{
		Success = true;
		Value = HeaderTypeResult->GetValue();
		
		const std::bitset< 8 > & HeaderType{std::experimental::any_cast< const std::bitset< 8 > & >(HeaderTypeResult->GetAny())};
		
		Value->Append("Continuation", HeaderType[0]);
		Value->Append("BeginOfStream", HeaderType[1]);
		Value->Append("EndOfStream", HeaderType[2]);
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t Length)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto SegmentTableResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
	
	if(SegmentTableResult->GetSuccess() == true)
	{
		Success = true;
		
		const std::vector< std::uint8_t > & SegmentTable{std::experimental::any_cast< std::vector< std::uint8_t > >(SegmentTableResult->GetAny())};
		
		for(auto SegmentTableEntry : SegmentTable)
		{
			Value->Append(SegmentTableEntry);
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto CapturePatternResult{Get_ASCII_String_Alphabetical_EndedTemplateByLength(Buffer, "OggS")};
	
	if(CapturePatternResult->GetSuccess() == true)
	{
		Value->Append("CapturePattern", CapturePatternResult->GetValue());
		
		auto StreamStructureVersionResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		if(StreamStructureVersionResult->GetSuccess() == true)
		{
			Value->Append("StreamStructureVersion", StreamStructureVersionResult->GetValue());
			
			auto HeaderTypeResult{Get_Ogg_Page_HeaderType(Buffer)};
			
			if(HeaderTypeResult->GetSuccess() == true)
			{
				Value->Append("HeaderType", HeaderTypeResult->GetValue());
				
				auto GranulePositionResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
				
				if(GranulePositionResult->GetSuccess() == true)
				{
					Value->Append("GranulePosition", GranulePositionResult->GetValue());
					
					auto BitStreamSerialNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					if(BitStreamSerialNumberResult->GetSuccess() == true)
					{
						Value->Append("BitStreamSerialNumber", BitStreamSerialNumberResult->GetValue());
						
						auto PageSequenceNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						if(PageSequenceNumberResult->GetSuccess() == true)
						{
							Value->Append("PageSequenceNumber", PageSequenceNumberResult->GetValue());
							
							auto ChecksumResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							if(ChecksumResult->GetSuccess() == true)
							{
								Value->Append("Checksum", ChecksumResult->GetValue());
								
								auto PageSegmentsResult{Get_UnsignedInteger_8Bit(Buffer)};
								
								if(PageSegmentsResult->GetSuccess() == true)
								{
									Value->Append("PageSegments", PageSegmentsResult->GetValue());
									
									auto PageSegments{std::experimental::any_cast< std::uint8_t >(PageSegmentsResult->GetAny())};
									auto SegmentTableResult{Get_Ogg_Page_SegmentTable(Buffer, PageSegments)};
									
									if(SegmentTableResult->GetSuccess() == true)
									{
										Value->Append("SegmentTable", SegmentTableResult->GetValue());
										Success = true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto StreamStarted{false};
	auto StreamEnded{false};
	auto Value{std::make_shared< Inspection::Value >()};
	
	while(StreamEnded == false)
	{
		auto OggPageResult{Get_Ogg_Page(Buffer)};
		
		if(OggPageResult->GetSuccess() == true)
		{
			Value->Append("OggPage", OggPageResult->GetValue());
			
			bool BeginOfStream{std::experimental::any_cast< bool >(OggPageResult->GetValue("HeaderType")->GetAny("BeginOfStream"))};
			
			if(BeginOfStream == true)
			{
				if(StreamStarted == false)
				{
					StreamStarted = true;
					Success = true;
				}
				else
				{
					Success = false;
					
					break;
				}
			}
			StreamEnded = std::experimental::any_cast< bool >(OggPageResult->GetValue("HeaderType")->GetAny("EndOfStream"));
		}
		else
		{
			// Success = false;
			
			break;
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

void PrintValue(const std::string & Indentation, std::shared_ptr< Inspection::Value > Value)
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
				Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
				auto OggStreamResult(Get_Ogg_Stream(Buffer));
				
				if(OggStreamResult->GetSuccess() == true)
				{
					OggStreamResult->GetValue()->SetName("OggStream");
					PrintValue("", OggStreamResult->GetValue());
				}
				else
				{
					std::cerr << "The file does not start with an OggStream." << std::endl;
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
