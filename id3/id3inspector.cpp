#include <string.h>

#include <algorithm>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <sstream>
#include <vector>

#include "../common/5th.h"
#include "../common/file_handling.h"
#include "../common/helper.h"
#include "../common/values.h"

using namespace std::string_literals;

enum class TextEncoding
{
	Undefined,
	ISO_IEC_8859_1_1998,
	UCS_2
};

enum class UCS2ByteOrderMark
{
	Undefined,
	LittleEndian,
	BigEndian
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// global variables and configuration                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::map< unsigned int, std::string > g_NumericGenresID3_1;
std::map< unsigned int, std::string > g_NumericGenresWinamp;
std::map< std::string, std::string > g_ISO_3166_1_Alpha_2_Codes;
std::map< TextEncoding, std::string > g_EncodingNames;
std::map< std::string, std::string > g_GUIDDescriptions;
std::map< std::string, std::function< std::uint64_t (const uint8_t *, std::uint64_t) > > g_FrameHandlers_2_3;
std::map< std::string, std::string > g_FrameNames_2_3;
bool g_PrintBytes(false);

void AppendSeparated(std::string & String, const std::string & Append, const std::string & Separator)
{
	if(String.empty() == false)
	{
		String += Separator;
	}
	String += Append;
}

std::string GetHexadecimalStringFromUInt8(uint8_t Value)
{
	std::stringstream Stream;
	
	Stream << std::hex << std::setfill('0') << std::setw(2) << std::right << static_cast< unsigned int >(Value);
	
	return Stream.str();
}

std::string GetHexadecimalStringFromUInt8Buffer(const uint8_t * Buffer, int Length)
{
	std::stringstream Result;
	
	Result << std::hex << std::setfill('0');
	for(int Index = 0; Index < Length; ++Index)
	{
		if(Index > 0)
		{
			Result << ' ';
		}
		Result << std::setw(2) << std::right << static_cast< unsigned int >(Buffer[Index]);
	}
	
	return Result.str();
}

std::string GetBinaryStringFromBoolean(bool Value)
{
	if(Value == true)
	{
		return "1";
	}
	else
	{
		return "0";
	}
}

std::string GetBinaryStringFromUInt8(uint8_t Value)
{
	std::string Result;
	
	Result += GetBinaryStringFromBoolean((Value & 0x80) == 0x80);
	Result += GetBinaryStringFromBoolean((Value & 0x40) == 0x40);
	Result += GetBinaryStringFromBoolean((Value & 0x20) == 0x20);
	Result += GetBinaryStringFromBoolean((Value & 0x10) == 0x10);
	Result += GetBinaryStringFromBoolean((Value & 0x08) == 0x08);
	Result += GetBinaryStringFromBoolean((Value & 0x04) == 0x04);
	Result += GetBinaryStringFromBoolean((Value & 0x02) == 0x02);
	Result += GetBinaryStringFromBoolean((Value & 0x01) == 0x01);
	
	return Result;
}

std::string GetBinaryStringFromUInt16(uint16_t Value)
{
	return GetBinaryStringFromUInt8((Value >> 8) & 0xFF) + " " + GetBinaryStringFromUInt8(Value & 0xFF);
}

std::vector< std::string > SplitStringByCharacterPreserveEmpty(const std::string & WMUniqueFileIdentifier, char Separator)
{
	std::vector< std::string > Result;
	std::string::size_type Begin(0);
	std::string::size_type End(0);
	
	while(Begin != std::string::npos)
	{
		End = WMUniqueFileIdentifier.find(Separator, Begin);
		Result.push_back(WMUniqueFileIdentifier.substr(Begin, End - Begin));
		Begin = End + ((End == std::string::npos) ? (0) : (1));
	}
	
	return Result;
}

bool IsValidIdentifierCharacter(char Character)
{
	return ((Character >= 'A') && (Character <= 'Z')) || ((Character >= '0') && (Character <= '9'));
}

std::pair< bool, unsigned int > GetUnsignedIntegerFromDecimalASCIIDigit(uint8_t ASCIIDigit)
{
	std::pair< bool, unsigned int > Result(false, 0);
	
	if((ASCIIDigit >= 0x30) && (ASCIIDigit <= 0x39))
	{
		Result.first = true;
		Result.second = ASCIIDigit - 0x30;
	}
	
	return Result;
}

std::pair< bool, unsigned int > GetUnsignedIntegerFromDecimalASCIIString(const std::string & DecimalASCIIString)
{
	std::pair< bool, unsigned int > Result(true, 0);
	
	for(std::string::const_iterator CharacterIterator = DecimalASCIIString.begin(); CharacterIterator != DecimalASCIIString.end(); ++CharacterIterator)
	{
		std::pair< bool, unsigned int > Digit(GetUnsignedIntegerFromDecimalASCIIDigit(*CharacterIterator));
		
		if(Digit.first == true)
		{
			Result.second = Result.second * 10 + Digit.second;
		}
		else
		{
			Result.first = false;
			Result.second = 0;
			
			break;
		}
	}
	
	return Result;
}

std::pair< bool, std::string > GetSimpleID3_1GenreReferenceInterpretation(const std::string & ContentType)
{
	std::pair< bool, std::string > Result(false, "");
	
	if((ContentType.length() >= 3) && (ContentType[0] == '(') && (ContentType[ContentType.length() - 1] == ')'))
	{
		std::pair< bool, unsigned int > GenreNumber(GetUnsignedIntegerFromDecimalASCIIString(ContentType.substr(1, ContentType.length() - 2)));
		
		if(GenreNumber.first == true)
		{
			std::map< unsigned int, std::string >::iterator NumericGenreID3_1Iterator(g_NumericGenresID3_1.find(GenreNumber.second));
			
			if(NumericGenreID3_1Iterator != g_NumericGenresID3_1.end())
			{
				Result.first = true;
				Result.second = '"' + NumericGenreID3_1Iterator->second + "\" (reference to numeric genre from ID3v1)";
			}
		}
	}
	
	return Result;
}

std::pair< bool, std::string > GetSimpleWinampExtensionGenreReferenceInterpretation(const std::string & ContentType)
{
	std::pair< bool, std::string > Result(false, "");
	
	if((ContentType.length() >= 3) && (ContentType[0] == '(') && (ContentType[ContentType.length() - 1] == ')'))
	{
		std::pair< bool, unsigned int > GenreNumber(GetUnsignedIntegerFromDecimalASCIIString(ContentType.substr(1, ContentType.length() - 2)));
		
		if(GenreNumber.first == true)
		{
			std::map< unsigned int, std::string >::iterator NumericGenreWinampIterator(g_NumericGenresWinamp.find(GenreNumber.second));
			
			if(NumericGenreWinampIterator != g_NumericGenresWinamp.end())
			{
				Result.first = true;
				Result.second = '"' + NumericGenreWinampIterator->second + "\" (reference to numeric genre from Winamp extension)";
			}
		}
	}
	
	return Result;
}

std::pair< bool, std::string > GetContentTypeInterpretation2_3(const std::string & ContentType)
{
	std::pair< bool, std::string > Interpretation(false, "");
	
	Interpretation = GetSimpleID3_1GenreReferenceInterpretation(ContentType);
	if(Interpretation.first == false)
	{
		Interpretation = GetSimpleWinampExtensionGenreReferenceInterpretation(ContentType);
	}
	
	return Interpretation;
}

std::pair< bool, std::string > GetWMUniqueFileIdentifierAsAMGIdentifier(const std::string & WMUniqueFileIdentifier)
{
	std::pair< bool, std::string > Result(false, "");
	std::vector< std::string > Tokens(SplitStringByCharacterPreserveEmpty(WMUniqueFileIdentifier, ';'));
	
	if(Tokens.size() == 3)
	{
		if((Tokens[0].substr(0, 8) == "AMGa_id=") && (Tokens[1].substr(0, 8) == "AMGp_id=") && (Tokens[2].substr(0, 8) == ("AMGt_id=")))
		{
			Result.second += "\t\t\t\t\tAlbum Identifier: \"" + Tokens[0].substr(8) + "\"\n";
			Result.second += "\t\t\t\t\tArtist Identifier: \"" + Tokens[1].substr(8) + "\"\n";
			Result.second += "\t\t\t\t\tTitle Identifier: \"" + Tokens[2].substr(8) + "\"";
			Result.first = true;
		}
	}
	
	return Result;
}

std::pair< bool, std::string > GetWMUniqueFileIdentifierInterpretation(const std::string & WMUniqueFileIdentifier)
{
	std::pair< bool, std::string > Result(false, "");
	
	Result = GetWMUniqueFileIdentifierAsAMGIdentifier(WMUniqueFileIdentifier);
	
	return Result;
}

std::pair< int, std::string > GetHexadecimalStringTerminatedByLength(const uint8_t * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(Result.first < Length)
	{
		if(Result.first > 0)
		{
			Result.second += ' ';
		}
		Result.second += GetHexadecimalStringFromUInt8(*(Buffer + Result.first));
		Result.first += 1;
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 2nd generation getters                                                                        //
//   - These functions validate and extract in one go.                                           //
//   - They have two or three return values:                                                     //
//       - a Boolean value indicating success                                                    //
//       - an Integer value indicating the length of the processed data                          //
//       - if appropriate, the actual result value                                               //
//   - If the Success return value is false, the length and return values may contain bogus data //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_Character(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByLength(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByTermination(const uint8_t * Buffer, int Length);
std::tuple< bool, int > Get_ISO_IEC_8859_1_Termination(const uint8_t * Buffer, int Length);
std::tuple< bool, int, float > Get_ISO_IEC_IEEE_60559_2011_binary32(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint16_t > Get_UInt16_BE(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint16_t > Get_UInt16_LE(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint32_t > Get_UInt32_BE(const uint8_t * Buffer, int Length);

std::tuple< bool, int, char > Get_ISO_IEC_646_1991_CarriageReturnCharacter(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, char > Result(false, 0, '\x00');
	
	if((Length >= 1) && (Buffer[0] == 0x0d))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = '\x0d';
	}
	
	return Result;
}

std::tuple< bool, int, char > Get_ISO_IEC_646_1991_LineFeedCharacter(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, char > Result(false, 0, '\x00');
	
	if((Length >= 1) && (Buffer[0] == 0x0a))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = '\x0a';
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_Character(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if((Length >= 1) && (((Buffer[0] >= 0x20) && (Buffer[0] <= 0x7e)) || (Buffer[0] >= 0xa0)))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(static_cast< std::uint32_t >(Buffer[0]));
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Character(Get_ISO_IEC_8859_1_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Character) == true)
		{
			std::get<1>(Result) += std::get<1>(Character);
			std::get<2>(Result) += std::get<2>(Character);
		}
		else
		{
			std::get<0>(Result) = false;
			
			break;
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringWithCarriageReturnsEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Character(Get_ISO_IEC_8859_1_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Character) == true)
		{
			std::get<1>(Result) += std::get<1>(Character);
			std::get<2>(Result) += std::get<2>(Character);
		}
		else
		{
			auto CarriageReturnCharacter(Get_ISO_IEC_646_1991_CarriageReturnCharacter(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(CarriageReturnCharacter) == true)
			{
				std::get<1>(Result) += std::get<1>(CarriageReturnCharacter);
				std::get<2>(Result) += std::get<2>(CarriageReturnCharacter);
			}
			else
			{
				std::get<0>(Result) = false;
				
				break;
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringWithLineFeedsEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Character(Get_ISO_IEC_8859_1_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Character) == true)
		{
			std::get<1>(Result) += std::get<1>(Character);
			std::get<2>(Result) += std::get<2>(Character);
		}
		else
		{
			auto LineFeedCharacter(Get_ISO_IEC_646_1991_LineFeedCharacter(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(LineFeedCharacter) == true)
			{
				std::get<1>(Result) += std::get<1>(LineFeedCharacter);
				std::get<2>(Result) += std::get<2>(LineFeedCharacter);
			}
			else
			{
				std::get<0>(Result) = false;
				
				break;
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<0>(Result) < Length)
	{
		auto Termination(Get_ISO_IEC_8859_1_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_ISO_IEC_8859_1_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(Character) == true)
			{
				std::get<1>(Result) += std::get<1>(Character);
				std::get<2>(Result) += std::get<2>(Character);
			}
			else
			{
				break;
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringWithCarriageReturnsEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<0>(Result) < Length)
	{
		auto Termination(Get_ISO_IEC_8859_1_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_ISO_IEC_8859_1_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(Character) == true)
			{
				std::get<1>(Result) += std::get<1>(Character);
				std::get<2>(Result) += std::get<2>(Character);
			}
			else
			{
				auto CarriageReturnCharacter(Get_ISO_IEC_646_1991_CarriageReturnCharacter(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
				
				if(std::get<0>(CarriageReturnCharacter) == true)
				{
					std::get<1>(Result) += std::get<1>(CarriageReturnCharacter);
					std::get<2>(Result) += std::get<2>(CarriageReturnCharacter);
				}
				else
				{
					break;
				}
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringWithLineFeedsEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<0>(Result) < Length)
	{
		auto Termination(Get_ISO_IEC_8859_1_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_ISO_IEC_8859_1_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(Character) == true)
			{
				std::get<1>(Result) += std::get<1>(Character);
				std::get<2>(Result) += std::get<2>(Character);
			}
			else
			{
				auto LineFeedCharacter(Get_ISO_IEC_646_1991_LineFeedCharacter(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
				
				if(std::get<0>(LineFeedCharacter) == true)
				{
					std::get<1>(Result) += std::get<1>(LineFeedCharacter);
					std::get<2>(Result) += std::get<2>(LineFeedCharacter);
				}
				else
				{
					break;
				}
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int > Get_ISO_IEC_8859_1_Termination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int > Result(false, 0);
	
	if((Length >= 1) && (Buffer[0] == 0x00))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
	}
	
	return Result;
}

std::tuple< bool, int, float > Get_ISO_IEC_IEEE_60559_2011_binary32(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, float > Result(false, 0, 0.0f);
	
	if(Length >= 4)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 4;
		std::get<2>(Result) = *(reinterpret_cast< const float * >(Buffer));
	}
	
	return Result;
}

std::tuple< bool, int, UCS2ByteOrderMark > Get_UCS_2_ByteOrderMark(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, UCS2ByteOrderMark > Result(false, 0, UCS2ByteOrderMark::Undefined);
	
	if(Length >= 2)
	{
		if((Buffer[0] == 0xfe) && (Buffer[1] == 0xff))
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 2;
			std::get<2>(Result) = UCS2ByteOrderMark::BigEndian;
		}
		else if((Buffer[0] == 0xff) && (Buffer[1] == 0xfe))
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 2;
			std::get<2>(Result) = UCS2ByteOrderMark::LittleEndian;
		}
	}
	
	return Result;
}

std::tuple< bool, int > Get_UCS_2_Termination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int > Result(false, 0);
	
	if((Length >= 2) && (Buffer[0] == 0x00) && (Buffer[1] == 0x00))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 2;
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UCS_2BE_Character(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if((Length >= 2) && ((Buffer[0] < 0xd8) || (Buffer[0] > 0xdf)))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 2;
		std::get<2>(Result) = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(static_cast< std::uint32_t >((static_cast< std::uint32_t >(Buffer[0]) << 8) | static_cast< std::uint32_t >(Buffer[1])));
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Character(Get_UCS_2BE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Character) == true)
		{
			std::get<1>(Result) += std::get<1>(Character);
			std::get<2>(Result) += std::get<2>(Character);
		}
		else
		{
			std::get<0>(Result) = false;
			
			break;
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UCS_2_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_UCS_2BE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(Character) == true)
			{
				std::get<1>(Result) += std::get<1>(Character);
				std::get<2>(Result) += std::get<2>(Character);
			}
			else
			{
				break;
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UCS_2LE_Character(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if((Length >= 2) && ((Buffer[1] < 0xd8) || (Buffer[1] > 0xdf)))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 2;
		std::get<2>(Result) = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(static_cast< std::uint32_t >((static_cast< std::uint32_t >(Buffer[1]) << 8) | static_cast< std::uint32_t >(Buffer[0])));
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Character(Get_UCS_2LE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Character) == true)
		{
			std::get<1>(Result) += std::get<1>(Character);
			std::get<2>(Result) += std::get<2>(Character);
		}
		else
		{
			std::get<0>(Result) = false;
			
			break;
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UCS_2_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_UCS_2LE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(Character) == true)
			{
				std::get<1>(Result) += std::get<1>(Character);
				std::get<2>(Result) += std::get<2>(Character);
			}
			else
			{
				break;
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, uint16_t > Get_UInt16_BE(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, uint16_t > Result(false, 0, 0);
	
	if(Length >= 2)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 2;
		std::get<2>(Result) = (static_cast< uint32_t >(Buffer[0]) << 8) + Buffer[1];
	}
	
	return Result;
}

std::tuple< bool, int, uint16_t > Get_UInt16_LE(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, uint16_t > Result(false, 0, 0);
	
	if(Length >= 2)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 2;
		std::get<2>(Result) = (static_cast< uint32_t >(Buffer[1]) << 8) + Buffer[0];
	}
	
	return Result;
}

std::tuple< bool, int, uint32_t > Get_UInt32_BE(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, uint32_t > Result(false, 0, 0);
	
	if(Length >= 4)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 4;
		std::get<2>(Result) = (static_cast< uint32_t >(Buffer[0]) << 24) + (static_cast< uint32_t >(Buffer[1]) << 16) + (static_cast< uint32_t >(Buffer[2]) << 8) + Buffer[3];
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 3rd generation getters                                                                        //
//   - These functions validate and extract in one go.                                           //
//   - They have three return values:                                                            //
//       - a Boolean value indicating success (type bool)                                        //
//       - an Integer value indicating the length of the processed data (type std::uint64_t)     //
//       - a Values object with results (type Values)                                            //
//   - If the Success return value is false, the length and return values may contain bogus data //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::tuple< bool, std::uint64_t, Values > Get_GUID_String(const std::uint8_t * Buffer, std::uint64_t Length);
std::tuple< bool, std::uint64_t, Values > Get_ID3_2_3_Encoding(const std::uint8_t * Buffer, std::uint64_t Length);

std::tuple< bool, std::uint64_t, Values > Get_GUID_String(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	Values Result;
	
	if(Length >= 16)
	{
		Success = true;
		
		std::stringstream StringStream;
		
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << '-';
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << '-';
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << '-';
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << '-';
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		StringStream << GetHexadecimalStringFromUInt8(Buffer[Index++]);
		Result.Add("Result", StringStream.str());
		
		auto DescriptionIterator{g_GUIDDescriptions.find(StringStream.str())};
		
		if(DescriptionIterator != g_GUIDDescriptions.end())
		{
			Result.Add("GUIDDescription", DescriptionIterator->second);
		}
	}
	
	return std::make_tuple(Success, Index, Result);
}

std::tuple< bool, std::uint64_t, Values > Get_ID3_2_3_Encoding(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	Values Result;
	
	if(Length >= 1)
	{
		if(Buffer[0] == 0x00)
		{
			Success = true;
			Index += 1;
			Result.Add("Result", TextEncoding::ISO_IEC_8859_1_1998);
		}
		else if(Buffer[0] == 0x01)
		{
			Success = true;
			Index += 1;
			Result.Add("Result", TextEncoding::UCS_2);
		}
	}
	if(Result.Has("Result") == true)
	{
		auto EncodingIterator{g_EncodingNames.find(std::experimental::any_cast< TextEncoding >(Result.Get("Result")))};
		
		if(EncodingIterator != g_EncodingNames.end())
		{
			Result.Add("Name", EncodingIterator->second);
		}
		else
		{
			Result.Add("Name", "<invalid encoding>");
		}
	}
	
	return std::make_tuple(Success, Index, Result);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation helpers                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string Get_ID3_2_PictureType_Interpretation(std::uint8_t Value);
std::string Get_ID3_2_3_FileType_Interpretation(const std::string & Value);
std::string Get_ID3_2_4_FrameIdentifier_Interpretation(const std::string & Value);

std::string Get_ID3_2_PictureType_Interpretation(std::uint8_t Value)
{
	if(Value == 0x00)
	{
		return "Other";
	}
	else if(Value == 0x01)
	{
		return "32x32 pixels 'file icon' (PNG only)";
	}
	else if(Value == 0x02)
	{
		return "Other file icon";
	}
	else if(Value == 0x03)
	{
		return "Cover (front)";
	}
	else if(Value == 0x04)
	{
		return "Cover (back)";
	}
	else if(Value == 0x05)
	{
		return "Leaflet page";
	}
	else if(Value == 0x06)
	{
		return "Media (e.g. label side of CD";
	}
	else if(Value == 0x07)
	{
		return "Lead artist/lead performer/soloist";
	}
	else if(Value == 0x08)
	{
		return "Artist/performer";
	}
	else if(Value == 0x09)
	{
		return "Conductor";
	}
	else if(Value == 0x0a)
	{
		return "Band/Orchestra";
	}
	else if(Value == 0x0b)
	{
		return "Composer";
	}
	else if(Value == 0x0c)
	{
		return "Lyricist/text writer";
	}
	else if(Value == 0x0d)
	{
		return "Recording Location";
	}
	else if(Value == 0x0e)
	{
		return "During recording";
	}
	else if(Value == 0x0f)
	{
		return "During performance";
	}
	else if(Value == 0x10)
	{
		return "Movie/video screen capture";
	}
	else if(Value == 0x11)
	{
		return "A bright coloured fish";
	}
	else if(Value == 0x12)
	{
		return "Illustration";
	}
	else if(Value == 0x13)
	{
		return "Band/artist logotype";
	}
	else if(Value == 0x14)
	{
		return "Publisher/Studio logotype";
	}
	else
	{
		throw Inspection::UnknownValueException(to_string_cast(Value));
	}
}

std::string Get_ID3_2_3_FileType_Interpretation(const std::string & Value)
{
	if(Value == "MPG")
	{
		return "MPEG Audio";
	}
	else if(Value == "MPG/1")
	{
		return "MPEG 1/2 layer I";
	}
	else if(Value == "MPG/2")
	{
		return "MPEG 1/2 layer II";
	}
	else if(Value == "MPG/3")
	{
		return "MPEG 1/2 layer III";
	}
	else if(Value == "MPG/2.5")
	{
		return "MPEG 2.5";
	}
	else if(Value == "MPG/AAC")
	{
		return "Advanced audio compression";
	}
	else if(Value == "VQF")
	{
		return "Transform-domain Weighted Interleave Vector Quantization";
	}
	else if(Value == "PCM")
	{
		return "Pulse Code Modulated audio";
	}
	else
	{
		throw Inspection::UnknownValueException(Value);
	}
}

std::string Get_ID3_2_4_FrameIdentifier_Interpretation(const std::string & Value)
{
	if(Value == "AENC")
	{
		return "Audio encryption";
	}
	else if(Value == "APIC")
	{
		return "Attached Picture";
	}
	else if(Value == "ASPI")
	{
		return "Audio seek point index";
	}
	else if(Value == "COMM")
	{
		return "Comments";
	}
	else if(Value == "COMR")
	{
		return "Commercial frame";
	}
	else if(Value == "ENCR")
	{
		return "Encryption method registration";
	}
	else if(Value == "EQU2")
	{
		return "Equalisation (2)";
	}
	else if(Value == "ETCO")
	{
		return "Event timing codes";
	}
	else if(Value == "GEOB")
	{
		return "General encapsulated object";
	}
	else if(Value == "GRID")
	{
		return "Group identification registration";
	}
	else if(Value == "LINK")
	{
		return "Linked information";
	}
	else if(Value == "MCDI")
	{
		return "Music CD identifier";
	}
	else if(Value == "MLLT")
	{
		return "MPEG location lookup table";
	}
	else if(Value == "OWNE")
	{
		return "Ownership frame";
	}
	else if(Value == "PRIV")
	{
		return "Private frame";
	}
	else if(Value == "PCNT")
	{
		return "Play counter";
	}
	else if(Value == "POPM")
	{
		return "Popularimeter";
	}
	else if(Value == "POSS")
	{
		return "Position synchronisation frame";
	}
	else if(Value == "RBUF")
	{
		return "Recommended buffer size";
	}
	else if(Value == "RVA2")
	{
		return "Relative volume adjustment (2)";
	}
	else if(Value == "RVRB")
	{
		return "Reverb";
	}
	else if(Value == "SEEK")
	{
		return "Seek frame";
	}
	else if(Value == "SIGN")
	{
		return "Signature frame";
	}
	else if(Value == "SYLT")
	{
		return "Synchronised lyric/text";
	}
	else if(Value == "SYTC")
	{
		return "Synchronised tempo codes";
	}
	else if(Value == "TALB")
	{
		return "Album/Movie/Show title";
	}
	else if(Value == "TBMP")
	{
		return "BMP (beats per minute)";
	}
	else if(Value == "TCOM")
	{
		return "Composer";
	}
	else if(Value == "TCON")
	{
		return "Content type";
	}
	else if(Value == "TCOP")
	{
		return "Copyright message";
	}
	else if(Value == "TDEN")
	{
		return "Encoding time";
	}
	else if(Value == "TDLY")
	{
		return "Playlist delay";
	}
	else if(Value == "TDOR")
	{
		return "Original release time";
	}
	else if(Value == "TDRC")
	{
		return "Recording time";
	}
	else if(Value == "TDRL")
	{
		return "Release time";
	}
	else if(Value == "TDTG")
	{
		return "Tagging time";
	}
	else if(Value == "TENC")
	{
		return "Encoded by";
	}
	else if(Value == "TEXT")
	{
		return "Lyricist/Text writer";
	}
	else if(Value == "TFLT")
	{
		return "File type";
	}
	else if(Value == "TIPL")
	{
		return "Involved people list";
	}
	else if(Value == "TIT1")
	{
		return "Content group description";
	}
	else if(Value == "TIT2")
	{
		return "Title/songname/content description";
	}
	else if(Value == "TIT3")
	{
		return "Subtitle/Description refinement";
	}
	else if(Value == "TKEY")
	{
		return "Initial key";
	}
	else if(Value == "TLAN")
	{
		return "Language(s)";
	}
	else if(Value == "TLEN")
	{
		return "Length";
	}
	else if(Value == "TMCL")
	{
		return "Musician credits list";
	}
	else if(Value == "TMED")
	{
		return "Media type";
	}
	else if(Value == "TMOO")
	{
		return "Mood";
	}
	else if(Value == "TOAL")
	{
		return "Original album/movie/show title";
	}
	else if(Value == "TOFN")
	{
		return "Original filename";
	}
	else if(Value == "TOLY")
	{
		return "Original lyricist(s)/text writer(s)";
	}
	else if(Value == "TOPE")
	{
		return "Original artist(s)/performer(s)";
	}
	else if(Value == "TOWN")
	{
		return "File owner/licensee";
	}
	else if(Value == "TPE1")
	{
		return "Lead performer(s)/Soloist(s)";
	}
	else if(Value == "TPE2")
	{
		return "Band/orchestra/accompaniment";
	}
	else if(Value == "TPE3")
	{
		return "Conductor/performer refinement";
	}
	else if(Value == "TPE4")
	{
		return "Interpreted, remixed, or otherwise modified by";
	}
	else if(Value == "TPOS")
	{
		return "Part of a set";
	}
	else if(Value == "TPRO")
	{
		return "Produced notice";
	}
	else if(Value == "TPUB")
	{
		return "Publisher";
	}
	else if(Value == "TRCK")
	{
		return "Track number/Position in set";
	}
	else if(Value == "TRSN")
	{
		return "Internet radio station name";
	}
	else if(Value == "TRSO")
	{
		return "Internet radio station owner";
	}
	else if(Value == "TSOA")
	{
		return "Album sort order";
	}
	else if(Value == "TSOP")
	{
		return "Performer sort order";
	}
	else if(Value == "TSOT")
	{
		return "Title sort order";
	}
	else if(Value == "TSRC")
	{
		return "ISRC (international standard recording code)";
	}
	else if(Value == "TSSE")
	{
		return "Software/Hardware and settings used for encoding";
	}
	else if(Value == "TSST")
	{
		return "Set subtitle";
	}
	else if(Value == "TXXX")
	{
		return "User defined text information frame";
	}
	else if(Value == "UFID")
	{
		return "Unique file identifier";
	}
	else if(Value == "USER")
	{
		return "Terms of use";
	}
	else if(Value == "USLT")
	{
		return "Unsynchronised lyric/text transcription";
	}
	else if(Value == "WCOM")
	{
		return "Commercial information";
	}
	else if(Value == "WCOP")
	{
		return "Copyright/legal information";
	}
	else if(Value == "WOAF")
	{
		return "Official audio file webpage";
	}
	else if(Value == "WOAR")
	{
		return "Official artist/performer webpage";
	}
	else if(Value == "WOAS")
	{
		return "Official audio source webpage";
	}
	else if(Value == "WORS")
	{
		return "Official internet radio station webpage";
	}
	else if(Value == "WPAY")
	{
		return "Payment";
	}
	else if(Value == "WPUB")
	{
		return "Publisher's official webpage";
	}
	else if(Value == "WXXX")
	{
		return "User defined URL link frame";
	}
	else
	{
		throw Inspection::UnknownValueException(Value);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Buffer & Buffer, std::uint8_t FirstTrackNumber, std::uint8_t LastTrackNumber);
std::unique_ptr< Inspection::Result > Get_ID3_1_Tag(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_COM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_PictureType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_T__(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_UFI(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Language(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_Tag_Header_Flags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextEncoding(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding);
std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PCNT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCMP(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCON(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TFLT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TLAN(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Language(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Tag_Header_Flags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextEncoding(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Language(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_Header_Flags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextEncoding(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding);
std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_Tag_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("standard", "IEC 60908:1999"s);
	Result->GetValue()->AppendTag("name", "Compact Disc Digital Audio"s);
	
	auto HeaderResult{Get_IEC_60908_1999_TableOfContents_Header(Buffer)};
	
	Result->GetValue()->Append(HeaderResult->GetValue()->GetValues());
	if(HeaderResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto FirstTrackNumber{std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("FirstTrackNumber"))};
		auto LastTrackNumber{std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("LastTrackNumber"))};
		auto TracksResult{Get_IEC_60908_1999_TableOfContents_Tracks(Buffer, FirstTrackNumber, LastTrackNumber)};
		
		Result->GetValue()->Append(TracksResult->GetValue()->GetValues());
		Result->SetSuccess(TracksResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataLengthResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	Result->GetValue()->Append("DataLength", DataLengthResult->GetValue());
	if(DataLengthResult->GetSuccess() == true)
	{
		auto FirstTrackNumberResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->Append("FirstTrackNumber", FirstTrackNumberResult->GetValue());
		if(FirstTrackNumberResult->GetSuccess() == true)
		{
			auto LastTrackNumberResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->Append("LastTrackNumber", LastTrackNumberResult->GetValue());
			Result->SetSuccess(LastTrackNumberResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TrackResult{Get_IEC_60908_1999_TableOfContents_Track(Buffer)};
	
	Result->SetValue(TrackResult->GetValue());
	Result->SetSuccess((TrackResult->GetSuccess() == true) && (TrackResult->GetValue("Number")->HasTag("interpretation") == true) && (std::experimental::any_cast< const std::string & >(TrackResult->GetValue("Number")->GetTagAny("interpretation")) == "Lead-Out"));
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Reserved1Result{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 8))};
	
	Result->GetValue()->Append("Reserved", Reserved1Result->GetValue());
	if(Reserved1Result->GetSuccess() == true)
	{
		auto ADRResult{Get_UnsignedInteger_4Bit(Buffer)};
		
		Result->GetValue()->Append("ADR", ADRResult->GetValue());
		if((ADRResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(ADRResult->GetAny())))
		{
			auto ControlResult{Get_IEC_60908_1999_TableOfContents_Track_Control(Buffer)};
			
			Result->GetValue()->Append("Control", ControlResult->GetValue());
			if(ControlResult->GetSuccess() == true)
			{
				auto NumberResult{Get_UnsignedInteger_8Bit(Buffer)};
				
				Result->GetValue()->Append("Number", NumberResult->GetValue());
				if(NumberResult->GetSuccess() == true)
				{
					auto Number{std::experimental::any_cast< std::uint8_t >(NumberResult->GetAny())};
					
					if(Number == 0xaa)
					{
						Result->GetValue("Number")->PrependTag("interpretation", "Lead-Out"s);
					}
					
					auto Reserved2Result{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 8))};
					
					Result->GetValue()->Append("Reserved", Reserved2Result->GetValue());
					if(Reserved2Result->GetSuccess() == true)
					{
						auto StartAddressResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
						
						Result->GetValue()->Append("StartAddress", StartAddressResult->GetValue());
						Result->SetSuccess(StartAddressResult->GetSuccess());
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ControlResult{Get_BitSet_4Bit_MostSignificantBitFirst(Buffer)};
	
	Result->SetValue(ControlResult->GetValue());
	if(ControlResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 4 > & Control{std::experimental::any_cast< const std::bitset< 4 > & >(ControlResult->GetAny())};
		
		if(Control[1] == true)
		{
			if(Control[0] == true)
			{
				Result->SetSuccess(false);
				
				auto Value{Result->GetValue()->Append("Reserved", true)};
				
				Value->AppendTag("error", "The track type is \"Data\" so this bit must be off.");
			}
			else
			{
				Result->GetValue()->Append("Reserved", false);
			}
			Result->GetValue()->Append("TrackType", "Data"s);
			Result->GetValue()->Append("DigitalCopyProhibited", !Control[2]);
			if(Control[3] == true)
			{
				Result->GetValue()->Append("DataRecorded", "incrementally"s);
			}
			else
			{
				Result->GetValue()->Append("DataRecorded", "uninterrupted"s);
			}
		}
		else
		{
			if(Control[0] == true)
			{
				Result->GetValue()->Append("NumberOfChannels", 4);
			}
			else
			{
				Result->GetValue()->Append("NumberOfChannels", 2);
			}
			Result->GetValue()->Append("TrackType", "Audio"s);
			Result->GetValue()->Append("DigitalCopyProhibited", !Control[2]);
			Result->GetValue()->Append("PreEmphasis", Control[3]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Buffer & Buffer, std::uint8_t FirstTrackNumber, std::uint8_t LastTrackNumber)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	for(auto TrackNumber = FirstTrackNumber; TrackNumber <= LastTrackNumber; ++TrackNumber)
	{
		auto TrackResult{Get_IEC_60908_1999_TableOfContents_Track(Buffer)};
		auto TrackValue{Result->GetValue()->Append("Track", TrackResult->GetValue())};
		
		if(TrackResult->GetSuccess() == true)
		{
			auto TrackNumber{std::experimental::any_cast< std::uint8_t >(TrackResult->GetAny("Number"))};
			
			TrackValue->SetName("Track " + to_string_cast(TrackNumber));
		}
		else
		{
			Result->SetSuccess(false);
			
			break;
		}
	}
	if(Result->GetSuccess() == true)
	{
		auto LeadOutTrackResult{Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Buffer)};
		
		Result->GetValue()->Append("LeadOutTrack", LeadOutTrackResult->GetValue());
		Result->SetSuccess(LeadOutTrackResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_1_Tag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TagIdentifierResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "TAG")};
	
	Result->GetValue()->Append("Identifier", TagIdentifierResult->GetValue());
	if(TagIdentifierResult->GetSuccess() == true)
	{
		auto TitelResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
		
		Result->GetValue()->Append("Title", TitelResult->GetValue());
		if(TitelResult->GetSuccess() == true)
		{
			auto ArtistResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
			
			Result->GetValue()->Append("Artist", ArtistResult->GetValue());
			if(ArtistResult->GetSuccess() == true)
			{
				auto AlbumResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
				
				Result->GetValue()->Append("Album", AlbumResult->GetValue());
				if(AlbumResult->GetSuccess() == true)
				{
					auto YearResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(4ull, 0))};
					
					Result->GetValue()->Append("Year", YearResult->GetValue());
					if(YearResult->GetSuccess() == true)
					{
						auto StartOfComment{Buffer.GetPosition()};
						auto CommentResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
						auto Continue{false};
						
						if(CommentResult->GetSuccess() == true)
						{
							Result->GetValue()->Append("Comment", CommentResult->GetValue());
							Continue = true;
						}
						else
						{
							Buffer.SetPosition(StartOfComment);
							CommentResult = Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Buffer, Inspection::Length(29ull, 0));
							Result->GetValue()->Append("Comment", CommentResult->GetValue());
							if(CommentResult->GetSuccess() == true)
							{
								auto AlbumTrackResult{Get_UnsignedInteger_8Bit(Buffer)};
								
								Result->GetValue()->Append("AlbumTrack", AlbumTrackResult->GetValue());
								Continue = AlbumTrackResult->GetSuccess();
							}
						}
						if(Continue == true)
						{
							auto GenreResult{Get_UnsignedInteger_8Bit(Buffer)};
							
							Result->GetValue()->Append("Genre", GenreResult->GetValue());
							if(GenreResult->GetSuccess() == true)
							{
								Result->SetSuccess(true);
								
								auto Genre{std::experimental::any_cast< std::uint8_t >(GenreResult->GetAny())};
								auto NumericGenreIterator(g_NumericGenresID3_1.find(Genre));
								
								if(NumericGenreIterator != g_NumericGenresID3_1.end())
								{
									Result->GetValue("Genre")->PrependTag("interpretation", NumericGenreIterator->second);
									Result->GetValue("Genre")->PrependTag("standard", "ID3v1"s);
								}
								else
								{
									NumericGenreIterator = g_NumericGenresWinamp.find(Genre);
									if(NumericGenreIterator != g_NumericGenresWinamp.end())
									{
										Result->GetValue("Genre")->PrependTag("interpretation", NumericGenreIterator->second);
										Result->GetValue("Genre")->PrependTag("standard", "Winamp extension"s);
									}
									else
									{
										Result->GetValue("Genre")->PrependTag("interpretation", "<unrecognized>"s);
									}
								}
							}
						}
						Result->SetSuccess(true);
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_2_Frame_Header(Buffer)};
	
	Result->SetValue(HeaderResult->GetValue());
	if(HeaderResult->GetSuccess() == true)
	{
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(HeaderResult->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(HeaderResult->GetAny("Size")), 0)};
		std::unique_ptr< Inspection::Result > BodyResult;
		
		if(Identifier == "COM")
		{
			BodyResult = Get_ID3_2_2_Frame_Body_COM(Buffer, Size);
		}
		else if(Identifier == "PIC")
		{
			BodyResult = Get_ID3_2_2_Frame_Body_PIC(Buffer, Size);
		}
		else if((Identifier == "TAL") || (Identifier == "TCM") || (Identifier == "TCO") || (Identifier == "TCP") || (Identifier == "TEN") || (Identifier == "TP1") || (Identifier == "TP2") || (Identifier == "TPA") || (Identifier == "TRK") || (Identifier == "TT1") || (Identifier == "TT2") || (Identifier == "TYE"))
		{
			BodyResult = Get_ID3_2_2_Frame_Body_T__(Buffer, Size);
		}
		else if(Identifier == "UFI")
		{
			BodyResult = Get_ID3_2_2_Frame_Body_UFI(Buffer, Size);
		}
		if(BodyResult)
		{
			if(Start + Size > Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "Frame size is stated larger than the handled size."s);
			}
			else if(Start + Size < Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "Handled size is larger than the stated frame size."s);
			}
			Result->GetValue()->Append(BodyResult->GetValue()->GetValues());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		else
		{
			Result->SetSuccess(false);
		}
		Buffer.SetPosition(Start + Size);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_COM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_2_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_2_Language(Buffer)};
		
		Result->GetValue()->Append("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ShortContentDescriptionResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->Append("ShortContentDescription", ShortContentDescriptionResult->GetValue());
			if(ShortContentDescriptionResult->GetSuccess() == true)
			{
				auto CommentResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Comment", CommentResult->GetValue());
				Result->SetSuccess(CommentResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_2_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto ImageFormatResult{Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Buffer)};
		
		Result->GetValue()->Append("ImageFormat", ImageFormatResult->GetValue());
		if(ImageFormatResult->GetSuccess() == true)
		{
			auto PictureTypeResult{Get_ID3_2_2_Frame_Body_PIC_PictureType(Buffer)};
			
			Result->GetValue()->Append("PictureType", PictureTypeResult->GetValue());
			if(PictureTypeResult->GetSuccess() == true)
			{
				auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
				auto DescriptionResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->Append("Description", DescriptionResult->GetValue());
				if(DescriptionResult->GetSuccess() == true)
				{
					auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->Append("PictureData", PictureDataResult->GetValue());
					Result->SetSuccess(PictureDataResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ImageFormatResult{Get_ISO_IEC_8859_1_1998_String_EndedByLength(Buffer, Inspection::Length(3ull, 0))};
	
	Result->SetValue(ImageFormatResult->GetValue());
	if(ImageFormatResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & ImageFormat{std::experimental::any_cast< const std::string & >(ImageFormatResult->GetAny())};
		
		if(ImageFormat == "-->")
		{
			Result->GetValue()->PrependTag("mime-type", "application/x-url"s);
		}
		else if(ImageFormat == "PNG")
		{
			Result->GetValue()->PrependTag("mime-type", "image/png"s);
		}
		else if(ImageFormat == "JPG")
		{
			Result->GetValue()->PrependTag("mime-type", "image/jpeg"s);
		}
		else
		{
			Result->GetValue()->PrependTag("mime-type", "<unrecognized>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(PictureTypeResult->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3v2"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
			Result->SetSuccess(true);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_T__(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_2_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto InformationResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Information", InformationResult->GetValue());
		Result->SetSuccess(InformationResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_UFI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	Result->GetValue()->Append("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		auto IdentifierResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Identifier", IdentifierResult->GetValue());
		Result->SetSuccess(IdentifierResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByLength(Buffer, Inspection::Length(3ull, 0))};
	
	Result->GetValue()->Append("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(IdentifierResult->GetAny())};
		
		if(Identifier == "COM")
		{
			Result->GetValue()->AppendTag("name", "Comment"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "PIC")
		{
			Result->GetValue()->AppendTag("name", "Attached Picture"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TAL")
		{
			Result->GetValue()->AppendTag("name", "Album/Movie/Show title"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TCM")
		{
			Result->GetValue()->AppendTag("name", "Composer"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TCO")
		{
			Result->GetValue()->AppendTag("name", "Content type"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TCP")
		{
			Result->GetValue()->AppendTag("error", "This frame is not officially defined for tag version 2.2 but has been seen used nonetheless."s);
			Result->GetValue()->AppendTag("name", "Compilation"s);
			Result->GetValue()->AppendTag("standard", "<from the internet>"s);
		}
		else if(Identifier == "TEN")
		{
			Result->GetValue()->AppendTag("name", "Encoded by"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TP1")
		{
			Result->GetValue()->AppendTag("name", "Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TP2")
		{
			Result->GetValue()->AppendTag("name", "Band/Orchestra/Accompaniment"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TPA")
		{
			Result->GetValue()->AppendTag("name", "Part of a set"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TRK")
		{
			Result->GetValue()->AppendTag("name", "Track number/Position in set"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TT1")
		{
			Result->GetValue()->AppendTag("name", "Content group description"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TT2")
		{
			Result->GetValue()->AppendTag("name", "Title/Songname/Content description"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "TYE")
		{
			Result->GetValue()->AppendTag("name", "Year"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		else if(Identifier == "UFI")
		{
			Result->GetValue()->AppendTag("name", "Unique file identifier"s);
			Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		}
		
		auto SizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("Size", SizeResult->GetValue());
		Result->SetSuccess(SizeResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_2_Frame(Buffer)};
		
		if(FrameResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("Frame", FrameResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(Start);
			
			auto PaddingResult{Get_Bits_Unset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->Append("Padding", PaddingResult->GetValue());
			if(PaddingResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				Buffer.SetPosition(Boundary);
			}
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Language(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FieldResult{Get_ISO_639_2_1998_Code(Buffer)};
	
	if(FieldResult->GetSuccess() == true)
	{
		Result->SetValue(FieldResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Start);
		FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length(3ull, 0));
		Result->SetValue(FieldResult->GetValue());
		if(FieldResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		Result->GetValue()->PrependTag("bit order", "7-0"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->Append("Unsynchronization", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag = Result->GetValue()->Append("Compression", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag = Result->GetValue()->Append("Reserved", false);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 5; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UCS-2"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_8859_1_1998_StringResult->GetSuccess());
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_8859_1_1998_StringResult->GetSuccess());
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto MIMETypeResult{Get_ID3_2_3_Frame_Body_APIC_MIMEType(Buffer)};
		
		Result->GetValue()->Append("MIMEType", MIMETypeResult->GetValue());
		if(MIMETypeResult->GetSuccess() == true)
		{
			auto PictureTypeResult{Get_ID3_2_3_Frame_Body_APIC_PictureType(Buffer)};
			
			Result->GetValue()->Append("PictureType", PictureTypeResult->GetValue());
			if(PictureTypeResult->GetSuccess() == true)
			{
				auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
				auto DescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->Append("Description", DescriptionResult->GetValue());
				if(DescriptionResult->GetSuccess() == true)
				{
					auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->Append("PictureData", PictureDataResult->GetValue());
					Result->SetSuccess(PictureDataResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MIMETypeResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
	/// @todo As per [ID3 2.3.0], the value '-->' is also permitted to signal a URL [RFC 1738] in the picture data.
	Result->SetValue(MIMETypeResult->GetValue());
	Result->SetSuccess(MIMETypeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(PictureTypeResult->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3v3"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
			Result->SetSuccess(true);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto LanguageResult{Get_ID3_2_3_Language(Buffer)};
		
		Result->GetValue()->Append("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto ShortContentDescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->Append("ShortContentDescription", ShortContentDescriptionResult->GetValue());
			if(ShortContentDescriptionResult->GetSuccess() == true)
			{
				auto CommentResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Comment", CommentResult->GetValue());
				Result->SetSuccess(CommentResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto MIMETypeResult{Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Buffer)};
		
		Result->GetValue()->Append("MIMEType", MIMETypeResult->GetValue());
		if(MIMETypeResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto FileNameResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->Append("FileName", FileNameResult->GetValue());
			if(FileNameResult->GetSuccess() == true)
			{
				auto ContentDescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->Append("ContentDescription", ContentDescriptionResult->GetValue());
				if(ContentDescriptionResult->GetSuccess() == true)
				{
					auto EncapsulatedObjectResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->Append("EncapsulatedObject", EncapsulatedObjectResult->GetValue());
					Result->SetSuccess(EncapsulatedObjectResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TableOfContentsResult{Get_IEC_60908_1999_TableOfContents(Buffer)};
	
	Result->SetValue(TableOfContentsResult->GetValue());
	Result->SetSuccess(TableOfContentsResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MIMETypeResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
	Result->SetValue(MIMETypeResult->GetValue());
	Result->SetSuccess(MIMETypeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PCNT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.GetPosition() + Inspection::Length(4ul, 0) > Boundary)
	{
		auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Counter", CounterResult->GetValue());
		Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
		Result->GetValue("Counter")->PrependTag("standard", "ID3 2.3"s);
	}
	else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) == Boundary)
	{
		auto CounterResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("Counter", CounterResult->GetValue());
		Result->SetSuccess(CounterResult->GetSuccess());
	}
	else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
	{
		auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Counter", CounterResult->GetValue());
		Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto EMailToUserResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
	
	Result->GetValue()->Append("EMailToUser", EMailToUserResult->GetValue());
	if(EMailToUserResult->GetSuccess() == true)
	{
		auto RatingResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->Append("Rating", RatingResult->GetValue());
		if(RatingResult->GetSuccess() == true)
		{
			auto Rating{std::experimental::any_cast< std::uint8_t >(RatingResult->GetAny())};
			
			if(Rating == 0)
			{
				Result->GetValue("Rating")->PrependTag("standard", "ID3 2.3"s);
				Result->GetValue("Rating")->PrependTag("interpretation", "unknown"s);
			}
			if(Buffer.GetPosition() == Boundary)
			{
				auto CounterValue{std::make_shared< Inspection::Value >()};
				
				CounterValue->SetName("Counter");
				CounterValue->AppendTag("omitted"s);
				Result->GetValue()->Append(CounterValue);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) > Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
				Result->GetValue("Counter")->PrependTag("standard", "ID3 2.3"s);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) == Boundary)
			{
				auto CounterResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->Append("Counter", CounterResult->GetValue());
				Result->SetSuccess(CounterResult->GetSuccess());
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto InformationResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Information", InformationResult->GetValue());
		Result->SetSuccess(InformationResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCMP(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		
		if(Information == "1")
		{
			Result->GetValue("Information")->PrependTag("interpretation", "yes, this is part of a comilation"s);
		}
		else if(Information == "0")
		{
			Result->GetValue("Information")->PrependTag("interpretation", "no, this is not part of a compilation"s);
		}
		else
		{
			Result->GetValue("Information")->PrependTag("interpretation", "<unknown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCON(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		auto Interpretation{GetContentTypeInterpretation2_3(Information)};
		
		if(std::get<0>(Interpretation) == true)
		{
			Result->GetValue("Information")->PrependTag("interpretation", std::get<1>(Interpretation));
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TFLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		std::string Interpretation;
		
		Result->GetValue("Information")->PrependTag("standard", "ID3 2.3"s);
		try
		{
			Interpretation = Get_ID3_2_3_FileType_Interpretation(Information);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Information == "/3")
			{
				Interpretation = "MPEG 1/2 layer III";
				Result->GetValue("Information")->PrependTag("error", "The file type could not be interpreted strictly according to the standard, but this seems plausible."s);
			}
			else
			{
				Interpretation = "unkown";
				Result->GetValue("Information")->PrependTag("error", "The file type could not be interpreted."s);
			}
		}
		Result->GetValue("Information")->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TLAN(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		
		try
		{
			auto Interpretation{Inspection::Get_LanguageName_From_ISO_639_2_1998_Code(Information)};
			
			Result->GetValue("Information")->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue("Information")->PrependTag("interpretation", Interpretation);
		}
		catch(...)
		{
			Result->GetValue("Information")->PrependTag("standard", "ID3 2.3"s);
			Result->GetValue("Information")->PrependTag("error", "The language frame needs to contain a three letter code from ISO 639-2:1998 (alpha-3)."s);
			Result->GetValue("Information")->PrependTag("interpretation", "<unkown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->Append("Description", DescriptionResult->GetValue());
		if(DescriptionResult->GetSuccess() == true)
		{
			auto ValueResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->Append("Value", ValueResult->GetValue());
			Result->SetSuccess(ValueResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	Result->GetValue()->Append("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		auto IdentifierResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Identifier", IdentifierResult->GetValue());
		Result->SetSuccess(IdentifierResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_3_Language(Buffer)};
		
		Result->GetValue()->Append("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ContentDescriptorResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->Append("ContentDescriptor", ContentDescriptorResult->GetValue());
			if(ContentDescriptorResult->GetSuccess() == true)
			{
				auto LyricsTextResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Lyrics/Text", LyricsTextResult->GetValue());
				Result->SetSuccess(LyricsTextResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
	
	Result->GetValue()->Append("URL", URLResult->GetValue());
	Result->SetSuccess(URLResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptorResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->Append("Descriptor", DescriptorResult->GetValue());
		if(DescriptorResult->GetSuccess() == true)
		{
			auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->Append("URL", URLResult->GetValue());
			Result->SetSuccess(URLResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Language(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FieldResult{Get_ISO_639_2_1998_Code(Buffer)};
	
	if(FieldResult->GetSuccess() == true)
	{
		Result->SetValue(FieldResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Start);
		FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length(3ull, 0));
		Result->SetValue(FieldResult->GetValue());
		if(FieldResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		Result->GetValue()->PrependTag("bit order", "7-0"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->Append("Unsynchronization", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag->AppendTag("bit name", "a"s);
		Flag = Result->GetValue()->Append("ExtendedHeader", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag->AppendTag("bit name", "b"s);
		Flag = Result->GetValue()->Append("ExperimentalIndicator", Flags[5]);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit name", "c"s);
		Flag = Result->GetValue()->Append("Reserved", false);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 4; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UCS-2"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", Result->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		if(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", Result->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", Result->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		if(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", Result->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Frame_Header(Buffer)};
	
	Result->SetValue(HeaderResult->GetValue());
	if(HeaderResult->GetSuccess() == true)
	{
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(HeaderResult->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(HeaderResult->GetAny("Size")), 0)};
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > BodyHandler;
		
		if(Identifier == "APIC")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_APIC;
		}
		else if(Identifier == "COMM")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_COMM;
		}
		else if(Identifier == "MCDI")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_MCDI;
		}
		else if(Identifier == "POPM")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_POPM;
		}
		else if((Identifier == "TALB") || (Identifier == "TCOM") || (Identifier == "TCON") || (Identifier == "TCOP") || (Identifier == "TDRC") || (Identifier == "TDRL") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIT2") || (Identifier == "TLAN") || (Identifier == "TLEN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TSSE") || (Identifier == "TYER"))
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_T___;
		}
		else if(Identifier == "TXXX")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_TXXX;
		}
		else if(Identifier == "UFID")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_UFID;
		}
		else if(Identifier == "USLT")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_USLT;
		}
		else if(Identifier == "WCOM")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_W___;
		}
		else if(Identifier == "WXXX")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_WXXX;
		}
		if(BodyHandler != nullptr)
		{
			auto BodyResult{BodyHandler(Buffer, Size)};
			
			if(Start + Size > Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated larger than the actual handled size."s);
			}
			else if(Start + Size < Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the acutal handled size is larger than the stated frame size."s);
			}
			Result->GetValue()->Append(BodyResult->GetValue()->GetValues());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			auto BodyResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size)};
			
			Result->GetValue()->Append("Data", BodyResult->GetValue());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		Buffer.SetPosition(Start + Size);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto MIMETypeResult{Get_ID3_2_4_Frame_Body_APIC_MIMEType(Buffer)};
		
		Result->GetValue()->Append("MIMEType", MIMETypeResult->GetValue());
		if(MIMETypeResult->GetSuccess() == true)
		{
			auto PictureTypeResult{Get_ID3_2_4_Frame_Body_APIC_PictureType(Buffer)};
			
			Result->GetValue()->Append("PictureType", PictureTypeResult->GetValue());
			if(PictureTypeResult->GetSuccess() == true)
			{
				auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
				auto DescriptionResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->Append("Description", DescriptionResult->GetValue());
				if(DescriptionResult->GetSuccess() == true)
				{
					auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->Append("PictureData", PictureDataResult->GetValue());
					Result->SetSuccess(PictureDataResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MIMETypeResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
	/// @todo As per [ID3 2.4.0], the value '-->' is also permitted to signal a URL [RFC 1738] in the picture data.
	Result->SetValue(MIMETypeResult->GetValue());
	Result->SetSuccess(MIMETypeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(PictureTypeResult->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3v4"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
			Result->SetSuccess(true);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_4_Language(Buffer)};
		
		Result->GetValue()->Append("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ShortContentDescriptionResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->Append("ShortContentDescription", ShortContentDescriptionResult->GetValue());
			if(ShortContentDescriptionResult->GetSuccess() == true)
			{
				auto CommentResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Comment", CommentResult->GetValue());
				Result->SetSuccess(CommentResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TableOfContentsResult{Get_IEC_60908_1999_TableOfContents(Buffer)};
	
	Result->SetValue(TableOfContentsResult->GetValue());
	Result->SetSuccess(TableOfContentsResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto EMailToUserResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
	
	Result->GetValue()->Append("EMailToUser", EMailToUserResult->GetValue());
	if(EMailToUserResult->GetSuccess() == true)
	{
		auto RatingResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->Append("Rating", RatingResult->GetValue());
		if(RatingResult->GetSuccess() == true)
		{
			auto Rating{std::experimental::any_cast< std::uint8_t >(RatingResult->GetAny())};
			
			if(Rating == 0)
			{
				Result->GetValue("Rating")->PrependTag("standard", "ID3 2.3"s);
				Result->GetValue("Rating")->PrependTag("interpretation", "unknown"s);
			}
			if(Buffer.GetPosition() == Boundary)
			{
				auto CounterValue{std::make_shared< Inspection::Value >()};
				
				CounterValue->SetName("Counter");
				CounterValue->AppendTag("omitted"s);
				Result->GetValue()->Append(CounterValue);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) > Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
				Result->GetValue("Counter")->PrependTag("standard", "ID3 2.4"s);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) == Boundary)
			{
				auto CounterResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->Append("Counter", CounterResult->GetValue());
				Result->SetSuccess(CounterResult->GetSuccess());
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto InformationResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Information[0]", InformationResult->GetValue());
		if(InformationResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
			
			auto InformationIndex{1ul};
			
			while(Buffer.GetPosition() < Boundary)
			{
				InformationResult = Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition());
				Result->GetValue()->Append("Information[" + to_string_cast(InformationIndex) + "]", InformationResult->GetValue());
				if(InformationResult->GetSuccess() == false)
				{
					Result->SetSuccess(false);
					
					break;
				}
				InformationIndex += 1;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptionResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->Append("Description", DescriptionResult->GetValue());
		if(DescriptionResult->GetSuccess() == true)
		{
			auto ValueResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->Append("Value", ValueResult->GetValue());
			Result->SetSuccess(ValueResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	Result->GetValue()->Append("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		auto IdentifierResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append("Identifier", IdentifierResult->GetValue());
		Result->SetSuccess(IdentifierResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_4_Language(Buffer)};
		
		Result->GetValue()->Append("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ContentDescriptorResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->Append("ContentDescriptor", ContentDescriptorResult->GetValue());
			if(ContentDescriptorResult->GetSuccess() == true)
			{
				auto LyricsTextResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->Append("Lyrics/Text", LyricsTextResult->GetValue());
				Result->SetSuccess(LyricsTextResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
	
	Result->GetValue()->Append("URL", URLResult->GetValue());
	Result->SetSuccess(URLResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->Append("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptorResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->Append("Descriptor", DescriptorResult->GetValue());
		if(DescriptorResult->GetSuccess() == true)
		{
			auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->Append("URL", URLResult->GetValue());
			Result->SetSuccess(URLResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByLength(Buffer, Inspection::Length(4ull, 0))};
	
	Result->GetValue()->Append("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(IdentifierResult->GetAny())};
		std::string Interpretation;
		
		try
		{
			Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
			Interpretation = Get_ID3_2_4_FrameIdentifier_Interpretation(Identifier);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Identifier == "TYER")
			{
				Interpretation = "Year";
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.4. It has only been valid until tag version 2.3."s);
			}
			else
			{
				Result->GetValue()->PrependTag("error", "Unkown frame identifier \"" + Identifier + "\"."s);
				Result->SetSuccess(false);
			}
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
		if(Result->GetSuccess() == true)
		{
			auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
			
			Result->GetValue()->Append("Size", SizeResult->GetValue());
			if(SizeResult->GetSuccess() == true)
			{
				auto FlagsResult{Get_BitSet_16Bit_BigEndian(Buffer)};
				
				Result->GetValue()->Append("Flags", FlagsResult->GetValue());
				Result->SetSuccess(FlagsResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_4_Frame(Buffer)};
		
		if(FrameResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("Frame", FrameResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(Start);
			
			auto PaddingResult{Get_Bits_Unset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			
			if(PaddingResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("Padding", PaddingResult->GetValue());
			}
			else
			{
				Result->GetValue()->Append("Frame", FrameResult->GetValue());
				Result->SetSuccess(false);
				Buffer.SetPosition(Boundary);
			}
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Language(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FieldResult{Get_ISO_639_2_1998_Code(Buffer)};
	
	if(FieldResult->GetSuccess() == true)
	{
		Result->SetValue(FieldResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Start);
		FieldResult = Get_ASCII_String_Alphabetical_EndedByLength(Buffer, Inspection::Length(3ull, 0));
		if(FieldResult->GetSuccess() == true)
		{
			Result->SetValue(FieldResult->GetValue());
			
			const std::string & Code{std::experimental::any_cast< const std::string & >(FieldResult->GetAny())};
			
			if(Code == "XXX")
			{
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "<unknown>"s);
				Result->SetSuccess(true);
			}
		}
		else
		{
			Buffer.SetPosition(Start);
			FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length(3ull, 0));
			Result->SetValue(FieldResult->GetValue());
			if(FieldResult->GetSuccess() == true)
			{
				Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
	
	Result->GetValue()->Append("Size", SizeResult->GetValue());
	if(SizeResult->GetSuccess() == true)
	{
		auto NumberOfFlagBytesResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->Append("NumberOfFlagBytes", NumberOfFlagBytesResult->GetValue());
		if(NumberOfFlagBytesResult->GetSuccess() == true)
		{
			auto NumberOfFlagBytes{std::experimental::any_cast< std::uint8_t >(NumberOfFlagBytesResult->GetAny())};
			
			if(NumberOfFlagBytes == 0x01)
			{
				auto ExtendedHeaderFlagsResult{Get_ID3_2_4_Tag_ExtendedHeader_Flags(Buffer)};
				
				Result->GetValue()->Append("ExtendedFlags", ExtendedHeaderFlagsResult->GetValue());
				if(ExtendedHeaderFlagsResult->GetSuccess() == true)
				{
					Result->SetSuccess(true);
					if(Result->GetSuccess() == true)
					{
						auto TagIsAnUpdate{std::experimental::any_cast< bool >(ExtendedHeaderFlagsResult->GetAny("TagIsAnUpdate"))};
						
						if(TagIsAnUpdate == true)
						{
							auto TagIsAnUpdateDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Buffer)};
							
							Result->GetValue()->Append("TagIsAnUpdateData", TagIsAnUpdateDataResult->GetValue());
							Result->SetSuccess(TagIsAnUpdateDataResult->GetSuccess());
						}
					}
					if(Result->GetSuccess() == true)
					{
						auto CRCDataPresent{std::experimental::any_cast< bool >(ExtendedHeaderFlagsResult->GetAny("CRCDataPresent"))};
						
						if(CRCDataPresent == true)
						{
							auto CRCDataPresentDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Buffer)};
							
							Result->GetValue()->Append("CRCDataPresentData", CRCDataPresentDataResult->GetValue());
							Result->SetSuccess(CRCDataPresentDataResult->GetSuccess());
						}
					}
					if(Result->GetSuccess() == true)
					{
						auto TagRestrictions{std::experimental::any_cast< bool >(ExtendedHeaderFlagsResult->GetAny("TagRestrictions"))};
						
						if(TagRestrictions == true)
						{
							auto TagRestrictionsDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Buffer)};
							
							Result->GetValue()->Append("TagRestrictionsData", TagRestrictionsDataResult->GetValue());
							Result->SetSuccess(TagRestrictionsDataResult->GetSuccess());
						}
					}
				}
			}
			else
			{
				Result->SetSuccess(false);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
	
	Result->GetValue()->Append(HeaderResult->GetValue()->GetValues());
	if((HeaderResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("Size")) == 0x05))
	{
		auto TotalFrameCRCResult{Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Buffer)};
		
		Result->GetValue()->Append("TotalFrameCRC", TotalFrameCRCResult->GetValue());
		Result->SetSuccess(TotalFrameCRCResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
	
	Result->GetValue()->Append(HeaderResult->GetValue()->GetValues());
	Result->SetSuccess((HeaderResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("Size")) == 0x00));
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
	
	Result->GetValue()->Append(HeaderResult->GetValue()->GetValues());
	if((HeaderResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("Size")) == 0x01))
	{
		auto RestrictionResult{Get_BitSet_8Bit(Buffer)};
		auto Value{Result->GetValue()->Append("Restrictions", RestrictionResult->GetValue())};
		
		Value->AppendTag("error", "This program is missing the interpretation of the restriction flags."s); 
		Result->SetSuccess(RestrictionResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SizeResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Buffer)};
	
	Result->GetValue()->Append("Size", SizeResult->GetValue());
	Result->SetSuccess(SizeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		Result->GetValue()->PrependTag("bit order", "7-0"s);
		Result->GetValue()->AppendTag("synchsafe"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->Append("Reserved", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag = Result->GetValue()->Append("TagIsAnUpdate", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag->AppendTag("bit name", "b"s);
		Flag = Result->GetValue()->Append("CRCDataPresent", Flags[5]);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit name", "c"s);
		Flag = Result->GetValue()->Append("TagRestrictions", Flags[4]);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit name", "d"s);
		Flag = Result->GetValue()->Append("Reserved", false);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		Result->GetValue()->PrependTag("bit order", "7-0"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->Append("Unsynchronization", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag->AppendTag("bit name", "a"s);
		Flag = Result->GetValue()->Append("ExtendedHeader", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag->AppendTag("bit name", "b"s);
		Flag = Result->GetValue()->Append("ExperimentalIndicator", Flags[5]);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit name", "c"s);
		Flag = Result->GetValue()->Append("FooterPresent", Flags[4]);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit name", "d"s);
		Flag = Result->GetValue()->Append("Reserved", false);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UTF-16"s);
			Result->GetValue()->PrependTag("standard", "RFC 2781"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x02)
		{
			Result->GetValue()->PrependTag("name", "UTF-16BE"s);
			Result->GetValue()->PrependTag("standard", "RFC 2781"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x03)
		{
			Result->GetValue()->PrependTag("name", "UTF-8"s);
			Result->GetValue()->PrependTag("standard", "RFC 2279"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", ISO_IEC_8859_1_1998_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto UTF_16_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(UTF_16_StringResult->GetValue());
		if(UTF_16_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", UTF_16_StringResult->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x02)
	{
		auto UTF_16_BE_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(UTF_16_BE_StringResult->GetValue());
		if(UTF_16_BE_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_16_BE_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x03)
	{
		auto UTF_8_StringResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Buffer)};
		
		Result->SetValue(UTF_8_StringResult->GetValue());
		if(UTF_8_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_8_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", ISO_IEC_8859_1_1998_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto UTF_16_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(UTF_16_StringResult->GetValue());
		if(UTF_16_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", UTF_16_StringResult->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x02)
	{
		auto UTF_16_BE_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(UTF_16_BE_StringResult->GetValue());
		if(UTF_16_BE_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_16_BE_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x03)
	{
		auto UTF_8_StringResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(UTF_8_StringResult->GetValue());
		if(UTF_8_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_8_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_Tag_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FileIdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Buffer, "ID3")};
	
	Result->GetValue()->Append("FileIdentifier", FileIdentifierResult->GetValue());
	if(FileIdentifierResult->GetSuccess() == true)
	{
		auto MajorVersionResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Buffer)};
		
		Result->GetValue()->Append("MajorVersion", MajorVersionResult->GetValue());
		if(MajorVersionResult->GetSuccess() == true)
		{
			auto RevisionNumberResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Buffer)};
			
			Result->GetValue()->Append("RevisionNumber", RevisionNumberResult->GetValue());
			if(RevisionNumberResult->GetSuccess() == true)
			{
				auto MajorVersion{std::experimental::any_cast< std::uint8_t >(MajorVersionResult->GetAny())};
				std::unique_ptr< Inspection::Result > FlagsResult;
				
				if(MajorVersion == 0x02)
				{
					FlagsResult = Get_ID3_2_2_Tag_Header_Flags(Buffer);
				}
				else if(MajorVersion == 0x03)
				{
					FlagsResult = Get_ID3_2_3_Tag_Header_Flags(Buffer);
				}
				else if(MajorVersion == 0x04)
				{
					FlagsResult = Get_ID3_2_4_Tag_Header_Flags(Buffer);
				}
				if(FlagsResult)
				{
					Result->GetValue()->Append("Flags", FlagsResult->GetValue());
					if(FlagsResult->GetSuccess() == true)
					{
						auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
						
						Result->GetValue()->Append("Size", SizeResult->GetValue());
						Result->SetSuccess(SizeResult->GetSuccess());
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Inspection::Length(1ull, 0)) == true)
	{
		if(Buffer.Get1Bits() == 0x00)
		{
			std::uint8_t First{Buffer.Get7Bits()};
			
			Result->GetValue()->SetAny(First);
			Result->GetValue()->AppendTag("integer"s);
			Result->GetValue()->AppendTag("unsigned"s);
			Result->GetValue()->AppendTag("7bit value"s);
			Result->GetValue()->AppendTag("8bit field"s);
			Result->GetValue()->AppendTag("synchsafe"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Inspection::Length(4ull, 0)) == true)
	{
		if(Buffer.Get1Bits() == 0x00)
		{
			std::uint32_t First{Buffer.Get7Bits()};
			
			if(Buffer.Get1Bits() == 0x00)
			{
				std::uint32_t Second{Buffer.Get7Bits()};
				
				if(Buffer.Get1Bits() == 0x00)
				{
					std::uint32_t Third{Buffer.Get7Bits()};
					
					if(Buffer.Get1Bits() == 0x00)
					{
						std::uint32_t Fourth{Buffer.Get7Bits()};
						
						Result->GetValue()->SetAny((First << 21) | (Second << 14) | (Third << 7) | (Fourth));
						Result->GetValue()->AppendTag("integer"s);
						Result->GetValue()->AppendTag("unsigned"s);
						Result->GetValue()->AppendTag("28bit value"s);
						Result->GetValue()->AppendTag("32bit field"s);
						Result->GetValue()->AppendTag("synchsafe"s);
						Result->SetSuccess(true);
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Inspection::Length(5ull, 0)) == true)
	{
		if(Buffer.Get4Bits() == 0x00)
		{
			std::uint32_t First{Buffer.Get4Bits()};
			
			if(Buffer.Get1Bits() == 0x00)
			{
				std::uint32_t Second{Buffer.Get7Bits()};
				
				if(Buffer.Get1Bits() == 0x00)
				{
					std::uint32_t Third{Buffer.Get7Bits()};
					
					if(Buffer.Get1Bits() == 0x00)
					{
						std::uint32_t Fourth{Buffer.Get7Bits()};
						
						if(Buffer.Get1Bits() == 0x00)
						{
							std::uint32_t Fifth{Buffer.Get7Bits()};
							
							Result->GetValue()->SetAny((First << 28) | (Second << 21) | (Third << 14) | (Fourth << 7) | Fifth);
							Result->GetValue()->AppendTag("integer"s);
							Result->GetValue()->AppendTag("unsigned"s);
							Result->GetValue()->AppendTag("32bit value"s);
							Result->GetValue()->AppendTag("40bit field"s);
							Result->GetValue()->AppendTag("synchsafe"s);
							Result->SetSuccess(true);
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Tag and Frame header classes                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

class FrameHeader
{
public:
	// constructor
	FrameHeader(std::uint8_t MajorVersion, Inspection::Buffer & Buffer) :
		_Compression(false),
		_DataLengthIndicator(false),
		_Encryption(false),
		_FileAlterPreservation(false),
		_Forbidden(false),
		_GroupingIdentity(false),
		_HeaderSize(0),
		_ReadOnly(false),
		_SupportsCompression(false),
		_SupportsDataLengthIndicator(false),
		_SupportsEncryption(false),
		_SupportsFileAlterPreservation(0),
		_SupportsFlags(0),
		_SupportsGroupingIdentity(0),
		_SupportsReadOnly(0),
		_SupportsTagAlterPreservation(0),
		_SupportsUnsynchronisation(0),
		_TagAlterPreservation(0),
		_Unsynchronisation(0)
	{
		auto RawBuffer{Buffer.GetDataAtPosition()};
		
		if(MajorVersion == 0x03)
		{
			_HeaderSize = 10;
			_Identifier = std::string(reinterpret_cast< const char * const >(RawBuffer), 4);
			_Name = g_FrameNames_2_3[_Identifier];
			_DataSize = (static_cast< unsigned int >(static_cast< unsigned char >(RawBuffer[4])) << 24) + (static_cast< unsigned int >(static_cast< unsigned char >(RawBuffer[5])) << 16) + (static_cast< unsigned int >(static_cast< unsigned char >(RawBuffer[6])) << 8) + static_cast< unsigned int >(static_cast< unsigned char >(RawBuffer[7]));
			_SupportsFlags = true;
			_SupportsTagAlterPreservation = true;
			_TagAlterPreservation = (RawBuffer[8] & 0x80) == 0x80;
			_SupportsFileAlterPreservation = true;
			_FileAlterPreservation = (RawBuffer[8] & 0x40) == 0x40;
			_SupportsReadOnly = true;
			_ReadOnly = (RawBuffer[8] & 0x20) == 0x20;
			_SupportsCompression = true;
			_Compression = (RawBuffer[9] & 0x80) == 0x80;
			_SupportsEncryption = true;
			_Encryption = (RawBuffer[9] & 0x40) == 0x40;
			_SupportsGroupingIdentity = true;
			_GroupingIdentity = (RawBuffer[9] & 0x20) == 0x20;
			_SupportsUnsynchronisation = false;
			_SupportsDataLengthIndicator = false;
			
			std::map< std::string, std::string >::iterator ForbiddenIterator(_Forbidden23.find(_Identifier));
			
			if(ForbiddenIterator != _Forbidden23.end())
			{
				_Forbidden = true;
				_ForbiddenReason = ForbiddenIterator->second;
			}
		}
		Buffer.SetPosition(Buffer.GetPosition() + Inspection::Length(_HeaderSize, 0));
	}
	
	// getters
	bool GetCompression(void) const
	{
		return _Compression;
	}
	
	bool GetDataLengthIndicator(void) const
	{
		return _DataLengthIndicator;
	}
	
	bool GetEncryption(void) const
	{
		return _Encryption;
	}
	
	bool GetFileAlterPreservation(void) const
	{
		return _FileAlterPreservation;
	}
	
	std::string GetFlagsAsString(void) const
	{
		std::string Result;
		
		if((SupportsTagAlterPreservation() == true) && (GetTagAlterPreservation() == true))
		{
			AppendSeparated(Result, "Tag alter preservation", ", ");
		}
		if((SupportsFileAlterPreservation() == true) && (GetFileAlterPreservation() == true))
		{
			AppendSeparated(Result, "File alter preservation", ", ");
		}
		if((SupportsReadOnly() == true) && (GetReadOnly() == true))
		{
			AppendSeparated(Result, "Read only", ", ");
		}
		if((SupportsCompression() == true) && (GetCompression() == true))
		{
			AppendSeparated(Result, "Compression", ", ");
		}
		if((SupportsEncryption() == true) && (GetEncryption() == true))
		{
			AppendSeparated(Result, "Encryption", ", ");
		}
		if((SupportsGroupingIdentity() == true) && (GetGroupingIdentity() == true))
		{
			AppendSeparated(Result, "Grouping identity", ", ");
		}
		if((SupportsUnsynchronisation() == true) && (GetUnsynchronisation() == true))
		{
			AppendSeparated(Result, "Unsynchronisation", ", ");
		}
		if((SupportsDataLengthIndicator() == true) && (GetDataLengthIndicator() == true))
		{
			AppendSeparated(Result, "Data length indicator", ", ");
		}
		if(Result.empty() == true)
		{
			Result = "None";
		}
		
		return Result;
	}
	
	bool GetGroupingIdentity(void) const
	{
		return _GroupingIdentity;
	}
	
	unsigned int GetHeaderSize(void) const
	{
		return _HeaderSize;
	}
	
	std::string GetIdentifier(void) const
	{
		return _Identifier;
	}
	
	std::string GetName(void) const
	{
		return _Name;
	}
	
	bool GetReadOnly(void) const
	{
		return _ReadOnly;
	}
	
	bool GetForbidden(void)
	{
		return _Forbidden;
	}
	
	std::string GetForbiddenReason(void) const
	{
		return _ForbiddenReason;
	}
	
	unsigned int GetDataSize(void) const
	{
		return _DataSize;
	}
	
	bool GetTagAlterPreservation(void) const
	{
		return _TagAlterPreservation;
	}
	
	bool GetUnsynchronisation(void) const
	{
		return _Unsynchronisation;
	}
	
	bool IsValid(void) const
	{
		for(std::string::size_type Index = 0; Index < _Identifier.length(); ++Index)
		{
			if(IsValidIdentifierCharacter(_Identifier[Index]) == false)
			{
				return false;
			}
		}
		
		return true;
	}
	
	bool SupportsCompression(void) const
	{
		return _SupportsCompression;
	}
	
	bool SupportsDataLengthIndicator(void) const
	{
		return _SupportsDataLengthIndicator;
	}
	
	bool SupportsEncryption(void) const
	{
		return _SupportsEncryption;
	}
	
	bool SupportsFileAlterPreservation(void) const
	{
		return _SupportsFileAlterPreservation;
	}
	
	bool SupportsFlags(void) const
	{
		return _SupportsFlags;
	}
	
	bool SupportsGroupingIdentity(void) const
	{
		return _SupportsGroupingIdentity;
	}
	
	bool SupportsReadOnly(void) const
	{
		return _SupportsReadOnly;
	}
	
	bool SupportsTagAlterPreservation(void) const
	{
		return _SupportsTagAlterPreservation;
	}
	
	bool SupportsUnsynchronisation(void) const
	{
		return _SupportsUnsynchronisation;
	}
	
	// static setup
	static void Forbid23(const std::string & Identifier, const std::string & Reason)
	{
		_Forbidden23.insert(std::make_pair(Identifier, Reason));
	}
	
	static void Handle23(const std::string & Identifier, const std::string & Name, std::uint64_t (* Handler) (const uint8_t *, std::uint64_t))
	{
		g_FrameHandlers_2_3.insert(std::make_pair(Identifier, Handler));
		g_FrameNames_2_3.insert(std::make_pair(Identifier, Name));
	}
	
	// static setup
	static std::map< std::string, std::string > _Forbidden23;
private:
	// member variables
	bool _Compression;
	bool _DataLengthIndicator;
	unsigned int _DataSize;
	bool _Encryption;
	bool _FileAlterPreservation;
	bool _Forbidden;
	std::string _ForbiddenReason;
	bool _GroupingIdentity;
	unsigned int _HeaderSize;
	std::string _Identifier;
	std::string _Name;
	bool _ReadOnly;
	bool _SupportsCompression;
	bool _SupportsDataLengthIndicator;
	bool _SupportsEncryption;
	bool _SupportsFileAlterPreservation;
	bool _SupportsFlags;
	bool _SupportsGroupingIdentity;
	bool _SupportsReadOnly;
	bool _SupportsTagAlterPreservation;
	bool _SupportsUnsynchronisation;
	bool _TagAlterPreservation;
	bool _Unsynchronisation;
};

std::map< std::string, std::string > FrameHeader::_Forbidden23;

///////////////////////////////////////////////////////////////////////////////////////////////////
// specific to tag version 2.3                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::uint64_t Handle23APICFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_APIC(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23COMMFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_COMM(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23GEOB_Frame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_GEOB(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23MCDIFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_MCDI(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23MJCFFrame(const uint8_t * Buffer, std::uint64_t Length)
{
	std::uint64_t Index(0);
	
	if((Length >= 4) && (Buffer[0] == 0x00) && (Buffer[1] == 0x00) && (Buffer[2] == 0x00) && (Buffer[3] == 0x00))
	{
		auto Zeroes(GetHexadecimalStringTerminatedByLength(Buffer, 4));
		
		std::cout << "\t\t\t\tFour bytes of zeroes: " << Zeroes.second << std::endl;
		Index += Zeroes.first;
		
		auto Caption(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
		
		if(std::get<0>(Caption) == true)
		{
			std::string::size_type ColonPosition(std::get<2>(Caption).find(':'));
			
			if(ColonPosition != std::string::npos)
			{
				std::cout << "\t\t\t\tOne zero-terminated ISO/IEC 8859-1 string:" << std::endl;
				std::cout << "\t\t\t\t\tIdentifier: \"" << std::get<2>(Caption).substr(0, ColonPosition) << '"' << std::endl;
				std::cout << "\t\t\t\t\tSeparator: ':'" << std::endl;
				std::cout << "\t\t\t\t\tField Name: \"" << std::get<2>(Caption).substr(ColonPosition + 1) << '"' << std::endl;
				std::cout << "\t\t\t\t\tTerminator: 00" << std::endl;
				Index += std::get<1>(Caption);
				
				auto Value(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Value) == true)
				{
					Index += std::get<1>(Value);
					std::cout << "\t\t\t\t\tValue: \"" << std::get<2>(Value) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** Any data after the caption is expected to be a non-terminated ISO/IEC 8859-1:1998 string for the value." << std::endl;
					Index = Length;
				}
			}
			else
			{
				std::cout << "*** ERROR *** The first field is expected to contain a ':' character." << std::endl;
				Index = Length;
			}
		}
		else
		{
			std::cout << "*** ERROR *** Expected to see a terminated ISO/IEC 8859-1 string for the caption." << std::endl;
			Index = Length;
		}
	}
	else
	{
		std::cout << "*** ERROR *** Expected to see 4 zero-filled bytes at the start of the frame content." << std::endl;
		Index = Length;
	}
	
	return Index;
}

std::uint64_t Handle23PCNTFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_PCNT(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23POPMFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_POPM(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23PRIVFrame(const uint8_t * Buffer, std::uint64_t Length)
{
	std::uint64_t Index(0);
	auto ReadOwnerIdentifier(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
	
	if(std::get<0>(ReadOwnerIdentifier) == true)
	{
		Index += std::get<1>(ReadOwnerIdentifier);
		std::cout << "\t\t\t\tOwner Identifier: " << std::get<2>(ReadOwnerIdentifier) << std::endl;
		if(std::get<2>(ReadOwnerIdentifier) == "WM/MediaClassPrimaryID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tPrimary Media Class: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/MediaClassSecondaryID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tSecondary Media Class: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/WMContentID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tContent ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/WMCollectionID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tCollection ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/WMCollectionGroupID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tCollection Group ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/Provider")
		{
			auto PrivateData(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(PrivateData) == true)
			{
				Index += std::get<1>(PrivateData);
				std::cout << "\t\t\t\tContent provider: \"" << std::get<2>(PrivateData) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
			}
			else
			{
				auto PrivateData(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(PrivateData) == true)
				{
					Index += std::get<1>(PrivateData);
					std::cout << "\t\t\t\tContent provider: \"" << std::get<2>(PrivateData) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/UniqueFileIdentifier")
		{
			std::string PrivateDataString;
			auto PrivateData(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(PrivateData) == true)
			{
				Index += std::get<1>(PrivateData);
				std::cout << "\t\t\t\tUnique file identifier: \"" << std::get<2>(PrivateData) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
				PrivateDataString = std::get<2>(PrivateData);
			}
			else
			{
				auto PrivateData(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(PrivateData) == true)
				{
					Index += std::get<1>(PrivateData);
					std::cout << "\t\t\t\tUnique file identifier: \"" << std::get<2>(PrivateData) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
					PrivateDataString = std::get<2>(PrivateData);
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
				}
			}
			
			auto Interpretation(GetWMUniqueFileIdentifierInterpretation(PrivateDataString));
			
			if(Interpretation.first == true)
			{
				std::cout << "\t\t\t\tInterpretation as AllMusicGuide fields (http://www.allmusic.com/):" << std::endl;
				std::cout << Interpretation.second << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneAlbumArtistMediaID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tZune Album Artist Media ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneAlbumMediaID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tZune Album Media ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneCollectionID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tZune Collection ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneMediaID")
		{
			auto ReadGUID(Get_GUID_String(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadGUID) == true)
			{
				Index += std::get<1>(ReadGUID);
				std::cout << "\t\t\t\tZune Media ID: " << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("Result")) << std::endl;
				if(std::get<2>(ReadGUID).Has("GUIDDescription") == true)
				{
					std::cout << "\t\t\t\t\tGUID Description: \"" << std::experimental::any_cast< std::string >(std::get<2>(ReadGUID).Get("GUIDDescription")) << '"' << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tGUID Description: <unknown value>" << std::endl;
				}
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "CompID")
		{
			auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
			
			std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
			
			auto Interpretation(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Interpretation) == true)
			{
				std::cout << "\t\t\t\tString: \"" << std::get<2>(Interpretation) << "\" (interpreted as UCS-2LE without byte order mark with termination)" << std::endl;
			}
			Index += ReadHexadecimal.first;
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "MachineCode")
		{
			auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
			
			std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
			
			auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
			}
			Index += ReadHexadecimal.first;
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "PeakValue")
		{
			if(Length - Index == 4)
			{
				auto ReadUInt16(Get_UInt16_LE(Buffer + Index, Length - Index));
				
				assert(std::get<0>(ReadUInt16) == true);
				std::cout << "\t\t\t\tPeak Value: " << std::get<2>(ReadUInt16) << std::endl;
				Index += std::get<1>(ReadUInt16);
				if((Buffer[Index] != 0) || (Buffer[Index + 1] != 0))
				{
					auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
					
					Index += ReadHexadecimal.first;
					std::cout << "\t\t\t\tThis value is defined by Microsoft to be of type DWORD which requires 4 byte." << std::endl;
					std::cout << "\t\t\t\tBy definition an unsigned 2 byte value is stored in here. The other two bytes should be zero but they are not." << std::endl;
					std::cout << "\t\t\t\tBinary Content of the rest: " << ReadHexadecimal.second << std::endl;
				}
				else
				{
					Index += 2;
				}
			}
			else
			{
				auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
				
				Index += ReadHexadecimal.first;
				std::cout << "\t\t\t\tThis value is defined by Microsoft to be of type DWORD which requires 4 byte." << std::endl;
				std::cout << "\t\t\t\tInstead " << (Length - Index) << " bytes are available. Skipped reading." << std::endl;
				std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "AverageLevel")
		{
			if(Length - Index == 4)
			{
				auto ReadUInt16(Get_UInt16_LE(Buffer + Index, Length - Index));
				
				assert(std::get<0>(ReadUInt16) == true);
				std::cout << "\t\t\t\tAverage Level: " << std::get<2>(ReadUInt16) << std::endl;
				Index += std::get<1>(ReadUInt16);
				if((Buffer[Index] != 0) || (Buffer[Index + 1] != 0))
				{
					auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
					
					Index += ReadHexadecimal.first;
					std::cout << "\t\t\t\tThis value is defined by Microsoft to be of type DWORD which requires 4 byte." << std::endl;
					std::cout << "\t\t\t\tBy definition an unsigned 2 byte value is stored in here. The other two bytes should be zero but they are not." << std::endl;
					std::cout << "\t\t\t\tBinary Content of the rest: " << ReadHexadecimal.second << std::endl;
				}
				else
				{
					Index += 2;
				}
			}
			else
			{
				auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
				
				Index += ReadHexadecimal.first;
				std::cout << "\t\t\t\tThis value is defined by Microsoft to be of type DWORD which requires 4 byte." << std::endl;
				std::cout << "\t\t\t\tInstead " << (Length - Index) << " bytes are available. Skipped reading." << std::endl;
				std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
			}
		}
		else
		{
			auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
			
			Index += ReadHexadecimal.first;
			std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.28], \"PRIV\" frames MUST contain an \"Owner identifier\" field with a valid URL in ISO/IEC 8859-1:1998 encoding." << std::endl;
		Index = Length;
	}
	
	return Index;
}

std::uint64_t Handle23RGADFrame(const uint8_t * Buffer, std::uint64_t Length)
{
	std::uint64_t Index(0);
	auto PeakAmplitude(Get_ISO_IEC_IEEE_60559_2011_binary32(Buffer + Index, Length - Index));
	
	if(std::get<0>(PeakAmplitude) == true)
	{
		Index += std::get<1>(PeakAmplitude);
		std::cout << "\t\t\t\tPeak amplitude: " << std::get<2>(PeakAmplitude) << std::endl;
		
		auto TrackReplayGainAdjustment(Get_UInt16_BE(Buffer + Index, Length - Index));
		
		if(std::get<0>(TrackReplayGainAdjustment) == true)
		{
			Index += std::get<1>(TrackReplayGainAdjustment);
			std::cout << "\t\t\t\tTrack replay gain adjustment: " << GetBinaryStringFromUInt16(std::get<2>(TrackReplayGainAdjustment)) << std::endl;
			
			auto AlbumReplayGainAdjustment(Get_UInt16_BE(Buffer + Index, Length - Index));
			
			if(std::get<0>(AlbumReplayGainAdjustment) == true)
			{
				Index += std::get<1>(AlbumReplayGainAdjustment);
				std::cout << "\t\t\t\tAlbum replay gain adjustment: " << GetBinaryStringFromUInt16(std::get<2>(AlbumReplayGainAdjustment)) << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** According to the unofficial Hydrogenaudio specification, \"RGAD\" frames should contain a 16bit \"Album replay gain adjustment\" field." << std::endl;
			}
		}
		else
		{
			std::cout << "*** ERROR *** According to the unofficial Hydrogenaudio specification, \"RGAD\" frames should contain a 16bit \"Track replay gain adjustment\" field." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to the unofficial Hydrogenaudio specification, \"RGAD\" frames should contain a 32bit \"Peak amplitude\" field." << std::endl;
	}
	
	return Index;
}

std::uint64_t Handle23T___Frames(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23TCONFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_TCON(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23TFLTFrames(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_TFLT(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23TLANFrames(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_TLAN(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23TCMPFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_TCMP(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23TSRCFrame(const uint8_t * Buffer, std::uint64_t Length)
{
	std::uint64_t Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << std::experimental::any_cast< std::string >(std::get<2>(Encoding).Get("Name")) << std::endl;
		
		std::string ReadString;
		
		if(std::experimental::any_cast< TextEncoding >(std::get<2>(Encoding).Get("Result")) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto String(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				ReadString = std::get<2>(String);
			}
			else
			{
				auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
					ReadString = std::get<2>(String);
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
				}
			}
		}
		else if(std::experimental::any_cast< TextEncoding >(std::get<2>(Encoding).Get("Result")) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: big endian" << std::endl;
					
					auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by termination)" << std::endl;
						ReadString = std::get<2>(String);
					}
					else
					{
						auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by boundary)" << std::endl;
							ReadString = std::get<2>(String);
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tContent type:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: little endian" << std::endl;
					
					auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
						ReadString = std::get<2>(String);
					}
					else
					{
						auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by boundary)" << std::endl;
							ReadString = std::get<2>(String);
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark." << std::endl;
			}
		}
		if(ReadString.length() == 12)
		{
			std::string ISO_3166_1_Alpha_2_Code(ReadString.substr(0, 2));
			auto ISO_3166_1_Alpha_2_Iterator(g_ISO_3166_1_Alpha_2_Codes.find(ISO_3166_1_Alpha_2_Code));
			
			if(ISO_3166_1_Alpha_2_Iterator != g_ISO_3166_1_Alpha_2_Codes.end())
			{
				std::cout << "\t\t\t\t\tCountry: " << ISO_3166_1_Alpha_2_Iterator->second << " (\"" << ISO_3166_1_Alpha_2_Code << "\")" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\t\tCountry: <unknown> (\"" << ISO_3166_1_Alpha_2_Code << "\")" << std::endl;
				std::cout << "*** ERROR *** The country code '" << ISO_3166_1_Alpha_2_Code << "' is not defined by ISO 3166-1 alpha-2." << std::endl;
			}
			std::cout << "\t\t\t\t\tRegistrant code: \"" << ReadString.substr(2, 3) << '"' << std::endl;
			std::cout << "\t\t\t\t\tYear of registration: \"" << ReadString.substr(5, 2) << '"' << std::endl;
			std::cout << "\t\t\t\t\tRegistration number: \"" << ReadString.substr(7, 5) << '"' << std::endl;
		}
		else
		{
			std::cout << "*** ERROR *** The international standard recording code defined by ISO 3901 requires the code to be 12 characters long, not " << ReadString.length() << "." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.1], a \"TSRC\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

std::uint64_t Handle23TXXXFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_TXXX(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23UFIDFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_UFID(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23USLTFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_USLT(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23W___Frames(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_W___(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}

std::uint64_t Handle23WXXXFrame(const uint8_t * RawBuffer, std::uint64_t Length)
{
	Inspection::Buffer Buffer{RawBuffer, Inspection::Length(Length, 0)};
	auto FrameResult{Get_ID3_2_3_Frame_Body_WXXX(Buffer, Buffer.GetLength())};
	
	PrintValue(FrameResult->GetValue(), "\t\t\t\t");
	
	return FrameResult->GetLength().GetBytes();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// application                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadID3v2Tag(Inspection::Buffer & Buffer)
{
	auto TagHeaderResult{Get_ID3_2_Tag_Header(Buffer)};
	
	if(TagHeaderResult->GetSuccess() == true)
	{
		TagHeaderResult->GetValue()->SetName("ID3v2");
		
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(TagHeaderResult->GetAny("MajorVersion"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(TagHeaderResult->GetAny("Size")), 0)};
		
		if(MajorVersion == 0x02)
		{
			auto FramesResult{Get_ID3_2_2_Frames(Buffer, Size)};
			
			TagHeaderResult->GetValue()->Append(FramesResult->GetValue()->GetValues());
			TagHeaderResult->SetSuccess(FramesResult->GetSuccess());
			PrintValue(TagHeaderResult->GetValue(), "    ");
		}
		else if(MajorVersion == 0x03)
		{
			PrintValue(TagHeaderResult->GetValue(), "    ");
			if(TagHeaderResult->GetSuccess() == true)
			{
				auto Boundary{Buffer.GetPosition() + Size};
				auto SkippingSize{0};

				std::cout << "\tFrames:" << std::endl;
				while(Buffer.GetPosition() < Boundary)
				{
					auto Start{Buffer.GetPosition()};
					FrameHeader * NewFrameHeader(new FrameHeader(MajorVersion, Buffer));
					
					if(NewFrameHeader->IsValid() == true)
					{
						if(SkippingSize > 0)
						{
							std::cout << "# Skipped " << SkippingSize << " bytes of invalid data." << std::endl;
							SkippingSize = 0;
						}
						std::cout << "\t\tIdentifier: \"" << NewFrameHeader->GetIdentifier() << "\"" << std::endl;
						if(NewFrameHeader->GetForbidden() == true)
						{
							std::cout << "*** ERROR *** This frame is forbidden! " << NewFrameHeader->GetForbiddenReason() << std::endl;
						}
						std::cout << "\t\t\tName: " << NewFrameHeader->GetName() << std::endl;
						std::cout << "\t\t\tSize: " << NewFrameHeader->GetDataSize() << std::endl;
						if(NewFrameHeader->SupportsFlags() == true)
						{
							std::cout << "\t\t\tFlags: " << NewFrameHeader->GetFlagsAsString() << std::endl;
						}
						
						auto RawBuffer{Buffer.GetDataAtPosition()};
						
						if(g_PrintBytes == true)
						{
							std::cout << "\t\t\tBytes: " << GetHexadecimalStringFromUInt8Buffer(RawBuffer, NewFrameHeader->GetDataSize()) << std::endl;
						}
						std::cout << "\t\t\tContent:" << std::endl;
						
						auto HandledFrameSize{0ull};
						std::function< std::uint64_t (const std::uint8_t *, std::uint64_t) > Handler;
						
						if(MajorVersion == 0x03)
						{
							auto HandlerIterator{g_FrameHandlers_2_3.find(NewFrameHeader->GetIdentifier())};
							
							if(HandlerIterator != g_FrameHandlers_2_3.end())
							{
								Handler = HandlerIterator->second;
							}
						}
						if(Handler != nullptr)
						{
							HandledFrameSize = Handler(RawBuffer, NewFrameHeader->GetDataSize());
						}
						else
						{
							std::cout << "*** ERROR *** No handler defined for the frame type \"" << NewFrameHeader->GetIdentifier() << "\" in tag version 2." << to_string_cast(MajorVersion) << "." << std::endl;
							HandledFrameSize = NewFrameHeader->GetDataSize();
						}
						if(HandledFrameSize < NewFrameHeader->GetDataSize())
						{
							std::cout << "*** ERROR *** Frame size exceeds frame data. (handled=" << HandledFrameSize << " < size=" << NewFrameHeader->GetDataSize() << ')' << std::endl;
						}
						else if(HandledFrameSize > NewFrameHeader->GetDataSize())
						{
							std::cout << "*** ERROR *** Frame data exceeds frame size. (handled=" << HandledFrameSize << " > size=" << NewFrameHeader->GetDataSize() << ')' << std::endl;
						}
						std::cout << std::endl;
						Buffer.SetPosition(Start + NewFrameHeader->GetHeaderSize() + NewFrameHeader->GetDataSize());
					}
					else
					{
						SkippingSize += 1;
						Buffer.SetPosition(Start + 1);
					}
					delete NewFrameHeader;
				}
				if(SkippingSize > 0)
				{
					std::cout << "# Skipped " << SkippingSize << " bytes of padding." << std::endl;
				}
				std::cout << std::endl;
			}
		}
		else if(MajorVersion == 0x04)
		{
			if((TagHeaderResult->GetValue("Flags")->HasValue("ExtendedHeader") == true) && (std::experimental::any_cast< bool >(TagHeaderResult->GetValue("Flags")->GetValueAny("ExtendedHeader")) == true))
			{
				auto ExtendedHeaderResult{Get_ID3_2_4_Tag_ExtendedHeader(Buffer)};
				
				TagHeaderResult->GetValue()->Append("ExtendedHeader", ExtendedHeaderResult->GetValue());
				TagHeaderResult->SetSuccess(ExtendedHeaderResult->GetSuccess());
				Size -= ExtendedHeaderResult->GetLength();
			}
			if(TagHeaderResult->GetSuccess() == true)
			{
				auto FramesResult{Get_ID3_2_4_Frames(Buffer, Size)};
				
				TagHeaderResult->GetValue()->Append(FramesResult->GetValue()->GetValues());
				TagHeaderResult->SetSuccess(FramesResult->GetSuccess());
			}
			PrintValue(TagHeaderResult->GetValue(), "    ");
		}
		else
		{
			TagHeaderResult->GetValue()->PrependTag("error", "Unknown major version \"" + to_string_cast(MajorVersion) + "\".");
			PrintValue(TagHeaderResult->GetValue(), "    ");
		}
	}
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	ReadID3v2Tag(Buffer);
	
	auto Position{Buffer.GetPosition()};
	
	if(Buffer.GetLength() >= Inspection::Length(128ull, 0))
	{
		Buffer.SetPosition(Buffer.GetLength() - Inspection::Length(128ull, -0));
		
		auto ID3v1TagResult{Get_ID3_1_Tag(Buffer)};
		
		if(ID3v1TagResult->GetSuccess() == true)
		{
			if(ID3v1TagResult->GetValue()->HasValue("AlbumTrack") == true)
			{
				Result->GetValue()->Append("ID3v1.1", ID3v1TagResult->GetValue());
			}
			else
			{
				Result->GetValue()->Append("ID3v1", ID3v1TagResult->GetValue());
			}
		}
		else
		{
			Buffer.SetPosition(Position);
		}
	}
	Result->SetSuccess(true);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

int main(int argc, char **argv)
{
	std::deque< std::string > Paths;
	unsigned int Arguments(argc);
	unsigned int Argument(0);

	while(++Argument < Arguments)
	{
		if(std::string(argv[Argument]) == "--print-bytes")
		{
			g_PrintBytes = true;
		}
		else
		{
			Paths.push_back(argv[Argument]);
		}
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " [--print-bytes] <paths> ..." << std::endl;

		return 1;
	}
	
	// GUID descriptions
	/// WM/MediaClassPrimaryID: audio, no music (bytes swapped to big endian)
	g_GUIDDescriptions.insert(std::make_pair("290fcd01-4eda-5741-897b-6275d50c4f11", "audio, no music"));
	/// WM/MediaClassPrimaryID: audio, music (bytes swapped to big endian)
	g_GUIDDescriptions.insert(std::make_pair("bc7d60d1-23e3-e24b-86a1-48a42a28441e", "audio, music"));
	/// WM/MediaClassPrimaryID: video (bytes swapped to big endian)
	g_GUIDDescriptions.insert(std::make_pair("bd3098db-b33a-ab4f-8a37-1a995f7ff74b", "video"));
	/// WM/MediaClassPrimaryID: neither audio nor video (bytes swapped to big endian)
	g_GUIDDescriptions.insert(std::make_pair("764af2fc-579a-3640-990d-e35dd8b244e1", "neither audio nor video"));
	
	// numeric genres for version ID3v1
	g_NumericGenresID3_1.insert(std::make_pair(0, "Blues"));
	g_NumericGenresID3_1.insert(std::make_pair(1, "Classic Rock"));
	g_NumericGenresID3_1.insert(std::make_pair(2, "Country"));
	g_NumericGenresID3_1.insert(std::make_pair(3, "Dance"));
	g_NumericGenresID3_1.insert(std::make_pair(4, "Disco"));
	g_NumericGenresID3_1.insert(std::make_pair(5, "Funk"));
	g_NumericGenresID3_1.insert(std::make_pair(6, "Grunge"));
	g_NumericGenresID3_1.insert(std::make_pair(7, "Hip-Hop"));
	g_NumericGenresID3_1.insert(std::make_pair(8, "Jazz"));
	g_NumericGenresID3_1.insert(std::make_pair(9, "Metal"));
	g_NumericGenresID3_1.insert(std::make_pair(10, "New Age"));
	g_NumericGenresID3_1.insert(std::make_pair(11, "Oldies"));
	g_NumericGenresID3_1.insert(std::make_pair(12, "Other"));
	g_NumericGenresID3_1.insert(std::make_pair(13, "Pop"));
	g_NumericGenresID3_1.insert(std::make_pair(14, "R&B"));
	g_NumericGenresID3_1.insert(std::make_pair(15, "Rap"));
	g_NumericGenresID3_1.insert(std::make_pair(16, "Reggae"));
	g_NumericGenresID3_1.insert(std::make_pair(17, "Rock"));
	g_NumericGenresID3_1.insert(std::make_pair(18, "Techno"));
	g_NumericGenresID3_1.insert(std::make_pair(19, "Industrial"));
	g_NumericGenresID3_1.insert(std::make_pair(20, "Alternative"));
	g_NumericGenresID3_1.insert(std::make_pair(21, "Ska"));
	g_NumericGenresID3_1.insert(std::make_pair(22, "Death Metal"));
	g_NumericGenresID3_1.insert(std::make_pair(23, "Pranks"));
	g_NumericGenresID3_1.insert(std::make_pair(24, "Soundtrack"));
	g_NumericGenresID3_1.insert(std::make_pair(25, "Euro-Techno"));
	g_NumericGenresID3_1.insert(std::make_pair(26, "Ambient"));
	g_NumericGenresID3_1.insert(std::make_pair(27, "Trip-Hop"));
	g_NumericGenresID3_1.insert(std::make_pair(28, "Vocal"));
	g_NumericGenresID3_1.insert(std::make_pair(29, "Jazz+Funk"));
	g_NumericGenresID3_1.insert(std::make_pair(30, "Fusion"));
	g_NumericGenresID3_1.insert(std::make_pair(31, "Trance"));
	g_NumericGenresID3_1.insert(std::make_pair(32, "Classical"));
	g_NumericGenresID3_1.insert(std::make_pair(33, "Instrumental"));
	g_NumericGenresID3_1.insert(std::make_pair(34, "Acid"));
	g_NumericGenresID3_1.insert(std::make_pair(35, "House"));
	g_NumericGenresID3_1.insert(std::make_pair(36, "Game"));
	g_NumericGenresID3_1.insert(std::make_pair(37, "Sound Clip"));
	g_NumericGenresID3_1.insert(std::make_pair(38, "Gospel"));
	g_NumericGenresID3_1.insert(std::make_pair(39, "Noise"));
	g_NumericGenresID3_1.insert(std::make_pair(40, "AlternRock"));
	g_NumericGenresID3_1.insert(std::make_pair(41, "Bass"));
	g_NumericGenresID3_1.insert(std::make_pair(42, "Soul"));
	g_NumericGenresID3_1.insert(std::make_pair(43, "Punk"));
	g_NumericGenresID3_1.insert(std::make_pair(44, "Space"));
	g_NumericGenresID3_1.insert(std::make_pair(45, "Meditative"));
	g_NumericGenresID3_1.insert(std::make_pair(46, "Instrumental Pop"));
	g_NumericGenresID3_1.insert(std::make_pair(47, "Instrumental Rock"));
	g_NumericGenresID3_1.insert(std::make_pair(48, "Ethnic"));
	g_NumericGenresID3_1.insert(std::make_pair(49, "Gothic"));
	g_NumericGenresID3_1.insert(std::make_pair(50, "Darkwave"));
	g_NumericGenresID3_1.insert(std::make_pair(51, "Techno-Industrial"));
	g_NumericGenresID3_1.insert(std::make_pair(52, "Electronic"));
	g_NumericGenresID3_1.insert(std::make_pair(53, "Pop-Folk"));
	g_NumericGenresID3_1.insert(std::make_pair(54, "Eurodance"));
	g_NumericGenresID3_1.insert(std::make_pair(55, "Dream"));
	g_NumericGenresID3_1.insert(std::make_pair(56, "Southern Rock"));
	g_NumericGenresID3_1.insert(std::make_pair(57, "Comedy"));
	g_NumericGenresID3_1.insert(std::make_pair(58, "Cult"));
	g_NumericGenresID3_1.insert(std::make_pair(59, "Gangsta"));
	g_NumericGenresID3_1.insert(std::make_pair(60, "Top 40"));
	g_NumericGenresID3_1.insert(std::make_pair(61, "Christian Rap"));
	g_NumericGenresID3_1.insert(std::make_pair(62, "Pop/Funk"));
	g_NumericGenresID3_1.insert(std::make_pair(63, "Jungle"));
	g_NumericGenresID3_1.insert(std::make_pair(64, "Native American"));
	g_NumericGenresID3_1.insert(std::make_pair(65, "Cabaret"));
	g_NumericGenresID3_1.insert(std::make_pair(66, "New Wave"));
	g_NumericGenresID3_1.insert(std::make_pair(67, "Psychadelic"));
	g_NumericGenresID3_1.insert(std::make_pair(68, "Rave"));
	g_NumericGenresID3_1.insert(std::make_pair(69, "Showtunes"));
	g_NumericGenresID3_1.insert(std::make_pair(70, "Trailer"));
	g_NumericGenresID3_1.insert(std::make_pair(71, "Lo-Fi"));
	g_NumericGenresID3_1.insert(std::make_pair(72, "Tribal"));
	g_NumericGenresID3_1.insert(std::make_pair(73, "Acid Punk"));
	g_NumericGenresID3_1.insert(std::make_pair(74, "Acid Jazz"));
	g_NumericGenresID3_1.insert(std::make_pair(75, "Polka"));
	g_NumericGenresID3_1.insert(std::make_pair(76, "Retro"));
	g_NumericGenresID3_1.insert(std::make_pair(77, "Musical"));
	g_NumericGenresID3_1.insert(std::make_pair(78, "Rock & Roll"));
	g_NumericGenresID3_1.insert(std::make_pair(79, "Hard Rock"));
	// numeric genres for Winamp extension
	g_NumericGenresWinamp.insert(std::make_pair(80, "Folk"));
	g_NumericGenresWinamp.insert(std::make_pair(81, "Folk-Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(82, "National Folk"));
	g_NumericGenresWinamp.insert(std::make_pair(83, "Swing"));
	g_NumericGenresWinamp.insert(std::make_pair(84, "Fast Fusion"));
	g_NumericGenresWinamp.insert(std::make_pair(85, "Bebob"));
	g_NumericGenresWinamp.insert(std::make_pair(86, "Latin"));
	g_NumericGenresWinamp.insert(std::make_pair(87, "Revival"));
	g_NumericGenresWinamp.insert(std::make_pair(88, "Celtic"));
	g_NumericGenresWinamp.insert(std::make_pair(89, "Bluegrass"));
	g_NumericGenresWinamp.insert(std::make_pair(90, "Avantgarde"));
	g_NumericGenresWinamp.insert(std::make_pair(91, "Gothic Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(92, "Progressive Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(93, "Psychedelic Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(94, "Symphonic Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(95, "Slow Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(96, "Big Band"));
	g_NumericGenresWinamp.insert(std::make_pair(97, "Chorus"));
	g_NumericGenresWinamp.insert(std::make_pair(98, "Easy Listening"));
	g_NumericGenresWinamp.insert(std::make_pair(99, "Acoustic"));
	g_NumericGenresWinamp.insert(std::make_pair(100, "Humour"));
	g_NumericGenresWinamp.insert(std::make_pair(101, "Speech"));
	g_NumericGenresWinamp.insert(std::make_pair(102, "Chanson"));
	g_NumericGenresWinamp.insert(std::make_pair(103, "Opera"));
	g_NumericGenresWinamp.insert(std::make_pair(104, "Chamber Music"));
	g_NumericGenresWinamp.insert(std::make_pair(105, "Sonata"));
	g_NumericGenresWinamp.insert(std::make_pair(106, "Symphony"));
	g_NumericGenresWinamp.insert(std::make_pair(107, "Booty Bass"));
	g_NumericGenresWinamp.insert(std::make_pair(108, "Primus"));
	g_NumericGenresWinamp.insert(std::make_pair(109, "Porn Groove"));
	g_NumericGenresWinamp.insert(std::make_pair(110, "Satire"));
	g_NumericGenresWinamp.insert(std::make_pair(111, "Slow Jam"));
	g_NumericGenresWinamp.insert(std::make_pair(112, "Club"));
	g_NumericGenresWinamp.insert(std::make_pair(113, "Tango"));
	g_NumericGenresWinamp.insert(std::make_pair(114, "Samba"));
	g_NumericGenresWinamp.insert(std::make_pair(115, "Folklore"));
	g_NumericGenresWinamp.insert(std::make_pair(116, "Ballad"));
	g_NumericGenresWinamp.insert(std::make_pair(117, "Power Ballad"));
	g_NumericGenresWinamp.insert(std::make_pair(118, "Rhythmic Soul"));
	g_NumericGenresWinamp.insert(std::make_pair(119, "Freestyle"));
	g_NumericGenresWinamp.insert(std::make_pair(120, "Duet"));
	g_NumericGenresWinamp.insert(std::make_pair(121, "Punk Rock"));
	g_NumericGenresWinamp.insert(std::make_pair(122, "Drum Solo"));
	g_NumericGenresWinamp.insert(std::make_pair(123, "Acapella"));
	g_NumericGenresWinamp.insert(std::make_pair(124, "Euro-House"));
	g_NumericGenresWinamp.insert(std::make_pair(125, "Dance Hall"));
	
	// encodings for version 2.2
	g_EncodingNames.insert(std::make_pair(TextEncoding::ISO_IEC_8859_1_1998, "ISO/IEC 8859-1:1998"));
	g_EncodingNames.insert(std::make_pair(TextEncoding::UCS_2, "ISO/IEC 10646-1:1993, UCS-2"));
	
	// country codes according to ISO 3166-1 alpha-2
	g_ISO_3166_1_Alpha_2_Codes.insert(std::make_pair("GB", "United Kingdom"));
	g_ISO_3166_1_Alpha_2_Codes.insert(std::make_pair("ZA", "South Africa"));
	
	// ID3v2.3.0
	FrameHeader::Handle23("APIC", "Attached picture", Handle23APICFrame);
	FrameHeader::Handle23("COMM", "Comments", Handle23COMMFrame);
	FrameHeader::Handle23("GEOB", "General encapsulated object", Handle23GEOB_Frame);
	FrameHeader::Handle23("MCDI", "Music CD identifier", Handle23MCDIFrame);
	FrameHeader::Handle23("PCNT", "Play counter", Handle23PCNTFrame);
	FrameHeader::Handle23("POPM", "Popularimeter", Handle23POPMFrame);
	FrameHeader::Handle23("PRIV", "Private frame", Handle23PRIVFrame);
	FrameHeader::Handle23("TALB", "Album/Movie/Show title", Handle23T___Frames);
	FrameHeader::Handle23("TBPM", "BPM (beats per minute)", Handle23T___Frames);
	FrameHeader::Handle23("TCOM", "Composer", Handle23T___Frames);
	FrameHeader::Handle23("TCON", "Content type", Handle23TCONFrame);
	FrameHeader::Handle23("TCOP", "Copyright message", Handle23T___Frames);
	FrameHeader::Handle23("TDAT", "Date", Handle23T___Frames);
	FrameHeader::Handle23("TENC", "Encoded by", Handle23T___Frames);
	FrameHeader::Handle23("TFLT", "File type", Handle23TFLTFrames);
	FrameHeader::Handle23("TIME", "Time", Handle23T___Frames);
	FrameHeader::Handle23("TIT1", "Content group description", Handle23T___Frames);
	FrameHeader::Handle23("TIT2", "Title/songname/content description", Handle23T___Frames);
	FrameHeader::Handle23("TIT3", "Subtitle/Description refinement", Handle23T___Frames);
	FrameHeader::Handle23("TLAN", "Language(s)", Handle23TLANFrames);
	FrameHeader::Handle23("TLEN", "Length", Handle23T___Frames);
	FrameHeader::Handle23("TMED", "Media type", Handle23T___Frames);
	FrameHeader::Handle23("TOAL", "Original album/movie/show title", Handle23T___Frames);
	FrameHeader::Handle23("TOFN", "Original filename", Handle23T___Frames);
	FrameHeader::Handle23("TOPE", "Original artist(s)/performer(s)", Handle23T___Frames);
	FrameHeader::Handle23("TOWN", "File owner/licensee", Handle23T___Frames);
	FrameHeader::Handle23("TPE1", "Lead Performer(s) / Solo Artist(s)", Handle23T___Frames);
	FrameHeader::Handle23("TPE2", "Band / Orchestra / Accompaniment", Handle23T___Frames);
	FrameHeader::Handle23("TPE3", "Conductor / Performer Refinement", Handle23T___Frames);
	FrameHeader::Handle23("TPE4", "Interpreted, Remixed, or otherwise modified by", Handle23T___Frames);
	FrameHeader::Handle23("TPOS", "Part of a set", Handle23T___Frames);
	FrameHeader::Handle23("TPUB", "Publisher", Handle23T___Frames);
	FrameHeader::Handle23("TRCK", "Track number/Position in set", Handle23T___Frames);
	FrameHeader::Handle23("TRDA", "Recording dates", Handle23T___Frames);
	FrameHeader::Handle23("TSIZ", "Size", Handle23T___Frames);
	FrameHeader::Handle23("TSRC", "ISRC (international standard recording code)", Handle23TSRCFrame);
	FrameHeader::Handle23("TSSE", "Software/Hardware and settings used for encoding", Handle23T___Frames);
	FrameHeader::Handle23("TXXX", "User defined text information frame", Handle23TXXXFrame);
	FrameHeader::Handle23("TYER", "Year", Handle23T___Frames);
	FrameHeader::Handle23("USLT", "Unsynchronised lyrics/text transcription", Handle23USLTFrame);
	FrameHeader::Handle23("UFID", "Unique file identifier", Handle23UFIDFrame);
	FrameHeader::Handle23("WCOM", "Commercial information", Handle23W___Frames);
	FrameHeader::Handle23("WOAF", "Official audio file webpage", Handle23W___Frames);
	FrameHeader::Handle23("WOAR", "Official artist/performer webpage", Handle23W___Frames);
	FrameHeader::Handle23("WXXX", "User defined URL link frame", Handle23WXXXFrame);
	// forbidden tags
	FrameHeader::Forbid23("MJCF", "This frame is not defined in tag version 2.3. It is a non-standard frame added by the MediaJukebox.");
	FrameHeader::Handle23("MJCF", "Mediajukebox", Handle23MJCFFrame);
	FrameHeader::Forbid23("RGAD", "This frame is not defined in tag version 2.3. It is a non-standard frame which is acknowledged as an 'in the wild' tag by id3.org.");
	FrameHeader::Handle23("RGAD", "Replay Gain Adjustment", Handle23RGADFrame);
	FrameHeader::Forbid23("TCMP", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate whether a title is a part of a compilation.");
	FrameHeader::Handle23("TCMP", "Part of a compilation (by iTunes)", Handle23TCMPFrame);
	FrameHeader::Forbid23("TDRC", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDRC", "Recording time (from tag version 2.4)", Handle23T___Frames);
	FrameHeader::Forbid23("TDTG", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDTG", "Tagging time (from tag version 2.4)", Handle23T___Frames);
	FrameHeader::Forbid23("TSST", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TSST", "Set subtitle (from tag version 2.4)", Handle23T___Frames);
	FrameHeader::Forbid23("TSOA", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TSOA", "Album sort order (from tag version 2.4)", Handle23T___Frames);
	FrameHeader::Forbid23("TSOP", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TSOP", "Performer sort order (from tag version 2.4)", Handle23T___Frames);
	FrameHeader::Forbid23("TSO2", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate the Album Artist sort order.");
	FrameHeader::Handle23("TSO2", "Album artist sort order (by iTunes)", Handle23T___Frames);
	
	// processing
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}

	return 0;
}
