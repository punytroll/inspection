#include <deque>

#include "../common/common.h"

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
	
	if(Length == Inspection::Length(0ul, 0))
	{
		Result->GetValue()->AppendTag("interpretation", "OGG nil"s);
		Result->SetSuccess(true);
	}
	else
	{
		auto VorbisHeaderPacketResult{Get_Vorbis_HeaderPacket(Buffer, Length)};
		
		Result->SetValue(VorbisHeaderPacketResult->GetValue());
		if(VorbisHeaderPacketResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
		else
		{
			// reset buffer position, so we can try something else
			Buffer.SetPosition(Start);
			
			auto VorbisAudioPacketResult{Get_Vorbis_AudioPacket(Buffer, Length)};
			
			Result->SetValue(VorbisAudioPacketResult->GetValue());
			if(VorbisAudioPacketResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
			else
			{
				// reset buffer position, so we can try something else
				Buffer.SetPosition(Start);
				
				auto PacketResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Length)};
				
				Result->GetValue()->AppendTag("interpretation", "OGG unknown"s);
				Result->GetValue()->AppendValue("Data", PacketResult->GetValue());
				Result->SetSuccess(PacketResult->GetSuccess());
			}
		}
	}
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
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 8}}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 8 > & HeaderType{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetAny())};
		
		Result->GetValue()->AppendValue("Continuation", HeaderType[0]);
		Result->GetValue()->AppendValue("BeginOfStream", HeaderType[1]);
		Result->GetValue()->AppendValue("EndOfStream", HeaderType[2]);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t NumberOfEntries)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	for(auto SegmentTableEntryIndex = 0; SegmentTableEntryIndex < NumberOfEntries; ++SegmentTableEntryIndex)
	{
		auto SegmentTableEntryResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("", SegmentTableEntryResult->GetValue());
		if(SegmentTableEntryResult->GetSuccess() == false)
		{
			Result->SetSuccess(false);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CapturePatternResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "OggS")};
	
	Result->GetValue()->AppendValue("CapturePattern", CapturePatternResult->GetValue());
	if(CapturePatternResult->GetSuccess() == true)
	{
		auto StreamStructureVersionResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("StreamStructureVersion", StreamStructureVersionResult->GetValue());
		if(StreamStructureVersionResult->GetSuccess() == true)
		{
			auto HeaderTypeResult{Get_Ogg_Page_HeaderType(Buffer)};
			
			Result->GetValue()->AppendValue("HeaderType", HeaderTypeResult->GetValue());
			if(HeaderTypeResult->GetSuccess() == true)
			{
				auto GranulePositionResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("GranulePosition", GranulePositionResult->GetValue());
				if(GranulePositionResult->GetSuccess() == true)
				{
					auto BitStreamSerialNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("BitStreamSerialNumber", BitStreamSerialNumberResult->GetValue());
					if(BitStreamSerialNumberResult->GetSuccess() == true)
					{
						auto PageSequenceNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->AppendValue("PageSequenceNumber", PageSequenceNumberResult->GetValue());
						if(PageSequenceNumberResult->GetSuccess() == true)
						{
							auto ChecksumResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->AppendValue("Checksum", ChecksumResult->GetValue());
							if(ChecksumResult->GetSuccess() == true)
							{
								auto PageSegmentsResult{Get_UnsignedInteger_8Bit(Buffer)};
								
								Result->GetValue()->AppendValue("PageSegments", PageSegmentsResult->GetValue());
								if(PageSegmentsResult->GetSuccess() == true)
								{
									auto PageSegments{std::experimental::any_cast< std::uint8_t >(PageSegmentsResult->GetAny())};
									auto SegmentTableResult{Get_Ogg_Page_SegmentTable(Buffer, PageSegments)};
									
									Result->GetValue()->AppendValue("SegmentTable", SegmentTableResult->GetValue());
									if(SegmentTableResult->GetSuccess() == true)
									{
										Result->SetSuccess(true);
										
										auto PacketStart{Buffer.GetPosition()};
										auto PacketLength{0ull};
										
										for(auto SegmentTableEntryValue : SegmentTableResult->GetValue()->GetValues())
										{
											auto SegmentTableEntry{std::experimental::any_cast< std::uint8_t >(SegmentTableEntryValue->GetAny())};
											
											PacketLength += SegmentTableEntry;
											if(SegmentTableEntry != 0xff)
											{
												// the packet ends here, read its content and try interpretation
												auto PacketResult{Get_Ogg_Packet(Buffer, PacketLength)};
												
												if(PacketResult->GetSuccess() == true)
												{
													Result->GetValue()->AppendValue("Packet", PacketResult->GetValue());
												}
												// No matter what data gets read before - successfully or ansuccessfully - we heed the values from the segment table!
												Buffer.SetPosition(PacketStart + PacketLength);
												PacketStart = Buffer.GetPosition();
												PacketLength = 0ull;
											}
										}
										if(PacketLength > 0ull)
										{
											auto PacketResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, PacketLength)};
											auto PacketValue{Result->GetValue()->AppendValue("Packet", PacketResult->GetValue())};
											
											PacketValue->AppendTag("error", "The packet spans multiple pages, which is not yet supported."s);
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

std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto StreamStarted{false};
	auto StreamEnded{false};
	
	while(StreamEnded == false)
	{
		auto OggPageResult{Get_Ogg_Page(Buffer)};
		
		Result->GetValue()->AppendValue("OggPage", OggPageResult->GetValue());
		if(OggPageResult->GetSuccess() == true)
		{
			bool BeginOfStream{std::experimental::any_cast< bool >(OggPageResult->GetValue("HeaderType")->GetValueAny("BeginOfStream"))};
			
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
			StreamEnded = std::experimental::any_cast< bool >(OggPageResult->GetValue("HeaderType")->GetValueAny("EndOfStream"));
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
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 1))};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		
		Result->GetValue()->AppendValue("PacketType", FieldResult->GetValue());
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto PacketType{std::experimental::any_cast< std::uint8_t >(Result->GetAny("PacketType"))};
		
		if(PacketType == 0x00)
		{
			Result->GetValue()->AppendTag("interpretation", "Vorbis Audio"s);
		}
		else
		{
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Data", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Boundary{Buffer.GetPosition() + Length};
	auto PacketTypeResult{Get_Vorbis_HeaderPacket_Type(Buffer)};
	
	Result->GetValue()->AppendValue("PacketType", PacketTypeResult->GetValue());
	if(PacketTypeResult->GetSuccess() == true)
	{
		auto VorbisIdentifierResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "vorbis")};
		
		Result->GetValue()->AppendValue("VorbisIdentifier", VorbisIdentifierResult->GetValue());
		if(VorbisIdentifierResult->GetSuccess() == true)
		{
			auto PacketType{std::experimental::any_cast< std::uint8_t >(PacketTypeResult->GetAny())};
			
			if(PacketType == 0x01)
			{
				auto IdentificationHeaderResult{Get_Vorbis_IdentificationHeader(Buffer)};
				
				if(IdentificationHeaderResult->GetSuccess() == true)
				{
					Result->GetValue()->AppendValues(IdentificationHeaderResult->GetValue()->GetValues());
					Result->SetSuccess(true);
				}
			}
			else if(PacketType == 0x03)
			{
				auto CommentHeaderResult{Get_Vorbis_CommentHeader(Buffer)};
				
				if(CommentHeaderResult->GetSuccess() == true)
				{
					Result->GetValue()->AppendValues(CommentHeaderResult->GetValue()->GetValues());
					Result->SetSuccess(true);
				}
			}
			else if(PacketType == 0x05)
			{
				auto SetupHeaderResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Data", SetupHeaderResult->GetValue());
				Result->SetSuccess(SetupHeaderResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket_Type(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto Type{std::experimental::any_cast< std::uint8_t >(TypeResult->GetAny())};
		
		if(Type == 0x01)
		{
			Result->GetValue()->AppendTag("interpretation", "Vorbis Identification Header"s);
			Result->SetSuccess(true);
		}
		else if(Type == 0x03)
		{
			Result->GetValue()->AppendTag("interpretation", "Vorbis Comment Header"s);
			Result->SetSuccess(true);
		}
		else if(Type == 0x05)
		{
			Result->GetValue()->AppendTag("interpretation", "Vorbis Setup Header"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "Unknown packet type " + to_string_cast(Type) + ".");
		}
	}
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
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("VorbisVersion", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("AudioChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("AudioSampleRate", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("BitrateMaximum", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("BitrateNominal", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("BitrateMinimum", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 4}}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockSize0", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 4}}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockSize1", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Boolean_1Bit(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("FramingFlag", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< bool >(Result->GetAny("FramingFlag"));
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
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}
	
	return 0;
}
