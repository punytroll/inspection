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
std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlockData(Inspection::Buffer & Buffer);
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto BitsPerSampleResult{Get_UnsignedInteger_5Bit(Buffer)};
	
	if(BitsPerSampleResult->GetSuccess() == true)
	{
		Result->SetValue(BitsPerSampleResult->GetValue());
		Result->GetValue()->Append(Inspection::MakeValue("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(BitsPerSampleResult->GetAny()) + 1)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Header", MetaDataBlockHeaderResult->GetValue());
		
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetAny("Interpretation"))};
		auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(MetaDataBlockHeaderResult->GetAny("Length"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockDataResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Data", StreamInfoBlockDataResult->GetValue());
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "Padding")
		{
			auto PaddingBlockDataResult{Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Buffer, MetaDataBlockDataLength)};
			
			if(PaddingBlockDataResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Data", PaddingBlockDataResult->GetValue());
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			if(MetaDataBlockDataLength % 18 == 0)
			{
				auto SeekTableBlockDataResult{Get_FLAC_SeekTableBlockData(Buffer, MetaDataBlockDataLength / 18)};
				
				if(SeekTableBlockDataResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("Data", SeekTableBlockDataResult->GetValue());
					Result->SetSuccess(true);
				}
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			auto VorbisCommentBlockDataResult{Get_FLAC_VorbisCommentBlockData(Buffer)};
			
			if(VorbisCommentBlockDataResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Data", VorbisCommentBlockDataResult->GetValue());
				Result->SetSuccess(true);
			}
		}
		else if(MetaDataBlockType == "Picture")
		{
			auto PictureBlockDataResult{Get_FLAC_PictureBlockData(Buffer)};
			
			if(PictureBlockDataResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Data", PictureBlockDataResult->GetValue());
				Result->SetSuccess(true);
			}
		}
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto LastMetaDataBlockResult{Get_Boolean_OneBit(Buffer)};
	
	if(LastMetaDataBlockResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("LastMetaDataBlock", LastMetaDataBlockResult->GetValue());
		
		auto MetaDataBlockTypeResult{Get_FLAC_MetaDataBlockType(Buffer)};
		
		if(MetaDataBlockTypeResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("BlockType", MetaDataBlockTypeResult->GetValue());
			
			auto LengthResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(LengthResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Length", LengthResult->GetValue());
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MetaDataBlockTypeResult{Get_UnsignedInteger_7Bit(Buffer)};
	
	if(MetaDataBlockTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		Result->GetValue()->SetAny(MetaDataBlockTypeResult->GetAny());
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockTypeResult->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("StreamInfo")));
		}
		else if(NumericValue == 0x01)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Padding")));
		}
		else if(NumericValue == 0x02)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Application")));
		}
		else if(NumericValue == 0x03)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("SeekTable")));
		}
		else if(NumericValue == 0x04)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("VorbisComment")));
		}
		else if(NumericValue == 0x05)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("CueSheet")));
		}
		else if(NumericValue == 0x06)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Picture")));
		}
		else if(NumericValue == 0xff)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Invalid")));
		}
		else
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Reserved")));
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_NumberOfChannels(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto NumberOfChannelsResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	if(NumberOfChannelsResult->GetSuccess() == true)
	{
		Result->SetValue(NumberOfChannelsResult->GetValue());
		Result->GetValue()->Append(Inspection::MakeValue("Interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(NumberOfChannelsResult->GetAny()) + 1)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
	
	if(PictureTypeResult->GetSuccess() == true)
	{
		Result->SetValue(PictureTypeResult->GetValue());
		Result->SetSuccess(true);
		
		auto PictureType{std::experimental::any_cast< std::uint32_t >(PictureTypeResult->GetAny())};
		
		if(PictureType == 0)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Other")));
		}
		else if(PictureType == 1)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("32x32 pixels 'file icon' (PNG only)")));
		}
		else if(PictureType == 2)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Other file icon")));
		}
		else if(PictureType == 3)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Cover (front)")));
		}
		else if(PictureType == 4)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Cover (back)")));
		}
		else if(PictureType == 5)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Leaflet page")));
		}
		else if(PictureType == 6)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Media (e.g. label side of CD")));
		}
		else if(PictureType == 7)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Lead artist/lead performer/soloist")));
		}
		else if(PictureType == 8)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Artist/performer")));
		}
		else if(PictureType == 9)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Conductor")));
		}
		else if(PictureType == 10)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Band/Orchestra")));
		}
		else if(PictureType == 11)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Composer")));
		}
		else if(PictureType == 12)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Lyricist/text writer")));
		}
		else if(PictureType == 13)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Recording Location")));
		}
		else if(PictureType == 14)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("During recording")));
		}
		else if(PictureType == 15)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("During performance")));
		}
		else if(PictureType == 16)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Movie/video screen capture")));
		}
		else if(PictureType == 17)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("A bright coloured fish")));
		}
		else if(PictureType == 18)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Illustration")));
		}
		else if(PictureType == 19)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Band/artist logotype")));
		}
		else if(PictureType == 20)
		{
			Result->GetValue()->Append(Inspection::MakeValue("Interpretation", std::string("Publisher/Studio logotype")));
		}
		else
		{
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlockData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto PictureTypeResult{Get_FLAC_PictureBlock_PictureType(Buffer)};
	
	if(PictureTypeResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("PictureType", PictureTypeResult->GetValue());
		
		auto MIMETypeLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		if(MIMETypeLengthResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("MIMETypeLength", MIMETypeLengthResult->GetValue());
			
			auto MIMETypeLength{std::experimental::any_cast< std::uint32_t >(MIMETypeLengthResult->GetAny())};
			auto MIMETypeResult{Get_ASCII_String_Printable_EndedByByteLength(Buffer, MIMETypeLength)};
			
			if(MIMETypeResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("MIMType", MIMETypeResult->GetValue());
				
				auto DescriptionLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				if(DescriptionLengthResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("DescriptionLength", DescriptionLengthResult->GetValue());
					
					auto DescriptionLength{std::experimental::any_cast< std::uint32_t >(DescriptionLengthResult->GetAny())};
					auto DescriptionResult{Get_UTF8_String_EndedByByteLength(Buffer, DescriptionLength)};
					
					if(DescriptionResult->GetSuccess() == true)
					{
						Result->GetValue()->Append("Description", DescriptionResult->GetValue());
						
						auto PictureWidthInPixelsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
						
						if(PictureWidthInPixelsResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("PictureWidthInPixels", PictureWidthInPixelsResult->GetValue());
							
							auto PictureHeightInPixelsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
							
							if(PictureHeightInPixelsResult->GetSuccess() == true)
							{
								Result->GetValue()->Append("PictureHeightInPixels", PictureHeightInPixelsResult->GetValue());
								
								auto BitsPerPixelResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
								
								if(BitsPerPixelResult->GetSuccess() == true)
								{
									Result->GetValue()->Append("BitsPerPixel", BitsPerPixelResult->GetValue());
									
									auto NumberOfColorsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
									
									if(NumberOfColorsResult->GetSuccess() == true)
									{
										Result->GetValue()->Append("NumberOfColors", NumberOfColorsResult->GetValue());
										
										auto PictureDataLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
										
										if(PictureDataLengthResult->GetSuccess() == true)
										{
											Result->GetValue()->Append("PictureDataLength", PictureDataLengthResult->GetValue());
											
											auto PictureDataLength{std::experimental::any_cast< std::uint32_t >(PictureDataLengthResult->GetAny())};
											auto PictureDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, PictureDataLength)};
											
											if(PictureDataResult->GetSuccess() == true)
											{
												Result->GetValue()->Append("PictureData", PictureDataResult->GetValue());
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

std::unique_ptr< Inspection::Result > Get_FLAC_SeekPoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto SampleNumberOfFirstSampleInTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
	
	if(SampleNumberOfFirstSampleInTargetFrameResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("SampleNumberOfFirstSampleInTargetFrame", SampleNumberOfFirstSampleInTargetFrameResult->GetValue());
		
		auto ByteOffsetOfTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
		
		if(ByteOffsetOfTargetFrameResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("ByteOffsetOfTargetFrame", ByteOffsetOfTargetFrameResult->GetValue());
			
			auto NumberOfSamplesInTargetFrameResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
			
			if(NumberOfSamplesInTargetFrameResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("NumberOfSamplesInTargetFrame", NumberOfSamplesInTargetFrameResult->GetValue());
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlockData(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints)
{
	auto Result{Inspection::InitializeResult(true, Buffer)};
	
	for(auto SeekPointIndex = 0ul; SeekPointIndex < NumberOfSeekPoints; ++SeekPointIndex)
	{
		auto SeekPointResult{Get_FLAC_SeekPoint(Buffer)};
		
		if(SeekPointResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("SeekPoint", SeekPointResult->GetValue());
		}
		else
		{
			Result->SetSuccess(false);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto FLACStreamMarkerResult{Get_ASCII_String_Alphabetical_EndedTemplateByLength(Buffer, "fLaC")};
	
	if(FLACStreamMarkerResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("FLAC stream marker", FLACStreamMarkerResult->GetValue());
		
		auto FLACStreamInfoBlockResult{Get_FLAC_StreamInfoBlock(Buffer)};
		
		if(FLACStreamInfoBlockResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("StreamInfoBlock", FLACStreamInfoBlockResult->GetValue());
			
			auto LastMetaDataBlock{std::experimental::any_cast< bool >(FLACStreamInfoBlockResult->GetValue("Header")->GetAny("LastMetaDataBlock"))};
			
			Result->SetSuccess(true);
			while(LastMetaDataBlock == false)
			{
				auto MetaDataBlockResult{Get_FLAC_MetaDataBlock(Buffer)};
				
				if(MetaDataBlockResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("MetaDataBlock", MetaDataBlockResult->GetValue());
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
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetAny("Interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			Result->GetValue()->Append("Header", MetaDataBlockHeaderResult->GetValue());
			
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockDataResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Data", StreamInfoBlockDataResult->GetValue());
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlockData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto MinimumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	if(MinimumBlockSizeResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("MinimumBlockSize", MinimumBlockSizeResult->GetValue());
		
		auto MaximumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		if(MaximumBlockSizeResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("MaximumBlockSize", MaximumBlockSizeResult->GetValue());
			
			auto MinimumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(MinimumFrameSizeResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("MinimumFrameSize", MinimumFrameSizeResult->GetValue());
				
				auto MaximumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
				
				if(MaximumFrameSizeResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("MaximumFrameSize", MaximumFrameSizeResult->GetValue());
					
					auto SampleRateResult{Get_UnsignedInteger_20Bit_BigEndian(Buffer)};
					
					if(SampleRateResult->GetSuccess() == true)
					{
						Result->GetValue()->Append("SampleRate", SampleRateResult->GetValue());
						
						auto NumberOfChannelsResult{Get_FLAC_NumberOfChannels(Buffer)};
						
						if(NumberOfChannelsResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("NumberOfChannels", NumberOfChannelsResult->GetValue());
							
							auto BitsPerSampleResult{Get_FLAC_BitsPerSample(Buffer)};
							
							if(BitsPerSampleResult->GetSuccess() == true)
							{
								Result->GetValue()->Append("BitsPerSample", BitsPerSampleResult->GetValue());
								
								auto TotalSamplesPerChannelResult{Get_UnsignedInteger_36Bit_BigEndian(Buffer)};
								
								if(TotalSamplesPerChannelResult->GetSuccess() == true)
								{
									Result->GetValue()->Append("TotalSamplesPerChannel", TotalSamplesPerChannelResult->GetValue());
									
									auto MD5SignatureOfUnencodedAudioDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 16)};
									
									if(MD5SignatureOfUnencodedAudioDataResult->GetSuccess() == true)
									{
										Result->GetValue()->Append("MD5SignatureOfUnencodedAudioData", MD5SignatureOfUnencodedAudioDataResult->GetValue());
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

std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlockData(Inspection::Buffer & Buffer)
{
	return Get_Vorbis_CommentHeader(Buffer);
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto VendorLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(VendorLengthResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("VendorLength", VendorLengthResult->GetValue());
		
		auto VendorLength{std::experimental::any_cast< std::uint32_t >(VendorLengthResult->GetAny())};
		auto VendorResult{Get_UTF8_String_EndedByByteLength(Buffer, VendorLength)};
		
		if(VendorResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("Vendor", VendorResult->GetValue());
			
			auto UserCommentListResult{Get_Vorbis_CommentHeader_UserCommentList(Buffer)};
			
			if(UserCommentListResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("UserCommentList", UserCommentListResult->GetValue());
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto UserCommentLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentLengthResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Length", UserCommentLengthResult->GetValue());
		
		auto UserCommentLength{std::experimental::any_cast< std::uint32_t >(UserCommentLengthResult->GetAny())};
		auto UserCommentResult{Get_UTF8_String_EndedByByteLength(Buffer, UserCommentLength)};
		
		if(UserCommentResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("String", UserCommentResult->GetValue());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto UserCommentListLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentListLengthResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Length", UserCommentListLengthResult->GetValue());
		Result->SetSuccess(true);
		
		auto UserCommentListLength{std::experimental::any_cast< std::uint32_t >(UserCommentListLengthResult->GetAny())};
		
		for(std::uint32_t Index = 0ul; Index < UserCommentListLength; ++Index)
		{
			auto UserCommentResult{Get_Vorbis_CommentHeader_UserComment(Buffer)};
			
			if(UserCommentResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("UserComment", UserCommentResult->GetValue());
			}
			else
			{
				Result->SetSuccess(false);
				
				break;
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
