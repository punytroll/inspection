#include <assert.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include <deque>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <vector>

inline void AppendSeparated(std::string & String, const std::string & Append, const std::string & Separator)
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

std::string GetUTF_8CharFromUnicodeCodepoint(unsigned int Codepoint)
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

std::string GetUTF_8CharFromISO_IEC_8859_1Character(const char * Buffer, int Length)
{
	assert(Length >= 1);
	
	return GetUTF_8CharFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
}

std::string GetUTF_8CharFromUCS_2_BECharacter(const char * Buffer, int Length)
{
	assert(Length >= 2);
	
	return GetUTF_8CharFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])));
}

std::string GetUTF_8CharFromUCS_2_LECharacter(const char * Buffer, int Length)
{
	assert(Length >= 2);
	
	return GetUTF_8CharFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
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

std::pair< bool, uint16_t > GetUInt16LE(const char * Buffer, int Length)
{
	std::pair< bool, uint16_t > Result(false, 0);
	
	if(Length >= 2)
	{
		Result.first = true;
		Result.second = Buffer[1] * 256 + Buffer[0];
	}
	
	return Result;
}

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
			
			std::map< std::string, int (*)(const char * const, int) >::iterator HanderIterator(_Handlers22.find(_Identifier));
			
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
			
			std::map< std::string, int (*)(const char * const, int) >::iterator HanderIterator(_Handlers23.find(_Identifier));
			
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
			
			std::map< std::string, int (*)(const char * const, int) >::iterator HanderIterator(_Handlers24.find(_Identifier));
			
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
	
	int HandleData(const char * const Buffer, unsigned int Length)
	{
		if(_Handler != 0)
		{
			return _Handler(Buffer, Length);
		}
		else
		{
			std::cout << "*** WARNING ***  No handler defined for the frame type \"" << _Identifier << "\" in this tag version." << std::endl;
			
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
	
	static void Handle22(const std::string & Identifier, const std::string & Name, int (* Handler) (const char *, int))
	{
		_Handlers22.insert(std::make_pair(Identifier, Handler));
		_Names22.insert(std::make_pair(Identifier, Name));
	}
	
	static void Forbid23(const std::string & Identifier, const std::string & Reason)
	{
		_Forbidden23.insert(std::make_pair(Identifier, Reason));
	}
	
	static void Handle23(const std::string & Identifier, const std::string & Name, int (* Handler) (const char *, int))
	{
		_Handlers23.insert(std::make_pair(Identifier, Handler));
		_Names23.insert(std::make_pair(Identifier, Name));
	}
	
	static void Forbid24(const std::string & Identifier, const std::string & Reason)
	{
		_Forbidden24.insert(std::make_pair(Identifier, Reason));
	}
	
	static void Handle24(const std::string & Identifier, const std::string & Name, int (* Handler) (const char *, int))
	{
		_Handlers24.insert(std::make_pair(Identifier, Handler));
		_Names24.insert(std::make_pair(Identifier, Name));
	}
private:
	// static setup
	static std::map< std::string, std::string > _Forbidden22;
	static std::map< std::string, std::string > _Forbidden23;
	static std::map< std::string, std::string > _Forbidden24;
	static std::map< std::string, int (*) (const char *, int) > _Handlers22;
	static std::map< std::string, int (*) (const char *, int) > _Handlers23;
	static std::map< std::string, int (*) (const char *, int) > _Handlers24;
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
	int (* _Handler)(const char * const Buffer, int Length);
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

std::map< unsigned int, std::string > g_Genres1_0;
std::map< std::string, std::string > g_ISO_639_2_Codes;
std::map< std::string, std::string > g_ISO_3166_1_Alpha_2_Codes;
std::map< unsigned int, std::string > g_PictureTypes;
std::map< unsigned int, std::string > g_Encodings2_2;
std::map< unsigned int, std::string > g_Encodings2_3;
std::map< unsigned int, std::string > g_Encodings2_4;
std::map< std::string, std::string > g_GUIDDescriptions;
std::map< std::string, std::string > FrameHeader::_Forbidden22;
std::map< std::string, std::string > FrameHeader::_Forbidden23;
std::map< std::string, std::string > FrameHeader::_Forbidden24;
std::map< std::string, std::string > FrameHeader::_Names22;
std::map< std::string, std::string > FrameHeader::_Names23;
std::map< std::string, std::string > FrameHeader::_Names24;
std::map< std::string, int (*) (const char *, int) > FrameHeader::_Handlers22;
std::map< std::string, int (*) (const char *, int) > FrameHeader::_Handlers23;
std::map< std::string, int (*) (const char *, int) > FrameHeader::_Handlers24;
bool g_PrintBytes(false);

std::string GetEncodingString(unsigned int Encoding, const std::map< unsigned int, std::string > & Encodings)
{
	std::stringstream Result;
	std::map< unsigned int, std::string >::const_iterator EncodingIterator(Encodings.find(Encoding));
	
	if(EncodingIterator != Encodings.end())
	{
		Result << EncodingIterator->second;
	}
	else
	{
		Result << "<invalid encoding>";
	}
	Result << " (" << Encoding << ")";
	
	return Result.str();
}

std::string GetEncodingString2_2(unsigned int Encoding)
{
	return GetEncodingString(Encoding, g_Encodings2_2);
}

std::string GetEncodingString2_3(unsigned int Encoding)
{
	return GetEncodingString(Encoding, g_Encodings2_3);
}

std::string GetEncodingString2_4(unsigned int Encoding)
{
	return GetEncodingString(Encoding, g_Encodings2_4);
}

std::pair< bool, std::string > GetSimpleID3_1GenreReferenceInterpretation(const std::string & ContentType)
{
	std::pair< bool, std::string > Result(false, "");
	
	if((ContentType.length() >= 3) && (ContentType[0] == '(') && (ContentType[ContentType.length() - 1] == ')'))
	{
		std::pair< bool, unsigned int > GenreNumber(GetUnsignedIntegerFromDecimalASCIIString(ContentType.substr(1, ContentType.length() - 2)));
		
		if(GenreNumber.first == true)
		{
			std::map< unsigned int, std::string >::iterator Genre1_0Iterator(g_Genres1_0.find(GenreNumber.second));
			
			if(Genre1_0Iterator != g_Genres1_0.end())
			{
				Result.first = true;
				Result.second = "reference to numeric genre from ID3v1 \"" + Genre1_0Iterator->second + '"';
			}
		}
	}
	
	return Result;
}

std::string GetContentTypeInterpretation2_3(const std::string & ContentType)
{
	std::pair< bool, std::string > Interpretation(false, "");
	
	Interpretation = GetSimpleID3_1GenreReferenceInterpretation(ContentType);
	if(Interpretation.first == true)
	{
		return Interpretation.second;
	}
	
	return "";
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

std::pair< int, std::string > GetHexadecimalStringTerminatedByLength(const char * Buffer, int Length)
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

std::pair< int, std::string > GetGUIDString(const char * Buffer, int Length)
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

///////////////////////////////////////////////////////////////////////////////////////////////////
// Queries                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
bool IsISO_IEC_8859_1Character(const char * Buffer, int Length);
bool IsISO_IEC_8859_1StringWithoutTermination(const char * Buffer, int Length);
bool IsISO_IEC_8859_1Termination(const char * Buffer, int Length);
bool IsUCS_2_BECharacter(const char * Buffer, int Length);
bool IsUCS_2_BEStringWithoutByteOrderMarkWithTermination(const char * Buffer, int Length);
bool IsUCS_2_BETermination(const char * Buffer, int Length);
bool IsUCS_2_LECharacter(const char * Buffer, int Length);
bool IsUCS_2_LEStringWithoutByteOrderMarkWithTermination(const char * Buffer, int Length);
bool IsUCS_2_LETermination(const char * Buffer, int Length);
bool StartsWithISO_IEC_8859_1StringWithTermination(const char * Buffer, int Length);

bool IsISO_IEC_8859_1Character(const char * Buffer, int Length)
{
	if(Length >= 1)
	{
		return ((Buffer[0] >= 0x20) && (Buffer[0] <= 0x7e)) || ((Buffer[0] >= 0xa0) && (Buffer[0] <= 0xff));
	}
	else
	{
		return false;
	}
}

bool IsISO_IEC_8859_1StringWithoutTermination(const char * Buffer, int Length)
{
	int Index(0);
	
	while(Index < Length)
	{
		if(IsISO_IEC_8859_1Character(Buffer + Index, Length - Index) == false)
		{
			return false;
		}
		Index += 1;
	}
	
	return true;
}

bool IsISO_IEC_8859_1Termination(const char * Buffer, int Length)
{
	if(Length >= 1)
	{
		return Buffer[0] == 0x00;
	}
	else
	{
		return false;
	}
}

bool IsUCS_2_BECharacter(const char * Buffer, int Length)
{
	if(Length >= 2)
	{
		return (Buffer[0] < 0xd8) || (Buffer[0] > 0xdf);
	}
	else
	{
		return false;
	}
}

bool IsUCS_2_BEStringWithoutByteOrderMarkWithTermination(const char * Buffer, int Length)
{
	int Index(0);
	
	if((Length < 2) || (Length % 2 != 0))
	{
		return false;
	}
	else
	{
		while(Index < Length - 2)
		{
			if(IsUCS_2_BECharacter(Buffer + Index, Length - Index) == false)
			{
				return false;
			}
			Index += 2;
		}
		if(IsUCS_2_BETermination(Buffer + Index, Length - Index) == false)
		{
			return false;
		}
	}
	
	return true;
}

bool IsUCS_2_BETermination(const char * Buffer, int Length)
{
	if(Length >= 2)
	{
		return Buffer[0] == 0x00 && Buffer[1] == 0x00;
	}
	else
	{
		return false;
	}
}

bool IsUCS_2_LECharacter(const char * Buffer, int Length)
{
	if(Length >= 2)
	{
		return (Buffer[1] < 0xd8) || (Buffer[1] > 0xdf);
	}
	else
	{
		return false;
	}
}

bool IsUCS_2_LEStringWithoutByteOrderMarkWithTermination(const char * Buffer, int Length)
{
	int Index(0);
	
	if((Length < 2) || (Length % 2 != 0))
	{
		return false;
	}
	else
	{
		while(Index < Length - 2)
		{
			if(IsUCS_2_LECharacter(Buffer + Index, Length - Index) == false)
			{
				return false;
			}
			Index += 2;
		}
		if(IsUCS_2_LETermination(Buffer + Index, Length - Index) == false)
		{
			return false;
		}
	}
	
	return true;
}

bool IsUCS_2_LETermination(const char * Buffer, int Length)
{
	if(Length >= 2)
	{
		return Buffer[1] == 0x00 && Buffer[0] == 0x00;
	}
	else
	{
		return false;
	}
}

bool StartsWithISO_IEC_8859_1StringWithTermination(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 <= Length)
		{
			if(IsISO_IEC_8859_1Termination(Buffer + Index, Length - Index) == true)
			{
				return true;
			}
			else if(IsISO_IEC_8859_1Character(Buffer + Index, Length - Index) == true)
			{
				Index += 1;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ISO/IEC 8859-1                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::pair< int, std::string > GetISO_IEC_8859_1StringTerminatedByEnd(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(true)
	{
		assert(Result.first + 1 <= Length);
		if(IsISO_IEC_8859_1Termination(Buffer + Result.first, Length - Result.first) == true)
		{
			Result.first += 1;
			
			break;
		}
		else if(IsISO_IEC_8859_1Character(Buffer + Result.first, Length - Result.first) == true)
		{
			Result.second += GetUTF_8CharFromISO_IEC_8859_1Character(Buffer + Result.first, Length - Result.first);
			Result.first += 1;
		}
		else
		{
			assert(false);
		}
	}
	
	return Result;
}

std::pair< int, std::string > GetISO_IEC_8859_1StringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(true)
	{
		if(Result.first + 1 <= Length)
		{
			if(IsISO_IEC_8859_1Termination(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.first += 1;
				
				break;
			}
			else if(IsISO_IEC_8859_1Character(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.second += GetUTF_8CharFromISO_IEC_8859_1Character(Buffer + Result.first, Length - Result.first);
				Result.first += 1;
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			break;
		}
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UTF-8                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUTF_8StringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index < Length)
		{
			if(Buffer[Index] != '\0')
			{
				std::cout << Buffer[Index];
				Index += 1;
			}
			else
			{
				return Index + 1;
			}
		}
		else
		{
			std::cout << "*** ERROR *** UTF-8 string should be null-terminated but exceeds its possible length." << std::endl;
			
			return Index;
		}
	}
}

int PrintUTF_8StringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index < Length)
		{
			if(Buffer[Index] != '\0')
			{
				std::cout << Buffer[Index];
				Index += 1;
			}
			else
			{
				return Index + 1;
			}
		}
		else
		{
			return Index;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UCS-2BE                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUCS_2_BEStringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 >= Length)
		{
			std::cout << "*** ERROR *** UCS-2BE string should be null-terminated but exceeds its possible length." << std::endl;
			
			return Index;
		}
		else
		{
			if((Buffer[Index] == '\0') && (Buffer[Index + 1] == '\0'))
			{
				return Index + 2;
			}
			else
			{
				std::cout << GetUTF_8CharFromUCS_2_BECharacter(Buffer + Index, Length - Index);
				Index += 2;
			}
		}
	}
}

std::pair< int, std::string > GetUCS_2_BEStringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(true)
	{
		if(Result.first + 2 <= Length)
		{
			if(IsUCS_2_BETermination(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.first += 1;
				
				break;
			}
			else if(IsUCS_2_BECharacter(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.second += GetUTF_8CharFromUCS_2_BECharacter(Buffer + Result.first, Length - Result.first);
				Result.first += 2;
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			break;
		}
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UCS-2LE                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::pair< int, std::string > GetUCS_2_LEStringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(true)
	{
		if(Result.first + 2 <= Length)
		{
			if(IsUCS_2_LETermination(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.first += 2;
				
				break;
			}
			else if(IsUCS_2_LECharacter(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.second += GetUTF_8CharFromUCS_2_LECharacter(Buffer + Result.first, Length - Result.first);
				Result.first += 2;
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			break;
		}
	}
	
	return Result;
}

int PrintUCS_2_LEStringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 >= Length)
		{
			std::cout << "*** ERROR *** UCS-2LE string should be null-terminated but exceeds its possible length." << std::endl;
			
			return Index;
		}
		else
		{
			if((Buffer[Index] == '\0') && (Buffer[Index + 1] == '\0'))
			{
				return Index + 2;
			}
			else
			{
				std::cout << GetUTF_8CharFromUCS_2_LECharacter(Buffer + Index, Length - Index);
				Index += 2;
			}
		}
	}
}

std::pair< int, std::string > GetUCS_2_LEStringWithoutByteOrderMarkWithTerminationAtStart(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(true)
	{
		if(Result.first + 2 > Length)
		{
			assert(false);
		}
		else
		{
			if(IsUCS_2_LETermination(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.first += 2;
				
				break;
			}
			else if(IsUCS_2_LECharacter(Buffer + Result.first, Length - Result.first) == true)
			{
				Result.second += GetUTF_8CharFromUCS_2_LECharacter(Buffer + Result.first, Length - Result.first);
				Result.first += 2;
			}
			else
			{
				assert(false);
			}
		}
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// UCS-2                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUCS_2StringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	if(Length >= 2)
	{
		if((static_cast< unsigned char >(Buffer[Index]) == 0xfe) && (static_cast< unsigned char >(Buffer[Index + 1]) == 0xff))
		{
			Index += 2;
			// Big Endian by Byte Order Marker
			Index += PrintUCS_2_BEStringTerminatedByEnd(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned char >(Buffer[Index]) == 0xff) && (static_cast< unsigned char >(Buffer[Index + 1]) == 0xfe))
		{
			Index += 2;
			// Little Endian by Byte Order Marker
			Index += PrintUCS_2_LEStringTerminatedByEnd(Buffer + Index, Length - Index);
		}
		else
		{
			std::cout << "*** ERROR *** UCS-2 string is expected to start with a Byte Order Mark but is not." << std::endl;
		}
	}
	else
	{
		std::cout << "*** The UCS-2 string is expected to contain a Byte Order Marker but is not long enough to hold that information." << std::endl;
		Index = Length;
	}
	
	return Index;
}

int PrintUCS_2StringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	int Index(0);
	
	if(Length >= 2)
	{
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index]) == 0xfe)) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1]) == 0xff)))
		{
			Index += 2;
			
			// Big Endian by Byte Order Marker
			std::pair< int, std::string > ReadString(GetUCS_2_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
			
			Index += ReadString.first;
			std::cout << ReadString.second;
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index]) == 0xff)) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1]) == 0xfe)))
		{
			Index += 2;
			
			// Little Endian by Byte Order Marker
			std::pair< int, std::string > ReadString(GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
			
			Index += ReadString.first;
			std::cout << ReadString.second;
		}
		else
		{
			std::cout << "*** ERROR *** UCS-2 string is expected to start with a Byte Order Mark but is not." << std::endl;
		}
	}
	else
	{
		std::cout << "*** The UCS-2 string is expected to contain a Byte Order Marker but is not long enough to hold that information." << std::endl;
		Index = Length;
	}
	
	return Index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UTF-16BE                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUTF_16_BEStringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 >= Length)
		{
			std::cout << "*** ERROR *** UTF-16BE string should be null-terminated but exceeds its possible length." << std::endl;
			
			return Index;
		}
		else
		{
			if((Buffer[Index] == '\0') && (Buffer[Index + 1] == '\0'))
			{
				return Index + 2;
			}
			else
			{
				if((Buffer[Index] > 0xd8) && (Buffer[Index] < 0xe0))
				{
					std::cout << "*** ERROR *** UTF-16BE contains unhandled characters outside the Basic Multilingual Plane." << std::endl;
				}
				else
				{
					std::cout << GetUTF_8CharFromUCS_2_BECharacter(Buffer + Index, Length - Index);
				}
				Index += 2;
			}
		}
	}
}

int PrintUTF_16_BEStringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 >= Length)
		{
			return Index;
		}
		else
		{
			if((Buffer[Index] == '\0') && (Buffer[Index + 1] == '\0'))
			{
				return Index + 2;
			}
			else
			{
				if((Buffer[Index] > 0xd8) && (Buffer[Index] < 0xe0))
				{
					std::cout << "*** ERROR *** UTF-16BE contains unhandled characters outside the Basic Multilingual Plane." << std::endl;
				}
				else
				{
					std::cout << GetUTF_8CharFromUCS_2_BECharacter(Buffer + Index, Length - Index);
				}
				Index += 2;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UTF-16LE                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUTF_16_LEStringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 >= Length)
		{
			std::cout << "*** ERROR *** UTF-16LE string should be null-terminated but exceeds its possible length." << std::endl;
			
			return Index;
		}
		else
		{
			if((Buffer[Index] == '\0') && (Buffer[Index + 1] == '\0'))
			{
				return Index + 2;
			}
			else
			{
				if((Buffer[Index + 1] > 0xd8) && (Buffer[Index + 1] < 0xe0))
				{
					std::cout << "*** ERROR *** UTF-16BE contains unhandled characters outside the Basic Multilingual Plane." << std::endl;
				}
				else
				{
					std::cout << GetUTF_8CharFromUCS_2_LECharacter(Buffer + Index, Length - Index);
				}
				Index += 2;
			}
		}
	}
}

int PrintUTF_16_LEStringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index + 1 >= Length)
		{
			return Index;
		}
		else
		{
			if((Buffer[Index] == '\0') && (Buffer[Index + 1] == '\0'))
			{
				return Index + 2;
			}
			else
			{
				if((Buffer[Index + 1] > 0xd8) && (Buffer[Index + 1] < 0xe0))
				{
					std::cout << "*** ERROR *** UTF-16BE contains unhandled characters outside the Basic Multilingual Plane." << std::endl;
				}
				else
				{
					std::cout << GetUTF_8CharFromUCS_2_LECharacter(Buffer + Index, Length - Index);
				}
				Index += 2;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UTF-16                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUTF_16StringTerminatedByEnd(const char * Buffer, int Length)
{
	int Index(0);
	
	if(Length >= 2)
	{
		if((static_cast< unsigned char >(Buffer[Index]) == 0xfe) && (static_cast< unsigned char >(Buffer[Index + 1]) == 0xff))
		{
			Index += 2;
			// Big Endian by Byte Order Marker
			Index += PrintUTF_16_BEStringTerminatedByEnd(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned char >(Buffer[Index]) == 0xff) && (static_cast< unsigned char >(Buffer[Index + 1]) == 0xfe))
		{
			Index += 2;
			// Little Endian by Byte Order Marker
			Index += PrintUTF_16_LEStringTerminatedByEnd(Buffer + Index, Length - Index);
		}
		else
		{
			std::cout << "*** ERROR *** UTF-16 string is expected to start with a Byte Order Mark but is not." << std::endl;
		}
	}
	else
	{
		std::cout << "*** The UTF-16 string is expected to contain a Byte Order Marker but is not long enough to hold that information." << std::endl;
		Index = Length;
	}
	
	return Index;
}

int PrintUTF_16StringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	int Index(0);
	
	if(Length >= 2)
	{
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index]) == 0xfe)) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1]) == 0xff)))
		{
			Index += 2;
			// Big Endian by Byte Order Marker
			Index += PrintUTF_16_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index]) == 0xff)) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1]) == 0xfe)))
		{
			Index += 2;
			// Little Endian by Byte Order Marker
			Index += PrintUTF_16_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else
		{
			std::cout << "*** ERROR *** UTF-16 string is expected to start with a Byte Order Mark but is not." << std::endl;
		}
	}
	else
	{
		std::cout << "*** The UTF-16 string is expected to contain a Byte Order Marker but is not long enough to hold that information." << std::endl;
		Index = Length;
	}
	
	return Index;
}

int Handle23UserTextFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	std::cout << "\t\t\t\tDescription: ";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << std::endl;
	std::cout << "\t\t\t\tString: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadString(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadString.first;
		std::cout << ReadString.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	return Index;
}

int Handle24UserTextFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_4(Encoding) << std::endl;
	std::cout << "\t\t\t\tDescription: ";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_16StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else if(Encoding == 2)
	{
		Index += PrintUTF_16_BEStringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else if(Encoding == 3)
	{
		Index += PrintUTF_8StringTerminatedByEnd(Buffer+ Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << std::endl;
	std::cout << "\t\t\t\tString: ";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadString(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadString.first;
		std::cout << ReadString.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_16StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 2)
	{
		Index += PrintUTF_16_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_8StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << std::endl;
	
	return Index;
}

int Handle23CommentFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	
	std::string ISO_639_2_Code(Buffer + Index, Buffer + Index + 3);
	
	Index += 3;
	if(ISO_639_2_Code.empty() == false)
	{
		std::map< std::string, std::string >::iterator ISO_639_2_Iterator(g_ISO_639_2_Codes.find(ISO_639_2_Code));
		
		if(ISO_639_2_Iterator != g_ISO_639_2_Codes.end())
		{
			std::cout << "\t\t\t\tLanguage: " << ISO_639_2_Iterator->second << " (\"" << ISO_639_2_Code << "\")" << std::endl;
		}
		else
		{
			std::cout << "\t\t\t\tLanguage: <unknown> (\"" << ISO_639_2_Code << "\")" << std::endl;
			std::cout << "*** ERROR *** The language code '" << ISO_639_2_Code << "' is not defined by ISO 639-2." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
	}
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl << "\t\t\t\tComment: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadComment(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadComment.first;
		std::cout << ReadComment.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	return Index;
}

int Handle22COMFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_2(Encoding) << std::endl;
	
	std::string ISO_639_2Code(Buffer + Index, Buffer + Index + 3);
	
	Index += 3;
	if(ISO_639_2Code.empty() == false)
	{
		std::map< std::string, std::string >::iterator ISO_639_2Iterator(g_ISO_639_2_Codes.find(ISO_639_2Code));
		
		if(ISO_639_2Iterator != g_ISO_639_2_Codes.end())
		{
			std::cout << "\t\t\t\tLanguage: " << ISO_639_2Iterator->second << " (\"" << ISO_639_2Code << "\")" << std::endl;
		}
		else
		{
			std::cout << "\t\t\t\tLanguage: <unknown> (\"" << ISO_639_2Code << "\")" << std::endl;
			std::cout << "*** ERROR *** The language code '" << ISO_639_2Code << "' is not defined by ISO 639-2." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
	}
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl << "\t\t\t\tComment: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadComment(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadComment.first;
		std::cout << ReadComment.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	return Index;
}

int Handle24COMMFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_4(Encoding) << std::endl;
	
	std::string ISO_639_2_Code(Buffer + Index, Buffer + Index + 3);
	
	Index += 3;
	if(ISO_639_2_Code.empty() == false)
	{
		std::map< std::string, std::string >::iterator ISO_639_2_Iterator(g_ISO_639_2_Codes.find(ISO_639_2_Code));
		
		if(ISO_639_2_Iterator != g_ISO_639_2_Codes.end())
		{
			std::cout << "\t\t\t\tLanguage: " << ISO_639_2_Iterator->second << " (\"" << ISO_639_2_Code << "\")" << std::endl;
		}
		else
		{
			std::cout << "\t\t\t\tLanguage: <unknown> (\"" << ISO_639_2_Code << "\")" << std::endl;
			std::cout << "*** ERROR *** The language code '" << ISO_639_2_Code << "' is not defined by ISO 639-2." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** The language code is empty, which is not allowed by either ID3 version 2.3 or ISO 639-2 for language codes." << std::endl;
	}
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_16StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else if(Encoding == 2)
	{
		Index += PrintUTF_16_BEStringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else if(Encoding == 3)
	{
		Index += PrintUTF_8StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl << "\t\t\t\tComment: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadComment(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadComment.first;
		std::cout << ReadComment.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_16StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 2)
	{
		Index += PrintUTF_16_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 3)
	{
		Index += PrintUTF_8StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' <<  std::endl;
	
	return Index;
}

int Handle23AttachedPicture(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	
	std::pair< int, std::string > ReadMIMEType(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
	
	Index += ReadMIMEType.first;
	std::cout << "\t\t\t\tMIME type: \"" << ReadMIMEType.second << '"' << std::endl;
	
	unsigned int PictureType(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tPicture type: " << GetPictureTypeString(PictureType) << std::endl;
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		if(StartsWithISO_IEC_8859_1StringWithTermination(Buffer + Index, Length - Index) == true)
		{
			std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
			
			Index += ReadDescription.first;
			std::cout << ReadDescription.second;
		}
		else
		{
			std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
			
			std::cout << "*** ERROR *** Invalid string for ISO/IEC 8859-1 encoding." << std::endl;
			std::cout << "              Binary content: " << ReadHexadecimal.second << std::endl;
		}
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	return Length;
}

int Handle24APICFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_4(Encoding) << std::endl;
	
	std::pair< int, std::string > ReadMIMEType(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
	
	Index += ReadMIMEType.first;
	std::cout << "\t\t\t\tMIME type: \"" << ReadMIMEType.second << '"' << std::endl;
	
	unsigned int PictureType(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	std::cout << "\t\t\t\tPicture type: " << GetPictureTypeString(PictureType) << std::endl;
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_16StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 2)
	{
		Index += PrintUTF_16_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 3)
	{
		Index += PrintUTF_8StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	std::cout << '"' << std::endl;
	
	return Length;
}

int Handle23URLFrame(const char * Buffer, int Length)
{
	int Index(0);
	std::pair< int, std::string > ReadURL(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
	
	Index += ReadURL.first;
	std::cout << "\t\t\t\tURL: \"" << ReadURL.second << '"' << std::endl;
	
	return Index;
}

int Handle23UserURLFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	std::pair< int, std::string > ReadURL(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
	
	Index += ReadURL.first;
	std::cout << "\t\t\t\tURL: \"" << ReadURL.second << '"' << std::endl;
	
	return Index;
}

int Handle22And23TextFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadString(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadString.first;
		std::cout << "\t\t\t\tString: \"" << ReadString.second;
	}
	else if(Encoding == 1)
	{
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xff))
		{
			Index += 2;
			// Big Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Big Endian" << std::endl;
			
			std::pair< int, std::string > ReadString(GetUCS_2_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
			
			Index += ReadString.first;
			std::cout << "\t\t\t\tString: \"" << ReadString.second;
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xfe))
		{
			Index += 2;
			// Little Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Little Endian" << std::endl;
			
			std::pair< int, std::string > ReadString(GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
			
			Index += ReadString.first;
			std::cout << "\t\t\t\tString: \"" << ReadString.second;
		}
		else
		{
			std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	return Index;
}

int Handle24TextFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_4(Encoding) << std::endl;
	if(Encoding == 1)
	{
		std::cout << "\t\t\t\tByte Order Mark: ";
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xff))
		{
			std::cout << "Big Endian";
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xfe))
		{
			std::cout << "Little Endian";
		}
		else
		{
			std::cout << "Bogus Byte Order Mark";
		}
		std::cout << " (" << GetHexadecimalStringFromUInt8(Buffer[Index]) << ' ' << GetHexadecimalStringFromUInt8(Buffer[Index + 1]) + ')' << std::endl;
	}
	std::cout << "\t\t\t\tString(s):\n";
	while(Index < Length)
	{
		std::cout << "\t\t\t\t\t\"";
		if(Encoding == 0)
		{
			std::pair< int, std::string > ReadString(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
			
			Index += ReadString.first;
			std::cout << ReadString.second;
		}
		else if(Encoding == 1)
		{
			Index += PrintUTF_16StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if(Encoding == 2)
		{
			Index += PrintUTF_16_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if(Encoding == 3)
		{
			Index += PrintUTF_8StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		std::cout << '"' << std::endl;
	}
	
	return Index;
}

int Handle23TCMPFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	if(Encoding == 1)
	{
		std::cout << "\t\t\t\tByte Order Mark: ";
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xff))
		{
			std::cout << "Big Endian";
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xfe))
		{
			std::cout << "Little Endian";
		}
		else
		{
			std::cout << "Bogus Byte Order Mark";
		}
		std::cout << " (" << GetHexadecimalStringFromUInt8(Buffer[Index]) << ' ' << GetHexadecimalStringFromUInt8(Buffer[Index + 1]) + ')' << std::endl;
	}
	
	std::pair< int, std::string > ReadString;
	
	if(Encoding == 0)
	{
		ReadString = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 1)
	{
		/// @TODO Add this function.
		//~ ReadString = GetUCS_2StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << "\t\t\t\tPart of a compilation: ";
	if(ReadString.second == "1")
	{
		std::cout << "yes";
	}
	else if(ReadString.second == "0")
	{
		std::cout << "no";
	}
	else
	{
		std::cout << "<unknown value>";
	}
	std::cout << " (\"" << ReadString.second << "\")" << std::endl;
	Index += ReadString.first;
	
	return Index;
}

int Handle24WXXXFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_4(Encoding) << std::endl;
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
		Index += ReadDescription.first;
		std::cout << ReadDescription.second;
	}
	else if(Encoding == 1)
	{
		Index += PrintUTF_16StringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else if(Encoding == 2)
	{
		Index += PrintUTF_16_BEStringTerminatedByEnd(Buffer + Index, Length - Index);
	}
	else if(Encoding == 3)
	{
		Index += PrintUTF_8StringTerminatedByEnd(Buffer+ Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << '"' << std::endl;
	
	std::pair< int, std::string > ReadURL(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
	
	Index += ReadURL.first;
	std::cout << "\t\t\t\tURL: \"" << ReadURL.second << '"' << std::endl;
	
	return Index;
}

int Handle23MCDIFrame(const char * Buffer, int Length)
{
	int Index(0);
	std::pair< int, std::string > ReadString(GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
	
	Index += ReadString.first;
	std::cout << "\t\t\t\tCD Table Of Content: \"" << ReadString.second << '"' << std::endl;
	
	return Index;
}

int Handle23TCONFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	
	std::pair< int, std::string > ReadString;
	
	if(Encoding == 0)
	{
		ReadString = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 1)
	{
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xff))
		{
			Index += 2;
			// Big Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Big Endian" << std::endl;
			ReadString = GetUCS_2_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xfe))
		{
			Index += 2;
			// Little Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Little Endian" << std::endl;
			ReadString = GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else
		{
			std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	Index += ReadString.first;
	std::cout << "\t\t\t\tContent type: \"" << ReadString.second << '"' << std::endl;
	
	std::string Interpretation(GetContentTypeInterpretation2_3(ReadString.second));
	
	if(Interpretation.empty() == false)
	{
		std::cout << "\t\t\t\tInterpretation: " << Interpretation << std::endl;
	}
	
	return Index;
}

int Handle23TSRCFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	
	std::pair< int, std::string > ReadString;
	
	if(Encoding == 0)
	{
		ReadString = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 1)
	{
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xff))
		{
			Index += 2;
			// Big Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Big Endian" << std::endl;
			ReadString = GetUCS_2_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xfe))
		{
			Index += 2;
			// Little Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Little Endian" << std::endl;
			ReadString = GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else
		{
			std::cout << "*** ERROR *** Unicode string fails to provide a byte order mark." << std::endl;
		}
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	Index += ReadString.first;
	std::cout << "\t\t\t\tString: \"" << ReadString.second << '"' << std::endl;
	
	if(ReadString.second.length() == 12)
	{
		std::string ISO_3166_1_Alpha_2_Code(ReadString.second.substr(0, 2));
		std::map< std::string, std::string >::iterator ISO_3166_1_Alpha_2_Iterator(g_ISO_3166_1_Alpha_2_Codes.find(ISO_3166_1_Alpha_2_Code));
		
		if(ISO_3166_1_Alpha_2_Iterator != g_ISO_3166_1_Alpha_2_Codes.end())
		{
			std::cout << "\t\t\t\t\tCountry: " << ISO_3166_1_Alpha_2_Iterator->second << " (\"" << ISO_3166_1_Alpha_2_Code << "\")" << std::endl;
		}
		else
		{
			std::cout << "\t\t\t\t\tCountry: <unknown> (\"" << ISO_3166_1_Alpha_2_Code << "\")" << std::endl;
			std::cout << "*** ERROR *** The country code '" << ISO_3166_1_Alpha_2_Code << "' is not defined by ISO 3166-1 alpha-2." << std::endl;
		}
		std::cout << "\t\t\t\t\tRegistrant code: \"" << ReadString.second.substr(2, 3) << '"' << std::endl;
		std::cout << "\t\t\t\t\tYear of registration: \"" << ReadString.second.substr(5, 2) << '"' << std::endl;
		std::cout << "\t\t\t\t\tRegistration number: \"" << ReadString.second.substr(7, 5) << '"' << std::endl;
	}
	else
	{
		std::cout << "*** ERROR *** The international standard recording code defined by ISO 3901 requires the code to be 12 characters long, not " << ReadString.second.length() << "." << std::endl;
	}
	
	return Index;
}

int Handle23UFIDFrame(const char * Buffer, int Length)
{
	int Index(0);
	std::pair< int, std::string > ReadOwnerIdentifier(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
	
	Index += ReadOwnerIdentifier.first;
	std::cout << "\t\t\t\tOwner Identifier: \"" << ReadOwnerIdentifier.second << '"' << std::endl;
	if(ReadOwnerIdentifier.second.length() == 0)
	{
		std::cout << "*** ERROR *** Owner Identifier must be a valid URL, not empty." << std::endl;
	}
	if(Length - Index > 64)
	{
		std::cout << "*** ERROR *** Identifier field must not be longer than 64 bytes. Proceeding with interpretation." << std::endl;
	}
	
	std::pair< int, std::string > ReadIdentifier(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
	
	std::cout << "\t\t\t\tIdentifier (binary): " << ReadIdentifier.second << std::endl;
	
	if(IsISO_IEC_8859_1StringWithoutTermination(Buffer + Index, Length - Index) == true)
	{
		std::pair< int, std::string > ReadIdentifierAsString(GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		std::cout << "\t\t\t\tIdentifier (string): \"" << ReadIdentifierAsString.second << "\" (interpreted as ISO/IEC 8859-1)" << std::endl;
	}
	Index += ReadIdentifier.first;
	
	return Index;
}

int HandlePRIVFrame(const char * Buffer, int Length)
{
	int Index(0);
	std::pair< int, std::string > ReadOwnerIdentifier(GetISO_IEC_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
	
	Index += ReadOwnerIdentifier.first;
	std::cout << "\t\t\t\tOwner Identifier: " << ReadOwnerIdentifier.second << std::endl;
	if(ReadOwnerIdentifier.second == "WM/MediaClassPrimaryID")
	{
		std::pair< int, std::string > ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
		
		Index += ReadGUID.first;
		std::cout << "\t\t\t\tPrimary Media Class: ";
		
		std::map< std::string, std::string >::iterator GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
		
		if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
		{
			std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
		}
		else
		{
			std::cout << ReadGUID.second << " (unknown value)" << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "WM/MediaClassSecondaryID")
	{
		std::pair< int, std::string > ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
		
		Index += ReadGUID.first;
		std::cout << "\t\t\t\tSecondary Media Class: ";
		
		std::map< std::string, std::string >::iterator GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
		
		if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
		{
			std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
		}
		else
		{
			std::cout << ReadGUID.second << " (unknown value)" << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "WM/WMContentID")
	{
		std::pair< int, std::string > ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
		
		Index += ReadGUID.first;
		std::cout << "\t\t\t\tContent ID: ";
		
		std::map< std::string, std::string >::iterator GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
		
		if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
		{
			std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
		}
		else
		{
			std::cout << ReadGUID.second << " (unknown value)" << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "WM/WMCollectionID")
	{
		std::pair< int, std::string > ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
		
		Index += ReadGUID.first;
		std::cout << "\t\t\t\tCollection ID: ";
		
		std::map< std::string, std::string >::iterator GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
		
		if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
		{
			std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
		}
		else
		{
			std::cout << ReadGUID.second << " (unknown value)" << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "WM/WMCollectionGroupID")
	{
		std::pair< int, std::string > ReadGUID(GetGUIDString(Buffer + Index, Length - Index));
		
		Index += ReadGUID.first;
		std::cout << "\t\t\t\tCollection Group ID: ";
		
		std::map< std::string, std::string >::iterator GUIDDescriptionIterator(g_GUIDDescriptions.find(ReadGUID.second));
		
		if(GUIDDescriptionIterator != g_GUIDDescriptions.end())
		{
			std::cout << GUIDDescriptionIterator->second << " (" << ReadGUID.second << ")" << std::endl;
		}
		else
		{
			std::cout << ReadGUID.second << " (unknown value)" << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "WM/Provider")
	{
		std::pair< int, std::string > ReadString(GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadString.first;
		std::cout << "\t\t\t\tContent Provider: \"" << ReadString.second << '"' << std::endl;
	}
	else if(ReadOwnerIdentifier.second == "WM/UniqueFileIdentifier")
	{
		std::pair< int, std::string > ReadString(GetUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index));
		
		Index += ReadString.first;
		std::cout << "\t\t\t\tUnique File Identifier: \"" << ReadString.second << '"' << std::endl;
		
		std::pair< bool, std::string > Interpretation(GetWMUniqueFileIdentifierInterpretation(ReadString.second));
		
		if(Interpretation.first == true)
		{
			std::cout << "\t\t\t\tInterpretation as AllMusicGuide fields (http://www.allmusic.com/):" << std::endl;
			std::cout << Interpretation.second << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "CompID")
	{
		std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
		
		std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
		if(IsUCS_2_LEStringWithoutByteOrderMarkWithTermination(Buffer + Index, Length - Index) == true)
		{
			std::pair< int, std::string > ReadString(GetUCS_2_LEStringWithoutByteOrderMarkWithTerminationAtStart(Buffer + Index, Length - Index));
			
			std::cout << "\t\t\t\tString: \"" << ReadString.second << "\" (interpreted as UCS-2LE without BOM with termination)" << std::endl;
		}
		Index += ReadHexadecimal.first;
	}
	else if(ReadOwnerIdentifier.second == "PeakValue")
	{
		if(Length - Index == 4)
		{
			std::pair< bool, uint16_t > ReadUInt16(GetUInt16LE(Buffer + Index, Length - Index));
			
			assert(ReadUInt16.first == true);
			std::cout << "\t\t\t\tPeak Value: " << ReadUInt16.second << std::endl;
			Index += 2;
			if((Buffer[Index] != 0) || (Buffer[Index + 1] != 0))
			{
				std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
				
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
			std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
			
			Index += ReadHexadecimal.first;
			std::cout << "\t\t\t\tThis value is defined by Microsoft to be of type DWORD which requires 4 byte." << std::endl;
			std::cout << "\t\t\t\tInstead " << (Length - Index) << " bytes are available. Skipped reading." << std::endl;
			std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
		}
	}
	else if(ReadOwnerIdentifier.second == "AverageLevel")
	{
		if(Length - Index == 4)
		{
			std::pair< bool, uint16_t > ReadUInt16(GetUInt16LE(Buffer + Index, Length - Index));
			
			assert(ReadUInt16.first == true);
			std::cout << "\t\t\t\tAverage Level: " << ReadUInt16.second << std::endl;
			Index += 2;
			if((Buffer[Index] != 0) || (Buffer[Index + 1] != 0))
			{
				std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
				
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
			std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
			
			Index += ReadHexadecimal.first;
			std::cout << "\t\t\t\tThis value is defined by Microsoft to be of type DWORD which requires 4 byte." << std::endl;
			std::cout << "\t\t\t\tInstead " << (Length - Index) << " bytes are available. Skipped reading." << std::endl;
			std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
		}
	}
	else
	{
		std::pair< int, std::string > ReadHexadecimal(GetHexadecimalStringTerminatedByLength(Buffer + Index, Length - Index));
		
		Index += ReadHexadecimal.first;
		std::cout << "\t\t\t\tBinary Content: " << ReadHexadecimal.second << std::endl;
	}
	
	return Index;
}

void vReadFile(const std::string & Path);
void vReadDirectory(const std::string & Path);
void vReadItem(const std::string & Path);

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
	int BufferLength(1000);
	char * Buffer(new char[BufferLength]);
	
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
					std::cout << "\t\t\t*** ERROR *** This frame is forbidden! " << NewFrameHeader->GetForbiddenReason() << std::endl;
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
					Buffer = new char[BufferLength];
				}
				Stream.read(Buffer, NewFrameHeader->GetSize());
				if(g_PrintBytes == true)
				{
					std::cout << "\t\t\tBytes: ";
					for(unsigned long int Index = 0; Index < NewFrameHeader->GetSize(); ++Index)
					{
						std::cout << GetHexadecimalStringFromUInt8(Buffer[Index]) << ' ';
						
					}
					std::cout << std::endl;
				}
				std::cout << "\t\t\tContent:" << std::endl;
				
				int HandledFrameSize(0);
				
				HandledFrameSize = NewFrameHeader->HandleData(Buffer, NewFrameHeader->GetSize());
				if(HandledFrameSize != NewFrameHeader->GetSize())
				{
					std::cout << "*** WARNING *** Frame size exceeds frame data." << std::endl;
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

void vReadFile(const std::string & Path)
{
	char * Buffer(new char[1000]);
	int u4Track = 0;
	bool bID3v11 = false;

	std::ifstream ReadFile;

	ReadFile.open(Path.c_str(), std::ios::in | std::ios::binary);
	if(ReadFile == false)
	{
		std::cerr << Path << ": Can not be opened." << std::endl;
	}
	ReadFile.seekg(-128, std::ios::end);
	ReadFile.read(Buffer, 3);
	Buffer[3] = '\0';
	if(strcmp(Buffer, "TAG") == 0)
	{
		std::cout << "ID3v1 TAG:" << std::endl;
		
		std::pair< int, std::string > Read;
		
		ReadFile.read(Buffer, 30);
		Buffer[30] = '\0';
		Read = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\tTitle:\t \"" << Read.second << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		Buffer[30] = '\0';
		Read = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\tArtist:\t \"" << Read.second << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		Buffer[30] = '\0';
		Read = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\tAlbum:\t \"" << Read.second << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 4);
		Buffer[4] = '\0';
		Read = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer, 4);
		std::cout << "\tYear:\t \"" << Read.second << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		Buffer[30] = '\0';
		Read = GetISO_IEC_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\tComment: \"" << Read.second << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		bID3v11 = false;
		if(Buffer[28] == '\0')
		{
			bID3v11 = true;
			u4Track = static_cast< int >(Buffer[29]);
		}
		ReadFile.read(Buffer, 1);
		
		std::map< unsigned int, std::string >::iterator Genre1_0Iterator(g_Genres1_0.find(Buffer[0]));
		
		if(Genre1_0Iterator != g_Genres1_0.end())
		{
			std::cout << "\tGenre:\t " << Genre1_0Iterator->second << "  [number: " << static_cast< unsigned int >(static_cast< unsigned char >(*Buffer)) << "]" << std::endl;
		}
		else
		{
			std::cout << "\tGenre:\t non-ID3v1 standard genre  [number: " << static_cast< unsigned int >(static_cast< unsigned char >(*Buffer)) << "]" << std::endl;
		}
		if(bID3v11 == true)
		{
			std::cout << "ID3v1.1 TAG:" << std::endl;
			std::cout << "\tTrack:\t \"" << u4Track << "\"" << std::endl;
		}
	}
	ReadID3v2Tag(ReadFile);
}

void vReadDirectory(const std::string & Path)
{
	DIR * Directory(opendir(Path.c_str()));
	struct dirent * DirectoryEntry(0);
	
	while((DirectoryEntry = readdir(Directory)) != 0)
	{
		if((std::string(DirectoryEntry->d_name) != ".") && (std::string(DirectoryEntry->d_name) != ".."))
		{
			vReadItem(Path + '/' + DirectoryEntry->d_name);
		}
	}
}

void vReadItem(const std::string & Path)
{
	if(FileExists(Path) == true)
	{
		if(IsDirectory(Path) == true)
		{
			vReadDirectory(Path);
		}
		else if(IsRegularFile(Path) == true)
		{
			vReadFile(Path);
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
	
	// genres for version 1.0
	g_Genres1_0.insert(std::make_pair(0, "Blues"));
	g_Genres1_0.insert(std::make_pair(1, "Classic Rock"));
	g_Genres1_0.insert(std::make_pair(2, "Country"));
	g_Genres1_0.insert(std::make_pair(3, "Dance"));
	g_Genres1_0.insert(std::make_pair(4, "Disco"));
	g_Genres1_0.insert(std::make_pair(5, "Funk"));
	g_Genres1_0.insert(std::make_pair(6, "Grunge"));
	g_Genres1_0.insert(std::make_pair(7, "Hip-Hop"));
	g_Genres1_0.insert(std::make_pair(8, "Jazz"));
	g_Genres1_0.insert(std::make_pair(9, "Metal"));
	g_Genres1_0.insert(std::make_pair(10, "New Age"));
	g_Genres1_0.insert(std::make_pair(11, "Oldies"));
	g_Genres1_0.insert(std::make_pair(12, "Other"));
	g_Genres1_0.insert(std::make_pair(13, "Pop"));
	g_Genres1_0.insert(std::make_pair(14, "R&B"));
	g_Genres1_0.insert(std::make_pair(15, "Rap"));
	g_Genres1_0.insert(std::make_pair(16, "Reggae"));
	g_Genres1_0.insert(std::make_pair(17, "Rock"));
	g_Genres1_0.insert(std::make_pair(18, "Techno"));
	g_Genres1_0.insert(std::make_pair(19, "Industrial"));
	g_Genres1_0.insert(std::make_pair(20, "Alternative"));
	g_Genres1_0.insert(std::make_pair(21, "Ska"));
	g_Genres1_0.insert(std::make_pair(22, "Death Metal"));
	g_Genres1_0.insert(std::make_pair(23, "Pranks"));
	g_Genres1_0.insert(std::make_pair(24, "Soundtrack"));
	g_Genres1_0.insert(std::make_pair(25, "Euro-Techno"));
	g_Genres1_0.insert(std::make_pair(26, "Ambient"));
	g_Genres1_0.insert(std::make_pair(27, "Trip-Hop"));
	g_Genres1_0.insert(std::make_pair(28, "Vocal"));
	g_Genres1_0.insert(std::make_pair(29, "Jazz+Funk"));
	g_Genres1_0.insert(std::make_pair(30, "Fusion"));
	g_Genres1_0.insert(std::make_pair(31, "Trance"));
	g_Genres1_0.insert(std::make_pair(32, "Classical"));
	g_Genres1_0.insert(std::make_pair(33, "Instrumental"));
	g_Genres1_0.insert(std::make_pair(34, "Acid"));
	g_Genres1_0.insert(std::make_pair(35, "House"));
	g_Genres1_0.insert(std::make_pair(36, "Game"));
	g_Genres1_0.insert(std::make_pair(37, "Sound Clip"));
	g_Genres1_0.insert(std::make_pair(38, "Gospel"));
	g_Genres1_0.insert(std::make_pair(39, "Noise"));
	g_Genres1_0.insert(std::make_pair(40, "AlternRock"));
	g_Genres1_0.insert(std::make_pair(41, "Bass"));
	g_Genres1_0.insert(std::make_pair(42, "Soul"));
	g_Genres1_0.insert(std::make_pair(43, "Punk"));
	g_Genres1_0.insert(std::make_pair(44, "Space"));
	g_Genres1_0.insert(std::make_pair(45, "Meditative"));
	g_Genres1_0.insert(std::make_pair(46, "Instrumental Pop"));
	g_Genres1_0.insert(std::make_pair(47, "Instrumental Rock"));
	g_Genres1_0.insert(std::make_pair(48, "Ethnic"));
	g_Genres1_0.insert(std::make_pair(49, "Gothic"));
	g_Genres1_0.insert(std::make_pair(50, "Darkwave"));
	g_Genres1_0.insert(std::make_pair(51, "Techno-Industrial"));
	g_Genres1_0.insert(std::make_pair(52, "Electronic"));
	g_Genres1_0.insert(std::make_pair(53, "Pop-Folk"));
	g_Genres1_0.insert(std::make_pair(54, "Eurodance"));
	g_Genres1_0.insert(std::make_pair(55, "Dream"));
	g_Genres1_0.insert(std::make_pair(56, "Southern Rock"));
	g_Genres1_0.insert(std::make_pair(57, "Comedy"));
	g_Genres1_0.insert(std::make_pair(58, "Cult"));
	g_Genres1_0.insert(std::make_pair(59, "Gangsta"));
	g_Genres1_0.insert(std::make_pair(60, "Top 40"));
	g_Genres1_0.insert(std::make_pair(61, "Christian Rap"));
	g_Genres1_0.insert(std::make_pair(62, "Pop/Funk"));
	g_Genres1_0.insert(std::make_pair(63, "Jungle"));
	g_Genres1_0.insert(std::make_pair(64, "Native American"));
	g_Genres1_0.insert(std::make_pair(65, "Cabaret"));
	g_Genres1_0.insert(std::make_pair(66, "New Wave"));
	g_Genres1_0.insert(std::make_pair(67, "Psychadelic"));
	g_Genres1_0.insert(std::make_pair(68, "Rave"));
	g_Genres1_0.insert(std::make_pair(69, "Showtunes"));
	g_Genres1_0.insert(std::make_pair(70, "Trailer"));
	g_Genres1_0.insert(std::make_pair(71, "Lo-Fi"));
	g_Genres1_0.insert(std::make_pair(72, "Tribal"));
	g_Genres1_0.insert(std::make_pair(73, "Acid Punk"));
	g_Genres1_0.insert(std::make_pair(74, "Acid Jazz"));
	g_Genres1_0.insert(std::make_pair(75, "Polka"));
	g_Genres1_0.insert(std::make_pair(76, "Retro"));
	g_Genres1_0.insert(std::make_pair(77, "Musical"));
	g_Genres1_0.insert(std::make_pair(78, "Rock & Roll"));
	g_Genres1_0.insert(std::make_pair(79, "Hard Rock"));
	
	// encodings for version 2.2
	g_Encodings2_2.insert(std::make_pair(0x00, "ISO/IEC 8859-1"));
	g_Encodings2_2.insert(std::make_pair(0x01, "UCS-2 encoded Unicode"));
	
	// encodings for version 2.3
	g_Encodings2_3.insert(std::make_pair(0x00, "ISO/IEC 8859-1"));
	g_Encodings2_3.insert(std::make_pair(0x01, "UCS-2 encoded Unicode"));
	
	// encodings for version 2.4
	g_Encodings2_4.insert(std::make_pair(0x00, "ISO/IEC 8859-1"));
	g_Encodings2_4.insert(std::make_pair(0x01, "UTF-16 encoded Unicode with Byte Order Mark"));
	g_Encodings2_4.insert(std::make_pair(0x02, "UTF-16BE encoded Unicode in Big Endian"));
	g_Encodings2_4.insert(std::make_pair(0x03, "UTF-8 encoded Unicode"));
	
	// language codes according to ISO 639-2
	g_ISO_639_2_Codes.insert(std::make_pair("dut", "Dutch; Flemish"));
	g_ISO_639_2_Codes.insert(std::make_pair("eng", "English"));
	
	// country codes according to ISO 3166-1 alpha-2
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
	FrameHeader::Handle22("TAL", "Album/Movie/Show title", Handle22And23TextFrame);
	FrameHeader::Handle22("TCM", "Composer", Handle22And23TextFrame);
	FrameHeader::Handle22("TCO", "Content type", Handle22And23TextFrame);
	FrameHeader::Handle22("TEN", "Encoded by", Handle22And23TextFrame);
	FrameHeader::Handle22("TP1", "Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group", Handle22And23TextFrame);
	FrameHeader::Handle22("TPA", "Part of a set", Handle22And23TextFrame);
	FrameHeader::Handle22("TRK", "Track number/Position in set", Handle22And23TextFrame);
	FrameHeader::Handle22("TT1", "Content group description", Handle22And23TextFrame);
	FrameHeader::Handle22("TT2", "Title/Songname/Content description", Handle22And23TextFrame);
	FrameHeader::Handle22("TYE", "Year", Handle22And23TextFrame);
	// forbidden tags
	FrameHeader::Forbid22("TCP", "This frame is not officially defined for tag version 2.2 but has been seen used nonetheless.");
	FrameHeader::Handle22("TCP", "Compilation (from the internet)", Handle22And23TextFrame);
	
	// ID3v2.3.0
	FrameHeader::Handle23("APIC", "Attached picture", Handle23AttachedPicture);
	FrameHeader::Handle23("COMM", "Comments", Handle23CommentFrame);
	FrameHeader::Handle23("MCDI", "Music CD identifier", Handle23MCDIFrame);
	FrameHeader::Handle23("PRIV", "Private frame", HandlePRIVFrame);
	FrameHeader::Handle23("TALB", "Album/Movie/Show title", Handle22And23TextFrame);
	FrameHeader::Handle23("TBPM", "BPM (beats per minute)", Handle22And23TextFrame);
	FrameHeader::Handle23("TCOM", "Composer", Handle22And23TextFrame);
	FrameHeader::Handle23("TCON", "Content type", Handle23TCONFrame);
	FrameHeader::Handle23("TCOP", "Copyright message", Handle22And23TextFrame);
	FrameHeader::Handle23("TDAT", "Date", Handle22And23TextFrame);
	FrameHeader::Handle23("TENC", "Encoded by", Handle22And23TextFrame);
	FrameHeader::Handle23("TIT1", "Content group description", Handle22And23TextFrame);
	FrameHeader::Handle23("TIT2", "Title/songname/content description", Handle22And23TextFrame);
	FrameHeader::Handle23("TIT3", "Subtitle/Description refinement", Handle22And23TextFrame);
	FrameHeader::Handle23("TLAN", "Language(s)", Handle22And23TextFrame);
	FrameHeader::Handle23("TLEN", "Length", Handle22And23TextFrame);
	FrameHeader::Handle23("TMED", "Media type", Handle22And23TextFrame);
	FrameHeader::Handle23("TOPE", "Original artist(s)/performer(s)", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE1", "Lead Performer(s) / Solo Artist(s)", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE2", "Band / Orchestra / Accompaniment", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE3", "Conductor / Performer Refinement", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE4", "Interpreted, Remixed, or otherwise modified by", Handle22And23TextFrame);
	FrameHeader::Handle23("TPOS", "Part of a set", Handle22And23TextFrame);
	FrameHeader::Handle23("TPUB", "Publisher", Handle22And23TextFrame);
	FrameHeader::Handle23("TRCK", "Track number/Position in set", Handle22And23TextFrame);
	FrameHeader::Handle23("TSRC", "ISRC (international standard recording code)", Handle23TSRCFrame);
	FrameHeader::Handle23("TSSE", "Software/Hardware and settings used for encoding", Handle22And23TextFrame);
	FrameHeader::Handle23("TXXX", "User defined text information frame", Handle23UserTextFrame);
	FrameHeader::Handle23("TYER", "Year", Handle22And23TextFrame);
	FrameHeader::Handle23("UFID", "Unique file identifier", Handle23UFIDFrame);
	FrameHeader::Handle23("WCOM", "Commercial information", 0);
	FrameHeader::Handle23("WOAR", "Official artist/performer webpage", Handle23URLFrame);
	FrameHeader::Handle23("WXXX", "User defined URL link frame", Handle23UserURLFrame);
	// forbidden tags
	FrameHeader::Forbid23("TDRC", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDRC", "Recording time (from tag version 2.4)", Handle24TextFrame);
	FrameHeader::Forbid23("TDTG", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDTG", "Tagging time (from tag version 2.4)", Handle24TextFrame);
	FrameHeader::Forbid23("TCMP", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate whether a title is a part of a compilation.");
	FrameHeader::Handle23("TCMP", "Part of a compilation (by iTunes)", Handle23TCMPFrame);
	
	// ID3v2.4.0
	FrameHeader::Handle24("APIC", "Attached picture", Handle24APICFrame);
	FrameHeader::Handle24("COMM", "Comments", Handle24COMMFrame);
	FrameHeader::Handle24("PRIV", "Private frame", HandlePRIVFrame);
	FrameHeader::Handle24("TALB", "Album/Movie/Show title", Handle24TextFrame);
	FrameHeader::Handle24("TCOM", "Composer", Handle24TextFrame);
	FrameHeader::Handle24("TCON", "Content type", Handle24TextFrame);
	FrameHeader::Handle24("TCOP", "Copyright message", Handle24TextFrame);
	FrameHeader::Handle24("TDRC", "Recording time", Handle24TextFrame);
	FrameHeader::Handle24("TDTG", "Tagging time", Handle24TextFrame);
	FrameHeader::Handle24("TENC", "Encoded by", Handle24TextFrame);
	FrameHeader::Handle24("TIT2", "Title/songname/content description", Handle24TextFrame);
	FrameHeader::Handle24("TLAN", "Language(s)", Handle24TextFrame);
	FrameHeader::Handle24("TPE1", "Lead performer(s)/Soloist(s)", Handle24TextFrame);
	FrameHeader::Handle24("TPE2", "Band/orchestra/accompaniment", Handle24TextFrame);
	FrameHeader::Handle24("TPOS", "Part of a set", Handle24TextFrame);
	FrameHeader::Handle24("TRCK", "Track number/Position in set", Handle24TextFrame);
	FrameHeader::Handle24("TSSE", "Software/Hardware and settings used for encoding", Handle24TextFrame);
	FrameHeader::Handle24("TXXX", "User defined text information frame", Handle24UserTextFrame);
	FrameHeader::Handle24("WXXX", "User defined URL link frame", Handle24WXXXFrame);
	// forbidden tags
	FrameHeader::Forbid24("TYER", "This frame is not defined in tag version 2.4. It has only been valid until tag version 2.3.");
	FrameHeader::Handle24("TYER", "Year (from tag version 2.3)", Handle22And23TextFrame);
	
	// processing
	while(Paths.begin() != Paths.end())
	{
		vReadItem(Paths.front());
		Paths.pop_front();
	}

	return 0;
}
