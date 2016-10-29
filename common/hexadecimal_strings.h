#ifndef COMMON_HEXADECIMAL_STRINGS_H
#define COMMON_HEXADECIMAL_STRINGS_H

#include <cstdint>
#include <stdexcept>
#include <string>

std::uint16_t Get16BitUnsignedIntegerFromHexadecimalString(const std::string & HexadecimalString);
std::uint32_t Get32BitUnsignedIntegerFromHexadecimalString(const std::string & HexadecimalString);
std::uint8_t Get8BitUnsignedIntegerFromHexadecimalString(const std::string & HexadecimalString);
std::uint8_t Get8BitUnsignedIntegerFromHexadecimalDigit(char HexadecimalDigit);

#endif
