#include "helper.h"

bool Inspection::Is_ASCII_Character_Alphabetical(std::uint8_t Character)
{
	return ((Character >= 0x41) && (Character < 0x5b)) || ((Character >= 0x61) && (Character < 0x7b));
}

bool Inspection::Is_ASCII_Character_Printable(std::uint8_t Character)
{
	return (Character >= 0x20) && (Character < 0x7f);
}
