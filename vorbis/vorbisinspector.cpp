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
std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t NumberOfEntries);
std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, std::uint64_t Length);
std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto HeaderTypeResult{Get_BitSet_8Bit(Buffer)};
	
	if(HeaderTypeResult->GetSuccess() == true)
	{
		Result->SetValue(HeaderTypeResult->GetValue());
		Result->SetSuccess(true);
		
		const std::bitset< 8 > & HeaderType{std::experimental::any_cast< const std::bitset< 8 > & >(HeaderTypeResult->GetAny())};
		
		Result->GetValue()->Append("Continuation", HeaderType[0]);
		Result->GetValue()->Append("BeginOfStream", HeaderType[1]);
		Result->GetValue()->Append("EndOfStream", HeaderType[2]);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t NumberOfEntries)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto SegmentTableResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, NumberOfEntries)};
	
	if(SegmentTableResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::vector< std::uint8_t > & SegmentTable{std::experimental::any_cast< std::vector< std::uint8_t > >(SegmentTableResult->GetAny())};
		
		for(auto SegmentTableEntry : SegmentTable)
		{
			Result->GetValue()->Append("", SegmentTableEntry);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto CapturePatternResult{Get_ASCII_String_Alphabetical_EndedTemplateByLength(Buffer, "OggS")};
	
	if(CapturePatternResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("CapturePattern", CapturePatternResult->GetValue());
		
		auto StreamStructureVersionResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		if(StreamStructureVersionResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("StreamStructureVersion", StreamStructureVersionResult->GetValue());
			
			auto HeaderTypeResult{Get_Ogg_Page_HeaderType(Buffer)};
			
			if(HeaderTypeResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("HeaderType", HeaderTypeResult->GetValue());
				
				auto GranulePositionResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
				
				if(GranulePositionResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("GranulePosition", GranulePositionResult->GetValue());
					
					auto BitStreamSerialNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					if(BitStreamSerialNumberResult->GetSuccess() == true)
					{
						Result->GetValue()->Append("BitStreamSerialNumber", BitStreamSerialNumberResult->GetValue());
						
						auto PageSequenceNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						if(PageSequenceNumberResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("PageSequenceNumber", PageSequenceNumberResult->GetValue());
							
							auto ChecksumResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							if(ChecksumResult->GetSuccess() == true)
							{
								Result->GetValue()->Append("Checksum", ChecksumResult->GetValue());
								
								auto PageSegmentsResult{Get_UnsignedInteger_8Bit(Buffer)};
								
								if(PageSegmentsResult->GetSuccess() == true)
								{
									Result->GetValue()->Append("PageSegments", PageSegmentsResult->GetValue());
									
									auto PageSegments{std::experimental::any_cast< std::uint8_t >(PageSegmentsResult->GetAny())};
									auto SegmentTableResult{Get_Ogg_Page_SegmentTable(Buffer, PageSegments)};
									
									if(SegmentTableResult->GetSuccess() == true)
									{
										Result->GetValue()->Append("SegmentTable", SegmentTableResult->GetValue());
										Result->SetSuccess(true);
										
										for(auto SegmentTableEntryValue : SegmentTableResult->GetValue()->GetValues())
										{
											auto SegmentTableEntry{std::experimental::any_cast< std::uint8_t >(SegmentTableEntryValue->GetAny())};
											
											// data interpretation:
											// try different segment contents
											auto VorbisHeaderPacketResult{Get_Vorbis_HeaderPacket(Buffer, static_cast< std::uint64_t >(SegmentTableEntry))};
											
											if(VorbisHeaderPacketResult->GetSuccess() == true)
											{
												Result->GetValue()->Append("Segment", VorbisHeaderPacketResult->GetValue());
											}
											else
											{
												Result->SetSuccess(false);
												
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto StreamStarted{false};
	auto StreamEnded{false};
	
	while(StreamEnded == false)
	{
		auto OggPageResult{Get_Ogg_Page(Buffer)};
		
		if(OggPageResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("OggPage", OggPageResult->GetValue());
			
			bool BeginOfStream{std::experimental::any_cast< bool >(OggPageResult->GetValue("HeaderType")->GetAny("BeginOfStream"))};
			
			if(BeginOfStream == true)
			{
				if(StreamStarted == false)
				{
					StreamStarted = true;
					Result->SetSuccess(true);
				}
				else
				{
					Result->SetSuccess(false);
					
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
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto PacketTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	if(PacketTypeResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("PacketType", PacketTypeResult->GetValue());
		
		auto VorbisIdentifierResult{Get_ASCII_String_Alphabetical_EndedTemplateByLength(Buffer, "vorbis")};
		
		if(VorbisIdentifierResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("VorbisIdentifier", VorbisIdentifierResult->GetValue());
			
			auto PacketType{std::experimental::any_cast< std::uint8_t >(PacketTypeResult->GetAny())};
			
			if(PacketType == 0x01)
			{
				auto IdentificationHeader{Get_Vorbis_IdentificationHeader(Buffer)};
				
				if(IdentificationHeader->GetSuccess() == true)
				{
					Result->GetValue()->Append(IdentificationHeader->GetValue()->GetValues());
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto VorbisVersionResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(VorbisVersionResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("VorbisVersion", VorbisVersionResult->GetValue());
		
		auto AudioChannelsResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		if(AudioChannelsResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("AudioChannels", AudioChannelsResult->GetValue());
			
			auto AudioSampleRateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			if(AudioSampleRateResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("AudioSampleRate", AudioSampleRateResult->GetValue());
				
				auto BitrateMaximumResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
				
				if(BitrateMaximumResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("BitrateMaximum", BitrateMaximumResult->GetValue());
					
					auto BitrateNominalResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
					
					if(BitrateNominalResult->GetSuccess() == true)
					{
						Result->GetValue()->Append("BitrateNominal", BitrateNominalResult->GetValue());
						
						auto BitrateMinimumResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
						
						if(BitrateMinimumResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("BitrateMinimum", BitrateMinimumResult->GetValue());
							
							auto BlockSize0Result{Get_UnsignedInteger_4Bit(Buffer)};
							
							if(BlockSize0Result->GetSuccess() == true)
							{
								Result->GetValue()->Append("BlockSize0", BlockSize0Result->GetValue());
								
								auto BlockSize1Result{Get_UnsignedInteger_4Bit(Buffer)};
								
								if(BlockSize1Result->GetSuccess() == true)
								{
									Result->GetValue()->Append("BlockSize1", BlockSize1Result->GetValue());
									
									auto FramingFlagResult{Get_UnsignedInteger_1Bit(Buffer)};
									
									if(FramingFlagResult->GetSuccess() == true)
									{
										Result->GetValue()->Append("FramingFlag", FramingFlagResult->GetValue());
										Result->SetSuccess(true);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
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
				
				Buffer.SetBitstreamReaderType(Inspection::Buffer::BitstreamReaderType::LeastSignificantBitFirst);
				
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
