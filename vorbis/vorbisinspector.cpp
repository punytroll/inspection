#include <bitset>
#include <deque>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t NumberOfEntries);
std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_AudioPacket(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket_Type(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// interpretation
	if(Length == Inspection::Length{0, 0})
	{
		Result->GetValue()->AddTag("interpretation", "OGG nil"s);
	}
	else
	{
		auto VorbisHeaderPacketResult{Get_Vorbis_HeaderPacket(Buffer, Length)};
		
		Result->SetValue(VorbisHeaderPacketResult->GetValue());
		if(VorbisHeaderPacketResult->GetSuccess() == false)
		{
			// reset buffer position, so we can try something else
			Buffer.SetPosition(Start);
			
			auto VorbisAudioPacketResult{Get_Vorbis_AudioPacket(Buffer, Length)};
			
			Result->SetValue(VorbisAudioPacketResult->GetValue());
			if(VorbisAudioPacketResult->GetSuccess() == false)
			{
				// reset buffer position, so we can try something else
				Buffer.SetPosition(Start);
				
				Inspection::Reader FieldReader{Buffer, Length};
				auto FieldResult{Get_Data_SetOrUnset_EndedByLength(FieldReader)};
				auto FieldValue{Result->GetValue()->AppendField("Data", FieldResult->GetValue())};
				
				Result->GetValue()->AddTag("interpretation", "OGG unknown"s);
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 8 > & HeaderType{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetValue()->GetData())};
		
		Result->GetValue()->AppendField("Continuation", HeaderType[0]);
		Result->GetValue()->AppendField("BeginOfStream", HeaderType[1]);
		Result->GetValue()->AppendField("EndOfStream", HeaderType[2]);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t NumberOfEntries)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	for(auto SegmentTableEntryIndex = 0; (Continue == true) && (SegmentTableEntryIndex < NumberOfEntries); ++SegmentTableEntryIndex)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Buffer};
		auto PartResult{Get_String_ASCII_Alphabetic_ByTemplate(PartReader, {{"Template", "OggS"s}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("CapturePattern", PartResult->GetValue());
		Buffer.SetPosition(PartReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("StreamStructureVersion", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Ogg_Page_HeaderType(Buffer)};
		auto FieldValue{Result->GetValue()->AppendField("HeaderType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("GranulePosition", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("BitStreamSerialNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("PageSequenceNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("Checksum", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("PageSegments", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto PageSegments{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetField("PageSegments")->GetData())};
		auto FieldResult{Get_Ogg_Page_SegmentTable(Buffer, PageSegments)};
		auto FieldValue{Result->GetValue()->AppendField("SegmentTable", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto PacketStart{Buffer.GetPosition()};
		auto PacketLength{0ull};
		
		for(auto SegmentTableEntryValue : Result->GetValue()->GetField("SegmentTable")->GetFields())
		{
			auto SegmentTableEntry{std::experimental::any_cast< std::uint8_t >(SegmentTableEntryValue->GetData())};
			
			PacketLength += SegmentTableEntry;
			if(SegmentTableEntry != 0xff)
			{
				// the packet ends here, read its content and try interpretation
				auto PacketResult{Get_Ogg_Packet(Buffer, Inspection::Length{PacketLength, 0})};
				
				if(PacketResult->GetSuccess() == true)
				{
					Result->GetValue()->AppendField("Packet", PacketResult->GetValue());
				}
				// No matter what data gets read before - successfully or ansuccessfully - we heed the values from the segment table!
				Buffer.SetPosition(PacketStart + Inspection::Length{PacketLength, 0});
				PacketStart = Buffer.GetPosition();
				PacketLength = 0ull;
			}
		}
		if(PacketLength > 0ull)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{PacketLength, 0}};
			auto FieldResult{Get_Data_SetOrUnset_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendField("Packet", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			FieldValue->AddTag("error", "The packet spans multiple pages, which is not yet supported."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto StreamStarted{false};
	auto StreamEnded{false};
	
	while(StreamEnded == false)
	{
		auto OggPageResult{Get_Ogg_Page(Buffer)};
		
		Result->GetValue()->AppendField("OggPage", OggPageResult->GetValue());
		if(OggPageResult->GetSuccess() == true)
		{
			bool BeginOfStream{std::experimental::any_cast< bool >(OggPageResult->GetValue()->GetField("HeaderType")->GetField("BeginOfStream")->GetData())};
			
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
			StreamEnded = std::experimental::any_cast< bool >(OggPageResult->GetValue()->GetField("HeaderType")->GetField("EndOfStream")->GetData());
		}
		else
		{
			//~ Result->SetSuccess(false);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_AudioPacket(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Boundary{Buffer.GetPosition() + Length};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		
		Result->GetValue()->AppendField("PacketType", FieldResult->GetValue());
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto PacketType{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetField("PacketType")->GetData())};
		
		if(PacketType == 0x00)
		{
			Result->GetValue()->AddTag("interpretation", "Vorbis Audio"s);
		}
		else
		{
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_Data_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("Data", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Boundary{Buffer.GetPosition() + Length};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Vorbis_HeaderPacket_Type(Buffer)};
		auto FieldValue{Result->GetValue()->AppendField("PacketType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Buffer};
		auto PartResult{Get_String_ASCII_Alphabetic_ByTemplate(PartReader, {{"Template", "vorbis"s}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("VorbisIdentifier", PartResult->GetValue());
		Buffer.SetPosition(PartReader);
	}
	// reading
	if(Continue == true)
	{
		auto PacketType{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetField("PacketType")->GetData())};
		
		if(PacketType == 0x01)
		{
			auto FieldResult{Get_Vorbis_IdentificationHeader(Buffer)};
			
			Result->GetValue()->AppendFields(FieldResult->GetValue()->GetFields());
			UpdateState(Continue, FieldResult);
		}
		else if(PacketType == 0x03)
		{
			auto FieldResult{Get_Vorbis_CommentHeader(Buffer)};
			
			Result->GetValue()->AppendFields(FieldResult->GetValue()->GetFields());
			UpdateState(Continue, FieldResult);
		}
		else if(PacketType == 0x05)
		{
			Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
			auto FieldResult{Get_Data_SetOrUnset_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendField("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket_Type(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Type{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetData())};
		
		if(Type == 0x01)
		{
			Result->GetValue()->AddTag("interpretation", "Vorbis Identification Header"s);
		}
		else if(Type == 0x03)
		{
			Result->GetValue()->AddTag("interpretation", "Vorbis Comment Header"s);
		}
		else if(Type == 0x05)
		{
			Result->GetValue()->AddTag("interpretation", "Vorbis Setup Header"s);
		}
		else
		{
			Result->GetValue()->AddTag("error", "Unknown packet type " + to_string_cast(Type) + ".");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("VorbisVersion", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("AudioChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("AudioSampleRate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("BitrateMaximum", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("BitrateNominal", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("BitrateMinimum", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("BlockSize0", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("BlockSize1", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
		auto FieldResult{Get_Boolean_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendField("FramingFlag", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< bool >(Result->GetValue()->GetField("FramingFlag")->GetData());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	
	auto OggStreamResult(Get_Ogg_Stream(Buffer));
	
	OggStreamResult->GetValue()->SetName("OggStream");
	
	return OggStreamResult;
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
