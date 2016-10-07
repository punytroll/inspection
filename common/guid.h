#ifndef COMMON_GUID_H
#define COMMON_GUID_H

#include <cstdint>

class GUID
{
public:
	std::uint32_t Data1;
	std::uint16_t Data2;
	std::uint16_t Data3;
	std::uint8_t Data4[8];
};

#endif
