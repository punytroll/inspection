#include <cstring>

#include "../guid.h"
#include "4th.h"

std::unique_ptr< Results::Result > Get_ASCII_AlphaCharacter(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{'\0'};
	
	if(Length >= 1ull)
	{
		if(((Buffer[0] > 0x40) && (Buffer[0] < 0x5B)) || ((Buffer[0] > 0x60) && (Buffer[0] < 0x7B)))
		{
			Success = true;
			Index += 1ull;
			Value = Buffer[0];
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >(Value)));
}

std::unique_ptr< Results::Result > Get_ASCII_AlphaStringTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{true};
	auto Index{0ull};
	std::stringstream StringStream;
	
	while(Index < Length)
	{
		auto ASCII_AlphaCharacterResult{Get_ASCII_AlphaCharacter(Buffer + Index, Length - Index)};
		
		if(ASCII_AlphaCharacterResult->GetSuccess() == true)
		{
			Index += ASCII_AlphaCharacterResult->GetLength();
			StringStream << std::experimental::any_cast< char >(ASCII_AlphaCharacterResult->GetAny());
		}
		else
		{
			Success = false;
			
			break;
		}
	}
	
	return std::unique_ptr< Results::Result >(new Results::Result(Success, Index, std::make_shared< Results::Value >("", StringStream.str())));
}

std::unique_ptr< Results::Result > Get_GUID_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{GUID()};
	
	if(Length >= 16ull)
	{
		Success = true;
		Value.Data1 = static_cast< std::uint32_t >(Buffer[Index + 0]) + (static_cast< std::uint32_t >(Buffer[Index + 1]) << 8) + (static_cast< std::uint32_t >(Buffer[Index + 2]) << 16) + (static_cast< std::uint32_t >(Buffer[Index + 3]) << 24);
		Value.Data2 = static_cast< std::uint32_t >(Buffer[Index + 4]) + (static_cast< std::uint32_t >(Buffer[Index + 5]) << 8);
		Value.Data3 = static_cast< std::uint32_t >(Buffer[Index + 6]) + (static_cast< std::uint32_t >(Buffer[Index + 7]) << 8);
		Value.Data4[0] = Buffer[Index + 8];
		Value.Data4[1] = Buffer[Index + 9];
		Value.Data4[2] = Buffer[Index + 10];
		Value.Data4[3] = Buffer[Index + 11];
		Value.Data4[4] = Buffer[Index + 12];
		Value.Data4[5] = Buffer[Index + 13];
		Value.Data4[6] = Buffer[Index + 14];
		Value.Data4[7] = Buffer[Index + 15];
		Index += 16ull;
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_UnsignedInteger_64Bit_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	std::uint64_t Value{0ul};
	
	if(Length >= 4ull)
	{
		Success = true;
		Index = 4ull;
		Value = static_cast< std::uint64_t >(Buffer[0]) + (static_cast< std::uint64_t >(Buffer[1]) << 8) + (static_cast< std::uint64_t >(Buffer[2]) << 16) + (static_cast< std::uint64_t >(Buffer[3]) << 24) + (static_cast< std::uint64_t >(Buffer[4]) << 32) + (static_cast< std::uint64_t >(Buffer[5]) << 40) + (static_cast< std::uint64_t >(Buffer[6]) << 48) + (static_cast< std::uint64_t >(Buffer[7]) << 56);
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_UnsignedInteger_8Bit_BufferTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	std::vector< std::uint8_t > Value(Buffer, Buffer + Length);
	
	Success = true;
	Index = Length;
	
	return Results::MakeResult(Success, Index, Value);
}
