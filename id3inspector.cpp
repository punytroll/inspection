#include <dirent.h>
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

std::string GetHexadecimalStringFromCharacter(char Character)
{
	std::stringstream Stream;
	
	Stream << std::hex << std::setfill('0') << std::setw(2) << std::right << static_cast< unsigned int >(static_cast< unsigned char >(Character));
	
	return Stream.str();
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

std::string GetUTF_8CharFromISO_8859_1Char(const char Buffer[1])
{
	return GetUTF_8CharFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
}

std::string GetUTF_8CharFromUCS_2_BEChar(const char Buffer[2])
{
	return GetUTF_8CharFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])));
}

std::string GetUTF_8CharFromUCS_2_LEChar(const char Buffer[2])
{
	return GetUTF_8CharFromUnicodeCodepoint(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) * 256 + static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
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

std::string g_sGenres[] = { "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack", "Eurotechno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise", "Alternative Rock", "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Jungle", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Show Tunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",  "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing", "Fast-Fusion", "Bebop", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba", "Folklore", "Ballad", "Power Ballad", "Rhytmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum Solo", "Acapella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore", "Terror", "Indie", "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal", "Crossover", "Contemporary Christian", "Christian Rock", "Unknown","Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown",  "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown","Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown" };
std::map< std::string, std::string > g_Languages;
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

std::pair< int, std::string > GetGUIDString(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	if(Length >= 16)
	{
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += '-';
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
		Result.second += GetHexadecimalStringFromCharacter(Buffer[Result.first++]);
	}
	else
	{
			std::cout << "*** ERROR *** A GUID must be 16 bytes long but this one only has space for " << Length << " bytes." << std::endl;
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ISO-8859-1                                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::pair< int, std::string > GetISO_8859_1StringTerminatedByEnd(const char * Buffer, int Length)
{
	std::pair< int, std::string > Result;
	
	while(true)
	{
		if(Result.first < Length)
		{
			const char * Character(Buffer + Result.first++);
			
			if(*Character != '\0')
			{
				Result.second += GetUTF_8CharFromISO_8859_1Char(Character);
			}
			else
			{
				break;
			}
		}
		else
		{
			std::cout << "*** ERROR *** ISO-8859-1 string should be null-terminated but exceeds its possible length." << std::endl;
			
			break;
		}
	}
	
	return Result;
}

int PrintISO_8859_1StringTerminatedByEndOrLength(const char * Buffer, int Length)
{
	int Index(0);
	
	while(true)
	{
		if(Index < Length)
		{
			if(Buffer[Index] != '\0')
			{
				std::cout << GetUTF_8CharFromISO_8859_1Char(Buffer + Index);
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
				std::cout << GetUTF_8CharFromUCS_2_BEChar(Buffer + Index);
				Index += 2;
			}
		}
	}
}

int PrintUCS_2_BEStringTerminatedByEndOrLength(const char * Buffer, int Length)
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
				std::cout << GetUTF_8CharFromUCS_2_BEChar(Buffer + Index);
				Index += 2;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// UCS-2LE                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////
int PrintUCS_2_LEStringTerminatedByEndOrLength(const char * Buffer, int Length)
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
				std::cout << GetUTF_8CharFromUCS_2_LEChar(Buffer + Index);
				Index += 2;
			}
		}
	}
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
				std::cout << GetUTF_8CharFromUCS_2_LEChar(Buffer + Index);
				Index += 2;
			}
		}
	}
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
			Index += PrintUCS_2_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index]) == 0xff)) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1]) == 0xfe)))
		{
			Index += 2;
			// Little Endian by Byte Order Marker
			Index += PrintUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
					std::cout << GetUTF_8CharFromUCS_2_BEChar(Buffer + Index);
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
					std::cout << GetUTF_8CharFromUCS_2_BEChar(Buffer + Index);
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
					std::cout << GetUTF_8CharFromUCS_2_LEChar(Buffer + Index);
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
					std::cout << GetUTF_8CharFromUCS_2_LEChar(Buffer + Index);
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


int PrintHEXStringTerminatedByLength(const char * Buffer, int Length)
{
	int Index(0);
	
	while(Index < Length)
	{
		std::cout << GetHexadecimalStringFromCharacter(Buffer[Index++]) << ' ';
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
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
	std::cout << "\t\t\t\tLanguage: ";
	
	std::string LanguageCode(Buffer + Index, Buffer + Index + 3);
	
	Index += 3;
	
	std::map< std::string, std::string >::iterator LanguageIterator(g_Languages.find(LanguageCode));
	
	if(LanguageIterator != g_Languages.end())
	{
		std::cout << LanguageIterator->second;
	}
	else
	{
		std::cout << "<unknown>";
	}
	std::cout << " (\"" << LanguageCode << "\")" << std::endl;
	std::cout << "\t\t\t\tDescription: ";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
	std::cout << std::endl << "\t\t\t\tComment: ";
	if(Encoding == 0)
	{
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 1)
	{
		Index += PrintUCS_2StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else
	{
		std::cout << "*** ERROR *** Unknown encoding." << std::endl;
	}
	std::cout << std::endl;
	
	return Index;
}

int Handle22COMFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_2(Encoding) << std::endl;
	std::cout << "\t\t\t\tLanguage: ";
	
	std::string LanguageCode(Buffer + Index, Buffer + Index + 3);
	
	Index += 3;
	
	std::map< std::string, std::string >::iterator LanguageIterator(g_Languages.find(LanguageCode));
	
	if(LanguageIterator != g_Languages.end())
	{
		std::cout << LanguageIterator->second;
	}
	else
	{
		std::cout << "<unknown>";
	}
	std::cout << " (\"" << LanguageCode << "\")" << std::endl;
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
	std::cout << "\t\t\t\tLanguage: ";
	
	std::string LanguageCode(Buffer + Index, Buffer + Index + 3);
	
	Index += 3;
	
	std::map< std::string, std::string >::iterator LanguageIterator(g_Languages.find(LanguageCode));
	
	if(LanguageIterator != g_Languages.end())
	{
		std::cout << LanguageIterator->second;
	}
	else
	{
		std::cout << "<unknown>";
	}
	std::cout << " (\"" << LanguageCode << "\")" << std::endl;
	std::cout << "\t\t\t\tDescription: ";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
	std::cout << std::endl << "\t\t\t\tComment: ";
	if(Encoding == 0)
	{
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
	std::cout << std::endl;
	
	return Index;
}

int Handle23AttachedPicture(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: " << GetEncodingString2_3(Encoding) << std::endl;
	
	std::pair< int, std::string > ReadMIMEType(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
	
	Index += ReadMIMEType.first;
	std::cout << "\t\t\t\tMIME type: \"" << ReadMIMEType.second << '"' << std::endl;
	
	unsigned int PictureType(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	std::cout << "\t\t\t\tPicture type: " << GetPictureTypeString(PictureType) << std::endl;
	std::cout << "\t\t\t\tDescription: \"";
	if(Encoding == 0)
	{
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
	
	return Length;
}

int Handle23URLFrame(const char * Buffer, int Length)
{
	int Index(0);
	
	std::cout << "\t\t\t\tURL: \"";
	Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	std::cout << '"' << std::endl;
	
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
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
	std::cout << "\t\t\t\tURL: \"";
	Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	std::cout << '"' << std::endl;
	
	return Index;
}

int Handle22And23TextFrame(const char * Buffer, int Length)
{
	int Index(0);
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])));
	
	Index += 1;
	std::cout << "\t\t\t\tText Encoding: ";
	if(Encoding == 0)
	{
		std::cout << "ISO-8859-1";
	}
	else if(Encoding == 1)
	{
		std::cout << "Unicode UCS-2";
	}
	else
	{
		std::cout << "<invalid encoding>";
	}
	std::cout << " (" << Encoding << ")" << std::endl;
	if(Encoding == 0)
	{
		std::cout << "\t\t\t\tString: \"";
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	}
	else if(Encoding == 1)
	{
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xff))
		{
			Index += 2;
			// Big Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Big Endian" << std::endl;
			std::cout << "\t\t\t\tString: \"";
			Index += PrintUCS_2_BEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[Index + 1])) == 0xfe))
		{
			Index += 2;
			// Little Endian by BOM
			std::cout << "\t\t\t\tByte Order Marker: Little Endian" << std::endl;
			std::cout << "\t\t\t\tString: \"";
			Index += PrintUCS_2_LEStringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
		std::cout << " (" << GetHexadecimalStringFromCharacter(Buffer[Index]) << ' ' << GetHexadecimalStringFromCharacter(Buffer[Index + 1]) + ')' << std::endl;
	}
	std::cout << "\t\t\t\tString: \"";
	if(Encoding == 0)
	{
		Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
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
		std::pair< int, std::string > ReadDescription(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
		
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
	std::cout << "\t\t\t\tURL: \"";
	Index += PrintISO_8859_1StringTerminatedByEndOrLength(Buffer + Index, Length - Index);
	std::cout << '"' << std::endl;
	
	return Index;
}

int HandlePRIVFrame(const char * Buffer, int Length)
{
	int Index(0);
	std::pair< int, std::string > ReadOwnerIdentifier(GetISO_8859_1StringTerminatedByEnd(Buffer + Index, Length - Index));
	
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
	else
	{
		std::cout << "\t\t\t\tBinary Content: ";
		Index += PrintHEXStringTerminatedByLength(Buffer + Index, Length - Index);
	}
	std::cout << std::endl;
	
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
						std::cout << GetHexadecimalStringFromCharacter(Buffer[Index]) << ' ';
						
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
		Buffer[30] = '\0';
		ReadFile.read(Buffer, 30);
		std::cout << "\tTitle:\t \"";
		PrintISO_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		std::cout << "\tArtist:\t \"";
		PrintISO_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		std::cout << "\tAlbum:\t \"";
		PrintISO_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 4);
		Buffer[4] = '\0';
		std::cout << "\tYear:\t \"";
		PrintISO_8859_1StringTerminatedByEndOrLength(Buffer, 4);
		std::cout << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		std::cout << "\tComment: \"";
		PrintISO_8859_1StringTerminatedByEndOrLength(Buffer, 30);
		std::cout << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		bID3v11 = false;
		if(Buffer[28] == '\0')
		{
			bID3v11 = true;
			u4Track = static_cast< int >(Buffer[29]);
		}
		ReadFile.read(Buffer, 1);
		std::cout << "\tGenre:\t \"" << g_sGenres[static_cast< unsigned char >(*Buffer)] << "\"  [number: " << static_cast< unsigned int >(static_cast< unsigned char >(*Buffer)) << "]" << std::endl;
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
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

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
	
	// encodings for version 2.2
	g_Encodings2_2.insert(std::make_pair(0x00, "ISO-8859-1"));
	g_Encodings2_2.insert(std::make_pair(0x01, "UCS-2 encoded Unicode"));
	
	// encodings for version 2.3
	g_Encodings2_3.insert(std::make_pair(0x00, "ISO-8859-1"));
	g_Encodings2_3.insert(std::make_pair(0x01, "UCS-2 encoded Unicode"));
	
	// encodings for version 2.4
	g_Encodings2_4.insert(std::make_pair(0x00, "ISO-8859-1"));
	g_Encodings2_4.insert(std::make_pair(0x01, "UTF-16 encoded Unicode with Byte Order Mark"));
	g_Encodings2_4.insert(std::make_pair(0x02, "UTF-16BE encoded Unicode in Big Endian"));
	g_Encodings2_4.insert(std::make_pair(0x03, "UTF-8 encoded Unicode"));
	
	// languages according to ISO-639-2
	g_Languages.insert(std::make_pair("eng", "English"));
	
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
	FrameHeader::Handle23("MCDI", "Music CD identifier", 0);
	FrameHeader::Handle23("PRIV", "Private frame", HandlePRIVFrame);
	FrameHeader::Handle23("TALB", "Album/Movie/Show title", Handle22And23TextFrame);
	FrameHeader::Handle23("TBPM", "BPM (beats per minute)", Handle22And23TextFrame);
	FrameHeader::Handle23("TCOM", "Composer", Handle22And23TextFrame);
	FrameHeader::Handle23("TCON", "Content type", Handle22And23TextFrame);
	FrameHeader::Handle23("TCOP", "Copyright message", Handle22And23TextFrame);
	FrameHeader::Handle23("TENC", "Encoded by", Handle22And23TextFrame);
	FrameHeader::Handle23("TIT1", "Content group description", Handle22And23TextFrame);
	FrameHeader::Handle23("TIT2", "Title/songname/content description", Handle22And23TextFrame);
	FrameHeader::Handle23("TIT3", "Subtitle/Description refinement", Handle22And23TextFrame);
	FrameHeader::Handle23("TLAN", "Language(s)", Handle22And23TextFrame);
	FrameHeader::Handle23("TLEN", "Length", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE1", "Lead Performer(s) / Solo Artist(s)", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE2", "Band / Orchestra / Accompaniment", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE3", "Conductor / Performer Refinement", Handle22And23TextFrame);
	FrameHeader::Handle23("TPE4", "Interpreted, Remixed, or otherwise modified by", Handle22And23TextFrame);
	FrameHeader::Handle23("TPOS", "Part of a set", Handle22And23TextFrame);
	FrameHeader::Handle23("TPUB", "Publisher", Handle22And23TextFrame);
	FrameHeader::Handle23("TRCK", "Track number/Position in set", Handle22And23TextFrame);
	FrameHeader::Handle23("TSSE", "Software/Hardware and settings used for encoding", Handle22And23TextFrame);
	FrameHeader::Handle23("TXXX", "User defined text information frame", Handle23UserTextFrame);
	FrameHeader::Handle23("TYER", "Year", Handle22And23TextFrame);
	FrameHeader::Handle23("WCOM", "Commercial information", 0);
	FrameHeader::Handle23("WOAR", "Official artist/performer webpage", Handle23URLFrame);
	FrameHeader::Handle23("WXXX", "User defined URL link frame", Handle23UserURLFrame);
	// forbidden tags
	FrameHeader::Forbid23("TDRC", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDRC", "Recording time (from tag version 2.4)", Handle24TextFrame);
	FrameHeader::Forbid23("TDTG", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4.");
	FrameHeader::Handle23("TDTG", "Tagging time (from tag version 2.4)", Handle24TextFrame);
	
	// ID3v2.4.0
	FrameHeader::Handle24("APIC", "Attached picture", 0);
	FrameHeader::Handle24("COMM", "Comments", Handle24COMMFrame);
	FrameHeader::Handle24("TALB", "Album/Movie/Show title", Handle24TextFrame);
	FrameHeader::Handle24("TCON", "Content type", Handle24TextFrame);
	FrameHeader::Handle24("TCOP", "Copyright message", Handle24TextFrame);
	FrameHeader::Handle24("TDRC", "Recording time", Handle24TextFrame);
	FrameHeader::Handle24("TDTG", "Tagging time", Handle24TextFrame);
	FrameHeader::Handle24("TENC", "Encoded by", Handle24TextFrame);
	FrameHeader::Handle24("TIT2", "Title/songname/content description", Handle24TextFrame);
	FrameHeader::Handle24("TPE1", "Lead performer(s)/Soloist(s)", Handle24TextFrame);
	FrameHeader::Handle24("TPE2", "Band/orchestra/accompaniment", Handle24TextFrame);
	FrameHeader::Handle24("TPOS", "Part of a set", Handle24TextFrame);
	FrameHeader::Handle24("TRCK", "Track number/Position in set", Handle24TextFrame);
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
