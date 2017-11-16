#include <deque>
#include <string>

#include "../common/common.h"

using namespace std::string_literals;

/// RIFF GUIDs
Inspection::GUID g_KSDATAFORMAT_SUBTYPE_PCM{"00000001-0000-0010-8000-00aa00389b71"};

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_RIFF_Chunk(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_ChunkHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fact_ChunkData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_ChannelMask(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_CommonFields(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_SubFormat(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_RIFF_RIFF_ChunkData(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_RIFF_File(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_RIFF_Chunk(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ChunkHeaderResult{Get_RIFF_ChunkHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ChunkHeaderResult->GetValue()->GetValues());
	if(ChunkHeaderResult->GetSuccess() == true)
	{
		auto ChunkSize{std::experimental::any_cast< std::uint32_t >(ChunkHeaderResult->GetAny("Size"))};
		
		if(Buffer.Has(ChunkSize, 0) == true)
		{
			auto ChunkIdentifier{std::experimental::any_cast< std::string >(ChunkHeaderResult->GetAny("Identifier"))};
			std::unique_ptr< Inspection::Result > ChunkDataResult;
			
			if(ChunkIdentifier == "RIFF")
			{
				ChunkDataResult = Get_RIFF_RIFF_ChunkData(Buffer, ChunkSize);
				Result->GetValue()->AppendValues(ChunkDataResult->GetValue()->GetValues());
			}
			else if(ChunkIdentifier == "fact")
			{
				ChunkDataResult = Get_RIFF_fact_ChunkData(Buffer);
				Result->GetValue()->AppendValues(ChunkDataResult->GetValue()->GetValues());
			}
			else if(ChunkIdentifier == "fmt ")
			{
				ChunkDataResult = Get_RIFF_fmt_ChunkData(Buffer);
				Result->GetValue()->AppendValues(ChunkDataResult->GetValue()->GetValues());
			}
			else
			{
				ChunkDataResult = Get_Bits_SetOrUnset_EndedByLength(Buffer, ChunkSize);
				Result->GetValue()->AppendValue("Data", ChunkDataResult->GetValue());
			}
			Result->SetSuccess(ChunkDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_ChunkHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Buffer, 4ull)};
	
	Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		auto SizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		Result->SetSuccess(SizeResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fact_ChunkData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NumberOfSamplesResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("NumberOfSamples", NumberOfSamplesResult->GetValue());
	Result->SetSuccess(NumberOfSamplesResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CommonFieldsResult{Get_RIFF_fmt_ChunkData_CommonFields(Buffer)};
	
	Result->GetValue()->AppendValues(CommonFieldsResult->GetValue()->GetValues());
	if(CommonFieldsResult->GetSuccess() == true)
	{
		const std::string & Format{std::experimental::any_cast< const std::string & >(CommonFieldsResult->GetValue("FormatTag")->GetTagAny("constant name"))};
		std::unique_ptr< Inspection::Result > FormatSpecificFieldsResult;
		
		if(Format == "WAVE_FORMAT_PCM")
		{
			FormatSpecificFieldsResult = Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(Buffer);
		}
		else if(Format == "WAVE_FORMAT_EXTENSIBLE")
		{
			FormatSpecificFieldsResult = Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(Buffer);
		}
		if(FormatSpecificFieldsResult)
		{
			Result->GetValue()->AppendValues(FormatSpecificFieldsResult->GetValue()->GetValues());
			Result->SetSuccess(FormatSpecificFieldsResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_ChannelMask(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ChannelMaskResult{Get_BitSet_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(ChannelMaskResult->GetValue());
	if(ChannelMaskResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 32 > & ChannelMask{std::experimental::any_cast< const std::bitset< 32 > & >(ChannelMaskResult->GetAny())};
		
		if(ChannelMask[0] == true)
		{
			Result->GetValue()->AppendValue("[0]", "SPEAKER_FRONT_LEFT"s);
		}
		if(ChannelMask[1] == true)
		{
			Result->GetValue()->AppendValue("[1]", "SPEAKER_FRONT_RIGHT"s);
		}
		if(ChannelMask[2] == true)
		{
			Result->GetValue()->AppendValue("[2]", "SPEAKER_FRONT_CENTER"s);
		}
		if(ChannelMask[3] == true)
		{
			Result->GetValue()->AppendValue("[3]", "SPEAKER_LOW_FREQUENCY"s);
		}
		if(ChannelMask[4] == true)
		{
			Result->GetValue()->AppendValue("[4]", "SPEAKER_BACK_LEFT"s);
		}
		if(ChannelMask[5] == true)
		{
			Result->GetValue()->AppendValue("[5]", "SPEAKER_BACK_RIGHT"s);
		}
		if(ChannelMask[6] == true)
		{
			Result->GetValue()->AppendValue("[6]", "SPEAKER_FRONT_LEFT_OF_CENTER"s);
		}
		if(ChannelMask[7] == true)
		{
			Result->GetValue()->AppendValue("[7]", "SPEAKER_FRONT_RIGHT_OF_CENTER"s);
		}
		if(ChannelMask[8] == true)
		{
			Result->GetValue()->AppendValue("[8]", "SPEAKER_BACK_CENTER"s);
		}
		if(ChannelMask[9] == true)
		{
			Result->GetValue()->AppendValue("[9]", "SPEAKER_SIDE_LEFT"s);
		}
		if(ChannelMask[10] == true)
		{
			Result->GetValue()->AppendValue("[10]", "SPEAKER_SIDE_RIGHT"s);
		}
		if(ChannelMask[11] == true)
		{
			Result->GetValue()->AppendValue("[11]", "SPEAKER_TOP_CENTER"s);
		}
		if(ChannelMask[12] == true)
		{
			Result->GetValue()->AppendValue("[12]", "SPEAKER_TOP_FRONT_LEFT"s);
		}
		if(ChannelMask[13] == true)
		{
			Result->GetValue()->AppendValue("[13]", "SPEAKER_TOP_FRONT_CENTER"s);
		}
		if(ChannelMask[14] == true)
		{
			Result->GetValue()->AppendValue("[14]", "SPEAKER_TOP_FRONT_RIGHT"s);
		}
		if(ChannelMask[15] == true)
		{
			Result->GetValue()->AppendValue("[15]", "SPEAKER_TOP_BACK_LEFT"s);
		}
		if(ChannelMask[16] == true)
		{
			Result->GetValue()->AppendValue("[16]", "SPEAKER_TOP_BACK_CENTER"s);
		}
		if(ChannelMask[17] == true)
		{
			Result->GetValue()->AppendValue("[17]", "SPEAKER_TOP_BACK_RIGHT"s);
		}
		for(auto BitIndex = 18; BitIndex < 31; ++BitIndex)
		{
			if(ChannelMask[BitIndex] == true)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		if(ChannelMask[31] == true)
		{
			Result->GetValue()->AppendValue("[31]", "SPEAKER_ALL"s);
		}
		/// @todo there are the following bit combinations which have special names
		//~ #define SPEAKER_MONO             (SPEAKER_FRONT_CENTER)
		//~ #define SPEAKER_STEREO           (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
		//~ #define SPEAKER_2POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY)
		//~ #define SPEAKER_SURROUND         (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
		//~ #define SPEAKER_QUAD             (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
		//~ #define SPEAKER_4POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
		//~ #define SPEAKER_5POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
		//~ #define SPEAKER_7POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)
		//~ #define SPEAKER_5POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
		//~ #define SPEAKER_7POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_CommonFields(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FormatTagResult{Get_Microsoft_WaveFormat_FormatTag(Buffer)};
	
	Result->GetValue()->AppendValue("FormatTag", FormatTagResult->GetValue());
	if(FormatTagResult->GetSuccess() == true)
	{
		auto NumberOfChannelsResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("NumberOfChannels", NumberOfChannelsResult->GetValue());
		if(NumberOfChannelsResult->GetSuccess() == true)
		{
			auto SamplesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("SamplesPerSecond", SamplesPerSecondResult->GetValue());
			if(SamplesPerSecondResult->GetSuccess() == true)
			{
				auto AverageBytesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("AverageBytesPerSecond", AverageBytesPerSecondResult->GetValue());
				if(AverageBytesPerSecondResult->GetSuccess() == true)
				{
					auto BlockAlignResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("BlockAlign", BlockAlignResult->GetValue());
					Result->SetSuccess(BlockAlignResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("BitsPerSample", BitsPerSampleResult->GetValue());
	if(BitsPerSampleResult->GetSuccess() == true)
	{
		auto ExtensionSizeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("ExtensionSize", ExtensionSizeResult->GetValue());
		if(ExtensionSizeResult->GetSuccess() == true)
		{
			auto ValidBitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("ValidBitsPerSample", ValidBitsPerSampleResult->GetValue());
			if(ValidBitsPerSampleResult->GetSuccess() == true)
			{
				auto ChannelMaskResult{Get_RIFF_fmt_ChunkData_ChannelMask(Buffer)};
				
				Result->GetValue()->AppendValue("ChannelMask", ChannelMaskResult->GetValue());
				if(ChannelMaskResult->GetSuccess() == true)
				{
					auto SubFormatResult{Get_RIFF_fmt_ChunkData_SubFormat(Buffer)};
					
					Result->GetValue()->AppendValue("SubFormat", SubFormatResult->GetValue());
					Result->SetSuccess(SubFormatResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	auto BitsPerSampleValue{Result->GetValue()->AppendValue("BitsPerSample", BitsPerSampleResult->GetValue())};
	
	BitsPerSampleValue->AppendTag("units", "bits per sample"s);
	Result->SetSuccess(BitsPerSampleResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_SubFormat(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->SetValue(GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto GUID{std::experimental::any_cast< Inspection::GUID >(GUIDResult->GetAny())};
		
		if(GUID == g_KSDATAFORMAT_SUBTYPE_PCM)
		{
			Result->GetValue()->AppendTag("interpretation", "KSDATAFORMAT_SUBTYPE_PCM"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_RIFF_ChunkData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto EndPosition{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FormTypeResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Buffer, 4ull)};
	
	Result->GetValue()->AppendValue("FormType", FormTypeResult->GetValue());
	if(FormTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		while(Buffer.GetPosition() < EndPosition)
		{
			auto ChunkResult{Get_RIFF_Chunk(Buffer)};
			
			Result->GetValue()->AppendValue("Chunk", ChunkResult->GetValue());
			if(ChunkResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto RIFFChunkResult{Get_RIFF_Chunk(Buffer)};
	
	RIFFChunkResult->GetValue()->SetName("RIFFChunk");
	
	return RIFFChunkResult;
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
