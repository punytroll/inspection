#ifndef INSPECTION_COMMON_ID3_HELPER_H
#define INSPECTION_COMMON_ID3_HELPER_H

#include <cstdint>
#include <string>

#include "unknown_value_exception.h"

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
			try
			{
				auto Genre{Inspection::Get_ID3_1_Genre(GenreNumber.second)};
				
				Result.first = true;
				Result.second = '"' + Genre + "\" (reference to numeric genre from ID3v1)";
			}
			catch(Inspection::UnknownValueException & Exception)
			{
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
			try
			{
				auto Genre{Inspection::Get_ID3_1_Winamp_Genre(GenreNumber.second)};
				
				Result.first = true;
				Result.second = '"' + Genre + "\" (reference to numeric genre from ID3v1)";
			}
			catch(Inspection::UnknownValueException & Exception)
			{
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

#endif
