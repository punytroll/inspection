#include <deque>

#include "../common/common.h"

using namespace std::string_literals;

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
	auto Result{Inspection::InitializeResult(Buffer)};
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlock_Header(Buffer)};
	
	Result->GetValue()->Append("Header", MetaDataBlockHeaderResult->GetValue());
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetTagAny("interpretation"))};
		auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(MetaDataBlockHeaderResult->GetAny("Length"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", StreamInfoBlockDataResult->GetValue());
			Result->SetSuccess(StreamInfoBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "Padding")
		{
			auto PaddingBlockDataResult{Get_Bits_Unset_EndedByLength(Buffer, MetaDataBlockDataLength)};
			
			Result->GetValue()->Append("Data", PaddingBlockDataResult->GetValue());
			Result->SetSuccess(PaddingBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "Application")
		{
			auto ApplicationBlockDataResult{Get_FLAC_ApplicationBlock_Data(Buffer, MetaDataBlockDataLength)};
			
			Result->GetValue()->Append("Data", ApplicationBlockDataResult->GetValue());
			Result->SetSuccess(ApplicationBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			if(MetaDataBlockDataLength % 18 == 0)
			{
				auto SeekTableBlockDataResult{Get_FLAC_SeekTableBlock_Data(Buffer, MetaDataBlockDataLength / 18)};
				
				Result->GetValue()->Append("Data", SeekTableBlockDataResult->GetValue());
				Result->SetSuccess(SeekTableBlockDataResult->GetSuccess());
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			auto VorbisCommentBlockDataResult{Get_FLAC_VorbisCommentBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", VorbisCommentBlockDataResult->GetValue());
			Result->SetSuccess(VorbisCommentBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "Picture")
		{
			auto PictureBlockDataResult{Get_FLAC_PictureBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", PictureBlockDataResult->GetValue());
			Result->SetSuccess(PictureBlockDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MetaDataBlockTypeResult{Get_UnsignedInteger_7Bit(Buffer)};
	
	Result->SetValue(MetaDataBlockTypeResult->GetValue());
	if(MetaDataBlockTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockTypeResult->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Result->GetValue()->AppendTag("interpretation", "StreamInfo"s);
		}
		else if(NumericValue == 0x01)
		{
			Result->GetValue()->AppendTag("interpretation", "Padding"s);
		}
		else if(NumericValue == 0x02)
		{
			Result->GetValue()->AppendTag("interpretation", "Application"s);
		}
		else if(NumericValue == 0x03)
		{
			Result->GetValue()->AppendTag("interpretation", "SeekTable"s);
		}
		else if(NumericValue == 0x04)
		{
			Result->GetValue()->AppendTag("interpretation", "VorbisComment"s);
		}
		else if(NumericValue == 0x05)
		{
			Result->GetValue()->AppendTag("interpretation", "CueSheet"s);
		}
		else if(NumericValue == 0x06)
		{
			Result->GetValue()->AppendTag("interpretation", "Picture"s);
		}
		else if(NumericValue == 0xff)
		{
			Result->GetValue()->AppendTag("interpretation", "Invalid"s);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "Reserved"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto PictureType{std::experimental::any_cast< std::uint32_t >(PictureTypeResult->GetAny())};
		
		if(PictureType == 0)
		{
			Result->GetValue()->AppendTag("interpretation", "Other"s);
		}
		else if(PictureType == 1)
		{
			Result->GetValue()->AppendTag("interpretation", "32x32 pixels 'file icon' (PNG only)"s);
		}
		else if(PictureType == 2)
		{
			Result->GetValue()->AppendTag("interpretation", "Other file icon"s);
		}
		else if(PictureType == 3)
		{
			Result->GetValue()->AppendTag("interpretation", "Cover (front)"s);
		}
		else if(PictureType == 4)
		{
			Result->GetValue()->AppendTag("interpretation", "Cover (back)"s);
		}
		else if(PictureType == 5)
		{
			Result->GetValue()->AppendTag("interpretation", "Leaflet page"s);
		}
		else if(PictureType == 6)
		{
			Result->GetValue()->AppendTag("interpretation", "Media (e.g. label side of CD"s);
		}
		else if(PictureType == 7)
		{
			Result->GetValue()->AppendTag("interpretation", "Lead artist/lead performer/soloist"s);
		}
		else if(PictureType == 8)
		{
			Result->GetValue()->AppendTag("interpretation", "Artist/performer"s);
		}
		else if(PictureType == 9)
		{
			Result->GetValue()->AppendTag("interpretation", "Conductor"s);
		}
		else if(PictureType == 10)
		{
			Result->GetValue()->AppendTag("interpretation", "Band/Orchestra"s);
		}
		else if(PictureType == 11)
		{
			Result->GetValue()->AppendTag("interpretation", "Composer"s);
		}
		else if(PictureType == 12)
		{
			Result->GetValue()->AppendTag("interpretation", "Lyricist/text writer"s);
		}
		else if(PictureType == 13)
		{
			Result->GetValue()->AppendTag("interpretation", "Recording Location"s);
		}
		else if(PictureType == 14)
		{
			Result->GetValue()->AppendTag("interpretation", "During recording"s);
		}
		else if(PictureType == 15)
		{
			Result->GetValue()->AppendTag("interpretation", "During performance"s);
		}
		else if(PictureType == 16)
		{
			Result->GetValue()->AppendTag("interpretation", "Movie/video screen capture"s);
		}
		else if(PictureType == 17)
		{
			Result->GetValue()->AppendTag("interpretation", "A bright coloured fish"s);
		}
		else if(PictureType == 18)
		{
			Result->GetValue()->AppendTag("interpretation", "Illustration"s);
		}
		else if(PictureType == 19)
		{
			Result->GetValue()->AppendTag("interpretation", "Band/artist logotype"s);
		}
		else if(PictureType == 20)
		{
			Result->GetValue()->AppendTag("interpretation", "Publisher/Studio logotype"s);
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_FLAC_PictureBlock_PictureType(Buffer)};
	
	Result->GetValue()->Append("PictureType", PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto MIMETypeLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("MIMETypeLength", MIMETypeLengthResult->GetValue());
		if(MIMETypeLengthResult->GetSuccess() == true)
		{
			auto MIMETypeLength{std::experimental::any_cast< std::uint32_t >(MIMETypeLengthResult->GetAny())};
			auto MIMETypeResult{Get_ASCII_String_Printable_EndedByLength(Buffer, Inspection::Length(MIMETypeLength, 0))};
			
			Result->GetValue()->Append("MIMType", MIMETypeResult->GetValue());
			if(MIMETypeResult->GetSuccess() == true)
			{
				auto DescriptionLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->Append("DescriptionLength", DescriptionLengthResult->GetValue());
				if(DescriptionLengthResult->GetSuccess() == true)
				{
					auto DescriptionLength{std::experimental::any_cast< std::uint32_t >(DescriptionLengthResult->GetAny())};
					auto DescriptionResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, DescriptionLength)};
					
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
											auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Inspection::Length(PictureDataLength, 0))};
											
											Result->GetValue()->Append("PictureData", PictureDataResult->GetValue());
											Result->SetSuccess(PictureDataResult->GetSuccess());
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
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
	auto Result{Inspection::InitializeResult(Buffer)};
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
			Result->SetSuccess(NumberOfSamplesInTargetFrameResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FLACStreamMarkerResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "fLaC")};
	
	Result->GetValue()->Append("FLAC stream marker", FLACStreamMarkerResult->GetValue());
	if(FLACStreamMarkerResult->GetSuccess() == true)
	{
		auto FLACStreamInfoBlockResult{Get_FLAC_StreamInfoBlock(Buffer)};
		
		Result->GetValue()->Append("StreamInfoBlock", FLACStreamInfoBlockResult->GetValue());
		if(FLACStreamInfoBlockResult->GetSuccess() == true)
		{
			auto LastMetaDataBlock{std::experimental::any_cast< bool >(FLACStreamInfoBlockResult->GetValue("Header")->GetValueAny("LastMetaDataBlock"))};
			auto Continue{true};
			
			while(LastMetaDataBlock == false)
			{
				auto MetaDataBlockResult{Get_FLAC_MetaDataBlock(Buffer)};
				
				Result->GetValue()->Append("MetaDataBlock", MetaDataBlockResult->GetValue());
				if(MetaDataBlockResult->GetSuccess() == true)
				{
					LastMetaDataBlock = std::experimental::any_cast< bool >(MetaDataBlockResult->GetValue("Header")->GetValueAny("LastMetaDataBlock"));
				}
				else
				{
					Continue = false;
					
					break;
				}
			}
			if(Continue == true)
			{
				auto FramesResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Buffer.GetLength() - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Frames", FramesResult->GetValue());
				Result->SetSuccess(FramesResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlock_Header(Buffer)};
	
	Result->GetValue()->Append("Header", MetaDataBlockHeaderResult->GetValue());
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetTagAny("interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			
			Result->GetValue()->Append("Data", StreamInfoBlockDataResult->GetValue());
			Result->SetSuccess(StreamInfoBlockDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitsPerSampleResult{Get_UnsignedInteger_5Bit(Buffer)};
	
	Result->SetValue(BitsPerSampleResult->GetValue());
	if(BitsPerSampleResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendTag("interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(BitsPerSampleResult->GetAny()) + 1));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_Data(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
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
									Result->SetSuccess(MD5SignatureOfUnencodedAudioDataResult->GetSuccess());
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NumberOfChannelsResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	Result->SetValue(NumberOfChannelsResult->GetValue());
	if(NumberOfChannelsResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendTag("interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(NumberOfChannelsResult->GetAny()) + 1));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlock_Data(Inspection::Buffer & Buffer)
{
	return Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer);
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto FLACStreamResult{Get_FLAC_Stream(Buffer)};
	
	FLACStreamResult->GetValue()->SetName("FLACStream");
	
	return FLACStreamResult;
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
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}
	
	return 0;
}
