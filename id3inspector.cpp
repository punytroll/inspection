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
			
			std::map< std::string, void (*)(const char * const, unsigned int) >::iterator HanderIterator(_Handlers22.find(_Identifier));
			
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
			
			std::map< std::string, void (*)(const char * const, unsigned int) >::iterator HanderIterator(_Handlers23.find(_Identifier));
			
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
			
			std::map< std::string, void (*)(const char * const, unsigned int) >::iterator HanderIterator(_Handlers24.find(_Identifier));
			
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
	
	void HandleData(const char * const Buffer, unsigned int Length)
	{
		if(_Handler != 0)
		{
			_Handler(Buffer, Length);
		}
		else
		{
			std::cout << "*** WARNING ***  No handler defined for the frame type \"" << _Identifier << "\" in this tag version." << std::endl;
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
	static void Add22(const std::string & Identifier, const std::string & Name, void (* Handler) (const char *, unsigned int))
	{
		_Handlers22.insert(std::make_pair(Identifier, Handler));
		_Names22.insert(std::make_pair(Identifier, Name));
	}
	
	static void AddName23(const std::string & Identifier, const std::string & Name, void (* Handler) (const char *, unsigned int))
	{
		_Handlers23.insert(std::make_pair(Identifier, Handler));
		_Names23.insert(std::make_pair(Identifier, Name));
	}
	
	static void AddName24(const std::string & Identifier, const std::string & Name, void (* Handler) (const char *, unsigned int))
	{
		_Handlers24.insert(std::make_pair(Identifier, Handler));
		_Names24.insert(std::make_pair(Identifier, Name));
	}
private:
	// static setup
	static std::map< std::string, void (*) (const char *, unsigned int) > _Handlers22;
	static std::map< std::string, void (*) (const char *, unsigned int) > _Handlers23;
	static std::map< std::string, void (*) (const char *, unsigned int) > _Handlers24;
	static std::map< std::string, std::string > _Names22;
	static std::map< std::string, std::string > _Names23;
	static std::map< std::string, std::string > _Names24;
	// member variables
	bool _Compression;
	bool _DataLengthIndicator;
	bool _Encryption;
	bool _FileAlterPreservation;
	bool _GroupingIdentity;
	void (* _Handler)(const char * const Buffer, unsigned int Length);
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
std::map< std::string, std::string > FrameHeader::_Names22;
std::map< std::string, std::string > FrameHeader::_Names23;
std::map< std::string, std::string > FrameHeader::_Names24;
std::map< std::string, void (*) (const char *, unsigned int) > FrameHeader::_Handlers22;
std::map< std::string, void (*) (const char *, unsigned int) > FrameHeader::_Handlers23;
std::map< std::string, void (*) (const char *, unsigned int) > FrameHeader::_Handlers24;

void Handle22And23TextFrameWithoutNewlines(const char * Buffer, unsigned int Length)
{
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
	
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
	std::cout << "\t\t\t\tString: \"";
	if(Encoding == 0)
	{
		std::cout.write(Buffer + 1, Length - 1);
	}
	else if((Encoding == 1) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[2])) == 0xff))
	{
		// Big Endian by BOM
		for(int Index = 4; Index < Length; Index += 2)
		{
			std::cout << Buffer[Index];
		}
	}
	else if((Encoding == 1) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[2])) == 0xfe))
	{
		// Little Endian by BOM
		for(int Index = 3; Index < Length; Index += 2)
		{
			std::cout << Buffer[Index];
		}
	}
	std::cout << '"' << std::endl;
	std::cout << "\t\t\t\tBytes: ";
	for(unsigned long int Index = 1; Index < Length; ++Index)
	{
		std::cout << GetHexadecimalStringFromCharacter(Buffer[Index]) << ' ';
		
	}
	std::cout << std::endl;
}

void Handle24TextFrameWithoutNewlines(const char * Buffer, unsigned int Length)
{
	unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
	
	std::cout << "\t\t\t\tText Encoding: ";
	if(Encoding == 0)
	{
		std::cout << "ISO-8859-1";
	}
	else if(Encoding == 1)
	{
		std::cout << "UTF-16 encoded Unicode with Byte Order Mark";
	}
	else if(Encoding == 2)
	{
		std::cout << "UTF-16BE encoded Unicode in Big Endian";
	}
	else if(Encoding == 3)
	{
		std::cout << "UTF-8 encoded Unicode";
	}
	else
	{
		std::cout << "<invalid encoding>";
	}
	std::cout << " (" << Encoding << ")" << std::endl;
	if(Encoding == 1)
	{
		std::cout << "\t\t\t\tByte Order Mark: ";
		if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[2])) == 0xff))
		{
			std::cout << "Big Endian";
		}
		else if((static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[2])) == 0xfe))
		{
			std::cout << "Little Endian";
		}
		else
		{
			std::cout << "Bogus Byte Order Mark: " << GetHexadecimalStringFromCharacter(Buffer[1]) << ' ' << GetHexadecimalStringFromCharacter(Buffer[2]);
		}
		std::cout << std::endl;
	}
	std::cout << "\t\t\t\tString: \"";
	if((Encoding == 0) || (Encoding == 3))
	{
		std::cout.write(Buffer + 1, Length - 1);
	}
	else if(((Encoding == 1) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) == 0xfe) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[2])) == 0xff)) || (Encoding == 2))
	{
		// Big Endian either by BOM or by definition
		for(int Index = 4; Index < Length; Index += 2)
		{
			std::cout << Buffer[Index];
		}
	}
	else if((Encoding == 1) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[1])) == 0xff) && (static_cast< unsigned int >(static_cast< unsigned char >(Buffer[2])) == 0xfe))
	{
		// Little Endian by BOM
		for(int Index = 3; Index < Length; Index += 2)
		{
			std::cout << Buffer[Index];
		}
	}
	std::cout << '"' << std::endl;
	std::cout << "\t\t\t\tBytes: ";
	for(unsigned long int Index = 1; Index < Length; ++Index)
	{
		std::cout << GetHexadecimalStringFromCharacter(Buffer[Index]) << ' ';
		
	}
	std::cout << std::endl;
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
				std::cout << "\t\t\tName: " << NewFrameHeader->GetName() << std::endl;
				std::cout << "\t\t\tSize: " << NewFrameHeader->GetSize() << std::endl;
				if(NewFrameHeader->SupportsFlags() == true)
				{
					std::cout << "\t\t\tFlags: " << NewFrameHeader->GetFlagsAsString() << std::endl;
				}
				std::cout << "\t\t\tContent:" << std::endl;
				while(NewFrameHeader->GetSize() > BufferLength)
				{
					delete[] Buffer;
					BufferLength <<= 1;
					Buffer = new char[BufferLength];
				}
				Stream.read(Buffer, NewFrameHeader->GetSize());
				NewFrameHeader->HandleData(Buffer, NewFrameHeader->GetSize());
				if(NewFrameHeader->GetSize() != 0)
				{
					Size -= NewFrameHeader->GetSize() + 10;
					std::cout<< std::endl;
				}
				else
				{
					Size = 0;
				}
			}
			else
			{
				break;
			}
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
		std::cout << "\tTitle:\t \"" << Buffer << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		std::cout << "\tArtist:\t \"" << Buffer << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		std::cout << "\tAlbum:\t \"" << Buffer << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 4);
		Buffer[4] = '\0';
		std::cout << "\tYear:\t \"" << Buffer << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
		ReadFile.read(Buffer, 30);
		std::cout << "\tComment: \"" << Buffer << "\"  [length: " << strlen(Buffer) << "]" << std::endl;
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
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	
	// ID3v2.2.0
	FrameHeader::Add22("TAL", "Album/Movie/Show title", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::Add22("TEN", "Encoded by", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::Add22("TP1", "Lead artist(s)/Lead performer(s)/Soloist(s)/Performing group", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::Add22("TRK", "Track number/Position in set", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::Add22("TT2", "Title/Songname/Content description", Handle22And23TextFrameWithoutNewlines);
	
	// ID3v2.3.0
	FrameHeader::AddName23("APIC", "Attached picture", 0);
	FrameHeader::AddName23("COMM", "Comments", 0);
	FrameHeader::AddName23("MCDI", "Music CD identifier", 0);
	FrameHeader::AddName23("PRIV", "Private frame", 0);
	FrameHeader::AddName23("TALB", "Album/Movie/Show title", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TBPM", "BPM (beats per minute)", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TCOM", "Composer", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TCON", "Content type", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TCOP", "Copyright message", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TENC", "Encoded by", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TIT1", "Content group description", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TIT2", "Title/songname/content description", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TIT3", "Subtitle/Description refinement", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TLAN", "Language(s)", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TLEN", "Length", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TPE1", "Lead Performer(s) / Solo Artist(s)", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TPE2", "Band / Orchestra / Accompaniment", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TPE3", "Conductor / Performer Refinement", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TPE4", "Interpreted, Remixed, or otherwise modified by", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TPOS", "Part of a set", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TPUB", "Publisher", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TRCK", "Track number/Position in set", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TSSE", "Software/Hardware and settings used for encoding", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TXXX", "User defined text information frame", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("TYER", "Year", Handle22And23TextFrameWithoutNewlines);
	FrameHeader::AddName23("WCOM", "Commercial information", 0);
	FrameHeader::AddName23("WXXX", "User defined URL link frame", 0);
	
	// ID3v2.4.0
	FrameHeader::AddName24("TALB", "Album/Movie/Show title", Handle24TextFrameWithoutNewlines);
	FrameHeader::AddName24("TDRC", "Recording time", Handle24TextFrameWithoutNewlines);
	FrameHeader::AddName24("TIT2", "Title/songname/content description", Handle24TextFrameWithoutNewlines);
	FrameHeader::AddName24("TPE1", "Lead performer(s)/Soloist(s)", Handle24TextFrameWithoutNewlines);
	FrameHeader::AddName24("TPOS", "Part of a set", Handle24TextFrameWithoutNewlines);
	FrameHeader::AddName24("TRCK", "Track number/Position in set", Handle24TextFrameWithoutNewlines);
	
	// processing
	while(Paths.begin() != Paths.end())
	{
		vReadItem(Paths.front());
		Paths.pop_front();
	}

	return 0;
}
