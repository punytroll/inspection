#ifndef COMMON_GETTERS_4TH_H
#define COMMON_GETTERS_4TH_H

#include <memory>

#include "../results.h"

std::unique_ptr< Results::Result > Get_GUID_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_UnsignedInteger_64Bit_LittleEndian(const std::uint8_t * Buffer, std::uint64_t Length);

#endif
