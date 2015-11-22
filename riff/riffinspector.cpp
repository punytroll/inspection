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
#include "../common/results.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 4th generation getters                                                                        //
//   - These functions validate and extract in one go.                                           //
//   - They return a unique_ptr to an instance of type Result                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Results::Result > Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASCII_AlphaNumericOrSpaceCharacter(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_FormatTag(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_HexadecimalString_TerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_LittleEndian_16Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_LittleEndian_32Bit_BitSet(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_LittleEndian_32Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length);
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

std::unique_ptr< Results::Result > Get_FormatTag(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	auto FormatTagResult{Get_LittleEndian_16Bit_UnsignedInteger(Buffer + Index, Length - Index)};
		
	if(FormatTagResult->GetSuccess() == true)
	{
		Index += FormatTagResult->GetLength();
		Values->Append("FormatTag", FormatTagResult->GetValue());
		Success = true;
		
		auto FormatTag{std::experimental::any_cast< std::uint16_t >(FormatTagResult->GetAny())};
		
		switch(FormatTag)
		{
		case 0x0000:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_UNKNOWN"));
				Values->Append("Description", std::string("Unknown or invalid format tag"));
				
				break;
			}
		case 0x0001:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_PCM"));
				Values->Append("Description", std::string("Pulse Code Modulation"));
				
				break;
			}
		case 0x0002:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_ADPCM"));
				Values->Append("Description", std::string("Microsoft Adaptive Differental PCM"));
				
				break;
			}
		case 0x0003:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_IEEE_FLOAT"));
				Values->Append("Description", std::string("32-bit floating-point"));
				
				break;
			}
		case 0x0055:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_MPEGLAYER3"));
				Values->Append("Description", std::string("ISO/MPEG Layer3"));
				
				break;
			}
		case 0x0092:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_DOLBY_AC3_SPDIF"));
				Values->Append("Description", std::string("Dolby Audio Codec 3 over S/PDIF"));
				
				break;
			}
		case 0x0161:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO2"));
				Values->Append("Description", std::string("Windows Media Audio"));
				
				break;
			}
		case 0x0162:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO3"));
				Values->Append("Description", std::string("Windows Media Audio Pro"));
				
				break;
			}
		case 0x0164:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_WMASPDIF"));
				Values->Append("Description", std::string("Windows Media Audio over S/PDIF"));
				
				break;
			}
		case 0xFFFE:
			{
				Values->Append("ConstantName", std::string("WAVE_FORMAT_EXTENSIBLE"));
				Values->Append("Description", std::string("All new audio formats"));
				
				break;
			}
		default:
			{
				Values->Append("ConstantName", std::string("<no interpretation>"));
				Values->Append("Description", std::string("<no interpretation>"));
				
				break;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

std::unique_ptr< Results::Result > Get_HexadecimalString_TerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{true};
	auto Index{0ull};
	std::stringstream StringStream;
	
	StringStream << std::hex << std::setfill('0');
	while(Index < Length)
	{
		if(Index > 0)
		{
			StringStream << ' ';
		}
		StringStream << std::setw(2) << std::right << static_cast< std::uint32_t >(Buffer[Index]);
		Index += 1;
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >("", StringStream.str())));
}

std::unique_ptr< Results::Result > Get_LittleEndian_16Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	std::uint16_t Value{0ul};
	
	if(Length >= 2ull)
	{
		Success = true;
		Index = 2ull;
		Value = static_cast< std::uint16_t >(Buffer[0]) + (static_cast< std::uint16_t >(Buffer[1]) << 8);
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >(Value)));
}

std::unique_ptr< Results::Result > Get_LittleEndian_32Bit_BitSet(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	std::bitset<32> Value;
	
	if(Length >= 4ull)
	{
		Success = true;
		Index = 4ull;
		Value[0] = (Buffer[0] & 0x01) == 0x01;
		Value[1] = (Buffer[0] & 0x02) == 0x02;
		Value[2] = (Buffer[0] & 0x04) == 0x04;
		Value[3] = (Buffer[0] & 0x08) == 0x08;
		Value[4] = (Buffer[0] & 0x10) == 0x10;
		Value[5] = (Buffer[0] & 0x20) == 0x20;
		Value[6] = (Buffer[0] & 0x40) == 0x40;
		Value[7] = (Buffer[0] & 0x80) == 0x80;
		Value[8] = (Buffer[1] & 0x01) == 0x01;
		Value[9] = (Buffer[1] & 0x02) == 0x02;
		Value[10] = (Buffer[1] & 0x04) == 0x04;
		Value[11] = (Buffer[1] & 0x08) == 0x08;
		Value[12] = (Buffer[1] & 0x10) == 0x10;
		Value[13] = (Buffer[1] & 0x20) == 0x20;
		Value[14] = (Buffer[1] & 0x40) == 0x40;
		Value[15] = (Buffer[1] & 0x80) == 0x80;
		Value[16] = (Buffer[2] & 0x01) == 0x01;
		Value[17] = (Buffer[2] & 0x02) == 0x02;
		Value[18] = (Buffer[2] & 0x04) == 0x04;
		Value[19] = (Buffer[2] & 0x08) == 0x08;
		Value[20] = (Buffer[2] & 0x10) == 0x10;
		Value[21] = (Buffer[2] & 0x20) == 0x20;
		Value[22] = (Buffer[2] & 0x40) == 0x40;
		Value[23] = (Buffer[2] & 0x80) == 0x80;
		Value[24] = (Buffer[3] & 0x01) == 0x01;
		Value[25] = (Buffer[3] & 0x02) == 0x02;
		Value[26] = (Buffer[3] & 0x04) == 0x04;
		Value[27] = (Buffer[3] & 0x08) == 0x08;
		Value[28] = (Buffer[3] & 0x10) == 0x10;
		Value[29] = (Buffer[3] & 0x20) == 0x20;
		Value[30] = (Buffer[3] & 0x40) == 0x40;
		Value[31] = (Buffer[3] & 0x80) == 0x80;
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >(Value)));
}

std::unique_ptr< Results::Result > Get_LittleEndian_32Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	std::uint32_t Value{0ul};
	
	if(Length >= 4ull)
	{
		Success = true;
		Index = 4ull;
		Value = static_cast< std::uint32_t >(Buffer[0]) + (static_cast< std::uint32_t >(Buffer[1]) << 8) + (static_cast< std::uint32_t >(Buffer[2]) << 16) + (static_cast< std::uint32_t >(Buffer[3]) << 24);
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >(Value)));
}

std::unique_ptr< Results::Result > Get_RIFF_Chunk(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	auto ChunkHeaderResult{Get_RIFF_ChunkHeader(Buffer + Index, Length - Index)};
		
	if(ChunkHeaderResult->GetSuccess() == true)
	{
		Index += ChunkHeaderResult->GetLength();
		Values->Append(ChunkHeaderResult->GetValue("ChunkIdentifier"));
		Values->Append(ChunkHeaderResult->GetValue("ChunkSize"));
		
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
			if((ChunkDataResult) && (ChunkDataResult->GetSuccess() == true))
			{
				Index += ChunkDataResult->GetLength();
				Values->Append("ChunkData", ChunkDataResult->GetValue());
			}
			else
			{
				Index += ChunkSize;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

std::unique_ptr< Results::Result > Get_RIFF_ChunkHeader(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	
	if(Length - Index >= 4ull)
	{
		auto ChunkIdentifierResult{Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(Buffer + Index, 4ull)};
		
		if(ChunkIdentifierResult->GetSuccess() == true)
		{
			Index += ChunkIdentifierResult->GetLength();
			Values->Append("ChunkIdentifier", ChunkIdentifierResult->GetValue());
			
			auto ChunkSizeResult{Get_LittleEndian_32Bit_UnsignedInteger(Buffer + Index, Length - Index)};
			
			if(ChunkSizeResult->GetSuccess() == true)
			{
				Index += ChunkSizeResult->GetLength();
				Values->Append("ChunkSize", ChunkSizeResult->GetValue());
				Success = true;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

std::unique_ptr< Results::Result > Get_RIFF_fact_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	auto SampleLengthResult{Get_LittleEndian_32Bit_UnsignedInteger(Buffer + Index, Length - Index)};
		
	if(SampleLengthResult->GetSuccess() == true)
	{
		Index += SampleLengthResult->GetLength();
		Values->Append("SampleLength", SampleLengthResult->GetValue());
		Success = true;
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

std::unique_ptr< Results::Result > Get_RIFF_fmt_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	
	if(Length >= 16)
	{
		auto FormatTagResult{Get_FormatTag(Buffer + Index, Length - Index)};
		
		if(FormatTagResult->GetSuccess() == true)
		{
			Index += FormatTagResult->GetLength();
			Values->Append("FormatTag", FormatTagResult->GetValue());
			
			auto NumberOfChannelsResult{Get_LittleEndian_16Bit_UnsignedInteger(Buffer + Index, Length - Index)};
			
			if(NumberOfChannelsResult->GetSuccess() == true)
			{
				Index += NumberOfChannelsResult->GetLength();
				Values->Append("NumberOfChannels", NumberOfChannelsResult->GetValue());
				
				auto SamplesPerSecondResult{Get_LittleEndian_32Bit_UnsignedInteger(Buffer + Index, Length - Index)};
				
				if(SamplesPerSecondResult->GetSuccess() == true)
				{
					Index += SamplesPerSecondResult->GetLength();
					Values->Append("SamplesPerSecond", SamplesPerSecondResult->GetValue());
					
					auto AverageBytesPerSecondResult{Get_LittleEndian_32Bit_UnsignedInteger(Buffer + Index, Length - Index)};
					
					if(AverageBytesPerSecondResult->GetSuccess() == true)
					{
						Index += AverageBytesPerSecondResult->GetLength();
						Values->Append("AverageBytesPerSecond", AverageBytesPerSecondResult->GetValue());
						
						auto BlockAlignResult{Get_LittleEndian_16Bit_UnsignedInteger(Buffer + Index, Length - Index)};
						
						if(BlockAlignResult->GetSuccess() == true)
						{
							Index += BlockAlignResult->GetLength();
							Values->Append("BlockAlign", BlockAlignResult->GetValue());
							
							auto BitsPerSampleResult{Get_LittleEndian_16Bit_UnsignedInteger(Buffer + Index, Length - Index)};
							
							if(BitsPerSampleResult->GetSuccess() == true)
							{
								Index += BitsPerSampleResult->GetLength();
								Values->Append("BitsPerSample", BitsPerSampleResult->GetValue());
								
								if(Length - Index >= 18)
								{
									auto ExtensionSizeResult{Get_LittleEndian_16Bit_UnsignedInteger(Buffer + Index, Length - Index)};
									
									if(ExtensionSizeResult->GetSuccess() == true)
									{
										Index += ExtensionSizeResult->GetLength();
										Values->Append("ExtensionSize", ExtensionSizeResult->GetValue());
										
										auto ExtensionSize{std::experimental::any_cast< std::uint16_t >(ExtensionSizeResult->GetAny())};
										
										if((ExtensionSize == 0) || (ExtensionSize == 22))
										{
											Success = Length - Index == ExtensionSize;
											
											auto ValidBitsPerSampleResult{Get_LittleEndian_16Bit_UnsignedInteger(Buffer + Index, Length - Index)};
											
											if(ValidBitsPerSampleResult->GetSuccess() == true)
											{
												Index += ValidBitsPerSampleResult->GetLength();
												Values->Append("ValidBitsPerSample", ValidBitsPerSampleResult->GetValue());
												
												auto ChannelMaskResult{Get_LittleEndian_32Bit_BitSet(Buffer + Index, Length - Index)};
												
												if(ChannelMaskResult->GetSuccess() == true)
												{
													Index += ChannelMaskResult->GetLength();
													Values->Append("ChannelMask", ChannelMaskResult->GetValue());
													assert(Length - Index == 16);
													
													auto SubFormatResult{Get_HexadecimalString_TerminatedByLength(Buffer + Index, Length - Index)};
													
													if(SubFormatResult->GetSuccess() == true)
													{
														Index += SubFormatResult->GetLength();
														Values->Append("SubFormat", SubFormatResult->GetValue());
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
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

std::unique_ptr< Results::Result > Get_RIFF_RIFF_ChunkData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	
	if(Length - Index >= 4ull)
	{
		auto FormTypeResult{Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(Buffer + Index, 4ull)};
		
		if(FormTypeResult->GetSuccess() == true)
		{
			Index += FormTypeResult->GetLength();
			Values->Append("FormType", FormTypeResult->GetValue());
			
			auto Chunks{std::make_shared< Results::Values >("Chunks")};
			
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
				Values->Append(Chunks);
			}
			Success = true;
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

void PrintValue(const std::string & Indentation, std::shared_ptr< Results::ValueBase > ValueBase)
{
	auto Value{std::dynamic_pointer_cast< Results::Value >(ValueBase)};
	
	if(Value != nullptr)
	{
		std::cout << Indentation << Value->GetName() << ": " << Value->GetAny() << std::endl;
	}
	else
	{
		auto Values{std::dynamic_pointer_cast< Results::Values >(ValueBase)};
		
		if(Values != nullptr)
		{
			if(Values->GetName().empty() == false)
			{
				std::cout << Indentation << Values->GetName() << ":" << std::endl;
			}
			for(auto & SubValue : Values->GetValues())
			{
				PrintValue(Indentation + "    ", SubValue);
			}
		}
		else
		{
			throw new std::exception();
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
