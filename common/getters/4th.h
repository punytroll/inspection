#ifndef COMMON_GETTERS_4TH_H
#define COMMON_GETTERS_4TH_H

#include <memory>

#include "../results.h"

std::unique_ptr< Results::Result > Get_ASCII_AlphaCharacter(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASCII_AlphaString_EndedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_BitSet_32Bit_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_Buffer_UnsignedInteger_8Bit_EndedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_UnsignedInteger_16Bit_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_UnsignedInteger_32Bit_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_UnsignedInteger_64Bit_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_UnsignedInteger_8Bit(const std::uint8_t * Buffer, std::uint64_t Length);

#endif
