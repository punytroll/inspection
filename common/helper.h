#ifndef HELPER_H
#define HELPER_H

#include <cstdint>
#include <string>

namespace Inspection
{
	bool Is_ASCII_Character_Alphabetical(std::uint8_t Character);
	bool Is_ASCII_Character_Alphabetical_LowerCase(std::uint8_t Character);
	bool Is_ASCII_Character_Alphabetical_UpperCase(std::uint8_t Character);
	bool Is_ASCII_Character_DecimalDigit(std::uint8_t Character);
	bool Is_ASCII_Character_HexadecimalDigit(std::uint8_t Character);
	bool Is_ASCII_Character_Printable(std::uint8_t Character);
	bool Is_ASCII_Character_Space(std::uint8_t Character);
	bool Is_ISO_IEC_8859_1_1998_Character(std::uint8_t Character);
	std::uint8_t Get_UnsignedInteger_8Bit_FromHexadecimalDigit(char HexadecimalDigit);
	std::uint8_t Get_UnsignedInteger_8Bit_FromHexadecimalString(const std::string & HexadecimalString);
	std::uint16_t Get_UnsignedInteger_16Bit_FromHexadecimalString(const std::string & HexadecimalString);
	std::uint32_t Get_UnsignedInteger_32Bit_FromHexadecimalString(const std::string & HexadecimalString);
	std::string Get_UTF_8_Character_FromUnicodeCodePoint(std::uint32_t CodePoint);
}

#endif
