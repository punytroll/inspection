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

namespace FLAC
{
	enum class MetaDataBlockType
	{
		StreamInfo,
		Padding,
		Application,
		SeekTable,
		VorbisComment,
		CueSheet,
		Picture,
		Reserved,
		Invalid
	};
}

std::ostream & operator<<(std::ostream & OStream, FLAC::MetaDataBlockType Value)
{
	switch(Value)
	{
	case FLAC::MetaDataBlockType::StreamInfo:
		{
			return OStream << "StreamInfo";
		}
	case FLAC::MetaDataBlockType::Padding:
		{
			return OStream << "Padding";
		}
	case FLAC::MetaDataBlockType::Application:
		{
			return OStream << "Application";
		}
	case FLAC::MetaDataBlockType::SeekTable:
		{
			return OStream << "SeekTable";
		}
	case FLAC::MetaDataBlockType::VorbisComment:
		{
			return OStream << "VorbisComment";
		}
	case FLAC::MetaDataBlockType::CueSheet:
		{
			return OStream << "CueSheet";
		}
	case FLAC::MetaDataBlockType::Picture:
		{
			return OStream << "Picture";
		}
	case FLAC::MetaDataBlockType::Reserved:
		{
			return OStream << "Reserved";
		}
	case FLAC::MetaDataBlockType::Invalid:
	default:
		{
			return OStream << "Invalid";
		}
	}
}

#include "../common/any_printing.h"
#include "../common/file_handling.h"
#include "../common/5th/buffer.h"
#include "../common/5th/getters.h"
#include "../common/5th/result.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_FLAC_BitsPerSample(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_NumberOfChannels(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_SeekPoint(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlockData(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints);
std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlockData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlockData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_FLAC_BitsPerSample(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::shared_ptr< Inspection::Value > Value;
	auto BitsPerSample{Get_UnsignedInteger_5Bit(Buffer)};
	
	if(BitsPerSample->GetSuccess() == true)
	{
		Value = BitsPerSample->GetValue();
		Value->Append("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(BitsPerSample->GetAny()) + 1));
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MetaDataBlockHeader{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeader->GetSuccess() == true)
	{
		Value->Append("Header", MetaDataBlockHeader->GetValue());
		
		auto MetaDataBlockType{std::experimental::any_cast< FLAC::MetaDataBlockType >(MetaDataBlockHeader->GetValue("BlockType")->GetAny("Interpretation"))};
		auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(MetaDataBlockHeader->GetAny("Length"))};
		
		if(MetaDataBlockType == FLAC::MetaDataBlockType::StreamInfo)
		{
			auto StreamInfoBlockData{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockData->GetSuccess() == true)
			{
				Value->Append("Data", StreamInfoBlockData->GetValue());
				Success = true;
			}
		}
		else if(MetaDataBlockType == FLAC::MetaDataBlockType::Padding)
		{
			auto PaddingBlockData{Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Buffer, MetaDataBlockDataLength)};
			
			if(PaddingBlockData->GetSuccess() == true)
			{
				Value->Append("Data", PaddingBlockData->GetValue());
				Success = true;
			}
		}
		else if(MetaDataBlockType == FLAC::MetaDataBlockType::SeekTable)
		{
			if(MetaDataBlockDataLength % 18 == 0)
			{
				auto SeekTableBlockData{Get_FLAC_SeekTableBlockData(Buffer, MetaDataBlockDataLength / 18)};
				
				if(SeekTableBlockData->GetSuccess() == true)
				{
					Value->Append("Data", SeekTableBlockData->GetValue());
					Success = true;
				}
			}
		}
		else if(MetaDataBlockType == FLAC::MetaDataBlockType::VorbisComment)
		{
			auto VorbisCommentBlockData{Get_FLAC_VorbisCommentBlockData(Buffer)};
			
			if(VorbisCommentBlockData->GetSuccess() == true)
			{
				Value->Append("Data", VorbisCommentBlockData->GetValue());
				Success = true;
			}
		}
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockHeader(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto LastMetaDataBlock{Get_Boolean_OneBit(Buffer)};
	
	if(LastMetaDataBlock->GetSuccess() == true)
	{
		Value->Append("LastMetaDataBlock", LastMetaDataBlock->GetValue());
		
		auto MetaDataBlockType{Get_FLAC_MetaDataBlockType(Buffer)};
		
		if(MetaDataBlockType->GetSuccess() == true)
		{
			Value->Append("BlockType", MetaDataBlockType->GetValue());
			
			auto Length{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(Length->GetSuccess() == true)
			{
				Value->Append("Length", Length->GetValue());
				Success = true;
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockType(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MetaDataBlockType{Get_UnsignedInteger_7Bit(Buffer)};
	
	if(MetaDataBlockType->GetSuccess() == true)
	{
		Success = true;
		Value->SetAny(MetaDataBlockType->GetAny());
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockType->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::StreamInfo);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Padding);
		}
		else if(NumericValue == 0x02)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Application);
		}
		else if(NumericValue == 0x03)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::SeekTable);
		}
		else if(NumericValue == 0x04)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::VorbisComment);
		}
		else if(NumericValue == 0x05)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::CueSheet);
		}
		else if(NumericValue == 0x06)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Picture);
		}
		else if(NumericValue == 0xff)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Invalid);
		}
		else
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Reserved);
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_NumberOfChannels(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::shared_ptr< Inspection::Value > Value;
	auto NumberOfChannels{Get_UnsignedInteger_3Bit(Buffer)};
	
	if(NumberOfChannels->GetSuccess() == true)
	{
		Value = NumberOfChannels->GetValue();
		Value->Append("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(NumberOfChannels->GetAny()) + 1));
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_SeekPoint(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto SampleNumberOfFirstSampleInTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
	
	if(SampleNumberOfFirstSampleInTargetFrameResult->GetSuccess() == true)
	{
		Value->Append("SampleNumberOfFirstSampleInTargetFrame", SampleNumberOfFirstSampleInTargetFrameResult->GetValue());
		
		auto ByteOffsetOfTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
		
		if(ByteOffsetOfTargetFrameResult->GetSuccess() == true)
		{
			Value->Append("ByteOffsetOfTargetFrame", ByteOffsetOfTargetFrameResult->GetValue());
			
			auto NumberOfSamplesInTargetFrameResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
			
			if(NumberOfSamplesInTargetFrameResult->GetSuccess() == true)
			{
				Value->Append("NumberOfSamplesInTargetFrame", NumberOfSamplesInTargetFrameResult->GetValue());
				Success = true;
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlockData(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints)
{
	auto Success{true};
	auto Value{std::make_shared< Inspection::Value >()};
	
	for(auto SeekPointIndex = 0ul; SeekPointIndex < NumberOfSeekPoints; ++SeekPointIndex)
	{
		auto SeekPointResult{Get_FLAC_SeekPoint(Buffer)};
		
		if(SeekPointResult->GetSuccess() == true)
		{
			Value->Append("SeekPoint", SeekPointResult->GetValue());
		}
		else
		{
			Success = false;
			
			break;
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto FLACStreamMarkerResult{Get_ASCII_String_Alphabetical_EndedTemplateByLength(Buffer, "fLaC")};
	
	if(FLACStreamMarkerResult->GetSuccess() == true)
	{
		Value->Append("FLAC stream marker", FLACStreamMarkerResult->GetValue());
		
		auto FLACStreamInfoBlockResult{Get_FLAC_StreamInfoBlock(Buffer)};
		
		if(FLACStreamInfoBlockResult->GetSuccess() == true)
		{
			Value->Append("StreamInfoBlock", FLACStreamInfoBlockResult->GetValue());
			
			auto LastMetaDataBlock{std::experimental::any_cast< bool >(FLACStreamInfoBlockResult->GetValue("Header")->GetAny("LastMetaDataBlock"))};
			
			Success = true;
			while(LastMetaDataBlock == false)
			{
				auto MetaDataBlock{Get_FLAC_MetaDataBlock(Buffer)};
				
				if(MetaDataBlock->GetSuccess() == true)
				{
					Value->Append("MetaDataBlock", MetaDataBlock->GetValue());
					LastMetaDataBlock = std::experimental::any_cast< bool >(MetaDataBlock->GetValue("Header")->GetAny("LastMetaDataBlock"));
				}
				else
				{
					Success = false;
					
					break;
				}
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MetaDataBlockHeader{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeader->GetSuccess() == true)
	{
		auto MetaDataBlockType{std::experimental::any_cast< FLAC::MetaDataBlockType >(MetaDataBlockHeader->GetValue("BlockType")->GetAny("Interpretation"))};
		
		if(MetaDataBlockType == FLAC::MetaDataBlockType::StreamInfo)
		{
			Value->Append("Header", MetaDataBlockHeader->GetValue());
			
			auto StreamInfoBlockData{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockData->GetSuccess() == true)
			{
				Value->Append("Data", StreamInfoBlockData->GetValue());
				Success = true;
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlockData(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MinimumBlockSize{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	if(MinimumBlockSize->GetSuccess() == true)
	{
		Value->Append("MinimumBlockSize", MinimumBlockSize->GetValue());
		
		auto MaximumBlockSize{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		if(MaximumBlockSize->GetSuccess() == true)
		{
			Value->Append("MaximumBlockSize", MaximumBlockSize->GetValue());
			
			auto MinimumFrameSize{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(MinimumFrameSize->GetSuccess() == true)
			{
				Value->Append("MinimumFrameSize", MinimumFrameSize->GetValue());
				
				auto MaximumFrameSize{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
				
				if(MaximumFrameSize->GetSuccess() == true)
				{
					Value->Append("MaximumFrameSize", MaximumFrameSize->GetValue());
					
					auto SampleRate{Get_UnsignedInteger_20Bit_BigEndian(Buffer)};
					
					if(SampleRate->GetSuccess() == true)
					{
						Value->Append("SampleRate", SampleRate->GetValue());
						
						auto NumberOfChannels{Get_FLAC_NumberOfChannels(Buffer)};
						
						if(NumberOfChannels->GetSuccess() == true)
						{
							Value->Append("NumberOfChannels", NumberOfChannels->GetValue());
							
							auto BitsPerSample{Get_FLAC_BitsPerSample(Buffer)};
							
							if(BitsPerSample->GetSuccess() == true)
							{
								Value->Append("BitsPerSample", BitsPerSample->GetValue());
								
								auto TotalSamplesPerChannel{Get_UnsignedInteger_36Bit_BigEndian(Buffer)};
								
								if(TotalSamplesPerChannel->GetSuccess() == true)
								{
									Value->Append("TotalSamplesPerChannel", TotalSamplesPerChannel->GetValue());
									
									auto MD5SignatureOfUnencodedAudioData{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 16)};
									
									if(MD5SignatureOfUnencodedAudioData->GetSuccess() == true)
									{
										Value->Append("MD5SignatureOfUnencodedAudioData", MD5SignatureOfUnencodedAudioData->GetValue());
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

std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlockData(Inspection::Buffer & Buffer)
{
	auto VorbisCommentHeader{Get_Vorbis_CommentHeader(Buffer)};
	
	return Inspection::MakeResult(VorbisCommentHeader->GetSuccess(), VorbisCommentHeader->GetValue());
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto VendorLength{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(VendorLength->GetSuccess() == true)
	{
		Value->Append("VendorLength", VendorLength->GetValue());
		
		auto VendorLengthValue{std::experimental::any_cast< std::uint32_t >(VendorLength->GetAny())};
		auto VendorString{Get_UTF8_String_EndedByLength(Buffer, VendorLengthValue)};
		
		if(VendorString->GetSuccess() == true)
		{
			Value->Append("VendorString", VendorString->GetValue());
			
			auto UserCommentList{Get_Vorbis_CommentHeader_UserCommentList(Buffer)};
			
			if(UserCommentList->GetSuccess() == true)
			{
				Value->Append("UserCommentList", UserCommentList->GetValue());
				Success = true;
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto UserCommentLength{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentLength->GetSuccess() == true)
	{
		Value->Append("Length", UserCommentLength->GetValue());
		
		auto UserCommentLengthValue{std::experimental::any_cast< std::uint32_t >(UserCommentLength->GetAny())};
		auto UserCommentString{Get_UTF8_String_EndedByLength(Buffer, UserCommentLengthValue)};
		
		if(UserCommentString->GetSuccess() == true)
		{
			Value->Append("String", UserCommentString->GetValue());
			Success = true;
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto UserCommentListLength{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentListLength->GetSuccess() == true)
	{
		Value->Append("Length", UserCommentListLength->GetValue());
		Success = true;
		
		auto UserCommentListLengthValue{std::experimental::any_cast< std::uint32_t >(UserCommentListLength->GetAny())};
		
		for(std::uint32_t Index = 0ul; Index < UserCommentListLengthValue; ++Index)
		{
			auto UserComment{Get_Vorbis_CommentHeader_UserComment(Buffer)};
			
			if(UserComment->GetSuccess() == true)
			{
				Value->Append("UserComment", UserComment->GetValue());
			}
			else
			{
				Success = false;
				
				break;
			}
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

void PrintValue(std::shared_ptr< Inspection::Value > Value)
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
				Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
				auto FLACStream{Get_FLAC_Stream(Buffer)};
				
				if(FLACStream->GetSuccess() == true)
				{
					FLACStream->GetValue()->SetName("FLACStream");
					PrintValue(FLACStream->GetValue());
				}
				else
				{
					std::cerr << "The file does not start with a FLACStream." << std::endl;
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
