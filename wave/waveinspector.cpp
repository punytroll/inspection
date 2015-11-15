#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <deque>
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
std::unique_ptr< Results::Result > Get_LittleEndian_32Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_Chunk(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_Chunk_Header(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_fact_Chunk_Data(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_RIFF_RIFF_Chunk_Data(const std::uint8_t * Buffer, std::uint64_t Length);

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
	auto ChunkHeaderResult{Get_RIFF_Chunk_Header(Buffer + Index, Length - Index)};
		
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
				ChunkDataResult = Get_RIFF_RIFF_Chunk_Data(Buffer + Index, Length - Index);
			}
			else if(ChunkIdentifier == "fact")
			{
				ChunkDataResult = Get_RIFF_fact_Chunk_Data(Buffer + Index, Length - Index);
			}
			if((ChunkDataResult) && (ChunkDataResult->GetSuccess() == true))
			{
				Index += ChunkDataResult->GetLength();
				Values->Append("Data", ChunkDataResult->GetValue());
			}
			else
			{
				Index += ChunkSize;
			}
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, Values));
}

std::unique_ptr< Results::Result > Get_RIFF_Chunk_Header(const std::uint8_t * Buffer, std::uint64_t Length)
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

std::unique_ptr< Results::Result > Get_RIFF_fact_Chunk_Data(const std::uint8_t * Buffer, std::uint64_t Length)
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

std::unique_ptr< Results::Result > Get_RIFF_RIFF_Chunk_Data(const std::uint8_t * Buffer, std::uint64_t Length)
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
						std::cout << "Chunk identifier: \"" << RIFFChunkResult->GetAny("ChunkIdentifier") << '"' << std::endl;
						std::cout << "Chunk size: " << RIFFChunkResult->GetAny("ChunkSize") << std::endl;
						std::cout << "Chunk data:" << std::endl;
						std::cout << "    Form type: " << RIFFChunkResult->GetValue("Data")->GetAny("FormType") << std::endl;
						std::cout << "    Sub chunks:" << std::endl;
						for(auto & ChunkValue : RIFFChunkResult->GetValue("Data")->GetValue("Chunks")->GetValues())
						{
							auto ChunkValues{std::dynamic_pointer_cast< Results::Values >(ChunkValue)};
							
							std::cout << "        Chunk identifier: \"" << ChunkValues->GetAny("ChunkIdentifier") << '"' << std::endl;
							std::cout << "        Chunk size: " << ChunkValues->GetAny("ChunkSize") << std::endl;
							if(ChunkValues->Has("Data") == true)
							{
								std::cout << "        Chunk data:" << std::endl;
								for(auto & ChunkDataValue : ChunkValues->GetValue("Data")->GetValues())
								{
									std::cout << "            " << ChunkDataValue->GetName() << ": " << ChunkDataValue->GetAny() << std::endl;
								}
							}
						}
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
