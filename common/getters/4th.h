#ifndef COMMON_GETTERS_4TH_H
#define COMMON_GETTERS_4TH_H

#include <memory>

#include "../results.h"

std::unique_ptr< Results::Result > Get_ASCII_AlphaCharacter(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASCII_AlphaStringTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_GUID_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_32Bit_UnsignedInteger_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_64Bit_UnsignedInteger_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_8Bit_UnsignedInteger(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_8Bit_UnsignedInteger_BufferTerminatedByLength(const std::uint8_t * Buffer, std::uint64_t Length);

#endif
