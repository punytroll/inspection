#include <cassert>

#include "unicode.h"

std::string Get_UTF8_CharacterFromUnicodeCodePoint(std::uint32_t CodePoint)
{
	std::string Result;
	
	if(CodePoint < 0x00000080)
	{
		Result += static_cast< char >(CodePoint & 0x0000007f);
	}
	else if(CodePoint < 0x00000800)
	{
		Result += static_cast< char >(0x00000c0 + ((CodePoint & 0x00000700) >> 6) + ((CodePoint & 0x000000c0) >> 6));
		Result += static_cast< char >(0x0000080 + (CodePoint & 0x0000003f));
	}
	else if(CodePoint < 0x00010000)
	{
		Result += static_cast< char >(0x00000e0 + ((CodePoint & 0x0000f000) >> 12));
		Result += static_cast< char >(0x0000080 + ((CodePoint & 0x00000f00) >> 6) + ((CodePoint & 0x000000c0) >> 6));
		Result += static_cast< char >(0x0000080 + (CodePoint & 0x0000003f));
	}
	else if(CodePoint < 0x00110000)
	{
		Result += static_cast< char >(0x00000f0 + ((CodePoint & 0x001c0000) >> 18));
		Result += static_cast< char >(0x0000080 + ((CodePoint & 0x00030000) >> 12) + ((CodePoint & 0x0000f000) >> 12));
		Result += static_cast< char >(0x0000080 + ((CodePoint & 0x00000f00) >> 6) + ((CodePoint & 0x000000c0) >> 6));
		Result += static_cast< char >(0x0000080 + (CodePoint & 0x0000003f));
	}
	else
	{
		assert(false);
	}
	
	return Result;
}
