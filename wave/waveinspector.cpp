#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <deque>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/file_handling.h"
#include "../common/results.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 4th generation getters                                                                        //
//   - These functions validate and extract in one go.                                           //
//   - They have one return value:                                                               //
//       - a shared_ptr to an instance of type ResultBase,                                       //
//         which may be either a Result or Results instance                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::shared_ptr< Results::Result > Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::shared_ptr< Results::Result > Get_ASCII_AlphaNumericOrSpaceCharacter(const std::uint8_t * Buffer, std::uint64_t Length);
std::shared_ptr< Results::Result > Get_LittleEndian_32Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length);
std::shared_ptr< Results::Result > Get_RIFF_Chunk_Header(const std::uint8_t * Buffer, std::uint64_t Length);
std::shared_ptr< Results::Result > Get_RIFF_Form_Header(const std::uint8_t * Buffer, std::uint64_t Length);

std::shared_ptr< Results::Result > Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length)
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
			StringStream << std::experimental::any_cast< char >(ASCII_AlphaNumericOrSpaceCharacterResult->Get());
		}
		else
		{
			Success = false;
			
			break;
		}
	}
	
	return std::make_shared< Results::Result >(Success, Index, std::make_shared< Results::Value >("", StringStream.str()));
}

std::shared_ptr< Results::Result > Get_ASCII_AlphaNumericOrSpaceCharacter(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{'\0'};
	
	if(Length >= 1ull)
	{
		if(((Buffer[0] > 0x2F) && (Buffer[0] < 0x3A)) || ((Buffer[0] > 0x40) && (Buffer[0] < 0x5B)) || ((Buffer[0] > 0x60) && (Buffer[0] < 0x7B)))
		{
			Success = true;
			Index += 1ull;
			Value = Buffer[0];
		}
	}
	
	return std::make_shared< Results::Result >(Success, Index, std::make_shared< Results::Value >(Value));
}

std::shared_ptr< Results::Result > Get_LittleEndian_32Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length)
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
	
	return std::make_shared< Results::Result >(Success, Index, std::make_shared< Results::Value >(Value));
}

std::shared_ptr< Results::Result > Get_RIFF_Chunk_Header(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Values{std::make_shared< Results::Values >()};
	
	if(Length - Index >= 4ull)
	{
		auto ChunkIdentifierResult{Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(Buffer + Index, 4ull)};
		
		if(ChunkIdentifierResult->GetSuccess() == true)
		{
			if(std::experimental::any_cast< std::string >(ChunkIdentifierResult->Get()) == "RIFF")
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
	}
	
	return std::make_shared< Results::Result >(Success, Index, Values);
}

std::shared_ptr< Results::Result > Get_RIFF_Form_Header(const std::uint8_t * Buffer, std::uint64_t Length)
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
		
		if(Length - Index >= 4ull)
		{
			auto FormTypeResult{Get_ASCII_AlphaNumericStringWithSpaceTerminatedByLength(Buffer + Index, 4ull)};
			
			if(FormTypeResult->GetSuccess() == true)
			{
				Index += FormTypeResult->GetLength();
				Values->Append("FormType", FormTypeResult->GetValue());
				Success = true;
			}
		}
	}
	
	return std::make_shared< Results::Result >(Success, Index, Values);
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
					auto RIFFFormHeaderResult(Get_RIFF_Form_Header(Address + Index, FileSize - Index));
					
					if(RIFFFormHeaderResult->GetSuccess() == true)
					{
						Index += RIFFFormHeaderResult->GetLength();
						std::cout << "Chunk identifier: " << std::experimental::any_cast< std::string >(RIFFFormHeaderResult->Get("ChunkIdentifier")) << std::endl;
						std::cout << "Chunk size: " << std::experimental::any_cast< std::uint32_t >(RIFFFormHeaderResult->Get("ChunkSize")) << std::endl;
						std::cout << "Chunk format: " << std::experimental::any_cast< std::string >(RIFFFormHeaderResult->Get("FormType")) << std::endl;
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
