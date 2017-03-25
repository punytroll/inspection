#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <deque>

#include "../common/5th.h"
#include "../common/any_printing.h"
#include "../common/file_handling.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_FLAC_ApplicationBlock_Data(Inspection::Buffer & Buffer, std::uint64_t Length);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Type(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_Data(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_Data(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints);
std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_SeekPoint(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_Data(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_NumberOfChannels(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlock_Data(Inspection::Buffer & Buffer);


std::unique_ptr< Inspection::Result > Get_FLAC_ApplicationBlock_Data(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto RegisteredApplicationIdentifierResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
	
	Result->GetValue()->Append("RegisteredApplicationIdentifier", RegisteredApplicationIdentifierResult->GetValue());
	if(RegisteredApplicationIdentifierResult->GetSuccess() == true)
	{
		auto ApplicationDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length - 4)};
		
		Result->GetValue()->Append("ApplicationData", ApplicationDataResult->GetValue());
		if(ApplicationDataResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlock_Header(Buffer)};
	
	Result->GetValue()->Append("Header", MetaDataBlockHeaderResult->GetValue());
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetAny("Interpretation"))};
		auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(MetaDataBlockHeaderResult->GetAny("Length"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", StreamInfoBlockDataResult->GetValue());
			if(StreamInfoBlockDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "Padding")
		{
			auto PaddingBlockDataResult{Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Buffer, MetaDataBlockDataLength)};
			
			Result->GetValue()->Append("Data", PaddingBlockDataResult->GetValue());
			if(PaddingBlockDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "Application")
		{
			auto ApplicationBlockDataResult{Get_FLAC_ApplicationBlock_Data(Buffer, MetaDataBlockDataLength)};
			
			Result->GetValue()->Append("Data", ApplicationBlockDataResult->GetValue());
			if(ApplicationBlockDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			if(MetaDataBlockDataLength % 18 == 0)
			{
				auto SeekTableBlockDataResult{Get_FLAC_SeekTableBlock_Data(Buffer, MetaDataBlockDataLength / 18)};
				
				Result->GetValue()->Append("Data", SeekTableBlockDataResult->GetValue());
				if(SeekTableBlockDataResult->GetSuccess() == true)
				{
					Result->SetSuccess(true);
				}
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			auto VorbisCommentBlockDataResult{Get_FLAC_VorbisCommentBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", VorbisCommentBlockDataResult->GetValue());
			if(VorbisCommentBlockDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "Picture")
		{
			auto PictureBlockDataResult{Get_FLAC_PictureBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", PictureBlockDataResult->GetValue());
			if(PictureBlockDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto LastMetaDataBlockResult{Get_Boolean_1Bit(Buffer)};
	
	Result->GetValue()->Append("LastMetaDataBlock", LastMetaDataBlockResult->GetValue());
	if(LastMetaDataBlockResult->GetSuccess() == true)
	{
		auto MetaDataBlockTypeResult{Get_FLAC_MetaDataBlock_Type(Buffer)};
		
		Result->GetValue()->Append("BlockType", MetaDataBlockTypeResult->GetValue());
		if(MetaDataBlockTypeResult->GetSuccess() == true)
		{
			auto LengthResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			Result->GetValue()->Append("Length", LengthResult->GetValue());
			if(LengthResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Type(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MetaDataBlockTypeResult{Get_UnsignedInteger_7Bit(Buffer)};
	
	Result->GetValue()->SetAny(MetaDataBlockTypeResult->GetAny());
	if(MetaDataBlockTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockTypeResult->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Result->GetValue()->Append("Interpretation", std::string("StreamInfo"));
		}
		else if(NumericValue == 0x01)
		{
			Result->GetValue()->Append("Interpretation", std::string("Padding"));
		}
		else if(NumericValue == 0x02)
		{
			Result->GetValue()->Append("Interpretation", std::string("Application"));
		}
		else if(NumericValue == 0x03)
		{
			Result->GetValue()->Append("Interpretation", std::string("SeekTable"));
		}
		else if(NumericValue == 0x04)
		{
			Result->GetValue()->Append("Interpretation", std::string("VorbisComment"));
		}
		else if(NumericValue == 0x05)
		{
			Result->GetValue()->Append("Interpretation", std::string("CueSheet"));
		}
		else if(NumericValue == 0x06)
		{
			Result->GetValue()->Append("Interpretation", std::string("Picture"));
		}
		else if(NumericValue == 0xff)
		{
			Result->GetValue()->Append("Interpretation", std::string("Invalid"));
		}
		else
		{
			Result->GetValue()->Append("Interpretation", std::string("Reserved"));
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto PictureType{std::experimental::any_cast< std::uint32_t >(PictureTypeResult->GetAny())};
		
		if(PictureType == 0)
		{
			Result->GetValue()->Append("Interpretation", std::string("Other"));
		}
		else if(PictureType == 1)
		{
			Result->GetValue()->Append("Interpretation", std::string("32x32 pixels 'file icon' (PNG only)"));
		}
		else if(PictureType == 2)
		{
			Result->GetValue()->Append("Interpretation", std::string("Other file icon"));
		}
		else if(PictureType == 3)
		{
			Result->GetValue()->Append("Interpretation", std::string("Cover (front)"));
		}
		else if(PictureType == 4)
		{
			Result->GetValue()->Append("Interpretation", std::string("Cover (back)"));
		}
		else if(PictureType == 5)
		{
			Result->GetValue()->Append("Interpretation", std::string("Leaflet page"));
		}
		else if(PictureType == 6)
		{
			Result->GetValue()->Append("Interpretation", std::string("Media (e.g. label side of CD"));
		}
		else if(PictureType == 7)
		{
			Result->GetValue()->Append("Interpretation", std::string("Lead artist/lead performer/soloist"));
		}
		else if(PictureType == 8)
		{
			Result->GetValue()->Append("Interpretation", std::string("Artist/performer"));
		}
		else if(PictureType == 9)
		{
			Result->GetValue()->Append("Interpretation", std::string("Conductor"));
		}
		else if(PictureType == 10)
		{
			Result->GetValue()->Append("Interpretation", std::string("Band/Orchestra"));
		}
		else if(PictureType == 11)
		{
			Result->GetValue()->Append("Interpretation", std::string("Composer"));
		}
		else if(PictureType == 12)
		{
			Result->GetValue()->Append("Interpretation", std::string("Lyricist/text writer"));
		}
		else if(PictureType == 13)
		{
			Result->GetValue()->Append("Interpretation", std::string("Recording Location"));
		}
		else if(PictureType == 14)
		{
			Result->GetValue()->Append("Interpretation", std::string("During recording"));
		}
		else if(PictureType == 15)
		{
			Result->GetValue()->Append("Interpretation", std::string("During performance"));
		}
		else if(PictureType == 16)
		{
			Result->GetValue()->Append("Interpretation", std::string("Movie/video screen capture"));
		}
		else if(PictureType == 17)
		{
			Result->GetValue()->Append("Interpretation", std::string("A bright coloured fish"));
		}
		else if(PictureType == 18)
		{
			Result->GetValue()->Append("Interpretation", std::string("Illustration"));
		}
		else if(PictureType == 19)
		{
			Result->GetValue()->Append("Interpretation", std::string("Band/artist logotype"));
		}
		else if(PictureType == 20)
		{
			Result->GetValue()->Append("Interpretation", std::string("Publisher/Studio logotype"));
		}
		else
		{
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_Data(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto PictureTypeResult{Get_FLAC_PictureBlock_PictureType(Buffer)};
	
	Result->GetValue()->Append("PictureType", PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto MIMETypeLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("MIMETypeLength", MIMETypeLengthResult->GetValue());
		if(MIMETypeLengthResult->GetSuccess() == true)
		{
			auto MIMETypeLength{std::experimental::any_cast< std::uint32_t >(MIMETypeLengthResult->GetAny())};
			auto MIMETypeResult{Get_ASCII_String_Printable_EndedByByteLength(Buffer, MIMETypeLength)};
			
			Result->GetValue()->Append("MIMType", MIMETypeResult->GetValue());
			if(MIMETypeResult->GetSuccess() == true)
			{
				auto DescriptionLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->Append("DescriptionLength", DescriptionLengthResult->GetValue());
				if(DescriptionLengthResult->GetSuccess() == true)
				{
					auto DescriptionLength{std::experimental::any_cast< std::uint32_t >(DescriptionLengthResult->GetAny())};
					auto DescriptionResult{Get_UTF8_String_EndedByByteLength(Buffer, DescriptionLength)};
					
					Result->GetValue()->Append("Description", DescriptionResult->GetValue());
					if(DescriptionResult->GetSuccess() == true)
					{
						auto PictureWidthInPixelsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
						
						Result->GetValue()->Append("PictureWidthInPixels", PictureWidthInPixelsResult->GetValue());
						if(PictureWidthInPixelsResult->GetSuccess() == true)
						{
							auto PictureHeightInPixelsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
							
							Result->GetValue()->Append("PictureHeightInPixels", PictureHeightInPixelsResult->GetValue());
							if(PictureHeightInPixelsResult->GetSuccess() == true)
							{
								auto BitsPerPixelResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
								
								Result->GetValue()->Append("BitsPerPixel", BitsPerPixelResult->GetValue());
								if(BitsPerPixelResult->GetSuccess() == true)
								{
									auto NumberOfColorsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
									
									Result->GetValue()->Append("NumberOfColors", NumberOfColorsResult->GetValue());
									if(NumberOfColorsResult->GetSuccess() == true)
									{
										auto PictureDataLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
										
										Result->GetValue()->Append("PictureDataLength", PictureDataLengthResult->GetValue());
										if(PictureDataLengthResult->GetSuccess() == true)
										{
											auto PictureDataLength{std::experimental::any_cast< std::uint32_t >(PictureDataLengthResult->GetAny())};
											auto PictureDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, PictureDataLength)};
											
											Result->GetValue()->Append("PictureData", PictureDataResult->GetValue());
											if(PictureDataResult->GetSuccess() == true)
											{
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
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_Data(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints)
{
	auto Result{Inspection::InitializeResult(true, Buffer)};
	
	for(auto SeekPointIndex = 0ul; SeekPointIndex < NumberOfSeekPoints; ++SeekPointIndex)
	{
		auto SeekPointResult{Get_FLAC_SeekTableBlock_SeekPoint(Buffer)};
		
		Result->GetValue()->Append("SeekPoint", SeekPointResult->GetValue());
		if(SeekPointResult->GetSuccess() == false)
		{
			Result->SetSuccess(false);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_SeekPoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto SampleNumberOfFirstSampleInTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
	
	Result->GetValue()->Append("SampleNumberOfFirstSampleInTargetFrame", SampleNumberOfFirstSampleInTargetFrameResult->GetValue());
	if(SampleNumberOfFirstSampleInTargetFrameResult->GetSuccess() == true)
	{
		auto ByteOffsetOfTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("ByteOffsetOfTargetFrame", ByteOffsetOfTargetFrameResult->GetValue());
		if(ByteOffsetOfTargetFrameResult->GetSuccess() == true)
		{
			auto NumberOfSamplesInTargetFrameResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->Append("NumberOfSamplesInTargetFrame", NumberOfSamplesInTargetFrameResult->GetValue());
			if(NumberOfSamplesInTargetFrameResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto FLACStreamMarkerResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "fLaC")};
	
	Result->GetValue()->Append("FLAC stream marker", FLACStreamMarkerResult->GetValue());
	if(FLACStreamMarkerResult->GetSuccess() == true)
	{
		auto FLACStreamInfoBlockResult{Get_FLAC_StreamInfoBlock(Buffer)};
		
		Result->GetValue()->Append("StreamInfoBlock", FLACStreamInfoBlockResult->GetValue());
		if(FLACStreamInfoBlockResult->GetSuccess() == true)
		{
			auto LastMetaDataBlock{std::experimental::any_cast< bool >(FLACStreamInfoBlockResult->GetValue("Header")->GetAny("LastMetaDataBlock"))};
			
			Result->SetSuccess(true);
			while(LastMetaDataBlock == false)
			{
				auto MetaDataBlockResult{Get_FLAC_MetaDataBlock(Buffer)};
				
				Result->GetValue()->Append("MetaDataBlock", MetaDataBlockResult->GetValue());
				if(MetaDataBlockResult->GetSuccess() == true)
				{
					LastMetaDataBlock = std::experimental::any_cast< bool >(MetaDataBlockResult->GetValue("Header")->GetAny("LastMetaDataBlock"));
				}
				else
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlock_Header(Buffer)};
	
	Result->GetValue()->Append("Header", MetaDataBlockHeaderResult->GetValue());
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetAny("Interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", StreamInfoBlockDataResult->GetValue());
			if(StreamInfoBlockDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto BitsPerSampleResult{Get_UnsignedInteger_5Bit(Buffer)};
	
	Result->SetValue(BitsPerSampleResult->GetValue());
	if(BitsPerSampleResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(BitsPerSampleResult->GetAny()) + 1));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_Data(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MinimumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	Result->GetValue()->Append("MinimumBlockSize", MinimumBlockSizeResult->GetValue());
	if(MinimumBlockSizeResult->GetSuccess() == true)
	{
		auto MaximumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("MaximumBlockSize", MaximumBlockSizeResult->GetValue());
		if(MaximumBlockSizeResult->GetSuccess() == true)
		{
			auto MinimumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			Result->GetValue()->Append("MinimumFrameSize", MinimumFrameSizeResult->GetValue());
			if(MinimumFrameSizeResult->GetSuccess() == true)
			{
				auto MaximumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
				
				Result->GetValue()->Append("MaximumFrameSize", MaximumFrameSizeResult->GetValue());
				if(MaximumFrameSizeResult->GetSuccess() == true)
				{
					auto SampleRateResult{Get_UnsignedInteger_20Bit_BigEndian(Buffer)};
					
					Result->GetValue()->Append("SampleRate", SampleRateResult->GetValue());
					if(SampleRateResult->GetSuccess() == true)
					{
						auto NumberOfChannelsResult{Get_FLAC_StreamInfoBlock_NumberOfChannels(Buffer)};
						
						Result->GetValue()->Append("NumberOfChannels", NumberOfChannelsResult->GetValue());
						if(NumberOfChannelsResult->GetSuccess() == true)
						{
							auto BitsPerSampleResult{Get_FLAC_StreamInfoBlock_BitsPerSample(Buffer)};
							
							Result->GetValue()->Append("BitsPerSample", BitsPerSampleResult->GetValue());
							if(BitsPerSampleResult->GetSuccess() == true)
							{
								auto TotalSamplesPerChannelResult{Get_UnsignedInteger_36Bit_BigEndian(Buffer)};
								
								Result->GetValue()->Append("TotalSamplesPerChannel", TotalSamplesPerChannelResult->GetValue());
								if(TotalSamplesPerChannelResult->GetSuccess() == true)
								{
									auto MD5SignatureOfUnencodedAudioDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 16)};
									
									Result->GetValue()->Append("MD5SignatureOfUnencodedAudioData", MD5SignatureOfUnencodedAudioDataResult->GetValue());
									if(MD5SignatureOfUnencodedAudioDataResult->GetSuccess() == true)
									{
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

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_NumberOfChannels(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto NumberOfChannelsResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	Result->SetValue(NumberOfChannelsResult->GetValue());
	if(NumberOfChannelsResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(NumberOfChannelsResult->GetAny()) + 1));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlock_Data(Inspection::Buffer & Buffer)
{
	return Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer);
}

void PrintValue(std::shared_ptr< Inspection::Value > Value, const std::string & Indentation = "")
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
			PrintValue(SubValue, SubIndentation);
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
				auto FLACStreamResult{Get_FLAC_Stream(Buffer)};
				
				FLACStreamResult->GetValue()->SetName("FLACStream");
				PrintValue(FLACStreamResult->GetValue());
				if(FLACStreamResult->GetSuccess() == false)
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
