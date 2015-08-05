#include <assert.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <functional>
#include <list>
#include <map>
#include <sstream>
#include <vector>

class CDTableOfContents
{
public:
	class TrackDescriptor
	{
	public:
		uint32_t Reserved1;
		uint32_t ADR;
		bool HasFourChannels;
		bool IsDataTrack;
		bool IsDigitalCopyPermitted;
		bool AudioTrackWithEmphasisOrIncrementalDataTrack;
		uint32_t TrackNumber;
		uint32_t Reserved2;
		uint32_t TrackStartAddress;
	};
	
	uint32_t DataLength;
	uint32_t FirstTrackNumber;
	uint32_t LastTrackNumber;
	std::list< TrackDescriptor > TrackDescriptors;
};

enum class TextEncoding
{
	Undefined,
	ISO_IEC_8859_1_1998,
	UCS_2,
	UTF_16,
	UTF_16_BE,
	UTF_8
};

enum class UCS2ByteOrderMark
{
	Undefined,
	LittleEndian,
	BigEndian
};

enum class UTF16ByteOrderMark
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
std::map< std::string, std::string > g_ISO_639_2_Codes;
std::map< std::string, std::string > g_ISO_3166_1_Alpha_2_Codes;
std::map< unsigned int, std::string > g_PictureTypes;
std::map< TextEncoding, std::string > g_EncodingNames;
std::map< std::string, std::string > g_GUIDDescriptions;
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

std::string Get_UTF_8_CharacterFromUnicodeCodepoint(unsigned int Codepoint)
{
	std::string Result;
	
	if(Codepoint < 0x00000080)
	{
		Result += static_cast< char >(Codepoint & 0x0000007f);
	}
	else if(Codepoint < 0x00000800)
	{
		Result += static_cast< char >(0x00000c0 + ((Codepoint & 0x00000700) >> 6) + ((Codepoint & 0x000000c0) >> 6));
		Result += static_cast< char >(0x0000080 + (Codepoint & 0x0000003f));
	}
	else if(Codepoint < 0x00010000)
	{
		Result += static_cast< char >(0x00000e0 + ((Codepoint & 0x0000f000) >> 12));
		Result += static_cast< char >(0x0000080 + ((Codepoint & 0x00000f00) >> 6) + ((Codepoint & 0x000000c0) >> 6));
		Result += static_cast< char >(0x0000080 + (Codepoint & 0x0000003f));
	}
	else if(Codepoint < 0x00110000)
	{
		Result += static_cast< char >(0x00000f0 + ((Codepoint & 0x001c0000) >> 18));
		Result += static_cast< char >(0x0000080 + ((Codepoint & 0x00030000) >> 12) + ((Codepoint & 0x0000f000) >> 12));
		Result += static_cast< char >(0x0000080 + ((Codepoint & 0x00000f00) >> 6) + ((Codepoint & 0x000000c0) >> 6));
		Result += static_cast< char >(0x0000080 + (Codepoint & 0x0000003f));
	}
	else
	{
		std::cout << "*** ERROR *** Codepoint 0x" << std::hex << std::setfill('0') << std::setw(8) << std::right << Codepoint << " could not be encoded as UTF-8." << std::endl;
	}
	
	return Result;
}

std::string Get_UTF_8_CharacterFrom_UCS_2_BE_Character(const uint8_t * Buffer, int Length)
{
	assert(Length >= 2);
	
	return Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])));
}

std::string Get_UTF_8_CharacterFrom_UCS_2_LE_Character(const uint8_t * Buffer, int Length)
{
	assert(Length >= 2);
	
	return Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
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

std::pair< bool, std::string > GetInterpretation(const std::string & Content, const std::string & InterpretationKey, const std::string & InterpretationValue)
{
	if(Content == InterpretationKey)
	{
		return std::make_pair(true, InterpretationValue);
	}
	else
	{
		return std::make_pair(false, "");
	}
}

std::string GetEncodingName(TextEncoding Encoding)
{
	std::stringstream Result;
	std::map< TextEncoding, std::string >::const_iterator EncodingIterator(g_EncodingNames.find(Encoding));
	
	if(EncodingIterator != g_EncodingNames.end())
	{
		Result << EncodingIterator->second;
	}
	else
	{
		Result << "<invalid encoding>";
	}
	
	return Result.str();
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

std::tuple< bool, bool, std::string > GetFileTypeInterpretation2_3(const std::string FileType)
{
	std::pair< bool, std::string > Interpretation(false, "");
	bool Strict(true);
	
	Interpretation = GetInterpretation(FileType, "MPG", "MPEG Audio");
	if(std::get<0>(Interpretation) == false)
	{
		Interpretation = GetInterpretation(FileType, "MPG/1", "MPEG Audio, MPEG 1/2 layer I");
		if(std::get<0>(Interpretation) == false)
		{
			Interpretation = GetInterpretation(FileType, "MPG/2", "MPEG Audio, MPEG 1/2 layer II");
			if(std::get<0>(Interpretation) == false)
			{
				Interpretation = GetInterpretation(FileType, "MPG/3", "MPEG Audio, MPEG 1/2 layer III");
				if(std::get<0>(Interpretation) == false)
				{
					Interpretation = GetInterpretation(FileType, "MPG/2.5", "MPEG Audio, MPEG 2.5");
					if(std::get<0>(Interpretation) == false)
					{
						Interpretation = GetInterpretation(FileType, "AAC", "MPEG Audio, Advanced audio compression");
						if(std::get<0>(Interpretation) == false)
						{
							Interpretation = GetInterpretation(FileType, "VQF", "Transform-domain Weighted Interleave Vector Quantization");
							if(std::get<0>(Interpretation) == false)
							{
								Interpretation = GetInterpretation(FileType, "PCM", "Pulse Code Modulated audio");
								if(std::get<0>(Interpretation) == false)
								{
									Strict = false;
									Interpretation = GetInterpretation(FileType, "/3", "MPEG Audio, MPEG 1/2 layer III");
								}
							}
						}
					}
				}
			}
		}
	}
	
	return std::make_tuple(std::get<0>(Interpretation), Strict, std::get<1>(Interpretation));
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

std::string GetPictureTypeString(unsigned int PictureType)
{
	std::stringstream Result;
	std::map< unsigned int, std::string >::iterator PictureTypeIterator(g_PictureTypes.find(PictureType));
	
	if(PictureTypeIterator != g_PictureTypes.end())
	{
		Result << PictureTypeIterator->second;
	}
	else
	{
		Result << "<invalid picture type>";
	}
	Result << " (" << PictureType << ")";
	
	return Result.str();
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

std::pair< int, std::string > GetGUIDString(const uint8_t * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	if(Length >= 16)
	{
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromUInt8(Buffer[Result.first++]);
	}
	else
	{
		std::cout << "*** ERROR *** A GUID must be 16 bytes long but this one only has space for " << Length << " bytes." << std::endl;
	}
	
	return Result;
}

std::tuple< bool, int, uint8_t > GetHexadecimalValueFromHexadecimalCharacter(const std::string & String, int Offset)
{
	std::tuple< bool, int, uint8_t > Result(false, 0, 0);
	
	if(String.length() > 0)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		if(String[Offset] == '0')
		{
			std::get<2>(Result) = 0;
		}
		else if(String[Offset] == '1')
		{
			std::get<2>(Result) = 1;
		}
		else if(String[Offset] == '2')
		{
			std::get<2>(Result) = 2;
		}
		else if(String[Offset] == '3')
		{
			std::get<2>(Result) = 3;
		}
		else if(String[Offset] == '4')
		{
			std::get<2>(Result) = 4;
		}
		else if(String[Offset] == '5')
		{
			std::get<2>(Result) = 5;
		}
		else if(String[Offset] == '6')
		{
			std::get<2>(Result) = 6;
		}
		else if(String[Offset] == '7')
		{
			std::get<2>(Result) = 7;
		}
		else if(String[Offset] == '8')
		{
			std::get<2>(Result) = 8;
		}
		else if(String[Offset] == '9')
		{
			std::get<2>(Result) = 9;
		}
		else if((String[Offset] == 'a') || (String[Offset] == 'A'))
		{
			std::get<2>(Result) = 10;
		}
		else if((String[Offset] == 'b') || (String[Offset] == 'B'))
		{
			std::get<2>(Result) = 11;
		}
		else if((String[Offset] == 'c') || (String[Offset] == 'C'))
		{
			std::get<2>(Result) = 12;
		}
		else if((String[Offset] == 'd') || (String[Offset] == 'D'))
		{
			std::get<2>(Result) = 13;
		}
		else if((String[Offset] == 'e') || (String[Offset] == 'E'))
		{
			std::get<2>(Result) = 14;
		}
		else if((String[Offset] == 'f') || (String[Offset] == 'F'))
		{
			std::get<2>(Result) = 15;
		}
		else
		{
			std::get<0>(Result) = false;
		}
	}
	
	return Result;
}

std::tuple< bool, int, uint32_t > GetUInt32NumberFromUnformattedHexadecimalString(const std::string & String, int Offset)
{
	std::tuple< bool, int, uint32_t > Result(false, 0, 0);
	int StringLength(String.length());
	
	while(Offset + std::get<1>(Result) < StringLength)
	{
		auto HexadecimalDigit(GetHexadecimalValueFromHexadecimalCharacter(String, Offset + std::get<1>(Result)));
		
		if(std::get<0>(HexadecimalDigit) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(HexadecimalDigit);
			std::get<2>(Result) = (std::get<2>(Result) << 4) + std::get<2>(HexadecimalDigit);
		}
		else
		{
			break;
		}
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
std::tuple< bool, int, bool > Get_Boolean_0(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_1(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_2(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_3(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_4(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_5(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_6(const uint8_t * Buffer, int Length);
std::tuple< bool, int, bool > Get_Boolean_7(const uint8_t * Buffer, int Length);
std::tuple< bool, int, CDTableOfContents > Get_CDTableOfContents(const uint8_t * Buffer, int Length);
std::tuple< bool, int, TextEncoding > Get_ID3_2_2_Encoding(const uint8_t * Buffer, int Length);
std::tuple< bool, int, TextEncoding > Get_ID3_2_3_Encoding(const uint8_t * Buffer, int Length);
std::tuple< bool, int, TextEncoding > Get_ID3_2_4_Encoding(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_Character(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByBoundary(const uint8_t * Buffer, int Length, int Boundary);
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByLength(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByTermination(const uint8_t * Buffer, int Length);
std::tuple< bool, int > Get_ISO_IEC_8859_1_Termination(const uint8_t * Buffer, int Length);
std::tuple< bool, int, float > Get_ISO_IEC_IEEE_60559_2011_binary32(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint8_t > Get_UInt4_0_3(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint8_t > Get_UInt4_4_7(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint8_t > Get_UInt8(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint16_t > Get_UInt16_BE(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint16_t > Get_UInt16_LE(const uint8_t * Buffer, int Length);
std::tuple< bool, int, uint32_t > Get_UInt32_BE(const uint8_t * Buffer, int Length);
std::tuple< bool, int, UTF16ByteOrderMark > Get_UTF_16_ByteOrderMark(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16_StringWithByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16_StringWithByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length);
std::tuple< bool, int > Get_UTF_16_Termination(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16BE_Character(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16BE_StringWithoutByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16LE_Character(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16LE_StringWithoutByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_16LE_StringWithoutByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_8_Character(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_8_StringEndedByLength(const uint8_t * Buffer, int Length);
std::tuple< bool, int, std::string > Get_UTF_8_StringEndedByTermination(const uint8_t * Buffer, int Length);
std::tuple< bool, int > Get_UTF_8_Termination(const uint8_t * Buffer, int Length);
std::tuple< bool, int > Get_Zeroes_EndedByLength(const uint8_t * Buffer, int Length);


std::tuple< bool, int, bool > Get_Boolean_0(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 7) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_1(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 6) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_2(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 5) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_3(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 4) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_4(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 3) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_5(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 2) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_6(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = ((Buffer[0] >> 1) & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, bool > Get_Boolean_7(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, bool > Result(false, 0, false);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = (Buffer[0] & 0x01) == 0x01;
	}
	
	return Result;
}

std::tuple< bool, int, TextEncoding > Get_ID3_2_2_Encoding(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, TextEncoding > Result(false, 0, TextEncoding::Undefined);
	
	if(Length >= 1)
	{
		if(Buffer[0] == 0x00)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::ISO_IEC_8859_1_1998;
		}
		else if(Buffer[0] == 0x01)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::UCS_2;
		}
	}
	
	return Result;
}

std::tuple< bool, int, TextEncoding > Get_ID3_2_3_Encoding(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, TextEncoding > Result(false, 0, TextEncoding::Undefined);
	
	if(Length >= 1)
	{
		if(Buffer[0] == 0x00)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::ISO_IEC_8859_1_1998;
		}
		else if(Buffer[0] == 0x01)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::UCS_2;
		}
	}
	
	return Result;
}

std::tuple< bool, int, TextEncoding > Get_ID3_2_4_Encoding(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, TextEncoding > Result(false, 0, TextEncoding::Undefined);
	
	if(Length >= 1)
	{
		if(Buffer[0] == 0x00)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::ISO_IEC_8859_1_1998;
		}
		else if(Buffer[0] == 0x01)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::UTF_16;
		}
		else if(Buffer[0] == 0x02)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::UTF_16_BE;
		}
		else if(Buffer[0] == 0x03)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = TextEncoding::UTF_8;
		}
	}
	
	return Result;
}

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
		std::get<2>(Result) = Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_ISO_IEC_8859_1_StringEndedByBoundary(const uint8_t * Buffer, int Length, int Boundary)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if(Length >= Boundary)
	{
		Result = Get_ISO_IEC_8859_1_StringEndedByLength(Buffer, Boundary);
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

std::tuple< bool, int, CDTableOfContents > Get_CDTableOfContents(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, CDTableOfContents > Result(true, 0, CDTableOfContents());
	
	int & Index(std::get<1>(Result));
	CDTableOfContents & TableOfContents(std::get<2>(Result));
	
	auto DataLength(Get_UInt16_BE(Buffer + Index, Length - Index));
	
	if(std::get<0>(DataLength) == true)
	{
		Index += std::get<1>(DataLength);
		TableOfContents.DataLength = std::get<2>(DataLength);
		
		auto FirstTrackNumber(Get_UInt8(Buffer + Index, Length - Index));
		
		if(std::get<0>(FirstTrackNumber) == true)
		{
			Index += std::get<1>(FirstTrackNumber);
			TableOfContents.FirstTrackNumber = std::get<2>(FirstTrackNumber);
		
			auto LastTrackNumber(Get_UInt8(Buffer + Index, Length - Index));
			
			if(std::get<0>(LastTrackNumber) == true)
			{
				Index += std::get<1>(LastTrackNumber);
				TableOfContents.LastTrackNumber = std::get<2>(LastTrackNumber);
				std::get<0>(Result) = true;
				while(true)
				{
					CDTableOfContents::TrackDescriptor TrackDescriptor;
					auto Reserved1(Get_UInt8(Buffer + Index, Length - Index));
					
					if(std::get<0>(Reserved1) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					Index += std::get<1>(Reserved1);
					TrackDescriptor.Reserved1 = std::get<2>(Reserved1);
					
					auto ADR(Get_UInt4_0_3(Buffer + Index, Length - Index));
					
					if(std::get<0>(ADR) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					TrackDescriptor.ADR = std::get<2>(ADR);
					
					auto HasFourChannels(Get_Boolean_4(Buffer + Index, Length - Index));
					
					if(std::get<0>(HasFourChannels) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					TrackDescriptor.HasFourChannels = std::get<2>(HasFourChannels);
					
					auto IsDataTrack(Get_Boolean_5(Buffer + Index, Length - Index));
					
					if(std::get<0>(IsDataTrack) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					TrackDescriptor.IsDataTrack = std::get<2>(IsDataTrack);
					
					auto IsDigitalCopyPermitted(Get_Boolean_6(Buffer + Index, Length - Index));
					
					if(std::get<0>(IsDigitalCopyPermitted) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					TrackDescriptor.IsDigitalCopyPermitted = std::get<2>(IsDigitalCopyPermitted);
					
					auto AudioTrackWithEmphasisOrIncrementalDataTrack(Get_Boolean_7(Buffer + Index, Length - Index));
					
					if(std::get<0>(AudioTrackWithEmphasisOrIncrementalDataTrack) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					TrackDescriptor.AudioTrackWithEmphasisOrIncrementalDataTrack = std::get<2>(AudioTrackWithEmphasisOrIncrementalDataTrack);
					Index += std::get<1>(ADR);
					
					auto TrackNumber(Get_UInt8(Buffer + Index, Length - Index));
					
					if(std::get<0>(TrackNumber) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					Index += std::get<1>(TrackNumber);
					TrackDescriptor.TrackNumber = std::get<2>(TrackNumber);
					
					auto Reserved2(Get_UInt8(Buffer + Index, Length - Index));
					
					if(std::get<0>(Reserved2) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					Index += std::get<1>(Reserved2);
					TrackDescriptor.Reserved2 = std::get<2>(Reserved2);
					
					auto TrackStartAddress(Get_UInt32_BE(Buffer + Index, Length - Index));
					
					if(std::get<0>(TrackStartAddress) == false)
					{
						std::get<0>(Result) = false;
						
						break;
					}
					Index += std::get<1>(TrackStartAddress);
					TrackDescriptor.TrackStartAddress = std::get<2>(TrackStartAddress);
					TableOfContents.TrackDescriptors.push_back(TrackDescriptor);
					if(std::get<2>(TrackNumber) == 0xAA)
					{
						break;
					}
				}
			}
		}
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
		std::get<2>(Result) = Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])));
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
		std::get<2>(Result) = Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
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

std::tuple< bool, int, uint8_t > Get_UInt4_0_3(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, uint8_t > Result(false, 0, 0);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = (Buffer[0] >> 4) & 0x0F;
	}
	
	return Result;
}

std::tuple< bool, int, uint8_t > Get_UInt4_4_7(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, uint8_t > Result(false, 0, 0);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = Buffer[0] & 0x0F;
	}
	
	return Result;
}

std::tuple< bool, int, uint8_t > Get_UInt8(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, uint8_t > Result(false, 0, 0);
	
	if(Length >= 1)
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
		std::get<2>(Result) = Buffer[0];
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

std::tuple< bool, int, UTF16ByteOrderMark > Get_UTF_16_ByteOrderMark(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, UTF16ByteOrderMark > Result(false, 0, UTF16ByteOrderMark::Undefined);
	
	if(Length >= 2)
	{
		if((Buffer[0] == 0xfe) && (Buffer[1] == 0xff))
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 2;
			std::get<2>(Result) = UTF16ByteOrderMark::BigEndian;
		}
		else if((Buffer[0] == 0xff) && (Buffer[1] == 0xfe))
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 2;
			std::get<2>(Result) = UTF16ByteOrderMark::LittleEndian;
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16_StringWithByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	auto ByteOrderMark(Get_UTF_16_ByteOrderMark(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
	
	if(std::get<0>(ByteOrderMark) == true)
	{
		std::get<1>(Result) += std::get<1>(ByteOrderMark);
		if(std::get<2>(ByteOrderMark) == UTF16ByteOrderMark::BigEndian)
		{
			auto String(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByLength(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(String) == true)
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) += std::get<1>(String);
				std::get<2>(Result) = std::get<2>(String);
			}
		}
		else if(std::get<2>(ByteOrderMark) == UTF16ByteOrderMark::LittleEndian)
		{
			auto String(Get_UTF_16LE_StringWithoutByteOrderMarkEndedByLength(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(String) == true)
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) += std::get<1>(String);
				std::get<2>(Result) = std::get<2>(String);
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16_StringWithByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	auto ByteOrderMark(Get_UTF_16_ByteOrderMark(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
	
	if(std::get<0>(ByteOrderMark) == true)
	{
		std::get<1>(Result) += std::get<1>(ByteOrderMark);
		if(std::get<2>(ByteOrderMark) == UTF16ByteOrderMark::BigEndian)
		{
			auto String(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(String) == true)
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) += std::get<1>(String);
				std::get<2>(Result) = std::get<2>(String);
			}
		}
		else if(std::get<2>(ByteOrderMark) == UTF16ByteOrderMark::LittleEndian)
		{
			auto String(Get_UTF_16LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
			if(std::get<0>(String) == true)
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) += std::get<1>(String);
				std::get<2>(Result) = std::get<2>(String);
			}
		}
	}
	
	return Result;
}

std::tuple< bool, int > Get_UTF_16_Termination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int > Result(false, 0);
	
	if((Length >= 2) && (Buffer[0] == 0x00) && (Buffer[1] == 0x00))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 2;
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16BE_Character(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if(Length >= 2)
	{
		if((Buffer[0] < 0xd8) || (Buffer[0] > 0xdf))
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 2;
			std::get<2>(Result) = Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])));
		}
		else
		{
			/// @TODO
			assert(false);
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16BE_StringWithoutByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UTF_16_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == false)
		{
			auto Character(Get_UTF_16BE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
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
		else
		{
			std::get<0>(Result) = false;
			
			break;
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UTF_16_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_UTF_16BE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
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

std::tuple< bool, int, std::string > Get_UTF_16LE_Character(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if(Length >= 2)
	{
		if((Buffer[1] < 0xd8) || (Buffer[1] > 0xdf))
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 2;
			std::get<2>(Result) = Get_UTF_8_CharacterFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
		}
		else
		{
			/// @TODO
			assert(false);
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16LE_StringWithoutByteOrderMarkEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UTF_16_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == false)
		{
			auto Character(Get_UTF_16LE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
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
		else
		{
			std::get<0>(Result) = false;
			
			break;
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_16LE_StringWithoutByteOrderMarkEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UTF_16_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_UTF_16LE_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
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

std::tuple< bool, int, std::string > Get_UTF_8_Character(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	if(Length >= 1)
	{
		if((Buffer[0] & 0xf8) == 0xf0)
		{
			if((Length >= 4) && ((Buffer[1] & 0xc0) == 0x80) && ((Buffer[2] & 0xc0) == 0x80) && ((Buffer[3] & 0xc0) == 0x80))
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) = 4;
				std::get<2>(Result) = std::string(Buffer, Buffer + 4);
			}
		}
		else if((Buffer[0] & 0xf0) == 0xe0)
		{
			if((Length >= 3) && ((Buffer[1] & 0xc0) == 0x80) && ((Buffer[2] & 0xc0) == 0x80))
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) = 3;
				std::get<2>(Result) = std::string(Buffer, Buffer + 3);
			}
		}
		else if((Buffer[0] & 0xe0) == 0xc0)
		{
			if((Length >= 2) && ((Buffer[1] & 0xc0) == 0x80))
			{
				std::get<0>(Result) = true;
				std::get<1>(Result) = 2;
				std::get<2>(Result) = std::string(Buffer, Buffer + 2);
			}
		}
		else if((Buffer[0] & 0x80) == 0x00)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) = 1;
			std::get<2>(Result) = std::string(Buffer, Buffer + 1);
		}
	}
	
	return Result;
}

std::tuple< bool, int, std::string > Get_UTF_8_StringEndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(true, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Character(Get_UTF_8_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
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

std::tuple< bool, int, std::string > Get_UTF_8_StringEndedByTermination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int, std::string > Result(false, 0, "");
	
	while(std::get<1>(Result) < Length)
	{
		auto Termination(Get_UTF_8_Termination(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
		
		if(std::get<0>(Termination) == true)
		{
			std::get<0>(Result) = true;
			std::get<1>(Result) += std::get<1>(Termination);
			
			break;
		}
		else
		{
			auto Character(Get_UTF_8_Character(Buffer + std::get<1>(Result), Length - std::get<1>(Result)));
			
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

std::tuple< bool, int > Get_UTF_8_Termination(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int > Result(false, 0);
	
	if((Length >= 1) && (Buffer[0] == 0x00))
	{
		std::get<0>(Result) = true;
		std::get<1>(Result) = 1;
	}
	
	return Result;
}

std::tuple< bool, int > Get_Zeroes_EndedByLength(const uint8_t * Buffer, int Length)
{
	std::tuple< bool, int > Result(true, 0);
	
	while(std::get<1>(Result) < Length)
	{
		if(Buffer[std::get<1>(Result)] == 0x00)
		{
			std::get<1>(Result) += 1;
		}
		else
		{
			std::get<0>(Result) = false;
			
			break;
		}
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Tag and Frame header classes                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

class FrameHeader;

class TagHeader
{
public:
	// constructor
	TagHeader(std::istream & Stream)
	{
		Stream.read(_Buffer, 10);
	}
	
	// getters
	bool GetCompression(void) const
	{
		if(SupportsCompressionFlag() == true)
		{
			return (_Buffer[5] & 0x40) == 0x40;
		}
		else
		{
			throw "The compression flag is not supported by this tag version.";
		}
	}
	
	bool GetExperimentalIndicator(void) const
	{
		if(SupportsExperimentalIndicatorFlag() == true)
		{
			return (_Buffer[5] & 0x20) == 0x20;
		}
		else
		{
			throw "The experimental indicator flag is not supported by this tag version.";
		}
	}
	
	bool GetExtendedHeader(void) const
	{
		if(SupportsExtendedHeaderFlag() == true)
		{
			return (_Buffer[5] & 0x40) == 0x40;
		}
		else
		{
			throw "The extended header flag is not supported by this tag version.";
		}
	}
	
	std::string GetFlagsAsString(void) const
	{
		std::string Result;
		
		if((SupportsUnsynchronizationFlag() == true) && (GetUnsynchronization() == true))
		{
			AppendSeparated(Result, "Unsynchronization", ", ");
		}
		if((SupportsCompressionFlag() == true) && (GetCompression() == true))
		{
			AppendSeparated(Result, "Compression", ", ");
		}
		if((SupportsExtendedHeaderFlag() == true) && (GetExtendedHeader() == true))
		{
			AppendSeparated(Result, "Extended Header", ", ");
		}
		if((SupportsExperimentalIndicatorFlag() == true) && (GetExperimentalIndicator() == true))
		{
			AppendSeparated(Result, "Experimental Indicator", ", ");
		}
		if((SupportsFooterPresentFlag() == true) && (GetFooterPresent() == true))
		{
			AppendSeparated(Result, "Footer present", ", ");
		}
		if(Result.empty() == true)
		{
			Result = "None";
		}
		
		return Result;
	}
	
	bool GetFooterPresent(void) const
	{
		if(SupportsFooterPresentFlag() == true)
		{
			return (_Buffer[5] & 0x10) == 0x10;
		}
		else
		{
			throw "The footer present flag is not supported by this tag version.";
		}
	}
	
	std::string GetID3Identifier(void) const
	{
		return std::string(_Buffer, 3);
	}
	
	unsigned int GetMajorVersion(void) const
	{
		return static_cast< unsigned int >(static_cast< unsigned char >(_Buffer[3]));
	}
	
	unsigned int GetRevisionNumber(void) const
	{
		return static_cast< unsigned int >(static_cast< unsigned char >(_Buffer[4]));
	}
	
	unsigned int GetSize(void) const
	{
		return (static_cast< unsigned int >(static_cast< unsigned char >(_Buffer[6])) << 21) + (static_cast< unsigned int >(static_cast< unsigned char >(_Buffer[7])) << 14) + (static_cast< unsigned int >(static_cast< unsigned char >(_Buffer[8])) << 7) + static_cast< unsigned int >(static_cast< unsigned char >(_Buffer[9]));
	}
	
	bool GetUnsynchronization(void) const
	{
		return (_Buffer[5] & 0x80) == 0x80;
	}
	
	bool SupportsExperimentalIndicatorFlag(void) const
	{
		return GetMajorVersion() > 2;
	}
	
	bool SupportsExtendedHeaderFlag(void) const
	{
		return GetMajorVersion() > 2;
	}
	
	bool SupportsFooterPresentFlag(void) const
	{
		return GetMajorVersion() > 3;
	}
	
	bool SupportsCompressionFlag(void) const
	{
		return GetMajorVersion() == 2;
	}
	
	bool SupportsUnsynchronizationFlag(void) const
	{
		return true;
	}
private:
	char _Buffer[10];
};

class FrameHeader
{
public:
	// constructor
	FrameHeader(TagHeader * TagHeader, std::istream & Stream) :
		_Compression(false),
		_DataLengthIndicator(false),
		_Encryption(false),
		_FileAlterPreservation(false),
		_Forbidden(false),
		_GroupingIdentity(false),
		_Handler(0),
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
		if(TagHeader->GetMajorVersion() == 2)
		{
			char Buffer[6];
			
			Stream.read(Buffer, 6);
			_Identifier = std::string(Buffer, 3);
			_Name = _Names22[_Identifier];
			_Size = (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[3])) << 14) + (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[4])) << 7) + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[5]));
			_SupportsFlags = false;
			
			std::map< std::string, std::string >::iterator ForbiddenIterator(_Forbidden22.find(_Identifier));
			
			if(ForbiddenIterator != _Forbidden22.end())
			{
				_Forbidden = true;
				_ForbiddenReason = ForbiddenIterator->second;
			}
			
			std::map< std::string, int (*)(const uint8_t *, int) >::iterator HanderIterator(_Handlers22.find(_Identifier));
			
			if(HanderIterator != _Handlers22.end())
			{
				_Handler = HanderIterator->second;
			}
		}
		else if(TagHeader->GetMajorVersion() == 3)
		{
			char Buffer[10];
			
			Stream.read(Buffer, 10);
			_Identifier = std::string(Buffer, 4);
			_Name = _Names23[_Identifier];
			_Size = (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[4])) << 24) + (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[5])) << 16) + (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[6])) << 8) + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[7]));
			_SupportsFlags = true;
			_SupportsTagAlterPreservation = true;
			_TagAlterPreservation = (Buffer[8] & 0x80) == 0x80;
			_SupportsFileAlterPreservation = true;
			_FileAlterPreservation = (Buffer[8] & 0x40) == 0x40;
			_SupportsReadOnly = true;
			_ReadOnly = (Buffer[8] & 0x20) == 0x20;
			_SupportsCompression = true;
			_Compression = (Buffer[9] & 0x80) == 0x80;
			_SupportsEncryption = true;
			_Encryption = (Buffer[9] & 0x40) == 0x40;
			_SupportsGroupingIdentity = true;
			_GroupingIdentity = (Buffer[9] & 0x20) == 0x20;
			_SupportsUnsynchronisation = false;
			_SupportsDataLengthIndicator = false;
			
			std::map< std::string, std::string >::iterator ForbiddenIterator(_Forbidden23.find(_Identifier));
			
			if(ForbiddenIterator != _Forbidden23.end())
			{
				_Forbidden = true;
				_ForbiddenReason = ForbiddenIterator->second;
			}
			
			std::map< std::string, int (*)(const uint8_t *, int) >::iterator HanderIterator(_Handlers23.find(_Identifier));
			
			if(HanderIterator != _Handlers23.end())
			{
				_Handler = HanderIterator->second;
			}
		}
		else if(TagHeader->GetMajorVersion() == 4)
		{
			char Buffer[10];
			
			Stream.read(Buffer, 10);
			_Identifier = std::string(Buffer, 4);
			_Name = _Names24[_Identifier];
			_Size = (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[4])) << 21) + (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[5])) << 14) + (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[6])) << 7) + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[7]));
			_SupportsFlags = true;
			_SupportsTagAlterPreservation = true;
			_TagAlterPreservation = (Buffer[8] & 0x40) == 0x40;
			_SupportsFileAlterPreservation = true;
			_FileAlterPreservation = (Buffer[8] & 0x20) == 0x20;
			_SupportsReadOnly = true;
			_ReadOnly = (Buffer[8] & 0x10) == 0x10;
			_SupportsGroupingIdentity = true;
			_GroupingIdentity = (Buffer[9] & 0x40) == 0x40;
			_SupportsCompression = true;
			_Compression = (Buffer[9] & 0x08) == 0x08;
			_SupportsEncryption = true;
			_Encryption = (Buffer[9] & 0x04) == 0x04;
			_SupportsUnsynchronisation = true;
			_Unsynchronisation = (Buffer[9] & 0x02) == 0x02;
			_SupportsDataLengthIndicator = true;
			_DataLengthIndicator = (Buffer[9] & 0x01) == 0x01;
			
			std::map< std::string, std::string >::iterator ForbiddenIterator(_Forbidden24.find(_Identifier));
			
			if(ForbiddenIterator != _Forbidden24.end())
			{
				_Forbidden = true;
				_ForbiddenReason = ForbiddenIterator->second;
			}
			
			std::map< std::string, int (*)(const uint8_t *, int) >::iterator HanderIterator(_Handlers24.find(_Identifier));
			
			if(HanderIterator != _Handlers24.end())
			{
				_Handler = HanderIterator->second;
			}
		}
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
	
	unsigned int GetSize(void) const
	{
		return _Size;
	}
	
	bool GetTagAlterPreservation(void) const
	{
		return _TagAlterPreservation;
	}
	
	bool GetUnsynchronisation(void) const
	{
		return _Unsynchronisation;
	}
	
	int HandleData(const uint8_t * Buffer, unsigned int Length)
	{
		if(_Handler != 0)
		{
			return _Handler(Buffer, Length);
		}
		else
		{
			std::cout << "*** ERROR ***  No handler defined for the frame type \"" << _Identifier << "\" in this tag version." << std::endl;
			
			return Length;
		}
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
	static void Forbid22(const std::string & Identifier, const std::string & Reason)
	{
		_Forbidden22.insert(std::make_pair(Identifier, Reason));
	}
	
	static void Handle22(const std::string & Identifier, const std::string & Name, int (* Handler) (const uint8_t *, int))
	{
		_Handlers22.insert(std::make_pair(Identifier, Handler));
		_Names22.insert(std::make_pair(Identifier, Name));
	}
	
	static void Forbid23(const std::string & Identifier, const std::string & Reason)
	{
		_Forbidden23.insert(std::make_pair(Identifier, Reason));
	}
	
	static void Handle23(const std::string & Identifier, const std::string & Name, int (* Handler) (const uint8_t *, int))
	{
		_Handlers23.insert(std::make_pair(Identifier, Handler));
		_Names23.insert(std::make_pair(Identifier, Name));
	}
	
	static void Forbid24(const std::string & Identifier, const std::string & Reason)
	{
		_Forbidden24.insert(std::make_pair(Identifier, Reason));
	}
	
	static void Handle24(const std::string & Identifier, const std::string & Name, int (* Handler) (const uint8_t *, int))
	{
		_Handlers24.insert(std::make_pair(Identifier, Handler));
		_Names24.insert(std::make_pair(Identifier, Name));
	}
private:
	// static setup
	static std::map< std::string, std::string > _Forbidden22;
	static std::map< std::string, std::string > _Forbidden23;
	static std::map< std::string, std::string > _Forbidden24;
	static std::map< std::string, int (*) (const uint8_t *, int) > _Handlers22;
	static std::map< std::string, int (*) (const uint8_t *, int) > _Handlers23;
	static std::map< std::string, int (*) (const uint8_t *, int) > _Handlers24;
	static std::map< std::string, std::string > _Names22;
	static std::map< std::string, std::string > _Names23;
	static std::map< std::string, std::string > _Names24;
	// member variables
	bool _Compression;
	bool _DataLengthIndicator;
	bool _Encryption;
	bool _FileAlterPreservation;
	bool _Forbidden;
	std::string _ForbiddenReason;
	bool _GroupingIdentity;
	int (* _Handler)(const uint8_t * Buffer, int Length);
	std::string _Identifier;
	std::string _Name;
	bool _ReadOnly;
	unsigned int _Size;
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

std::map< std::string, std::string > FrameHeader::_Forbidden22;
std::map< std::string, std::string > FrameHeader::_Forbidden23;
std::map< std::string, std::string > FrameHeader::_Forbidden24;
std::map< std::string, std::string > FrameHeader::_Names22;
std::map< std::string, std::string > FrameHeader::_Names23;
std::map< std::string, std::string > FrameHeader::_Names24;
std::map< std::string, int (*) (const uint8_t *, int) > FrameHeader::_Handlers22;
std::map< std::string, int (*) (const uint8_t *, int) > FrameHeader::_Handlers23;
std::map< std::string, int (*) (const uint8_t *, int) > FrameHeader::_Handlers24;

///////////////////////////////////////////////////////////////////////////////////////////////////
// specific to tag version 2.2                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

int Handle22COMFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_2_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
	
		std::string ISO_639_2Code(Buffer + Index, Buffer + Index + 3);
		
		Index += 3;
		if(ISO_639_2Code.empty() == false)
		{
			auto ISO_639_2Iterator(g_ISO_639_2_Codes.find(ISO_639_2Code));
			
			if(ISO_639_2Iterator != g_ISO_639_2_Codes.end())
			{
				std::cout << "\t\t\t\tLanguage (ISO 639-2): " << ISO_639_2Iterator->second << " (\"" << ISO_639_2Code << "\")" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tLanguage (ISO 639-2): <unknown> (\"" << ISO_639_2Code << "\")" << std::endl;
				std::cout << "*** ERROR *** The language code '" << ISO_639_2Code << "' is not defined by ISO 639-2." << std::endl;
			}
		}
		else
		{
			std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto ReadDescription(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(ReadDescription) == true);
			Index += std::get<1>(ReadDescription);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(ReadDescription) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					assert(std::get<0>(Description) == true);
					Index += std::get<1>(Description);
					std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					assert(std::get<0>(Description) == true);
					Index += std::get<1>(Description);
					std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
				}
			}
			else
			{
				std::cout << "*** The UCS-2 string is expected to start with a byte order mark but it is not." << std::endl;
				Index = Length;
			}
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Comment(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				auto Comment(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Comment) == true)
				{
					Index += std::get<1>(Comment);
					std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					auto Comment(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Comment) == true)
					{
						Index += std::get<1>(Comment);
						std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Comment(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Comment) == true)
						{
							Index += std::get<1>(Comment);
							std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					auto Comment(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Comment) == true)
					{
						Index += std::get<1>(Comment);
						std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Comment(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Comment) == true)
						{
							Index += std::get<1>(Comment);
							std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				std::cout << "*** The UCS-2 string is expected to start with a byte order mark but it is not." << std::endl;
				Index = Length;
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.2.0 [4.11], a \"COM\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.2 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle22PICFrames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_2_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		auto ImageFormat(Get_ISO_IEC_8859_1_StringEndedByBoundary(Buffer + Index, Length - Index, 3));
		
		if(std::get<0>(ImageFormat) == true)
		{
			Index += std::get<1>(ImageFormat);
			std::cout << "\t\t\t\tImage format: \"" << std::get<2>(ImageFormat) << '"' << std::endl;
			if(Length - Index >= 1)
			{
				unsigned int PictureType(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
				
				Index += 1;
				std::cout << "\t\t\t\tPicture type: " << GetPictureTypeString(PictureType) << std::endl;
				if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
				{
					auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
					}
					else
					{
						auto Description(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Description) == true)
						{
							Index += std::get<1>(Description);
							std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
				{
					auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
					
					if(std::get<0>(ByteOrderMark) == true)
					{
						Index += std::get<1>(ByteOrderMark);
						if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
						{
							auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
							
							if(std::get<0>(Description) == true)
							{
								Index += std::get<1>(Description);
								std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
							}
							else
							{
								auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
								
								if(std::get<0>(Description) == true)
								{
									Index += std::get<1>(Description);
									std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
								}
								else
								{
									std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
								}
							}
						}
						else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
						{
							auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
							
							if(std::get<0>(Description) == true)
							{
								Index += std::get<1>(Description);
								std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
							}
							else
							{
								auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
								
								if(std::get<0>(Description) == true)
								{
									Index += std::get<1>(Description);
									std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
								}
								else
								{
									std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
								}
							}
						}
					}
					else
					{
						std::cout << "*** The UCS-2 string is expected to start with a byte order mark but it is not." << std::endl;
						Index = Length;
					}
				}
			}
			else
			{
				std::cout << "*** ERROR *** The frame data is not long enough for the \"Picture type\" field." << std::endl;
			}
		}
		else
		{
			std::cout << "*** ERROR *** According to ID3 2.2.0 [4.15], a \"PIC\" frame MUST contain a \"Image format\" field with 3 characters in the ISO/IEC 8859-1:1998 encoding." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.2.0 [4.15], a \"PIC\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.2 encoding identifier." << std::endl;
	}
	
	return Length;
}

int Handle22T__Frames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_2_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto String(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				std::cout << "*** The UCS-2 string is expected to start with a byte order mark but it is not." << std::endl;
				Index = Length;
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.2.0 [4.2], \"T__\" frames MUST contain a \"Text encoding\" field with a valid tag version 2.2 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle22UFIFrames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto OwnerIdentifier(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
	
	if(std::get<0>(OwnerIdentifier) == true)
	{
		Index += std::get<1>(OwnerIdentifier);
		std::cout << "\t\t\t\tOwner identifier: \"" << std::get<2>(OwnerIdentifier) << "\" (zero-terminated, ISO/IEC 8859-1:1998)" << std::endl;
		
		auto Identifier(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
		
		Index += Identifier.first;
		std::cout << "\t\t\t\tIdentifier: " << Identifier.second << " (boundary-terminated, binary)" << std::endl;
	}
	else
	{
		auto Hexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
		
		Index += Hexadecimal.first;
		std::cout << "*** ERROR *** Invalid string for ISO/IEC 8859-1 encoding." << std::endl;
		std::cout << "              Binary content: " << Hexadecimal.second << std::endl;
	}
	
	return Index;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// specific to tag version 2.3                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

int Handle23APICFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
	
		auto ReadMIMEType(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
		
		assert(std::get<0>(ReadMIMEType) == true);
		Index += std::get<1>(ReadMIMEType);
		std::cout << "\t\t\t\tMIME type: \"" << std::get<2>(ReadMIMEType) << '"' << std::endl;
		
		unsigned int PictureType(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
		
		Index += 1;
		std::cout << "\t\t\t\tPicture type: " << GetPictureTypeString(PictureType) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto ReadDescription(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(ReadDescription) == true)
			{
				Index += std::get<1>(ReadDescription);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(ReadDescription) << '"' << std::endl;
			}
			else
			{
				auto ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
				
				std::cout << "*** ERROR *** Invalid string for ISO/IEC 8859-1 encoding." << std::endl;
				std::cout << "              Binary content: " << ReadHexadecimal.second << std::endl;
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Description) == true)
						{
							Index += std::get<1>(Description);
							std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Description) == true)
						{
							Index += std::get<1>(Description);
							std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				std::cout << "*** The UCS-2 string is expected to start with a byte order mark but it is not." << std::endl;
				Index = Length;
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.15], a \"APIC\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Length;
}

int Handle23COMMFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		std::string ISO_639_2_Code(Buffer + Index, Buffer + Index + 3);
		
		Index += 3;
		if(ISO_639_2_Code.empty() == false)
		{
			auto ISO_639_2_Iterator(g_ISO_639_2_Codes.find(ISO_639_2_Code));
			
			if(ISO_639_2_Iterator != g_ISO_639_2_Codes.end())
			{
				std::cout << "\t\t\t\tLanguage (ISO 639-2): " << ISO_639_2_Iterator->second << " (\"" << ISO_639_2_Code << "\")" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tLanguage (ISO 639-2): <unknown> (\"" << ISO_639_2_Code << "\")" << std::endl;
				std::cout << "*** ERROR *** The language code '" << ISO_639_2_Code << "' is not defined by ISO 639-2." << std::endl;
			}
		}
		else
		{
			std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (zero-termianted)" << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The 'Description' string could be identified as big endian but does not seem to be a valid zero-terminated UCS-2 string." << std::endl;
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The 'Description' string could be identified as little endian but does not seem to be a valid zero-terminated UCS-2 string." << std::endl;
					}
				}
			}
			else
			{
				auto Termination(Get_UCS_2_Termination(Buffer + Index, Length - Index));
				
				if(std::get<0>(Termination) == true)
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [3.3], all unicode strings encoded using UCS-2 are required to start with a Byte Order Mark. The 'Description' string only consists of a UCS-2 terminator." << std::endl;
					std::cout << "\t\t\t\tDescription: \"\" (zero-terminated, missing endian specification)" << std::endl;
					Index += 2;
				}
				else
				{
					std::cout << "*** ERROR *** The 'Description' string is invalid because it does not start with a Byte Order Mark." << std::endl;
				}
			}
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Comment(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				auto Comment(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Comment) == true)
				{
					Index += std::get<1>(Comment);
					std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"Comment\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
					
					auto HexadecimalString(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
					
					Index += HexadecimalString.first;
					std::cout << "*** Binary content: " << HexadecimalString.second << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					auto Comment(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Comment) == true)
					{
						Index += std::get<1>(Comment);
						std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Comment(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Comment) == true)
						{
							Index += std::get<1>(Comment);
							std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					auto Comment(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Comment) == true)
					{
						Index += std::get<1>(Comment);
						std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Comment(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Comment) == true)
						{
							Index += std::get<1>(Comment);
							std::cout << "\t\t\t\tCommentn: \"" << std::get<2>(Comment) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				auto Termination(Get_UCS_2_Termination(Buffer + Index, Length - Index));
				
				if(std::get<0>(Termination) == true)
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [3.3], all unicode strings encoded using UCS-2 are required to start with a Byte Order Mark. The 'Description' string only consists of a UCS-2 terminator." << std::endl;
					std::cout << "\t\t\t\tDescription: \"\" (zero-terminated, missing endian specification)" << std::endl;
					Index += 2;
				}
				else
				{
					std::cout << "*** ERROR *** UCS-2 string is expected to start with a Byte Order Mark but is not. Trying to interpret as UCS-2 little endian." << std::endl;
					auto Comment(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Comment) == true)
					{
						Index += std::get<1>(Comment);
						std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto Comment(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(Comment) == true)
						{
							Index += std::get<1>(Comment);
							std::cout << "\t\t\t\tCommentn: \"" << std::get<2>(Comment) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.11], a \"COMM\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23GEOB_Frame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		auto MIMEType(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
		
		if(std::get<0>(MIMEType) == true)
		{
			Index += std::get<1>(MIMEType);
			std::cout << "\t\t\t\tMIME type: \"" << std::get<2>(MIMEType) << "\"" << std::endl;
			if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
			{
				auto FileName(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(FileName) == true)
				{
					Index += std::get<1>(FileName);
					std::cout << "\t\t\t\tFilename: \"" << std::get<2>(FileName) << "\"" << std::endl;
					
					auto ContentDescription(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(ContentDescription) == true)
					{
						Index += std::get<1>(ContentDescription);
						std::cout << "\t\t\t\tContent description: \"" << std::get<2>(ContentDescription) << "\"" << std::endl;
						Index = Length;
					}
					else
					{
						std::cout << "*** ERROR *** According to ID3 2.3.0 [4.16], a \"GEOB\" frame MUST contain a \"Content description\" field." << std::endl;
					}
				}
				else
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [4.16], a \"GEOB\" frame MUST contain a \"Filename\" field." << std::endl;
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
			{
				assert(false);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			std::cout << "*** ERROR *** According to ID3 2.3.0 [4.16], a \"GEOB\" frame MUST contain a \"MIME type\" field with a zero-terminated ISO/IEC 8859-1:1998 string." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.16], a \"GEOB\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23MCDIFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto TableOfContents(Get_CDTableOfContents(Buffer + Index, Length - Index));
	
	if(std::get<0>(TableOfContents) == true)
	{
		Index += std::get<1>(TableOfContents);
		std::cout << "\t\t\t\tLength: " << std::get<2>(TableOfContents).DataLength << std::endl;
		std::cout << "\t\t\t\tFirst track number: " << std::get<2>(TableOfContents).FirstTrackNumber << std::endl;
		std::cout << "\t\t\t\tLast track number: " << std::get<2>(TableOfContents).LastTrackNumber << std::endl;
		for(auto & TrackDescriptor : std::get<2>(TableOfContents).TrackDescriptors)
		{
			std::cout << std::endl;
			std::cout << "\t\t\t\tReserved: " << GetBinaryStringFromUInt8(TrackDescriptor.Reserved1) << 'b' << std::endl;
			std::cout << "\t\t\t\tADR: " << TrackDescriptor.ADR << std::endl;
			if((TrackDescriptor.HasFourChannels == true) && (TrackDescriptor.IsDataTrack == false))
			{
				std::cout << "\t\t\t\tNumber of channels: 4" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tNumber of channels: 2" << std::endl;
			}
			if(TrackDescriptor.IsDataTrack == true)
			{
				std::cout << "\t\t\t\tTrack type: data" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tTrack type: audio" << std::endl;
			}
			if(TrackDescriptor.IsDigitalCopyPermitted == true)
			{
				std::cout << "\t\t\t\tCopying permitted: true" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tCopying permitted: false" << std::endl;
			}
			if(TrackDescriptor.IsDataTrack == true)
			{
				if(TrackDescriptor.AudioTrackWithEmphasisOrIncrementalDataTrack == true)
				{
					std::cout << "\t\t\t\tRecorded incrementally: true" << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\tRecorded incrementally: false" << std::endl;
				}
			}
			else
			{
				if(TrackDescriptor.AudioTrackWithEmphasisOrIncrementalDataTrack == true)
				{
					std::cout << "\t\t\t\tPre-emphasis enabled: true" << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\tPre-emphasis enabled: false" << std::endl;
				}
			}
			std::cout << "\t\t\t\tTrack number: " << TrackDescriptor.TrackNumber << std::endl;
			std::cout << "\t\t\t\tReserved: " << GetBinaryStringFromUInt8(TrackDescriptor.Reserved2) << 'b' << std::endl;
			std::cout << "\t\t\t\tTrack start address: " << TrackDescriptor.TrackStartAddress << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.5], a \"MCDI\" frame MUST contain a valid CD table of content." << std::endl;
		std::cout << "*** ERROR *** Trying to interpret the data as a UCS-2 string in little endian as a table of content encoding." << std::endl;
		
		auto TableOfContentsString(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
		
		if(std::get<0>(TableOfContentsString) == true)
		{
			Index += std::get<1>(TableOfContentsString);
			std::cout << "\t\t\t\tTable of contents: \"" << std::get<2>(TableOfContentsString) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
		}
		else
		{
			auto TableOfContentsString(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
			
			if(std::get<0>(TableOfContentsString) == true)
			{
				Index += std::get<1>(TableOfContentsString);
				std::cout << "\t\t\t\tTable of contents: \"" << std::get<2>(TableOfContentsString) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
			}
		}
		if(std::get<0>(TableOfContentsString) == true)
		{
			std::cout << "\t\t\t\tInterpretation:" << std::endl;
			
			std::uint32_t IndexInString(0);
			auto NumberOfTracks(GetUInt32NumberFromUnformattedHexadecimalString(std::get<2>(TableOfContentsString), IndexInString));
			
			if(std::get<0>(NumberOfTracks) == true)
			{
				IndexInString += std::get<1>(NumberOfTracks);
				std::cout << "\t\t\t\t\tNumber of tracks:" << std::get<2>(NumberOfTracks) << std::endl;
				for(std::uint32_t TrackIndex = 0; TrackIndex <= std::get<2>(NumberOfTracks); ++TrackIndex)
				{
					if(IndexInString < std::get<2>(TableOfContentsString).length())
					{
						if(std::get<2>(TableOfContentsString)[IndexInString] != '+')
						{
							std::cout << "*** ERROR *** The table of content string contains invalid information." << std::endl;
							
							break;
						}
						else
						{
							IndexInString += 1;
							
							auto TrackOffset(GetUInt32NumberFromUnformattedHexadecimalString(std::get<2>(TableOfContentsString), IndexInString));
							
							if(std::get<0>(TrackOffset) == true)
							{
								IndexInString += std::get<1>(TrackOffset);
								if(TrackIndex < std::get<2>(NumberOfTracks))
								{
									std::cout << "\t\t\t\t\tTrack offset for track " << TrackIndex + 1;
								}
								else
								{
									std::cout << "\t\t\t\t\tTrack offset for lead out";
								}
								std::cout << ": " << std::get<2>(TrackOffset) << std::endl;
							}
						}
					}
				}
			}
			
			auto Rest(Get_Zeroes_EndedByLength(Buffer + Index, Length - Index));
			
			if(std::get<0>(Rest) == true)
			{
				Index += std::get<1>(Rest);
				std::cout << "\t\t\t\tAnother " << std::get<1>(Rest) << " bytes of zeroes until the end of the frame." << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The frame contains unrecognizable data after the table of contents string." << std::endl;
			}
		}
	}
	
	return Index;
}

int Handle23MJCFFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	
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

int Handle23PCNTFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	
	if(Length < 4)
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.17], a \"PCNT\" frame MUST contain at least four bytes for the counter." << std::endl;
	}
	else if(Length == 4)
	{
		auto Counter(Get_UInt32_BE(Buffer + Index, Length - Index));
		
		std::cout << "\t\t\t\tCounter: " << std::get<2>(Counter) << std::endl;
		Index += std::get<1>(Counter);
	}
	else
	{
		std::cout << "*** ERROR *** The program does not yet support printing the value of numbers with more than four bytes." << std::endl;
		Index = Length;
	}
	
	return Index;
}

int Handle23POPMFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	
	if(Length >= 6)
	{
		auto EMailToUser(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
		
		if(std::get<0>(EMailToUser) == true)
		{
			Index += std::get<1>(EMailToUser);
			std::cout << "\t\t\t\tEMail to user: \"" << std::get<2>(EMailToUser) << "\" (null-terminated ISO/IEC 8859-1 string)"  << std::endl;
			if(Length - Index >= 1)
			{
				auto Rating(Get_UInt8(Buffer + Index, Length - Index));
				
				assert(std::get<0>(Rating) == true);
				std::cout << "\t\t\t\tRating: " << static_cast< uint32_t >(std::get<2>(Rating)) << std::endl;
				Index += std::get<1>(Rating);
				if(Length - Index >= 4)
				{
					if(Length - Index == 4)
					{
						auto Counter(Get_UInt32_BE(Buffer + Index, Length - Index));
						
						std::cout << "\t\t\t\tCounter: " << std::get<2>(Counter) << std::endl;
						Index += std::get<1>(Counter);
					}
					else
					{
						/** @todo Implement an output function for arbitrary sized unsigned numbers. **/
						std::cout << "*** ERROR *** The program does not yet support printing the value of numbers with more than four bytes." << std::endl;
						Index = Length;
					}
				}
				else
				{
					std::cout << "\t\t\t\tCounter: - (The personal play counter is omitted.)" << std::endl;
				}
			}
			else
			{
				std::cout << "*** ERROR *** Expected to find at least one more byte containing the rating." << std::endl;
				Index = Length;
			}
		}
		else
		{
			std::cout << "*** ERROR *** Expected to find a null-terminated ISO/IEC 8859-1 string in the first field." << std::endl;
			Index = Length;
		}
	}
	else
	{
		std::cout << "*** ERROR *** Expected a data length of at least 6 bytes." << std::endl;
		Index = Length;
	}
	
	return Index;
}

int Handle23PRIVFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto ReadOwnerIdentifier(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
	
	if(std::get<0>(ReadOwnerIdentifier) == true)
	{
		Index += std::get<1>(ReadOwnerIdentifier);
		std::cout << "\t\t\t\tOwner Identifier: " << std::get<2>(ReadOwnerIdentifier) << std::endl;
		if(std::get<2>(ReadOwnerIdentifier) == "WM/MediaClassPrimaryID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tPrimary Media Class: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/MediaClassSecondaryID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tSecondary Media Class: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/WMContentID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tContent ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/WMCollectionID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tCollection ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "WM/WMCollectionGroupID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tCollection Group ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
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
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tZune Album Artist Media ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneAlbumMediaID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tZune Album Media ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneCollectionID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tZune Collection ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
			}
		}
		else if(std::get<2>(ReadOwnerIdentifier) == "ZuneMediaID")
		{
			auto ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
			
			Index += ReadGUID.first;
			std::cout << "\t\t\t\tZune Media ID: ";
			
			auto GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
			
			if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
			{
				std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
			}
			else
			{
				std::cout << ReadGUID.second << " (unknown value)" << std::endl;
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

int Handle23RGADFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
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

int Handle23T___Frames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto String(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tByte Order Mark: Big Endian" << std::endl;
					
					auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tByte Order Mark: Little Endian" << std::endl;
					
					auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				if(Index == Length)
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [3.3], all unicode strings encoded using UCS-2 must start with a Byte Order Mark, without explicitly excluding empty strings. The string for this text frame is empty without a Byte Order Mark and terminates at the frame boundary." << std::endl;
					std::cout << "\t\t\t\tString: \"\" (boundary-terminated, missing endian specification)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark. Trying to interpret as  UCS-2 little endian." << std::endl;
					
					auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.1], \"T___\" frames MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23TFLTFrames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		std::tuple< bool, int, std::string > FileTypeString;
		
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			FileTypeString = Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index);
			if(std::get<0>(FileTypeString) == true)
			{
				Index += std::get<1>(FileTypeString);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				FileTypeString = Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index);
				if(std::get<0>(FileTypeString) == true)
				{
					Index += std::get<1>(FileTypeString);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tByte Order Mark: Big Endian" << std::endl;
					FileTypeString = Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index);
					if(std::get<0>(FileTypeString) == true)
					{
						Index += std::get<1>(FileTypeString);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						FileTypeString = Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index);
						if(std::get<0>(FileTypeString) == true)
						{
							Index += std::get<1>(FileTypeString);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tByte Order Mark: Little Endian" << std::endl;
					FileTypeString = Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index);
					if(std::get<0>(FileTypeString) == true)
					{
						Index += std::get<1>(FileTypeString);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						FileTypeString = Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index);
						if(std::get<0>(FileTypeString) == true)
						{
							Index += std::get<1>(FileTypeString);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				if(Index == Length)
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [3.3], all unicode strings encoded using UCS-2 must start with a Byte Order Mark, without explicitly excluding empty strings. The string for this text frame is empty without a Byte Order Mark and terminates at the frame boundary." << std::endl;
					std::cout << "\t\t\t\tString: \"\" (boundary-terminated, missing endian specification)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark. Trying to interpret as  UCS-2 little endian." << std::endl;
					FileTypeString = Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index);
					if(std::get<0>(FileTypeString) == true)
					{
						Index += std::get<1>(FileTypeString);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						FileTypeString = Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index);
						if(std::get<0>(FileTypeString) == true)
						{
							Index += std::get<1>(FileTypeString);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(FileTypeString) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
		}
		if(std::get<0>(FileTypeString) == true)
		{
			auto Interpretation(GetFileTypeInterpretation2_3(std::get<2>(FileTypeString)));
			
			if(std::get<0>(Interpretation) == true)
			{
				if(std::get<1>(Interpretation) == true)
				{
					std::cout << "\t\t\t\t\tInterpretation: \"" << std::get<2>(Interpretation) << "\" (as per ID3 2.4)"<< std::endl;
				}
				else
				{
					std::cout << "\t\t\t\t\tInterpretation: \"" << std::get<2>(Interpretation) << "\" (not-standard but interpreted as per ID3 2.4)"<< std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.1], \"TFLT\" frames MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23TLANFrames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
	
		std::string ISO_639_2_Code_String;
		
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto ISO_639_2_Code(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(ISO_639_2_Code) == true)
			{
				Index += std::get<1>(ISO_639_2_Code);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(ISO_639_2_Code) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				ISO_639_2_Code_String = std::get<2>(ISO_639_2_Code);
			}
			else
			{
				auto ISO_639_2_Code(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(ISO_639_2_Code) == true)
				{
					
					Index += std::get<1>(ISO_639_2_Code);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(ISO_639_2_Code) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
					ISO_639_2_Code_String = std::get<2>(ISO_639_2_Code);
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: big endian" << std::endl;
					
					auto ISO_639_2_Code(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(ISO_639_2_Code) == true)
					{
						Index += std::get<1>(ISO_639_2_Code);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ISO_639_2_Code) << "\" (UCS-2, big endian, ended by termination)" << std::endl;
						ISO_639_2_Code_String = std::get<2>(ISO_639_2_Code);
					}
					else
					{
						auto ISO_639_2_Code(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(ISO_639_2_Code) == true)
						{
							Index += std::get<1>(ISO_639_2_Code);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ISO_639_2_Code) << "\" (UCS-2, big endian, ended by boundary)" << std::endl;
							ISO_639_2_Code_String = std::get<2>(ISO_639_2_Code);
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: little endian" << std::endl;
					
					auto ISO_639_2_Code(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(ISO_639_2_Code) == true)
					{
						Index += std::get<1>(ISO_639_2_Code);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ISO_639_2_Code) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
						ISO_639_2_Code_String = std::get<2>(ISO_639_2_Code);
					}
					else
					{
						auto ISO_639_2_Code(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(ISO_639_2_Code) == true)
						{
							Index += std::get<1>(ISO_639_2_Code);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ISO_639_2_Code) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
							ISO_639_2_Code_String = std::get<2>(ISO_639_2_Code);
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
						}
					}
				}
			}
			else
			{
				if(Index == Length)
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [3.3], all unicode strings encoded using UCS-2 must start with a byte order mark, without explicitly excluding empty strings. The string for this text frame is empty without a byte order mark and terminates at the frame boundary." << std::endl;
					std::cout << "\t\t\t\tString: \"\" (ISO/IEC 10646-1:1993, UCS-2, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark." << std::endl;
				}
			}
		}
		if(ISO_639_2_Code_String.length() > 0)
		{
			auto ISO_639_2_Iterator(g_ISO_639_2_Codes.find(ISO_639_2_Code_String));
			
			if(ISO_639_2_Iterator != g_ISO_639_2_Codes.end())
			{
				std::cout << "\t\t\t\tLanguage interpretation (ISO 639-2): " << ISO_639_2_Iterator->second << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tLanguage interpretation (ISO 639-2): <unknown>" << std::endl;
				std::cout << "*** ERROR *** The language code '" << ISO_639_2_Code_String << "' is not defined by ISO 639-2." << std::endl;
			}
		}
		else
		{
			std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.1], a \"TLAN\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23TCMPFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		std::string ReadString;
		
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
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
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
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
					}
					else
					{
						auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** The string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in big endian with or without termination." << std::endl;
						}
					}
				}
				else if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: little endian" << std::endl;
					
					auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							Index += std::get<1>(String);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by boundary)" << std::endl;
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
				if(Index == Length)
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [3.3], all unicode strings encoded using UCS-2 must start with a byte order mark, without explicitly excluding empty strings. The string for this text frame is empty without a byte order mark and terminates at the frame boundary." << std::endl;
					std::cout << "\t\t\t\tString: \"\" (ISO/IEC 10646-1:1993, UCS-2, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark." << std::endl;
				}
			}
		}
		std::cout << "\t\t\t\tPart of a compilation: ";
		if(ReadString == "1")
		{
			std::cout << "yes";
		}
		else if(ReadString == "0")
		{
			std::cout << "no";
		}
		else
		{
			std::cout << "<unknown value>";
		}
		std::cout << " (\"" << ReadString << "\")" << std::endl;
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0, all \"T___\" frames MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23TCONFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
			
		std::string ContentTypeString;
		
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto ContentType(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(ContentType) == true)
			{
				Index += std::get<1>(ContentType);
				std::cout << "\t\t\t\tContent type: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				ContentTypeString = std::get<2>(ContentType);
			}
			else
			{
				auto ContentType(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(ContentType) == true)
				{
					Index += std::get<1>(ContentType);
					std::cout << "\t\t\t\tContent type: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
					ContentTypeString = std::get<2>(ContentType);
				}
				else
				{
					std::cout << "*** ERROR *** The \"Content\" field was declarated to be ISO/IEC 8859-1:1998 but could not be interpreted." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto ByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(ByteOrderMark) == true)
			{
				Index += std::get<1>(ByteOrderMark);
				if(std::get<2>(ByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: big endian" << std::endl;
					
					auto ContentType(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(ContentType) == true)
					{
						Index += std::get<1>(ContentType);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by termination)" << std::endl;
						ContentTypeString = std::get<2>(ContentType);
					}
					else
					{
						auto ContentType(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(ContentType) == true)
						{
							Index += std::get<1>(ContentType);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by boundary)" << std::endl;
							ContentTypeString = std::get<2>(ContentType);
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
					
					auto ContentType(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(ContentType) == true)
					{
						Index += std::get<1>(ContentType);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
						ContentTypeString = std::get<2>(ContentType);
					}
					else
					{
						auto ContentType(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(ContentType) == true)
						{
							Index += std::get<1>(ContentType);
							std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by boundary)" << std::endl;
							ContentTypeString = std::get<2>(ContentType);
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
				std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark. Trying to interpret as  UCS-2LE." << std::endl;
				
				auto ContentType(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(ContentType) == true)
				{
					Index += std::get<1>(ContentType);
					std::cout << "\t\t\t\tContent type:" << std::endl;
					std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
					ContentTypeString = std::get<2>(ContentType);
				}
				else
				{
					auto ContentType(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(ContentType) == true)
					{
						Index += std::get<1>(ContentType);
						std::cout << "\t\t\t\tContent type:" << std::endl;
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(ContentType) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by boundary)" << std::endl;
						ContentTypeString = std::get<2>(ContentType);
					}
					else
					{
						std::cout << "*** ERROR *** The string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in little endian with or without termination." << std::endl;
					}
				}
			}
		}
		
		auto Interpretation(GetContentTypeInterpretation2_3(ContentTypeString));
		
		if(std::get<0>(Interpretation) == true)
		{
			std::cout << "\t\t\t\tInterpretation: " << std::get<1>(Interpretation) << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.1], a \"TCON\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23TSRCFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		std::string ReadString;
		
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
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
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
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

int Handle23TXXXFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Description) == true)
			{
				Index += std::get<1>(Description);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				if(Index < Length)
				{
					auto String(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
					}
					else
					{
						auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
						
						if(std::get<0>(String) == true)
						{
							
							Index += std::get<1>(String);
							std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
						}
						else
						{
							std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.2], a \"TXXX\" frame MUST contain a \"Description\" field using the text encoding. However, the content of the field could not be interpreted as an ISO/IEC 8859-1:1998 string." << std::endl;
							Index = Length;
						}
					}
				}
				else
				{
					std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.2], a \"TXXX\" frame MUST contain a \"Description\" field." << std::endl;
					Index = Length;
				}
			}
			else
			{
				std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.2], a \"TXXX\" frame MUST contain a \"String\" field." << std::endl;
				Index = Length;
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			auto DescriptionByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(DescriptionByteOrderMark) == true)
			{
				Index += std::get<1>(DescriptionByteOrderMark);
				if(std::get<2>(DescriptionByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tDescription:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: big endian" << std::endl;
					
					auto Description(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(Description) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The 'Description' string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in big endian with termination." << std::endl;
					}
				}
				else if(std::get<2>(DescriptionByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tDescription:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: little endian" << std::endl;
					
					auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(Description) == true)
					{
						Index += std::get<1>(Description);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(Description) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The 'Description' string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in little endian with termination." << std::endl;
					}
				}
			}
			else
			{
				std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark. Trying to interpret as  UCS-2LE." << std::endl;
				
				auto Description(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(Description) == true)
				{
					Index += std::get<1>(Description);
					std::cout << "\t\t\t\tDescription:" << std::endl;
					std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(Description) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The 'Description' string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in little endian with termination." << std::endl;
				}
			}
			
			auto StringByteOrderMark(Get_UCS_2_ByteOrderMark(Buffer + Index, Length - Index));
			
			if(std::get<0>(StringByteOrderMark) == true)
			{
				Index += std::get<1>(StringByteOrderMark);
				if(std::get<2>(StringByteOrderMark) == UCS2ByteOrderMark::BigEndian)
				{
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: big endian" << std::endl;
					
					auto String(Get_UCS_2BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, big endian, ended by termination)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The 'String' string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in big endian with termination." << std::endl;
					}
				}
				else if(std::get<2>(StringByteOrderMark) == UCS2ByteOrderMark::LittleEndian)
				{
					std::cout << "\t\t\t\tDescription:" << std::endl;
					std::cout << "\t\t\t\t\tByte order mark: little endian" << std::endl;
					
					auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The 'String' string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in little endian with termination." << std::endl;
					}
				}
			}
			else
			{
				std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark. Trying to interpret as  UCS-2LE." << std::endl;
				
				auto String(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString:" << std::endl;
					std::cout << "\t\t\t\t\tCharacters: \"" << std::get<2>(String) << "\" (ISO/IEC 10646-1:1993, UCS-2, little endian, ended by termination)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The 'String' string could not be interpreted as a ISO/IEC 10646-1:1993, UCS-2 string in little endian with termination." << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.2.2], a \"TXXX\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle23UFIDFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto OwnerIdentifier(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
	
	assert(std::get<0>(OwnerIdentifier) == true);
	if(std::get<2>(OwnerIdentifier).length() == 0)
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.1], the 'Owner Identifier' field must be non-empty." << std::endl;
	}
	std::cout << "\t\t\t\tOwner Identifier: \"" << std::get<2>(OwnerIdentifier) << '"' << std::endl;
	Index += std::get<1>(OwnerIdentifier);
	if(Length - Index > 64)
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.1], the 'Identifier' field must not exceed 64 bytes." << std::endl;
	}
	
	auto Identifier(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
	
	std::cout << "\t\t\t\tIdentifier (binary): " << Identifier.second << std::endl;
	
	auto IdentifierAsString(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
	
	if(std::get<0>(IdentifierAsString) == true)
	{
		std::cout << "\t\t\t\tIdentifier (string): \"" << std::get<2>(IdentifierAsString) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
	}
	Index += Identifier.first;
	
	return Index;
}

int Handle23W___Frames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto URL(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
	
	if(std::get<0>(URL) == true)
	{
		Index += std::get<1>(URL);
		std::cout << "\t\t\t\tURL: \"" << std::get<2>(URL) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
	}
	else
	{
		auto URL(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
		
		if(std::get<0>(URL) == true)
		{
			Index += std::get<1>(URL);
			std::cout << "\t\t\t\tURL: \"" << std::get<2>(URL) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
		}
		else
		{
			std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
		}
	}
	
	return Index;
}

int Handle23WXXXFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_3_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		std::cout << "\t\t\t\tDescription: \"";
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << std::get<2>(Description);
		}
		else if(std::get<2>(Encoding) == TextEncoding::UCS_2)
		{
			/// @TODO
			assert(false);
		}
		std::cout << '"' << std::endl;
		
		auto URL(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));

		if(std::get<0>(URL) == true)
		{
			Index += std::get<1>(URL);
			std::cout << "\t\t\t\tURL: \"" << std::get<2>(URL) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
		}
		else
		{
			auto URL(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
			
			if(std::get<0>(URL) == true)
			{
				Index += std::get<1>(URL);
				std::cout << "\t\t\t\tURL: \"" << std::get<2>(URL) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.3.0 [4.3.2], a \"WXXX\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.3 encoding identifier." << std::endl;
	}
	
	return Index;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// specific to tag version 2.4                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

int Handle24APICFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_4_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		auto MIMEType(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
		
		assert(std::get<0>(MIMEType) == 0);
		Index += std::get<1>(MIMEType);
		std::cout << "\t\t\t\tMIME type: \"" << std::get<2>(MIMEType) << '"' << std::endl;
		
		unsigned int PictureType(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
		
		Index += 1;
		std::cout << "\t\t\t\tPicture type: " << GetPictureTypeString(PictureType) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Description) == true)
			{
				Index += std::get<1>(Description);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Description\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with termination." << std::endl;
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
		{
			auto Description(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
		{
			auto Description(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
		{
			auto Description(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Description) == true)
			{
				Index += std::get<1>(Description);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UTF-8, ended by termination)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Description\" field could not be interpreted as an UTF-8 string with termination." << std::endl;
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.4.0 [4.14], a \"APIC\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.4 encoding identifier." << std::endl;
	}
	
	return Length;
}

int Handle24COMMFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_4_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		std::string ISO_639_2_Code(Buffer + Index, Buffer + Index + 3);
		
		Index += 3;
		if(ISO_639_2_Code.empty() == false)
		{
			if(ISO_639_2_Code == "XXX")
			{
				std::cout << "\t\t\t\tLanguage: <unknown> (as per ID3 2.4 \"" << ISO_639_2_Code << "\")" << std::endl;
			}
			else
			{
				auto ISO_639_2_Iterator(g_ISO_639_2_Codes.find(ISO_639_2_Code));
				
				if(ISO_639_2_Iterator != g_ISO_639_2_Codes.end())
				{
					std::cout << "\t\t\t\tLanguage: " << ISO_639_2_Iterator->second << " (as per ISO 639-2 \"" << ISO_639_2_Code << "\")" << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\tLanguage: <unknown> (\"" << ISO_639_2_Code << "\" is not defined by ISO 639-2)" << std::endl;
					std::cout << "*** ERROR *** The language code '" << ISO_639_2_Code << "' is not defined by ISO 639-2." << std::endl;
				}
			}
		}
		else
		{
			std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
		{
			auto Description(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
		{
			auto Description(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
		{
			auto Description(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Description) == true)
			{
				Index += std::get<1>(Description);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Description\" field string could not be interpreted as an UTF-8 string with termination." << std::endl;
			}
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Comment(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				auto Comment(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Comment) == true)
				{
					Index += std::get<1>(Comment);
					std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
		{
			auto Comment(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-16, ended by termination)" << std::endl;
			}
			else
			{
				auto Comment(Get_UTF_16_StringWithByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Comment) == true)
				{
					Index += std::get<1>(Comment);
					std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-16, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an UTF-16 string with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
		{
			auto Comment(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-16BE, ended by termination)" << std::endl;
			}
			else
			{
				auto Comment(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Comment) == true)
				{
					Index += std::get<1>(Comment);
					std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-16BE, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an UTF-16BE string with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
		{
			auto Comment(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-8, ended by termination)" << std::endl;
			}
			else
			{
				auto Comment(Get_UTF_8_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(Comment) == true)
				{
					Index += std::get<1>(Comment);
					std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-8, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The string could not be interpreted as an UTF-8 string with or without termination." << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.4.0 [4.10], a \"COMM\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.4 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle24MCDIFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto TableOfContents(Get_CDTableOfContents(Buffer + Index, Length - Index));
	
	if(std::get<0>(TableOfContents) == true)
	{
		Index += std::get<1>(TableOfContents);
		std::cout << "\t\t\t\tLength: " << std::get<2>(TableOfContents).DataLength << std::endl;
		std::cout << "\t\t\t\tFirst track number: " << std::get<2>(TableOfContents).FirstTrackNumber << std::endl;
		std::cout << "\t\t\t\tLast track number: " << std::get<2>(TableOfContents).LastTrackNumber << std::endl;
		for(auto & TrackDescriptor : std::get<2>(TableOfContents).TrackDescriptors)
		{
			std::cout << std::endl;
			std::cout << "\t\t\t\tReserved: " << GetBinaryStringFromUInt8(TrackDescriptor.Reserved1) << 'b' << std::endl;
			std::cout << "\t\t\t\tADR: " << TrackDescriptor.ADR << std::endl;
			if((TrackDescriptor.HasFourChannels == true) && (TrackDescriptor.IsDataTrack == false))
			{
				std::cout << "\t\t\t\tNumber of channels: 4" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tNumber of channels: 2" << std::endl;
			}
			if(TrackDescriptor.IsDataTrack == true)
			{
				std::cout << "\t\t\t\tTrack type: data" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tTrack type: audio" << std::endl;
			}
			if(TrackDescriptor.IsDigitalCopyPermitted == true)
			{
				std::cout << "\t\t\t\tCopying permitted: true" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tCopying permitted: false" << std::endl;
			}
			if(TrackDescriptor.IsDataTrack == true)
			{
				if(TrackDescriptor.AudioTrackWithEmphasisOrIncrementalDataTrack == true)
				{
					std::cout << "\t\t\t\tRecorded incrementally: true" << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\tRecorded incrementally: false" << std::endl;
				}
			}
			else
			{
				if(TrackDescriptor.AudioTrackWithEmphasisOrIncrementalDataTrack == true)
				{
					std::cout << "\t\t\t\tPre-emphasis enabled: true" << std::endl;
				}
				else
				{
					std::cout << "\t\t\t\tPre-emphasis enabled: false" << std::endl;
				}
			}
			std::cout << "\t\t\t\tTrack number: " << TrackDescriptor.TrackNumber << std::endl;
			std::cout << "\t\t\t\tReserved: " << GetBinaryStringFromUInt8(TrackDescriptor.Reserved2) << 'b' << std::endl;
			std::cout << "\t\t\t\tTrack start address: " << TrackDescriptor.TrackStartAddress << std::endl;
		}
	}
	else
	{
		auto TableOfContentsString(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
		
		if(std::get<0>(TableOfContentsString) == true)
		{
			Index += std::get<1>(TableOfContentsString);
			std::cout << "\t\t\t\tTable of contents: \"" << std::get<2>(TableOfContentsString) << "\" (UCS-2, little endian, ended by termination)" << std::endl;
		}
		else
		{
			auto TableOfContentsString(Get_UCS_2LE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
			
			if(std::get<0>(TableOfContentsString) == true)
			{
				Index += std::get<1>(TableOfContentsString);
				std::cout << "\t\t\t\tTable of contents: \"" << std::get<2>(TableOfContentsString) << "\" (UCS-2, little endian, ended by boundary)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The string could not be interpreted as a UCS-2 string in little endian with or without termination." << std::endl;
			}
		}
	}
	
	return Index;
}

int Handle24T___Frames(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_4_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		std::cout << "\t\t\t\tString(s):\n";
		while(Index < Length)
		{
			if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
			{
				auto String(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				}
				else
				{
					auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
					}
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
			{
				auto String(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (UTF-16, ended by termination)" << std::endl;
				}
				else
				{
					auto String(Get_UTF_16_StringWithByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (UTF-16, ended by boundary)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The string could not be interpreted as an UTF-16 string with or without termination." << std::endl;
					}
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
			{
				auto String(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (UTF-16BE, ended by termination)" << std::endl;
				}
				else
				{
					auto String(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (UTF-16BE, ended by boundary)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The string could not be interpreted as an UTF-16BE string with or without termination." << std::endl;
					}
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
			{
				auto String(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (UTF-8, ended by termination)" << std::endl;
				}
				else
				{
					auto String(Get_UTF_8_StringEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(String) == true)
					{
						Index += std::get<1>(String);
						std::cout << "\t\t\t\t\t\"" << std::get<2>(String) << "\" (UTF-8, ended by boundary)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The string could not be interpreted as an UTF-8 string with or without termination." << std::endl;
					}
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.4.0 [4.2], \"T___\" frames MUST contain a \"Text encoding\" field with a valid tag version 2.4 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle24TXXXFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_4_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Comment(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Comment\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with termination." << std::endl;
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
		{
			auto Description(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
		{
			auto Description(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
		{
			auto Comment(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Comment) == true)
			{
				Index += std::get<1>(Comment);
				std::cout << "\t\t\t\tComment: \"" << std::get<2>(Comment) << "\" (UTF-8, ended by termination)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Comment\" field could not be interpreted as an UTF-8 string with termination." << std::endl;
			}
		}
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto String(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				auto String(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"String\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
		{
			auto String(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UTF-16, ended by termination)" << std::endl;
			}
			else
			{
				auto String(Get_UTF_16_StringWithByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UTF-16, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"String\" field could not be interpreted as an UTF-16 string with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
		{
			auto String(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UTF-16BE, ended by termination)" << std::endl;
			}
			else
			{
				auto String(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UTF-16BE, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"String\" field could not be interpreted as an UTF-16BE string with or without termination." << std::endl;
				}
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
		{
			auto String(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(String) == true)
			{
				Index += std::get<1>(String);
				std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UTF-8, ended by termination)" << std::endl;
			}
			else
			{
				auto String(Get_UTF_8_StringEndedByLength(Buffer + Index, Length - Index));
				
				if(std::get<0>(String) == true)
				{
					Index += std::get<1>(String);
					std::cout << "\t\t\t\tString: \"" << std::get<2>(String) << "\" (UTF-8, ended by boundary)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"String\" field could not be interpreted as an UTF-8 string with or without termination." << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.4.0 [4.2.2], a \"TXXX\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.4 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle24USLTFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_4_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		
		auto Language(Get_ISO_IEC_8859_1_StringEndedByBoundary(Buffer + Index, Length - Index, 3));
		
		if(std::get<0>(Language) == true)
		{
			Index += std::get<1>(Language);
			
			auto ISO_639_2_Iterator(g_ISO_639_2_Codes.find(std::get<2>(Language)));
			
			if(ISO_639_2_Iterator != g_ISO_639_2_Codes.end())
			{
				std::cout << "\t\t\t\tLanguage (ISO 639-2): " << ISO_639_2_Iterator->second << " (\"" << std::get<2>(Language) << "\")" << std::endl;
			}
			else
			{
				std::cout << "\t\t\t\tLanguage (ISO 639-2): <unknown>" << std::endl;
				std::cout << "*** ERROR *** The language code '" << std::get<2>(Language) << "' is not defined by ISO 639-2." << std::endl;
			}
			if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
			{
				auto ContentDescriptor(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(ContentDescriptor) == true)
				{
					Index += std::get<1>(ContentDescriptor);
					std::cout << "\t\t\t\tContent descriptor: \"" << std::get<2>(ContentDescriptor) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"Content descriptor\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with termination." << std::endl;
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
			{
				auto ContentDescriptor(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(ContentDescriptor) == true)
				{
					Index += std::get<1>(ContentDescriptor);
					std::cout << "\t\t\t\tContent descriptor: \"" << std::get<2>(ContentDescriptor) << "\" (UTF-16, ended by termination)" << std::endl;
				}
				else
				{
					std::cout << "*** ERROR *** The content of the \"Content descriptor\" field could not be interpreted as an UTF-16 string with termination." << std::endl;
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
			{
				/// @TODO
				assert(false);
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
			{
				/// @TODO
				assert(false);
			}
			if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
			{
				auto Lyrics(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(Lyrics) == true)
				{
					Index += std::get<1>(Lyrics);
					std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
				}
				else
				{
					auto Lyrics(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(Lyrics) == true)
					{
						Index += std::get<1>(Lyrics);
						std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
					}
					else
					{
						auto Lyrics(Get_ISO_IEC_8859_1_StringWithLineFeedsEndedByTermination(Buffer + Index, Length - Index));
						
						if(std::get<0>(Lyrics) == true)
						{
							Index += std::get<1>(Lyrics);
							std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (ISO/IEC 8859-1:1998, with line feeds ended by boundary)" << std::endl;
						}
						else
						{
							auto Lyrics(Get_ISO_IEC_8859_1_StringWithLineFeedsEndedByLength(Buffer + Index, Length - Index));
							
							if(std::get<0>(Lyrics) == true)
							{
								Index += std::get<1>(Lyrics);
								std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (ISO/IEC 8859-1:1998, with line feeds, ended by boundary)" << std::endl;
							}
							else
							{
								auto Lyrics(Get_ISO_IEC_8859_1_StringWithCarriageReturnsEndedByTermination(Buffer + Index, Length - Index));
								
								if(std::get<0>(Lyrics) == true)
								{
									Index += std::get<1>(Lyrics);
									std::cout << "*** ERROR *** According to ID3 2.4.0 [4.], full text strings may contain line feeds but the \"Lyrics\" field wrongly contains carriage return characters." << std::endl;
									std::replace(std::get<2>(Lyrics).begin(), std::get<2>(Lyrics).end(), '\x0d', '\x0a');
									std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (ISO/IEC 8859-1:1998, with carriage returns ended by boundary)" << std::endl;
								}
								else
								{
									auto Lyrics(Get_ISO_IEC_8859_1_StringWithCarriageReturnsEndedByLength(Buffer + Index, Length - Index));
									
									if(std::get<0>(Lyrics) == true)
									{
										Index += std::get<1>(Lyrics);
										std::cout << "*** ERROR *** According to ID3 2.4.0 [4.], full text strings may contain line feeds but the \"Lyrics\" field wrongly contains carriage return characters." << std::endl;
										std::replace(std::get<2>(Lyrics).begin(), std::get<2>(Lyrics).end(), '\x0d', '\x0a');
										std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (ISO/IEC 8859-1:1998, with carriage returns, ended by boundary)" << std::endl;
									}
									else
									{
										std::cout << "*** ERROR *** The content of the \"Lyrics\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination with or without line feeds or with or without carriage returns." << std::endl;
									}
								}
							}
						}
					}
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
			{
				auto Lyrics(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
				
				if(std::get<0>(Lyrics) == true)
				{
					Index += std::get<1>(Lyrics);
					std::replace(std::get<2>(Lyrics).begin(), std::get<2>(Lyrics).end(), '\x0d', '\x0a');
					std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (UTF-16, ended by termination)" << std::endl;
				}
				else
				{
					auto Lyrics(Get_UTF_16_StringWithByteOrderMarkEndedByLength(Buffer + Index, Length - Index));
					
					if(std::get<0>(Lyrics) == true)
					{
						Index += std::get<1>(Lyrics);
						std::replace(std::get<2>(Lyrics).begin(), std::get<2>(Lyrics).end(), '\x0d', '\x0a');
						std::cout << "\t\t\t\tLyrics/text: \"" << std::get<2>(Lyrics) << "\" (UTF-16, ended by boundary)" << std::endl;
					}
					else
					{
						std::cout << "*** ERROR *** The content of the \"String\" field could not be interpreted as an UTF-16 string with or without termination." << std::endl;
					}
				}
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
			{
				/// @TODO
				assert(false);
			}
			else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
			{
				/// @TODO
				assert(false);
			}
		}
		else
		{
			std::cout << "*** ERROR *** According to ID3 2.4.0 [4.8], a \"USLT\" frame MUST contain a \"Language\" field with a valid ISO-639-2 language code." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.4.0 [4.8], a \"USLT\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.4 encoding identifier." << std::endl;
	}
	
	return Index;
}

int Handle24WXXXFrame(const uint8_t * Buffer, int Length)
{
	int Index(0);
	auto Encoding(Get_ID3_2_4_Encoding(Buffer + Index, Length - Index));
	
	if(std::get<0>(Encoding) == true)
	{
		Index += std::get<1>(Encoding);
		std::cout << "\t\t\t\tText Encoding: " << GetEncodingName(std::get<2>(Encoding)) << std::endl;
		if(std::get<2>(Encoding) == TextEncoding::ISO_IEC_8859_1_1998)
		{
			auto Description(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Description) == true)
			{
				Index += std::get<1>(Description);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Description\" field could not be interpreted as an ISO/IEC 8859-1:1998 string with termination." << std::endl;
			}
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16)
		{
			auto Description(Get_UTF_16_StringWithByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_16_BE)
		{
			auto Description(Get_UTF_16BE_StringWithoutByteOrderMarkEndedByTermination(Buffer + Index, Length - Index));
			
			assert(std::get<0>(Description) == true);
			Index += std::get<1>(Description);
			std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << '"' << std::endl;
		}
		else if(std::get<2>(Encoding) == TextEncoding::UTF_8)
		{
			auto Description(Get_UTF_8_StringEndedByTermination(Buffer + Index, Length - Index));
			
			if(std::get<0>(Description) == true)
			{
				Index += std::get<1>(Description);
				std::cout << "\t\t\t\tDescription: \"" << std::get<2>(Description) << "\" (UTF-8, ended by termination)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The content of the \"Description\" field could not be interpreted as an UTF-8 string with termination." << std::endl;
			}
		}
		
		auto URL(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer + Index, Length - Index));

		if(std::get<0>(URL) == true)
		{
			Index += std::get<1>(URL);
			std::cout << "\t\t\t\tURL: \"" << std::get<2>(URL) << "\" (ISO/IEC 8859-1:1998, ended by termination)" << std::endl;
		}
		else
		{
			auto URL(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer + Index, Length - Index));
			
			if(std::get<0>(URL) == true)
			{
				Index += std::get<1>(URL);
				std::cout << "\t\t\t\tURL: \"" << std::get<2>(URL) << "\" (ISO/IEC 8859-1:1998, ended by boundary)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The string could not be interpreted as an ISO/IEC 8859-1:1998 string with or without zero-termination." << std::endl;
			}
		}
	}
	else
	{
		std::cout << "*** ERROR *** According to ID3 2.4.0 [4.3.2], a \"WXXX\" frame MUST contain a \"Text encoding\" field with a valid tag version 2.4 encoding identifier." << std::endl;
	}
	
	return Index;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// application                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadFile(const std::string & Path);
void ReadDirectory(const std::string & Path);
void ReadItem(const std::string & Path);

inline bool FileExists(const std::string & Path)
{
	struct stat Stat;
	
	return stat(Path.c_str(), &Stat) != -1;
}

inline bool IsDirectory(const std::string & Path)
{
	struct stat Stat;
	
	stat(Path.c_str(), &Stat);
	
	return S_ISDIR(Stat.st_mode);
}

inline bool IsRegularFile(const std::string & Path)
{
	struct stat Stat;
	
	stat(Path.c_str(), &Stat);
	
	return S_ISREG(Stat.st_mode);
}

void ReadID3v2Tag(std::ifstream & Stream)
{
	Stream.seekg(0, std::ios::beg);
	
	TagHeader * NewTagHeader(new TagHeader(Stream));
	unsigned int BufferLength(1000);
	uint8_t * Buffer(new uint8_t[BufferLength]);
	
	if(NewTagHeader->GetID3Identifier() == "ID3")
	{
		std::cout << "ID3v2 TAG:" << std::endl;
		std::cout << "\tFile Identifier: " << NewTagHeader->GetID3Identifier() << std::endl;
		std::cout << "\tVersion: 2." << NewTagHeader->GetMajorVersion() << "." << NewTagHeader->GetRevisionNumber() << std::endl;
		std::cout << "\tFlags: " << NewTagHeader->GetFlagsAsString() << std::endl;
		std::cout << "\tSize: " << NewTagHeader->GetSize() << std::endl;
		std::cout << "\tFrames:" << std::endl;

		int Size = NewTagHeader->GetSize();

		while(Size > 0)
		{
			FrameHeader * NewFrameHeader(new FrameHeader(NewTagHeader, Stream));
			
			if(NewFrameHeader->IsValid() == true)
			{
				std::cout << "\t\tIdentifier: \"" << NewFrameHeader->GetIdentifier() << "\"" << std::endl;
				if(NewFrameHeader->GetForbidden() == true)
				{
					std::cout << "*** ERROR *** This frame is forbidden! " << NewFrameHeader->GetForbiddenReason() << std::endl;
				}
				std::cout << "\t\t\tName: " << NewFrameHeader->GetName() << std::endl;
				std::cout << "\t\t\tSize: " << NewFrameHeader->GetSize() << std::endl;
				if(NewFrameHeader->SupportsFlags() == true)
				{
					std::cout << "\t\t\tFlags: " << NewFrameHeader->GetFlagsAsString() << std::endl;
				}
				while(NewFrameHeader->GetSize() > BufferLength)
				{
					delete[] Buffer;
					BufferLength <<= 1;
					Buffer = new uint8_t[BufferLength];
				}
				Stream.read(reinterpret_cast< char * >(Buffer), NewFrameHeader->GetSize());
				if(g_PrintBytes == true)
				{
					std::cout << "\t\t\tBytes: " << GetHexadecimalStringFromUInt8Buffer(Buffer, NewFrameHeader->GetSize()) << std::endl;
				}
				std::cout << "\t\t\tContent:" << std::endl;
				
				unsigned int HandledFrameSize(0);
				
				HandledFrameSize = NewFrameHeader->HandleData(Buffer, NewFrameHeader->GetSize());
				if(HandledFrameSize < NewFrameHeader->GetSize())
				{
					std::cout << "*** ERROR *** Frame size exceeds frame data." << std::endl;
				}
				else if(HandledFrameSize > NewFrameHeader->GetSize())
				{
					std::cout << "*** ERROR *** Frame data exceeds frame size." << std::endl;
				}
				std::cout << std::endl;
			}
			else
			{
				break;
			}
			delete NewFrameHeader;
		}
		std::cout << std::endl;
	}
	delete NewTagHeader;
}

void ReadFile(const std::string & Path)
{
	uint8_t * Buffer(new uint8_t[1000]);
	int u4Track = 0;
	bool bID3v11 = false;

	std::ifstream ReadFile;

	ReadFile.open(Path.c_str(), std::ios::in | std::ios::binary);
	if(!ReadFile)
	{
		std::cerr << Path << ": Can not be opened." << std::endl;
	}
	ReadFile.seekg(-128, std::ios::end);
	ReadFile.read(reinterpret_cast< char * >(Buffer), 3);
	Buffer[3] = '\0';
	if(strcmp(reinterpret_cast< char * >(Buffer), "TAG") == 0)
	{
		std::cout << "ID3v1 TAG:" << std::endl;
		ReadFile.read(reinterpret_cast< char * >(Buffer), 30);
		
		auto Title(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer, 30));
		
		if(std::get<0>(Title) == true)
		{
			std::cout << "\tTitle:\t \"" << std::get<2>(Title) << "\"  (ISO/IEC 8859-1:1998, ended by termination, length: " << std::get<1>(Title) - 1 << " of 30)" << std::endl;
		}
		else
		{
			auto Title(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer, 30));
			
			if(std::get<0>(Title) == true)
			{
				std::cout << "\tTitle:\t \"" << std::get<2>(Title) << "\"  (ISO/IEC 8859-1:1998, ended by boundary, length: " << std::get<1>(Title) << " of 30)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The 'Title' field contains data that can not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				
				auto Title = GetHexadecimalStringTerminatedByLength(Buffer, 30);
				
				std::cout << "*** Binary content: " << Title.second << std::endl;
			}
		}
		ReadFile.read(reinterpret_cast< char * >(Buffer), 30);
		
		auto Artist(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer, 30));
		
		if(std::get<0>(Artist) == true)
		{
			std::cout << "\tArtist:\t \"" << std::get<2>(Artist) << "\"  (ISO/IEC 8859-1:1998, ended by termination, length: " << std::get<1>(Artist) - 1 << " of 30)" << std::endl;
		}
		else
		{
			auto Artist(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer, 30));
			
			if(std::get<0>(Artist) == true)
			{
				std::cout << "\tArtist:\t \"" << std::get<2>(Artist) << "\"  (ISO/IEC 8859-1:1998, ended by boundary, length: " << std::get<1>(Artist) << " of 30)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The 'Artist' field contains data that can not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				
				auto Artist = GetHexadecimalStringTerminatedByLength(Buffer, 30);
				
				std::cout << "*** Binary content: " << Artist.second << std::endl;
			}
		}
		ReadFile.read(reinterpret_cast< char * >(Buffer), 30);
		
		auto Album(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer, 30));
		
		if(std::get<0>(Album) == true)
		{
			std::cout << "\tAlbum:\t \"" << std::get<2>(Album) << "\"  (ISO/IEC 8859-1:1998, ended by termination, length: " << std::get<1>(Album) - 1 << " of 30)" << std::endl;
		}
		else
		{
			auto Album(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer, 30));
			
			if(std::get<0>(Album) == true)
			{
				std::cout << "\tAlbum:\t \"" << std::get<2>(Album) << "\"  (ISO/IEC 8859-1:1998, ended by boundary, length: " << std::get<1>(Album) << " of 30)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The 'Album' field contains data that can not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				
				auto Album = GetHexadecimalStringTerminatedByLength(Buffer, 30);
				
				std::cout << "*** Binary content: " << Album.second << std::endl;
			}
		}
		ReadFile.read(reinterpret_cast< char * >(Buffer), 4);
		
		auto Year(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer, 4));
		
		if(std::get<0>(Year) == true)
		{
			std::cout << "\tYear:\t \"" << std::get<2>(Year) << "\"  (ISO/IEC 8859-1:1998, ended by termination, length: " << std::get<1>(Year) - 1 << " of 4)" << std::endl;
		}
		else
		{
			auto Year(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer, 4));
			
			if(std::get<0>(Year) == true)
			{
				std::cout << "\tYear:\t \"" << std::get<2>(Year) << "\"  (ISO/IEC 8859-1:1998, ended by boundary, length: " << std::get<1>(Year) << " of 4)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The 'Year' field contains data that can not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				
				auto Year = GetHexadecimalStringTerminatedByLength(Buffer, 4);
				
				std::cout << "*** Binary content: " << Year.second << std::endl;
			}
		}
		ReadFile.read(reinterpret_cast< char * >(Buffer), 30);
		
		auto Comment(Get_ISO_IEC_8859_1_StringEndedByTermination(Buffer, 30));
		
		if(std::get<0>(Comment) == true)
		{
			std::cout << "\tComment: \"" << std::get<2>(Comment) << "\"  (ISO/IEC 8859-1:1998, ended by termination, length: " << std::get<1>(Comment) - 1 << " of 30)" << std::endl;
		}
		else
		{
			auto Comment(Get_ISO_IEC_8859_1_StringEndedByLength(Buffer, 30));
			
			if(std::get<0>(Comment) == true)
			{
				std::cout << "\tComment: \"" << std::get<2>(Album) << "\"  (ISO/IEC 8859-1:1998, ended by boundary, length: " << std::get<1>(Comment) << " of 30)" << std::endl;
			}
			else
			{
				std::cout << "*** ERROR *** The 'Comment' field contains data that can not be interpreted as an ISO/IEC 8859-1:1998 string with or without termination." << std::endl;
				
				auto Comment = GetHexadecimalStringTerminatedByLength(Buffer, 30);
				
				std::cout << "*** Binary content: " << Comment.second << std::endl;
			}
		}
		bID3v11 = false;
		if(Buffer[28] == '\0')
		{
			bID3v11 = true;
			u4Track = static_cast< int >(Buffer[29]);
		}
		ReadFile.read(reinterpret_cast< char * >(Buffer), 1);
		
		std::map< unsigned int, std::string >::iterator NumericGenreIterator(g_NumericGenresID3_1.find(Buffer[0]));
		
		if(NumericGenreIterator != g_NumericGenresID3_1.end())
		{
			std::cout << "\tGenre:\t " << NumericGenreIterator->second << "  [number: " << static_cast< unsigned int >(static_cast< unsigned char >(*Buffer)) << "] (ID3v1 standard)" << std::endl;
		}
		else
		{
			NumericGenreIterator = g_NumericGenresWinamp.find(Buffer[0]);
			if(NumericGenreIterator != g_NumericGenresWinamp.end())
			{
				std::cout << "\tGenre:\t " << NumericGenreIterator->second << "  [number: " << static_cast< unsigned int >(static_cast< unsigned char >(*Buffer)) << "] (Winamp extension)" << std::endl;
			}
			else
			{
				std::cout << "\tGenre:\t unrecognized genre  [number: " << static_cast< unsigned int >(static_cast< unsigned char >(*Buffer)) << "]" << std::endl;
			}
		}
		if(bID3v11 == true)
		{
			std::cout << "ID3v1.1 TAG:" << std::endl;
			std::cout << "\tTrack:\t \"" << u4Track << "\"" << std::endl;
		}
	}
	ReadID3v2Tag(ReadFile);
}

void ReadDirectory(const std::string & Path)
{
	DIR * Directory(opendir(Path.c_str()));
	struct dirent * DirectoryEntry(0);
	
	while((DirectoryEntry = readdir(Directory)) != 0)
	{
		if((std::string(DirectoryEntry->d_name) != ".") && (std::string(DirectoryEntry->d_name) != ".."))
		{
			ReadItem(Path + '/' + DirectoryEntry->d_name);
		}
	}
}

void ReadItem(const std::string & Path)
{
	if(FileExists(Path) == true)
	{
		if(IsDirectory(Path) == true)
		{
			ReadDirectory(Path);
		}
		else if(IsRegularFile(Path) == true)
		{
			ReadFile(Path);
		}
		else
		{
			std::cerr << '"' << Path << "\" is no file or directory!" << std::endl;
		}
	}
	else
	{
		std::cerr << '"' << Path << "\" does not exist!" << std::endl;
		
		return;
	}
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
	g_EncodingNames.insert(std::make_pair(TextEncoding::UTF_16, "UTF-16 encoded Unicode with Byte Order Mark"));
	g_EncodingNames.insert(std::make_pair(TextEncoding::UTF_16_BE, "UTF-16BE encoded Unicode in Big Endian"));
	g_EncodingNames.insert(std::make_pair(TextEncoding::UTF_8, "UTF-8 encoded Unicode"));
	
	// language codes according to ISO 639-2 (alpha-3 code)
	g_ISO_639_2_Codes.insert(std::make_pair("deu", "German"));
	g_ISO_639_2_Codes.insert(std::make_pair("dut", "Dutch; Flemish"));
	g_ISO_639_2_Codes.insert(std::make_pair("eng", "English"));
	g_ISO_639_2_Codes.insert(std::make_pair("ger", "German"));
	g_ISO_639_2_Codes.insert(std::make_pair("ita", "Italian"));
	g_ISO_639_2_Codes.insert(std::make_pair("rus", "Russian"));
	
	// country codes according to ISO 3166-1 alpha-2
	g_ISO_3166_1_Alpha_2_Codes.insert(std::make_pair("GB", "United Kingdom"));
	g_ISO_3166_1_Alpha_2_Codes.insert(std::make_pair("ZA", "South Africa"));
	
	// picture types
	g_PictureTypes.insert(std::make_pair(0x00, "Other"));
	g_PictureTypes.insert(std::make_pair(0x01, "32x32 pixels 'file icon' (PNG only)"));
	g_PictureTypes.insert(std::make_pair(0x02, "Other file icon"));
	g_PictureTypes.insert(std::make_pair(0x03, "Cover (front)"));
	g_PictureTypes.insert(std::make_pair(0x04, "Cover (back)"));
	g_PictureTypes.insert(std::make_pair(0x05, "Leaflet page"));
	g_PictureTypes.insert(std::make_pair(0x06, "Media (e.g. lable side of CD)"));
	g_PictureTypes.insert(std::make_pair(0x07, "Lead artist/lead performer/soloist"));
	g_PictureTypes.insert(std::make_pair(0x08, "Artist/performer"));
	g_PictureTypes.insert(std::make_pair(0x09, "Conductor"));
	g_PictureTypes.insert(std::make_pair(0x0a, "Band/Orchestra"));
	g_PictureTypes.insert(std::make_pair(0x0b, "Composer"));
	g_PictureTypes.insert(std::make_pair(0x0c, "Lyricist/text writer"));
	g_PictureTypes.insert(std::make_pair(0x0d, "Recording Location"));
	g_PictureTypes.insert(std::make_pair(0x0e, "During recording"));
	g_PictureTypes.insert(std::make_pair(0x0f, "During performance"));
	g_PictureTypes.insert(std::make_pair(0x10, "Movie/video screen capture"));
	g_PictureTypes.insert(std::make_pair(0x11, "A bright coloured fish"));
	g_PictureTypes.insert(std::make_pair(0x12, "Illustration"));
	g_PictureTypes.insert(std::make_pair(0x13, "Band/artist logotype"));
	g_PictureTypes.insert(std::make_pair(0x14, "Publisher/Studio logotype"));
	
	// ID3v2.2.0
	FrameHeader::Handle22("COM", "Comment", Handle22COMFrame);
	FrameHeader::Handle22("PIC", "Attached Picture", Handle22PICFrames);
	FrameHeader::Handle22("TAL", "Album/Movie/Show title", Handle22T__Frames);
	FrameHeader::Handle22("TCM", "Composer", Handle22T__Frames);
	FrameHeader::Handle22("TCO", "Content type", Handle22T__Frames);
	FrameHeader::Handle22("TEN", "Encoded by", Handle22T__Frames);
	FrameHeader::Handle22("TP1", "Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group", Handle22T__Frames);
	FrameHeader::Handle22("TP2", "Band/Orchestra/Accompaniment", Handle22T__Frames);
	FrameHeader::Handle22("TPA", "Part of a set", Handle22T__Frames);
	FrameHeader::Handle22("TRK", "Track number/Position in set", Handle22T__Frames);
	FrameHeader::Handle22("TT1", "Content group description", Handle22T__Frames);
	FrameHeader::Handle22("TT2", "Title/Songname/Content description", Handle22T__Frames);
	FrameHeader::Handle22("TYE", "Year", Handle22T__Frames);
	FrameHeader::Handle22("UFI", "Unique file identifier", Handle22UFIFrames);
	// forbidden tags
	FrameHeader::Forbid22("TCP", "This frame is not officially defined for tag version 2.2 but has been seen used nonetheless.");
	FrameHeader::Handle22("TCP", "Compilation (from the internet)", Handle22T__Frames);
	
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
	FrameHeader::Handle23("UFID", "Unique file identifier", Handle23UFIDFrame);
	FrameHeader::Handle23("WCOM", "Commercial information", Handle23W___Frames);
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
	FrameHeader::Handle23("TDRC", "Recording time (from tag version 2.4)", Handle24T___Frames);
	FrameHeader::Forbid23("TDTG", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDTG", "Tagging time (from tag version 2.4)", Handle24T___Frames);
	FrameHeader::Forbid23("TSST", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TSST", "Set subtitle (from tag version 2.4)", Handle24T___Frames);
	FrameHeader::Forbid23("TSOA", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TSOA", "Album sort order (from tag version 2.4)", Handle24T___Frames);
	FrameHeader::Forbid23("TSOP", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TSOP", "Performer sort order (from tag version 2.4)", Handle24T___Frames);
	FrameHeader::Forbid23("TSO2", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate the Album Artist sort order.");
	FrameHeader::Handle23("TSO2", "Album artist sort order (by iTunes)", Handle24T___Frames);
	
	// ID3v2.4.0
	FrameHeader::Handle24("APIC", "Attached picture", Handle24APICFrame);
	FrameHeader::Handle24("COMM", "Comments", Handle24COMMFrame);
	FrameHeader::Handle24("MCDI", "Music CD identifier", Handle24MCDIFrame);
	FrameHeader::Handle24("PRIV", "Private frame", 0);
	FrameHeader::Handle24("TALB", "Album/Movie/Show title", Handle24T___Frames);
	FrameHeader::Handle24("TCOM", "Composer", Handle24T___Frames);
	FrameHeader::Handle24("TCON", "Content type", Handle24T___Frames);
	FrameHeader::Handle24("TCOP", "Copyright message", Handle24T___Frames);
	FrameHeader::Handle24("TDRC", "Recording time", Handle24T___Frames);
	FrameHeader::Handle24("TDRL", "Release time", Handle24T___Frames);
	FrameHeader::Handle24("TDTG", "Tagging time", Handle24T___Frames);
	FrameHeader::Handle24("TENC", "Encoded by", Handle24T___Frames);
	FrameHeader::Handle24("TIT2", "Title/songname/content description", Handle24T___Frames);
	FrameHeader::Handle24("TLAN", "Language(s)", Handle24T___Frames);
	FrameHeader::Handle24("TLEN", "Length", Handle24T___Frames);
	FrameHeader::Handle24("TPE1", "Lead performer(s)/Soloist(s)", Handle24T___Frames);
	FrameHeader::Handle24("TPE2", "Band/orchestra/accompaniment", Handle24T___Frames);
	FrameHeader::Handle24("TPOS", "Part of a set", Handle24T___Frames);
	FrameHeader::Handle24("TPUB", "Publisher", Handle24T___Frames);
	FrameHeader::Handle24("TRCK", "Track number/Position in set", Handle24T___Frames);
	FrameHeader::Handle24("TSSE", "Software/Hardware and settings used for encoding", Handle24T___Frames);
	FrameHeader::Handle24("TXXX", "User defined text information frame", Handle24TXXXFrame);
	FrameHeader::Handle24("USLT", "Unsynchronised lyrics/text transcription", Handle24USLTFrame);
	FrameHeader::Handle24("WXXX", "User defined URL link frame", Handle24WXXXFrame);
	// forbidden tags
	FrameHeader::Forbid24("TYER", "This frame is not defined in tag version 2.4. It has only been valid until tag version 2.3.");
	FrameHeader::Handle24("TYER", "Year (from tag version 2.3)", Handle23T___Frames);
	
	// processing
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front());
		Paths.pop_front();
	}

	return 0;
}
