#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <deque>

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
	auto BitsPerSampleResult{Get_UnsignedInteger_5Bit(Buffer)};
	
	if(BitsPerSampleResult->GetSuccess() == true)
	{
		Value = BitsPerSampleResult->GetValue();
		Value->Append(Inspection::MakeValue("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(BitsPerSampleResult->GetAny()) + 1)));
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		Value->Append("Header", MetaDataBlockHeaderResult->GetValue());
		
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetAny("Interpretation"))};
		auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(MetaDataBlockHeaderResult->GetAny("Length"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockDataResult->GetSuccess() == true)
			{
				Value->Append("Data", StreamInfoBlockDataResult->GetValue());
				Success = true;
			}
		}
		else if(MetaDataBlockType == "Padding")
		{
			auto PaddingBlockDataResult{Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Buffer, MetaDataBlockDataLength)};
			
			if(PaddingBlockDataResult->GetSuccess() == true)
			{
				Value->Append("Data", PaddingBlockDataResult->GetValue());
				Success = true;
			}
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			if(MetaDataBlockDataLength % 18 == 0)
			{
				auto SeekTableBlockDataResult{Get_FLAC_SeekTableBlockData(Buffer, MetaDataBlockDataLength / 18)};
				
				if(SeekTableBlockDataResult->GetSuccess() == true)
				{
					Value->Append("Data", SeekTableBlockDataResult->GetValue());
					Success = true;
				}
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			auto VorbisCommentBlockDataResult{Get_FLAC_VorbisCommentBlockData(Buffer)};
			
			if(VorbisCommentBlockDataResult->GetSuccess() == true)
			{
				Value->Append("Data", VorbisCommentBlockDataResult->GetValue());
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
	auto LastMetaDataBlockResult{Get_Boolean_OneBit(Buffer)};
	
	if(LastMetaDataBlockResult->GetSuccess() == true)
	{
		Value->Append("LastMetaDataBlock", LastMetaDataBlockResult->GetValue());
		
		auto MetaDataBlockTypeResult{Get_FLAC_MetaDataBlockType(Buffer)};
		
		if(MetaDataBlockTypeResult->GetSuccess() == true)
		{
			Value->Append("BlockType", MetaDataBlockTypeResult->GetValue());
			
			auto LengthResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(LengthResult->GetSuccess() == true)
			{
				Value->Append("Length", LengthResult->GetValue());
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
	auto MetaDataBlockTypeResult{Get_UnsignedInteger_7Bit(Buffer)};
	
	if(MetaDataBlockTypeResult->GetSuccess() == true)
	{
		Success = true;
		Value->SetAny(MetaDataBlockTypeResult->GetAny());
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockTypeResult->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("StreamInfo")));
		}
		else if(NumericValue == 0x01)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("Padding")));
		}
		else if(NumericValue == 0x02)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("Application")));
		}
		else if(NumericValue == 0x03)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("SeekTable")));
		}
		else if(NumericValue == 0x04)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("VorbisComment")));
		}
		else if(NumericValue == 0x05)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("CueSheet")));
		}
		else if(NumericValue == 0x06)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("Picture")));
		}
		else if(NumericValue == 0xff)
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("Invalid")));
		}
		else
		{
			Value->Append(Inspection::MakeValue("Interpretation", std::string("Reserved")));
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_NumberOfChannels(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::shared_ptr< Inspection::Value > Value;
	auto NumberOfChannelsResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	if(NumberOfChannelsResult->GetSuccess() == true)
	{
		Value = NumberOfChannelsResult->GetValue();
		Value->Append(Inspection::MakeValue("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(NumberOfChannelsResult->GetAny()) + 1)));
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
				auto MetaDataBlockResult{Get_FLAC_MetaDataBlock(Buffer)};
				
				if(MetaDataBlockResult->GetSuccess() == true)
				{
					Value->Append("MetaDataBlock", MetaDataBlockResult->GetValue());
					LastMetaDataBlock = std::experimental::any_cast< bool >(MetaDataBlockResult->GetValue("Header")->GetAny("LastMetaDataBlock"));
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
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetAny("Interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			Value->Append("Header", MetaDataBlockHeaderResult->GetValue());
			
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockDataResult->GetSuccess() == true)
			{
				Value->Append("Data", StreamInfoBlockDataResult->GetValue());
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
	auto MinimumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	if(MinimumBlockSizeResult->GetSuccess() == true)
	{
		Value->Append("MinimumBlockSize", MinimumBlockSizeResult->GetValue());
		
		auto MaximumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		if(MaximumBlockSizeResult->GetSuccess() == true)
		{
			Value->Append("MaximumBlockSize", MaximumBlockSizeResult->GetValue());
			
			auto MinimumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(MinimumFrameSizeResult->GetSuccess() == true)
			{
				Value->Append("MinimumFrameSize", MinimumFrameSizeResult->GetValue());
				
				auto MaximumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
				
				if(MaximumFrameSizeResult->GetSuccess() == true)
				{
					Value->Append("MaximumFrameSize", MaximumFrameSizeResult->GetValue());
					
					auto SampleRateResult{Get_UnsignedInteger_20Bit_BigEndian(Buffer)};
					
					if(SampleRateResult->GetSuccess() == true)
					{
						Value->Append("SampleRate", SampleRateResult->GetValue());
						
						auto NumberOfChannelsResult{Get_FLAC_NumberOfChannels(Buffer)};
						
						if(NumberOfChannelsResult->GetSuccess() == true)
						{
							Value->Append("NumberOfChannels", NumberOfChannelsResult->GetValue());
							
							auto BitsPerSampleResult{Get_FLAC_BitsPerSample(Buffer)};
							
							if(BitsPerSampleResult->GetSuccess() == true)
							{
								Value->Append("BitsPerSample", BitsPerSampleResult->GetValue());
								
								auto TotalSamplesPerChannelResult{Get_UnsignedInteger_36Bit_BigEndian(Buffer)};
								
								if(TotalSamplesPerChannelResult->GetSuccess() == true)
								{
									Value->Append("TotalSamplesPerChannel", TotalSamplesPerChannelResult->GetValue());
									
									auto MD5SignatureOfUnencodedAudioDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 16)};
									
									if(MD5SignatureOfUnencodedAudioDataResult->GetSuccess() == true)
									{
										Value->Append("MD5SignatureOfUnencodedAudioData", MD5SignatureOfUnencodedAudioDataResult->GetValue());
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
	auto VorbisCommentHeaderResult{Get_Vorbis_CommentHeader(Buffer)};
	
	return Inspection::MakeResult(VorbisCommentHeaderResult->GetSuccess(), VorbisCommentHeaderResult->GetValue());
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto VendorLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(VendorLengthResult->GetSuccess() == true)
	{
		Value->Append("VendorLength", VendorLengthResult->GetValue());
		
		auto VendorLength{std::experimental::any_cast< std::uint32_t >(VendorLengthResult->GetAny())};
		auto VendorStringResult{Get_UTF8_String_EndedByLength(Buffer, VendorLength)};
		
		if(VendorStringResult->GetSuccess() == true)
		{
			Value->Append("VendorString", VendorStringResult->GetValue());
			
			auto UserCommentListResult{Get_Vorbis_CommentHeader_UserCommentList(Buffer)};
			
			if(UserCommentListResult->GetSuccess() == true)
			{
				Value->Append("UserCommentList", UserCommentListResult->GetValue());
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
	auto UserCommentLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentLengthResult->GetSuccess() == true)
	{
		Value->Append("Length", UserCommentLengthResult->GetValue());
		
		auto UserCommentLength{std::experimental::any_cast< std::uint32_t >(UserCommentLengthResult->GetAny())};
		auto UserCommentStringResult{Get_UTF8_String_EndedByLength(Buffer, UserCommentLength)};
		
		if(UserCommentStringResult->GetSuccess() == true)
		{
			Value->Append("String", UserCommentStringResult->GetValue());
			Success = true;
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto UserCommentListLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentListLengthResult->GetSuccess() == true)
	{
		Value->Append("Length", UserCommentListLengthResult->GetValue());
		Success = true;
		
		auto UserCommentListLength{std::experimental::any_cast< std::uint32_t >(UserCommentListLengthResult->GetAny())};
		
		for(std::uint32_t Index = 0ul; Index < UserCommentListLength; ++Index)
		{
			auto UserCommentResult{Get_Vorbis_CommentHeader_UserComment(Buffer)};
			
			if(UserCommentResult->GetSuccess() == true)
			{
				Value->Append("UserComment", UserCommentResult->GetValue());
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
