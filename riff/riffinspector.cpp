#include <deque>
#include <string>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>

using namespace std::string_literals;

/// RIFF GUIDs
Inspection::GUID g_KSDATAFORMAT_SUBTYPE_PCM{"00000001-0000-0010-8000-00aa00389b71"};

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_RIFF_Chunk(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_ChunkHeader(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fact_ChunkData(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_ChannelMask(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_CommonFields(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_SubFormat(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_RIFF_ChunkData(Inspection::Reader & Reader);
std::unique_ptr< Inspection::Result > Get_RIFF_File(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_RIFF_Chunk(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_RIFF_ChunkHeader(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto ChunkLength{Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetAny("Size")), 0}};
		
		if(Reader.Has(ChunkLength) == true)
		{
			auto ChunkIdentifier{std::experimental::any_cast< std::string >(Result->GetAny("Identifier"))};
			
			if(ChunkIdentifier == "RIFF")
			{
				Inspection::Reader PartReader{Reader, ChunkLength};
				auto PartResult{Get_RIFF_RIFF_ChunkData(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else if(ChunkIdentifier == "fact")
			{
				Inspection::Reader PartReader{Reader, ChunkLength};
				auto PartResult{Get_RIFF_fact_ChunkData(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else if(ChunkIdentifier == "fmt ")
			{
				Inspection::Reader PartReader{Reader, ChunkLength};
				auto PartResult{Get_RIFF_fmt_ChunkData(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Inspection::Reader PartReader{Reader, ChunkLength};
				auto PartResult{Get_Bits_SetOrUnset_EndedByLength(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The RIFF chunk claims to have a length of " + to_string_cast(ChunkLength) + " bytes and bits but only " + to_string_cast(Reader.GetRemainingLength()) + " bytes and bits are available.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_ChunkHeader(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{4, 0}};
		auto PartResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Identifier", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Size", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fact_ChunkData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("NumberOfSamples", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_RIFF_fmt_ChunkData_CommonFields(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		const std::string & FormatTag{std::experimental::any_cast< const std::string & >(Result->GetValue("FormatTag")->GetTagAny("constant name"))};
		
		if(FormatTag == "WAVE_FORMAT_PCM")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(FormatTag == "WAVE_FORMAT_EXTENSIBLE")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("Rest", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
			Result->GetValue()->AddTag("error", "Unknown format tag " + FormatTag + ".");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_ChannelMask(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_BitSet_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
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
			Continue &= !ChannelMask[BitIndex];
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_CommonFields(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Microsoft_WaveFormat_FormatTag(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("FormatTag", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("NumberOfChannels", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SamplesPerSecond", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("AverageBytesPerSecond", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BlockAlign", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_Extensible(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BitsPerSample", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ExtensionSize", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ValidBitsPerSample", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_RIFF_fmt_ChunkData_ChannelMask(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ChannelMask", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_RIFF_fmt_ChunkData_SubFormat(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SubFormat", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_FormatSpecificFields_PCM(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BitsPerSample", PartResult->GetValue());
		Result->GetValue("BitsPerSample")->AddTag("units", "bits per sample"s);
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_fmt_ChunkData_SubFormat(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_GUID_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetAny())};
		
		if(GUID == g_KSDATAFORMAT_SUBTYPE_PCM)
		{
			Result->GetValue()->AddTag("interpretation", "KSDATAFORMAT_SUBTYPE_PCM"s);
		}
		else
		{
			Result->GetValue()->AddTag("interpretation", nullptr);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_RIFF_RIFF_ChunkData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{4, 0}};
		auto PartResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("FormType", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByLength(PartReader, Get_RIFF_Chunk)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Chunks", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Chunk");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Buffer};
		auto PartResult{Get_RIFF_Chunk(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("RIFFChunk", PartResult->GetValue());
		Result->GetValue()->SetName("RIFFFile");
		Buffer.SetPosition(PartReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
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
