#ifndef HELPER_H
#define HELPER_H

#include <cstdint>
#include <string>

namespace Inspection
{
	bool Is_ASCII_Character_Alphabetical(std::uint8_t Character);
	bool Is_ASCII_Character_Printable(std::uint8_t Character);
	std::uint8_t Get_UnsignedInteger_8Bit_FromHexadecimalDigit(char HexadecimalDigit);
	std::uint8_t Get_UnsignedInteger_8Bit_FromHexadecimalString(const std::string & HexadecimalString);
	std::uint16_t Get_UnsignedInteger_16Bit_FromHexadecimalString(const std::string & HexadecimalString);
	std::uint32_t Get_UnsignedInteger_32Bit_FromHexadecimalString(const std::string & HexadecimalString);
	std::string Get_UTF8_Character_FromUnicodeCodePoint(std::uint32_t CodePoint);
}

#endif
