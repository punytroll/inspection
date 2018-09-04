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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_RIFF_ChunkHeader(Buffer)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto ChunkSize{std::experimental::any_cast< std::uint32_t >(Result->GetAny("Size"))};
		
		if(Buffer.Has(ChunkSize, 0) == true)
		{
			auto ChunkIdentifier{std::experimental::any_cast< std::string >(Result->GetAny("Identifier"))};
			
			if(ChunkIdentifier == "RIFF")
			{
				auto FieldResult{Get_RIFF_RIFF_ChunkData(Buffer, Inspection::Length{ChunkSize, 0})};
				
				Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
				UpdateState(Continue, FieldResult);
			}
			else if(ChunkIdentifier == "fact")
			{
				auto FieldResult{Get_RIFF_fact_ChunkData(Buffer)};
				
				Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
				UpdateState(Continue, FieldResult);
			}
			else if(ChunkIdentifier == "fmt ")
			{
				auto FieldResult{Get_RIFF_fmt_ChunkData(Buffer)};
				
				Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{ChunkSize, 0}};
				auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
				auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_ChunkHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fact_ChunkData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfSamples", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_BitSet_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		const std::bitset< 32 > & ChannelMask{std::experimental::any_cast< const std::bitset< 32 > & >(Result->GetAny())};
		
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
			Continue &= ~ChannelMask[BitIndex];
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_CommonFields(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_Microsoft_WaveFormat_FormatTag(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FormatTag", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SamplesPerSecond", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AverageBytesPerSecond", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockAlign", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BitsPerSample", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ExtensionSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ValidBitsPerSample", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_RIFF_fmt_ChunkData_ChannelMask(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("ChannelMask", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_RIFF_fmt_ChunkData_SubFormat(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("SubFormat", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BitsPerSample", FieldResult->GetValue())};
		
		FieldValue->AppendTag("units", "bits per sample"s);
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_SubFormat(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_GUID_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetAny())};
		
		if(GUID == g_KSDATAFORMAT_SUBTYPE_PCM)
		{
			Result->GetValue()->AppendTag("interpretation", "KSDATAFORMAT_SUBTYPE_PCM"s);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_RIFF_ChunkData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto EndPosition{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FormType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		while((Buffer.GetPosition() < EndPosition) && (Continue == true))
		{
			auto FieldResult{Get_RIFF_Chunk(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("Chunk", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
		ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
