#include <cassert>
#include <sstream>

#include "helper.h"

bool Inspection::Is_ASCII_Character_Alphabetical(std::uint8_t Character)
{
	return ((Character >= 0x41) && (Character < 0x5b)) || ((Character >= 0x61) && (Character < 0x7b));
}

bool Inspection::Is_ASCII_Character_Alphabetical_LowerCase(std::uint8_t Character)
{
	return (Character >= 0x61) && (Character < 0x7b);
}

bool Inspection::Is_ASCII_Character_Alphabetical_UpperCase(std::uint8_t Character)
{
	return (Character >= 0x41) && (Character < 0x5b);
}

bool Inspection::Is_ASCII_Character_DecimalDigit(std::uint8_t Character)
{
	return (Character >= 0x30) && (Character < 0x3a);
}

bool Inspection::Is_ASCII_Character_HexadecimalDigit(std::uint8_t Character)
{
	return ((Character >= 0x30) && (Character < 0x3a)) || ((Character >= 0x41) && (Character < 0x47)) || ((Character >= 0x61) && (Character < 0x67));
}

bool Inspection::Is_ASCII_Character_Printable(std::uint8_t Character)
{
	return (Character >= 0x20) && (Character < 0x7f);
}

bool Inspection::Is_ASCII_Character_Space(std::uint8_t Character)
{
	return Character == 0x20;
}

bool Inspection::Is_ISO_IEC_8859_1_Character(std::uint8_t Character)
{
	return ((Character >= 0x20) && (Character < 0x7f)) || (Character >= 0x10);
}

std::uint8_t Inspection::Get_UnsignedInteger_8Bit_FromHexadecimalDigit(char Character)
{
	if(Character == '0')
	{
		return 0x00;
	}
	else if(Character == '1')
	{
		return 0x01;
	}
	else if(Character == '2')
	{
		return 0x02;
	}
	else if(Character == '3')
	{
		return 0x03;
	}
	else if(Character == '4')
	{
		return 0x04;
	}
	else if(Character == '5')
	{
		return 0x05;
	}
	else if(Character == '6')
	{
		return 0x06;
	}
	else if(Character == '7')
	{
		return 0x07;
	}
	else if(Character == '8')
	{
		return 0x08;
	}
	else if(Character == '9')
	{
		return 0x09;
	}
	else if((Character == 'a') || (Character == 'A'))
	{
		return 0x0a;
	}
	else if((Character == 'b') || (Character == 'B'))
	{
		return 0x0b;
	}
	else if((Character == 'c') || (Character == 'C'))
	{
		return 0x0c;
	}
	else if((Character == 'd') || (Character == 'D'))
	{
		return 0x0d;
	}
	else if((Character == 'e') || (Character == 'E'))
	{
		return 0x0e;
	}
	else if((Character == 'f') || (Character == 'F'))
	{
		return 0x0f;
	}
	else
	{
		throw std::invalid_argument("The given character is not a hexadecimal digit.");
	}
}

std::uint8_t Inspection::Get_UnsignedInteger_8Bit_FromHexadecimalString(const std::string & HexadecimalString)
{
	if(HexadecimalString.length() <= 2)
	{
		try
		{
			std::uint8_t Result{0};
			auto Index{0ul};
			
			while(Index < HexadecimalString.length())
			{
				Result = Result * 16 + Get_UnsignedInteger_8Bit_FromHexadecimalDigit(HexadecimalString[Index]);
				++Index;
			}
			
			return Result;
		}
		catch(std::invalid_argument & Exception)
		{
			throw std::invalid_argument("The given hexadecimal string contains non hexadecimal digits.");
		}
	}
	else
	{
		throw std::invalid_argument("The given hexadecimal string was too long.");
	}
}

std::uint16_t Inspection::Get_UnsignedInteger_16Bit_FromHexadecimalString(const std::string & HexadecimalString)
{
	if(HexadecimalString.length() <= 4)
	{
		try
		{
			std::uint16_t Result{0};
			auto Index{0ul};
			
			while(Index < HexadecimalString.length())
			{
				Result = Result * 16 + Get_UnsignedInteger_8Bit_FromHexadecimalDigit(HexadecimalString[Index]);
				++Index;
			}
			
			return Result;
		}
		catch(std::invalid_argument & Exception)
		{
			throw std::invalid_argument("The given hexadecimal string contains non hexadecimal digits.");
		}
	}
	else
	{
		throw std::invalid_argument("The given hexadecimal string was too long.");
	}
}

std::uint32_t Inspection::Get_UnsignedInteger_32Bit_FromHexadecimalString(const std::string & HexadecimalString)
{
	if(HexadecimalString.length() <= 8)
	{
		try
		{
			std::uint32_t Result{0ul};
			auto Index{0ul};
			
			while(Index < HexadecimalString.length())
			{
				Result = Result * 16 + Get_UnsignedInteger_8Bit_FromHexadecimalDigit(HexadecimalString[Index]);
				++Index;
			}
			
			return Result;
		}
		catch(std::invalid_argument & Exception)
		{
			throw std::invalid_argument("The given hexadecimal string contains non hexadecimal digits.");
		}
	}
	else
	{
		throw std::invalid_argument("The given hexadecimal string was too long.");
	}
}

std::string Inspection::Get_UTF8_Character_FromUnicodeCodePoint(std::uint32_t CodePoint)
{
	std::stringstream Result;
	
	if(CodePoint < 0x00000080)
	{
		Result << static_cast< char >(CodePoint & 0x0000007f);
	}
	else if(CodePoint < 0x00000800)
	{
		Result << static_cast< char >(0x00000c0 + ((CodePoint & 0x00000700) >> 6) + ((CodePoint & 0x000000c0) >> 6));
		Result << static_cast< char >(0x0000080 + (CodePoint & 0x0000003f));
	}
	else if(CodePoint < 0x00010000)
	{
		Result << static_cast< char >(0x00000e0 + ((CodePoint & 0x0000f000) >> 12));
		Result << static_cast< char >(0x0000080 + ((CodePoint & 0x00000f00) >> 6) + ((CodePoint & 0x000000c0) >> 6));
		Result << static_cast< char >(0x0000080 + (CodePoint & 0x0000003f));
	}
	else if(CodePoint < 0x00110000)
	{
		Result << static_cast< char >(0x00000f0 + ((CodePoint & 0x001c0000) >> 18));
		Result << static_cast< char >(0x0000080 + ((CodePoint & 0x00030000) >> 12) + ((CodePoint & 0x0000f000) >> 12));
		Result << static_cast< char >(0x0000080 + ((CodePoint & 0x00000f00) >> 6) + ((CodePoint & 0x000000c0) >> 6));
		Result << static_cast< char >(0x0000080 + (CodePoint & 0x0000003f));
	}
	else
	{
		assert(false);
	}
	
	return Result.str();
}
