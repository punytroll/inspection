#include <deque>

#include "../common/common.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_MPEG_1_Frame(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Buffer & Buffer, std::uint8_t LayerDescription);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Copyright(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Emphasis(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Mode(Inspection::Buffer & Buffer, std::uint8_t LayerDescription);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Buffer & Buffer, std::uint8_t LayerDescription, std::uint8_t Mode);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_PaddingBit(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ProtectionBit(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_MPEG_1_Stream(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_MPEG_1_Frame(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameHeaderResult{Get_MPEG_1_FrameHeader(Buffer)};
	
	Result->GetValue()->Append("Header", FrameHeaderResult->GetValue());
	if(FrameHeaderResult->GetSuccess() == true)
	{
		auto ProtectionBit{std::experimental::any_cast< std::uint8_t >(FrameHeaderResult->GetAny("ProtectionBit"))};
		auto Continue{true};
		
		if(ProtectionBit == 0x00)
		{
			auto ErrorCheckResult{Get_BitSet_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->Append("ErrorCheck", ErrorCheckResult->GetValue());
			Continue = ErrorCheckResult->GetSuccess();
		}
		if(Continue == true)
		{
			auto LayerDescription{std::experimental::any_cast< std::uint8_t >(FrameHeaderResult->GetAny("LayerDescription"))};
			auto BitRate{std::experimental::any_cast< std::uint32_t >(FrameHeaderResult->GetValue("BitRateIndex")->GetTagAny("numeric"))};
			auto SamplingFrequency{std::experimental::any_cast< std::uint32_t >(FrameHeaderResult->GetValue("SamplingFrequency")->GetTagAny("numeric"))};
			auto PaddingBit{std::experimental::any_cast< std::uint8_t >(FrameHeaderResult->GetAny("PaddingBit"))};
			auto FrameLength{0ul};
			
			if(LayerDescription == 0x03)
			{
				FrameLength = (12 * BitRate / SamplingFrequency + PaddingBit) * 4;
			}
			else if((LayerDescription == 0x01) || (LayerDescription == 0x02))
			{
				FrameLength = 144 * BitRate / SamplingFrequency + PaddingBit;
			}
			
			auto AudioDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Inspection::Length(FrameLength) + Start - Buffer.GetPosition())};
			
			Result->GetValue()->Append("AudioData", AudioDataResult->GetValue());
			Result->SetSuccess(AudioDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameSyncResult{Get_Bits_Set_EndedByLength(Buffer, Inspection::Length(0ull, 12))};
	
	Result->GetValue()->Append("FrameSync", FrameSyncResult->GetValue());
	if(FrameSyncResult->GetSuccess() == true)
	{
		auto AudioVersionIDResult{Get_MPEG_1_FrameHeader_AudioVersionID(Buffer)};
		
		Result->GetValue()->Append("AudioVersionID", AudioVersionIDResult->GetValue());
		if(AudioVersionIDResult->GetSuccess() == true)
		{
			auto LayerDescriptionResult{Get_MPEG_1_FrameHeader_LayerDescription(Buffer)};
			
			Result->GetValue()->Append("LayerDescription", LayerDescriptionResult->GetValue());
			if(LayerDescriptionResult->GetSuccess() == true)
			{
				auto ProtectionBitResult{Get_MPEG_1_FrameHeader_ProtectionBit(Buffer)};
				
				Result->GetValue()->Append("ProtectionBit", ProtectionBitResult->GetValue());
				if(ProtectionBitResult->GetSuccess() == true)
				{
					auto LayerDescription{std::experimental::any_cast< std::uint8_t >(LayerDescriptionResult->GetAny())};
					auto BitRateIndexResult{Get_MPEG_1_FrameHeader_BitRateIndex(Buffer, LayerDescription)};
					
					Result->GetValue()->Append("BitRateIndex", BitRateIndexResult->GetValue());
					if(BitRateIndexResult->GetSuccess() == true)
					{
						auto SamplingFrequencyResult{Get_MPEG_1_FrameHeader_SamplingFrequency(Buffer)};
						
						Result->GetValue()->Append("SamplingFrequency", SamplingFrequencyResult->GetValue());
						if(SamplingFrequencyResult->GetSuccess() == true)
						{
							auto PaddingBitResult{Get_MPEG_1_FrameHeader_PaddingBit(Buffer)};
							
							Result->GetValue()->Append("PaddingBit", PaddingBitResult->GetValue());
							if(PaddingBitResult->GetSuccess() == true)
							{
								auto PrivateBitResult{Get_UnsignedInteger_1Bit(Buffer)};
								
								Result->GetValue()->Append("PrivateBit", PrivateBitResult->GetValue());
								if(PrivateBitResult->GetSuccess() == true)
								{
									auto ModeResult{Get_MPEG_1_FrameHeader_Mode(Buffer, LayerDescription)};
									
									Result->GetValue()->Append("Mode", ModeResult->GetValue());
									if(ModeResult->GetSuccess() == true)
									{
										auto Mode{std::experimental::any_cast< std::uint8_t >(ModeResult->GetAny())};
										auto ModeExtensionResult{Get_MPEG_1_FrameHeader_ModeExtension(Buffer, LayerDescription, Mode)};
										
										Result->GetValue()->Append("ModeExtension", ModeExtensionResult->GetValue());
										if(ModeExtensionResult->GetSuccess() == true)
										{
											auto CopyrightResult{Get_MPEG_1_FrameHeader_Copyright(Buffer)};
											
											Result->GetValue()->Append("Copyright", CopyrightResult->GetValue());
											if(CopyrightResult->GetSuccess() == true)
											{
												auto OriginalHomeResult{Get_MPEG_1_FrameHeader_OriginalHome(Buffer)};
												
												Result->GetValue()->Append("Original/Home", OriginalHomeResult->GetValue());
												if(OriginalHomeResult->GetSuccess() == true)
												{
													auto EmphasisResult{Get_MPEG_1_FrameHeader_Emphasis(Buffer)};
													
													Result->GetValue()->Append("Emphasis", EmphasisResult->GetValue());
													Result->SetSuccess(EmphasisResult->GetSuccess());
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
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto AudioVersionIDResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(AudioVersionIDResult->GetValue());
	if(AudioVersionIDResult->GetSuccess() == true)
	{
		auto AudioVersionID{std::experimental::any_cast< std::uint8_t >(AudioVersionIDResult->GetAny())};
		
		if(AudioVersionID == 0x01)
		{
			Result->GetValue()->PrependTag("MPEG Version 1 (ISO/IEC 11172-3)"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("<reserved>");
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Buffer & Buffer, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitRateIndexResult{Get_UnsignedInteger_4Bit(Buffer)};
	
	Result->SetValue(BitRateIndexResult->GetValue());
	if(BitRateIndexResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto BitRateIndex{std::experimental::any_cast< std::uint8_t >(BitRateIndexResult->GetAny())};
		
		if(LayerDescription == 0x03)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->PrependTag("free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->PrependTag("numeric", 32000u);
				Result->GetValue()->PrependTag("32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->PrependTag("numeric", 64000u);
				Result->GetValue()->PrependTag("64 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->PrependTag("numeric", 96000u);
				Result->GetValue()->PrependTag("96 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->PrependTag("numeric", 128000u);
				Result->GetValue()->PrependTag("128 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->PrependTag("numeric", 160000u);
				Result->GetValue()->PrependTag("160 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->PrependTag("numeric", 192000u);
				Result->GetValue()->PrependTag("192 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->PrependTag("numeric", 224000u);
				Result->GetValue()->PrependTag("224 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->PrependTag("numeric", 256000u);
				Result->GetValue()->PrependTag("256 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->PrependTag("numeric", 288000u);
				Result->GetValue()->PrependTag("288 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->PrependTag("numeric", 320000u);
				Result->GetValue()->PrependTag("320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->PrependTag("numeric", 352000u);
				Result->GetValue()->PrependTag("352 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->PrependTag("numeric", 384000u);
				Result->GetValue()->PrependTag("384 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->PrependTag("numeric", 416000u);
				Result->GetValue()->PrependTag("416 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->PrependTag("numeric", 448000u);
				Result->GetValue()->PrependTag("448 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->PrependTag("<reserved>"s);
				Result->SetSuccess(false);
			}
		}
		else if(LayerDescription == 0x02)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->PrependTag("free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->PrependTag("numeric", 32000u);
				Result->GetValue()->PrependTag("32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->PrependTag("numeric", 48000u);
				Result->GetValue()->PrependTag("48 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->PrependTag("numeric", 56000u);
				Result->GetValue()->PrependTag("56 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->PrependTag("numeric", 64000u);
				Result->GetValue()->PrependTag("64 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->PrependTag("numeric", 80000u);
				Result->GetValue()->PrependTag("80 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->PrependTag("numeric", 96000u);
				Result->GetValue()->PrependTag("96 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->PrependTag("numeric", 112000u);
				Result->GetValue()->PrependTag("112 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->PrependTag("numeric", 128000u);
				Result->GetValue()->PrependTag("128 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->PrependTag("numeric", 160000u);
				Result->GetValue()->PrependTag("160 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->PrependTag("numeric", 192000u);
				Result->GetValue()->PrependTag("192 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->PrependTag("numeric", 224000u);
				Result->GetValue()->PrependTag("224 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->PrependTag("numeric", 256000u);
				Result->GetValue()->PrependTag("256 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->PrependTag("numeric", 320000u);
				Result->GetValue()->PrependTag("320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->PrependTag("numeric", 384000u);
				Result->GetValue()->PrependTag("384 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->PrependTag("<reserved>"s);
				Result->SetSuccess(false);
			}
		}
		else if(LayerDescription == 0x01)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->PrependTag("free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->PrependTag("numeric", 32000u);
				Result->GetValue()->PrependTag("32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->PrependTag("numeric", 40000u);
				Result->GetValue()->PrependTag("40 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->PrependTag("numeric", 48000u);
				Result->GetValue()->PrependTag("48 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->PrependTag("numeric", 56000u);
				Result->GetValue()->PrependTag("56 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->PrependTag("numeric", 64000u);
				Result->GetValue()->PrependTag("64 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->PrependTag("numeric", 80000u);
				Result->GetValue()->PrependTag("80 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->PrependTag("numeric", 96000u);
				Result->GetValue()->PrependTag("96 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->PrependTag("numeric", 112000u);
				Result->GetValue()->PrependTag("112 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->PrependTag("numeric", 128000u);
				Result->GetValue()->PrependTag("128 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->PrependTag("numeric", 160000u);
				Result->GetValue()->PrependTag("160 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->PrependTag("numeric", 192000u);
				Result->GetValue()->PrependTag("192 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->PrependTag("numeric", 224000u);
				Result->GetValue()->PrependTag("224 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->PrependTag("numeric", 256000u);
				Result->GetValue()->PrependTag("256 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->PrependTag("numeric", 320000u);
				Result->GetValue()->PrependTag("320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->PrependTag("<reserved>"s);
				Result->SetSuccess(false);
			}
		}
		else
		{
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Copyright(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CopyrightResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(CopyrightResult->GetValue());
	if(CopyrightResult->GetSuccess() == true)
	{
		auto Copyright{std::experimental::any_cast< std::uint8_t >(CopyrightResult->GetAny())};
		
		if(Copyright == 0x00)
		{
			Result->GetValue()->PrependTag("copyright", false);
			Result->SetSuccess(true);
		}
		else if(Copyright == 0x01)
		{
			Result->GetValue()->PrependTag("copyright", true);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Emphasis(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto EmphasisResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(EmphasisResult->GetValue());
	if(EmphasisResult->GetSuccess() == true)
	{
		auto Emphasis{std::experimental::any_cast< std::uint8_t >(EmphasisResult->GetAny())};
		
		if(Emphasis == 0x00)
		{
			Result->GetValue()->PrependTag("no emphasis"s);
			Result->SetSuccess(true);
		}
		else if(Emphasis == 0x01)
		{
			Result->GetValue()->PrependTag("50/15 microsec. emphasis"s);
			Result->SetSuccess(true);
		}
		else if(Emphasis == 0x02)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
		}
		else if(Emphasis == 0x03)
		{
			Result->GetValue()->PrependTag("CCITT J.17"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LayerDescriptionResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(LayerDescriptionResult->GetValue());
	if(LayerDescriptionResult->GetSuccess() == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(LayerDescriptionResult->GetAny())};
		
		if(LayerDescription == 0x00)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
		}
		else if(LayerDescription == 0x01)
		{
			Result->GetValue()->PrependTag("Layer III"s);
			Result->SetSuccess(true);
		}
		else if(LayerDescription == 0x02)
		{
			Result->GetValue()->PrependTag("Layer II"s);
			Result->SetSuccess(true);
		}
		else if(LayerDescription == 0x03)
		{
			Result->GetValue()->PrependTag("Layer I"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Mode(Inspection::Buffer & Buffer, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ModeResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(ModeResult->GetValue());
	if(ModeResult->GetSuccess() == true)
	{
		auto Mode{std::experimental::any_cast< std::uint8_t >(ModeResult->GetAny())};
		
		if(Mode == 0x00)
		{
			Result->GetValue()->PrependTag("stereo"s);
			Result->SetSuccess(true);
		}
		else if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				Result->GetValue()->PrependTag("joint stereo (intensity_stereo)"s);
				Result->SetSuccess(true);
			}
			else if(LayerDescription == 0x01)
			{
				Result->GetValue()->PrependTag("joint stereo (intensity_stereo and/or ms_stereo)"s);
				Result->SetSuccess(true);
			}
		}
		else if(Mode == 0x02)
		{
			Result->GetValue()->PrependTag("dual_channel"s);
			Result->SetSuccess(true);
		}
		else if(Mode == 0x03)
		{
			Result->GetValue()->PrependTag("single_channel"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Buffer & Buffer, std::uint8_t LayerDescription, std::uint8_t Mode)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ModeExtensionResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(ModeExtensionResult->GetValue());
	if(ModeExtensionResult->GetSuccess() == true)
	{
		auto ModeExtension{std::experimental::any_cast< std::uint8_t >(ModeExtensionResult->GetAny())};
		
		if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->PrependTag("subbands 4-31 in intensity_stereo, bound==4"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->PrependTag("subbands 8-31 in intensity_stereo, bound==8"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->PrependTag("subbands 12-31 in intensity_stereo, bound==12"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->PrependTag("subbands 16-31 in intensity_stereo, bound==16"s);
					Result->SetSuccess(true);
				}
			}
			else if(LayerDescription == 0x01)
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->PrependTag("ms_stereo", "off"s);
					Result->GetValue()->PrependTag("intensity_stereo", "off"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->PrependTag("ms_stereo", "off"s);
					Result->GetValue()->PrependTag("intensity_stereo", "on"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->PrependTag("ms_stereo", "on"s);
					Result->GetValue()->PrependTag("intensity_stereo", "off"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->PrependTag("ms_stereo", "on"s);
					Result->GetValue()->PrependTag("intensity_stereo", "on"s);
					Result->SetSuccess(true);
				}
			}
		}
		else
		{
			Result->GetValue()->PrependTag("<ignored>"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OriginalHomeResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(OriginalHomeResult->GetValue());
	if(OriginalHomeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto OriginalHome{std::experimental::any_cast< std::uint8_t >(OriginalHomeResult->GetAny())};
		
		if(OriginalHome == 0x00)
		{
			Result->GetValue()->PrependTag("original", false);
		}
		else if(OriginalHome == 0x01)
		{
			Result->GetValue()->PrependTag("original", true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_PaddingBit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PaddingBitResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(PaddingBitResult->GetValue());
	if(PaddingBitResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto PaddingBit{std::experimental::any_cast< std::uint8_t >(PaddingBitResult->GetAny())};
		
		if(PaddingBit == 0x00)
		{
			Result->GetValue()->PrependTag("padding", false);
		}
		else if(PaddingBit == 0x01)
		{
			Result->GetValue()->PrependTag("padding", true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ProtectionBit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ProtectionBitResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(ProtectionBitResult->GetValue());
	if(ProtectionBitResult->GetSuccess() == true)
	{
		auto ProtectionBit{std::experimental::any_cast< std::uint8_t >(ProtectionBitResult->GetAny())};
		
		if(ProtectionBit == 0x00)
		{
			Result->GetValue()->PrependTag("redundancy", true);
			Result->SetSuccess(true);
		}
		else if(ProtectionBit == 0x01)
		{
			Result->GetValue()->PrependTag("redundancy", false);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SamplingFrequencyResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(SamplingFrequencyResult->GetValue());
	if(SamplingFrequencyResult->GetSuccess() == true)
	{
		auto SamplingFrequency{std::experimental::any_cast< std::uint8_t >(SamplingFrequencyResult->GetAny())};
		
		if(SamplingFrequency == 0x00)
		{
			Result->GetValue()->PrependTag("numeric", 44100u);
			Result->GetValue()->PrependTag("44.1 kHz"s);
			Result->SetSuccess(true);
		}
		else if(SamplingFrequency == 0x01)
		{
			Result->GetValue()->PrependTag("numeric", 48000u);
			Result->GetValue()->PrependTag("48 kHz"s);
			Result->SetSuccess(true);
		}
		else if(SamplingFrequency == 0x02)
		{
			Result->GetValue()->PrependTag("numeric", 32000u);
			Result->GetValue()->PrependTag("32 kHz"s);
			Result->SetSuccess(true);
		}
		else if(SamplingFrequency == 0x03)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_MPEG_1_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Buffer.GetLength())
	{
		auto Position{Buffer.GetPosition()};
		auto MPEGFrameResult{Get_MPEG_1_Frame(Buffer)};
		
		if(MPEGFrameResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("MPEGFrame", MPEGFrameResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(Position);
			Result->SetSuccess(false);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto MPEGStreamResult{Get_MPEG_1_Stream(Buffer)};
	
	MPEGStreamResult->GetValue()->SetName("MPEGStream");
	
	return MPEGStreamResult;
}

int main(int argc, char ** argv)
{
	std::cout << "This program is intentionally strict according to MPEG-1 audio (ISO/IEC 11172-3)!" << std::endl;
	
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
