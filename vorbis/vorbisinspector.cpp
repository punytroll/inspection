#include <deque>

#include "../common/common.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Buffer & Buffer, std::uint64_t Length);
std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Ogg_Page_SegmentTable(Inspection::Buffer & Buffer, std::uint8_t NumberOfEntries);
std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, std::uint64_t Length);
std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Buffer & Buffer);


std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
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
		
		auto PacketResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Length)};
		
		Result->GetValue()->Append("Data", PacketResult->GetValue());
		Result->SetSuccess(PacketResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderTypeResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(HeaderTypeResult->GetValue());
	if(HeaderTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 8 > & HeaderType{std::experimental::any_cast< const std::bitset< 8 > & >(HeaderTypeResult->GetAny())};
		
		Result->GetValue()->Append("Continuation", HeaderType[0]);
		Result->GetValue()->Append("BeginOfStream", HeaderType[1]);
		Result->GetValue()->Append("EndOfStream", HeaderType[2]);
	}
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
		
		Result->GetValue()->Append("", SegmentTableEntryResult->GetValue());
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
	
	if(CapturePatternResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("CapturePattern", CapturePatternResult->GetValue());
		
		auto StreamStructureVersionResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		if(StreamStructureVersionResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("StreamStructureVersion", StreamStructureVersionResult->GetValue());
			
			auto HeaderTypeResult{Get_Ogg_Page_HeaderType(Buffer)};
			
			if(HeaderTypeResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("HeaderType", HeaderTypeResult->GetValue());
				
				auto GranulePositionResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
				
				if(GranulePositionResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("GranulePosition", GranulePositionResult->GetValue());
					
					auto BitStreamSerialNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					if(BitStreamSerialNumberResult->GetSuccess() == true)
					{
						Result->GetValue()->Append("BitStreamSerialNumber", BitStreamSerialNumberResult->GetValue());
						
						auto PageSequenceNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						if(PageSequenceNumberResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("PageSequenceNumber", PageSequenceNumberResult->GetValue());
							
							auto ChecksumResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							if(ChecksumResult->GetSuccess() == true)
							{
								Result->GetValue()->Append("Checksum", ChecksumResult->GetValue());
								
								auto PageSegmentsResult{Get_UnsignedInteger_8Bit(Buffer)};
								
								if(PageSegmentsResult->GetSuccess() == true)
								{
									Result->GetValue()->Append("PageSegments", PageSegmentsResult->GetValue());
									
									auto PageSegments{std::experimental::any_cast< std::uint8_t >(PageSegmentsResult->GetAny())};
									auto SegmentTableResult{Get_Ogg_Page_SegmentTable(Buffer, PageSegments)};
									
									if(SegmentTableResult->GetSuccess() == true)
									{
										Result->GetValue()->Append("SegmentTable", SegmentTableResult->GetValue());
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
													Result->GetValue()->Append("Packet", PacketResult->GetValue());
												}
												// No matter what data gets read before - successfully or ansuccessfully - we heed the values from the segment table!
												Buffer.SetPosition(PacketStart + PacketLength);
												PacketStart = Buffer.GetPosition();
												PacketLength = 0ull;
											}
										}
										if(PacketLength > 0ull)
										{
											std::cerr << "A packet spans multiple pages, which is not yet supported." << std::endl;
											Result->SetSuccess(false);
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
		
		if(OggPageResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("OggPage", OggPageResult->GetValue());
			
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

std::unique_ptr< Inspection::Result > Get_Vorbis_HeaderPacket(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PacketTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	if(PacketTypeResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("PacketType", PacketTypeResult->GetValue());
		
		auto VorbisIdentifierResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "vorbis")};
		
		if(VorbisIdentifierResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("VorbisIdentifier", VorbisIdentifierResult->GetValue());
			
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
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_Vorbis_IdentificationHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto VorbisVersionResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(VorbisVersionResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("VorbisVersion", VorbisVersionResult->GetValue());
		
		auto AudioChannelsResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		if(AudioChannelsResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("AudioChannels", AudioChannelsResult->GetValue());
			
			auto AudioSampleRateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			if(AudioSampleRateResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("AudioSampleRate", AudioSampleRateResult->GetValue());
				
				auto BitrateMaximumResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
				
				if(BitrateMaximumResult->GetSuccess() == true)
				{
					Result->GetValue()->Append("BitrateMaximum", BitrateMaximumResult->GetValue());
					
					auto BitrateNominalResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
					
					if(BitrateNominalResult->GetSuccess() == true)
					{
						Result->GetValue()->Append("BitrateNominal", BitrateNominalResult->GetValue());
						
						auto BitrateMinimumResult{Get_SignedInteger_32Bit_LittleEndian(Buffer)};
						
						if(BitrateMinimumResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("BitrateMinimum", BitrateMinimumResult->GetValue());
							
							auto BlockSize0Result{Get_UnsignedInteger_4Bit(Buffer)};
							
							if(BlockSize0Result->GetSuccess() == true)
							{
								Result->GetValue()->Append("BlockSize0", BlockSize0Result->GetValue());
								
								auto BlockSize1Result{Get_UnsignedInteger_4Bit(Buffer)};
								
								if(BlockSize1Result->GetSuccess() == true)
								{
									Result->GetValue()->Append("BlockSize1", BlockSize1Result->GetValue());
									
									auto FramingFlagResult{Get_Boolean_1Bit(Buffer)};
									
									if(FramingFlagResult->GetSuccess() == true)
									{
										auto FramingFlag{std::experimental::any_cast< bool >(FramingFlagResult->GetAny())};
										
										Result->GetValue()->Append("FramingFlag", FramingFlagResult->GetValue());
										Result->SetSuccess(FramingFlag);
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
