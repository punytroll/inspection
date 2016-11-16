#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/any_printing.h"
#include "../common/file_handling.h"
#include "../common/getters/4th.h"
#include "../common/results.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 4th generation getters                                                                        //
//   - These functions validate and extract in one go.                                           //
//   - They return a unique_ptr to an instance of type Result                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Results::Result > Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASCII_AlphaNumericOrSpaceCharacter(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ChannelMask(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_FormatTag(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_Chunk(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_ChunkHeader(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_fact_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_fmt_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_RIFF_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length);

std::unique_ptr< Results::Result > Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{true};
	auto Index{0ull};
	std::stringstream StringStream;
	
	while(Index < Length)
	{
		auto ASCII_AlphaNumericOrSpaceCharacterResult{Get_ASCII_AlphaNumericOrSpaceCharacter(Buffer + Index, Length - Index)};
		
		if(ASCII_AlphaNumericOrSpaceCharacterResult->GetSuccess() == true)
		{
			Index += ASCII_AlphaNumericOrSpaceCharacterResult->GetLength();
			StringStream << std::experimental::any_cast< char >(ASCII_AlphaNumericOrSpaceCharacterResult->GetAny());
		}
		else
		{
			Success = false;
			
			break;
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >("", StringStream.str())));
}

std::unique_ptr< Results::Result > Get_ASCII_AlphaNumericOrSpaceCharacter(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{'\0'};
	
	if(Length >= 1ull)
	{
		if(((Buffer[0] > 0x2F) && (Buffer[0] < 0x3A)) || ((Buffer[0] > 0x40) && (Buffer[0] < 0x5B)) || ((Buffer[0] > 0x60) && (Buffer[0] < 0x7B)) || (Buffer[0] == 0x20))
		{
			Success = true;
			Index += 1ull;
			Value = Buffer[0];
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >(Value)));
}

std::unique_ptr< Results::Result > Get_ChannelMask(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto ChannelMaskResult{Get_BitSet_32Bit_LittleEndian(Buffer + Index, Length - Index)};
	
	if(ChannelMaskResult->GetSuccess() == true)
	{
		Index += ChannelMaskResult->GetLength();
		Value->SetAny(ChannelMaskResult->GetAny());
		Success = true;
		
		auto & ChannelMask{std::experimental::any_cast< const std::bitset< 32 > & >(ChannelMaskResult->GetAny())};
		
		if(ChannelMask[0] == true)
		{
			Value->Append(std::string("SPEAKER_FRONT_LEFT"));
		}
		if(ChannelMask[1] == true)
		{
			Value->Append(std::string("SPEAKER_FRONT_RIGHT"));
		}
		if(ChannelMask[2] == true)
		{
			Value->Append(std::string("SPEAKER_FRONT_CENTER"));
		}
		if(ChannelMask[3] == true)
		{
			Value->Append(std::string("SPEAKER_LOW_FREQUENCY"));
		}
		if(ChannelMask[4] == true)
		{
			Value->Append(std::string("SPEAKER_BACK_LEFT"));
		}
		if(ChannelMask[5] == true)
		{
			Value->Append(std::string("SPEAKER_BACK_RIGHT"));
		}
		if(ChannelMask[6] == true)
		{
			Value->Append(std::string("SPEAKER_FRONT_LEFT_OF_CENTER"));
		}
		if(ChannelMask[7] == true)
		{
			Value->Append(std::string("SPEAKER_FRONT_RIGHT_OF_CENTER"));
		}
		if(ChannelMask[8] == true)
		{
			Value->Append(std::string("SPEAKER_BACK_CENTER"));
		}
		if(ChannelMask[9] == true)
		{
			Value->Append(std::string("SPEAKER_SIDE_LEFT"));
		}
		if(ChannelMask[10] == true)
		{
			Value->Append(std::string("SPEAKER_SIDE_RIGHT"));
		}
		if(ChannelMask[11] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_CENTER"));
		}
		if(ChannelMask[12] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_FRONT_LEFT"));
		}
		if(ChannelMask[13] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_FRONT_CENTER"));
		}
		if(ChannelMask[14] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_FRONT_RIGHT"));
		}
		if(ChannelMask[15] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_BACK_LEFT"));
		}
		if(ChannelMask[16] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_BACK_CENTER"));
		}
		if(ChannelMask[17] == true)
		{
			Value->Append(std::string("SPEAKER_TOP_BACK_RIGHT"));
		}
		/// @todo bits 18 to 30 are reserved and should be warned against
		if(ChannelMask[31] == true)
		{
			Value->Append(std::string("SPEAKER_ALL"));
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
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

std::unique_ptr< Results::Result > Get_FormatTag(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto FormatTagResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
		
	if(FormatTagResult->GetSuccess() == true)
	{
		Index += FormatTagResult->GetLength();
		Value->SetAny(FormatTagResult->GetAny());
		Success = true;
		
		auto FormatTag{std::experimental::any_cast< std::uint16_t >(FormatTagResult->GetAny())};
		
		switch(FormatTag)
		{
		case 0x0000:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_UNKNOWN"));
				Value->Append("Description", std::string("Unknown or invalid format tag"));
				
				break;
			}
		case 0x0001:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_PCM"));
				Value->Append("Description", std::string("Pulse Code Modulation"));
				
				break;
			}
		case 0x0002:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_ADPCM"));
				Value->Append("Description", std::string("Microsoft Adaptive Differental PCM"));
				
				break;
			}
		case 0x0003:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_IEEE_FLOAT"));
				Value->Append("Description", std::string("32-bit floating-point"));
				
				break;
			}
		case 0x0055:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_MPEGLAYER3"));
				Value->Append("Description", std::string("ISO/MPEG Layer3"));
				
				break;
			}
		case 0x0092:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_DOLBY_AC3_SPDIF"));
				Value->Append("Description", std::string("Dolby Audio Codec 3 over S/PDIF"));
				
				break;
			}
		case 0x0161:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO2"));
				Value->Append("Description", std::string("Windows Media Audio"));
				
				break;
			}
		case 0x0162:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO3"));
				Value->Append("Description", std::string("Windows Media Audio Pro"));
				
				break;
			}
		case 0x0164:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_WMASPDIF"));
				Value->Append("Description", std::string("Windows Media Audio over S/PDIF"));
				
				break;
			}
		case 0xFFFE:
			{
				Value->Append("ConstantName", std::string("WAVE_FORMAT_EXTENSIBLE"));
				Value->Append("Description", std::string("All new audio formats"));
				
				break;
			}
		default:
			{
				Value->Append("ConstantName", std::string("<no interpretation>"));
				Value->Append("Description", std::string("<no interpretation>"));
				
				break;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

std::unique_ptr< Results::Result > Get_RIFF_Chunk(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto ChunkHeaderResult{Get_RIFF_ChunkHeader(Buffer + Index, Length - Index)};
		
	if(ChunkHeaderResult->GetSuccess() == true)
	{
		Index += ChunkHeaderResult->GetLength();
		Value->Append(ChunkHeaderResult->GetValue("ChunkIdentifier"));
		Value->Append(ChunkHeaderResult->GetValue("ChunkSize"));
		
		auto ChunkSize{std::experimental::any_cast< std::uint32_t >(ChunkHeaderResult->GetAny("ChunkSize"))};
		
		if(ChunkSize + ChunkHeaderResult->GetLength() <= Length)
		{
			Success = true;
			// trim the length field to only what is permissible by the chunk header
			Length = ChunkSize + ChunkHeaderResult->GetLength();
			
			auto ChunkIdentifier{std::experimental::any_cast< std::string >(ChunkHeaderResult->GetAny("ChunkIdentifier"))};
			std::shared_ptr< Results::Result > ChunkDataResult;
			
			if(ChunkIdentifier == "RIFF")
			{
				ChunkDataResult = Get_RIFF_RIFF_ChunkData(Buffer + Index, Length - Index);
			}
			else if(ChunkIdentifier == "fact")
			{
				ChunkDataResult = Get_RIFF_fact_ChunkData(Buffer + Index, Length - Index);
			}
			else if(ChunkIdentifier == "fmt ")
			{
				ChunkDataResult = Get_RIFF_fmt_ChunkData(Buffer + Index, Length - Index);
			}
			else
			{
				ChunkDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer + Index, Length - Index);
			}
			if((ChunkDataResult) && (ChunkDataResult->GetSuccess() == true))
			{
				Index += ChunkDataResult->GetLength();
				Value->Append("ChunkData", ChunkDataResult->GetValue());
			}
			else
			{
				Index += ChunkSize;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

std::unique_ptr< Results::Result > Get_RIFF_ChunkHeader(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	
	if(Length - Index >= 4ull)
	{
		auto ChunkIdentifierResult{Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(Buffer + Index, 4ull)};
		
		if(ChunkIdentifierResult->GetSuccess() == true)
		{
			Index += ChunkIdentifierResult->GetLength();
			Value->Append("ChunkIdentifier", ChunkIdentifierResult->GetValue());
			
			auto ChunkSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
			
			if(ChunkSizeResult->GetSuccess() == true)
			{
				Index += ChunkSizeResult->GetLength();
				Value->Append("ChunkSize", ChunkSizeResult->GetValue());
				Success = true;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

std::unique_ptr< Results::Result > Get_RIFF_fact_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto SampleLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
		
	if(SampleLengthResult->GetSuccess() == true)
	{
		Index += SampleLengthResult->GetLength();
		Value->Append("SampleLength", SampleLengthResult->GetValue());
		Success = true;
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

std::unique_ptr< Results::Result > Get_RIFF_fmt_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	
	if(Length >= 16)
	{
		auto FormatTagResult{Get_FormatTag(Buffer + Index, Length - Index)};
		
		if(FormatTagResult->GetSuccess() == true)
		{
			Index += FormatTagResult->GetLength();
			Value->Append("FormatTag", FormatTagResult->GetValue());
			
			auto NumberOfChannelsResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
			
			if(NumberOfChannelsResult->GetSuccess() == true)
			{
				Index += NumberOfChannelsResult->GetLength();
				Value->Append("NumberOfChannels", NumberOfChannelsResult->GetValue());
				
				auto SamplesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
				
				if(SamplesPerSecondResult->GetSuccess() == true)
				{
					Index += SamplesPerSecondResult->GetLength();
					Value->Append("SamplesPerSecond", SamplesPerSecondResult->GetValue());
					
					auto AverageBytesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
					
					if(AverageBytesPerSecondResult->GetSuccess() == true)
					{
						Index += AverageBytesPerSecondResult->GetLength();
						Value->Append("AverageBytesPerSecond", AverageBytesPerSecondResult->GetValue());
						
						auto BlockAlignResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
						
						if(BlockAlignResult->GetSuccess() == true)
						{
							Index += BlockAlignResult->GetLength();
							Value->Append("BlockAlign", BlockAlignResult->GetValue());
							
							auto BitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
							
							if(BitsPerSampleResult->GetSuccess() == true)
							{
								Index += BitsPerSampleResult->GetLength();
								Value->Append("BitsPerSample", BitsPerSampleResult->GetValue());
								
								if(Length - Index >= 18)
								{
									auto ExtensionSizeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
									
									if(ExtensionSizeResult->GetSuccess() == true)
									{
										Index += ExtensionSizeResult->GetLength();
										Value->Append("ExtensionSize", ExtensionSizeResult->GetValue());
										
										auto ExtensionSize{std::experimental::any_cast< std::uint16_t >(ExtensionSizeResult->GetAny())};
										
										if((ExtensionSize == 0) || (ExtensionSize == 22))
										{
											Success = Length - Index == ExtensionSize;
											
											auto ValidBitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
											
											if(ValidBitsPerSampleResult->GetSuccess() == true)
											{
												Index += ValidBitsPerSampleResult->GetLength();
												Value->Append("ValidBitsPerSample", ValidBitsPerSampleResult->GetValue());
												
												auto ChannelMaskResult{Get_ChannelMask(Buffer + Index, Length - Index)};
												
												if(ChannelMaskResult->GetSuccess() == true)
												{
													Index += ChannelMaskResult->GetLength();
													Value->Append("ChannelMask", ChannelMaskResult->GetValue());
													assert(Length - Index == 16);
													
													auto SubFormatResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer + Index, Length - Index)};
													
													if(SubFormatResult->GetSuccess() == true)
													{
														Index += SubFormatResult->GetLength();
														Value->Append("SubFormat", SubFormatResult->GetValue());
													}
												}
											}
										}
										else
										{
											Success = false;
										}
									}
								}
								else
								{
									Success = true;
								}
							}
						}
					}
				}
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

std::unique_ptr< Results::Result > Get_RIFF_RIFF_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	
	if(Length - Index >= 4ull)
	{
		auto FormTypeResult{Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(Buffer + Index, 4ull)};
		
		if(FormTypeResult->GetSuccess() == true)
		{
			Index += FormTypeResult->GetLength();
			Value->Append("FormType", FormTypeResult->GetValue());
			
			auto Chunks{std::make_shared< Results::Value >(std::string("Chunks"))};
			
			while(Length - Index > 0)
			{
				auto ChunkResult{Get_RIFF_Chunk(Buffer + Index, Length - Index)};
				
				if(ChunkResult->GetSuccess() == true)
				{
					Index += ChunkResult->GetLength();
					Chunks->Append(ChunkResult->GetValue());
				}
				else
				{
					break;
				}
			}
			if(Chunks->GetCount() > 0)
			{
				Value->Append(Chunks);
			}
			Success = true;
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Value));
}

void PrintValue(const std::string & Indentation, std::shared_ptr< Results::Value > Value)
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
	if(HeaderLine == true)
	{
		std::cout << std::endl;
	}
	if(Value->GetCount() > 0)
	{
		for(auto & SubValue : Value->GetValues())
		{
			PrintValue(Indentation + "    ", SubValue);
		}
	}
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
				std::int64_t Index{0};
				
				while(Index < FileSize)
				{
					auto RIFFChunkResult(Get_RIFF_Chunk(Address + Index, FileSize - Index));
					
					if(RIFFChunkResult->GetSuccess() == true)
					{
						Index += RIFFChunkResult->GetLength();
						PrintValue("", RIFFChunkResult->GetValue());
					}
					else
					{
						Index += 1;
					}
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
