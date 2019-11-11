#include <bitset>
#include <deque>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getter_repository.h>
#include <common/getters.h>
#include <common/result.h>

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Vorbis_AudioPacket(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket_Type(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);

std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(Reader.HasRemaining() == false)
		{
			Result->GetValue()->AddTag("interpretation", "OGG nil"s);
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Vorbis_AudioPacket(PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			if(Continue == true)
			{
				Result->SetValue(PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{Get_Vorbis_HeaderPacket(PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				if(Continue == true)
				{
					Result->SetValue(PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {})};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendField("Data", PartResult->GetValue());
					Result->GetValue()->AddTag("interpretation", "OGG unknown"s);
					Reader.AdvancePosition(PartReader.GetConsumedLength());
				}
			}
		}
	}
	// reading
	if(Continue == true)
	{
		if(Reader.GetPositionInBuffer().GetBits() > 0)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Data_Unset_Until8BitAlignment(PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendField("EndOfPacketAlignment", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_BitSet_8Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_String_ASCII_Alphabetic_ByTemplate(PartReader, {{"Template", "OggS"s}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("CapturePattern", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_8Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("StreamStructureVersion", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Ogg_Page_HeaderType(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("HeaderType", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("GranulePosition", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("BitStreamSerialNumber", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("PageSequenceNumber", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("Checksum", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_8Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("PageSegments", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto PageSegments{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetField("PageSegments")->GetData())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"ElementGetter", std::vector< std::string >{"Number", "Integer", "Unsigned", "8Bit"}}, {"NumberOfElements", static_cast< std::uint64_t >(PageSegments)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("SegmentTable", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Length PacketLength{0, 0};
		
		for(auto SegmentTableEntryValue : Result->GetValue()->GetField("SegmentTable")->GetFields())
		{
			auto SegmentTableEntry{std::experimental::any_cast< std::uint8_t >(SegmentTableEntryValue->GetData())};
			
			PacketLength += Inspection::Length{SegmentTableEntry, 0};
			if(SegmentTableEntry != 0xff)
			{
				Inspection::Reader PartReader{Reader, PacketLength};
				// the packet ends here, read its content and try interpretation
				auto PartResult{Get_Ogg_Packet(PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendField("Packet", PartResult->GetValue());
				assert(PacketLength == PartReader.GetConsumedLength());
				// No matter what data gets read before - successfully or unsuccessfully - we heed the values from the segment table!
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				PacketLength = Inspection::Length{0, 0};
			}
		}
		if(PacketLength > Inspection::Length{0, 0})
		{
			Inspection::Reader PartReader{Reader, PacketLength};
			auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {})};
			
			Result->GetValue()->AppendField("Packet", PartResult->GetValue());
			PartResult->GetValue()->AddTag("error", "The packet spans multiple pages, which is not yet supported."s);
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	auto StreamStarted{false};
	auto StreamEnded{false};
	
	while((Continue == true) && (StreamEnded == false))
	{
		// reading
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Ogg_Page(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("OggPage", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
		if(Continue == true)
		{
			bool BeginOfStream{std::experimental::any_cast< bool >(PartResult->GetValue()->GetField("HeaderType")->GetField("BeginOfStream")->GetData())};
			
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
			StreamEnded = std::experimental::any_cast< bool >(PartResult->GetValue()->GetField("HeaderType")->GetField("EndOfStream")->GetData());
		}
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_AudioPacket(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_1Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("PacketType", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
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
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("Data", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Vorbis_HeaderPacket_Type(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("PacketType", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_String_ASCII_Alphabetic_ByTemplate(PartReader, {{"Template", "vorbis"s}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("VorbisIdentifier", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto PacketType{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetField("PacketType")->GetData())};
		
		if(PacketType == 0x01)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Vorbis_IdentificationHeader(PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendFields(PartResult->GetValue()->GetFields());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(PacketType == 0x03)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Vorbis_CommentHeader(PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendFields(PartResult->GetValue()->GetFields());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(PacketType == 0x05)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Data_SetOrUnset_EndedByLength(PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendField("Data", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			assert(false);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket_Type(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_8Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("VorbisVersion", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_8Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("AudioChannels", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("AudioSampleRate", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_SignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("BitrateMaximum", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_SignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("BitrateNominal", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_SignedInteger_32Bit_LittleEndian(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("BitrateMinimum", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_4Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("BlockSize0", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_4Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("BlockSize1", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Boolean_1Bit(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendField("FramingFlag", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< bool >(Result->GetValue()->GetField("FramingFlag")->GetData());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	Inspection::Reader PartReader{Reader};
	
	PartReader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
	
	auto PartResult(Get_Ogg_Stream(PartReader, {}));
	
	PartResult->GetValue()->SetName("OggStream");
	Reader.AdvancePosition(PartReader.GetConsumedLength());
	
	return PartResult;
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
		ReadItem(Paths.front(), Process, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
