#include "hexadecimal_strings.h"

std::uint16_t Get16BitUnsignedIntegerFromHexadecimalString(const std::string & HexadecimalString)
{
	if(HexadecimalString.length() <= 4)
	{
		try
		{
			auto Result{0ul};
			auto Index{0ul};
			
			while(Index < HexadecimalString.length())
			{
				Result = Result * 16 + Get8BitUnsignedIntegerFromHexadecimalDigit(HexadecimalString[Index]);
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

std::uint32_t Get32BitUnsignedIntegerFromHexadecimalString(const std::string & HexadecimalString)
{
	if(HexadecimalString.length() <= 8)
	{
		try
		{
			auto Result{0ul};
			auto Index{0ul};
			
			while(Index < HexadecimalString.length())
			{
				Result = Result * 16 + Get8BitUnsignedIntegerFromHexadecimalDigit(HexadecimalString[Index]);
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

std::uint8_t Get8BitUnsignedIntegerFromHexadecimalString(const std::string & HexadecimalString)
{
	if(HexadecimalString.length() <= 2)
	{
		try
		{
			auto Result{0ul};
			auto Index{0ul};
			
			while(Index < HexadecimalString.length())
			{
				Result = Result * 16 + Get8BitUnsignedIntegerFromHexadecimalDigit(HexadecimalString[Index]);
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

std::uint8_t Get8BitUnsignedIntegerFromHexadecimalDigit(char Character)
{
	if(Character == '0')
	{
		return 0;
	}
	else if(Character == '1')
	{
		return 1;
	}
	else if(Character == '2')
	{
		return 2;
	}
	else if(Character == '3')
	{
		return 3;
	}
	else if(Character == '4')
	{
		return 4;
	}
	else if(Character == '5')
	{
		return 5;
	}
	else if(Character == '6')
	{
		return 6;
	}
	else if(Character == '7')
	{
		return 7;
	}
	else if(Character == '8')
	{
		return 8;
	}
	else if(Character == '9')
	{
		return 9;
	}
	else if((Character == 'a') || (Character == 'A'))
	{
		return 10;
	}
	else if((Character == 'b') || (Character == 'B'))
	{
		return 11;
	}
	else if((Character == 'c') || (Character == 'C'))
	{
		return 12;
	}
	else if((Character == 'd') || (Character == 'D'))
	{
		return 13;
	}
	else if((Character == 'e') || (Character == 'E'))
	{
		return 14;
	}
	else if((Character == 'f') || (Character == 'F'))
	{
		return 15;
	}
	else
	{
		throw std::invalid_argument("The given character is not a hexadecimal digit.");
	}
}
