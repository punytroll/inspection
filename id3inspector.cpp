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

typedef unsigned char u1byte;
typedef unsigned short int u2byte;
typedef unsigned long int index_t;
typedef unsigned long int count_t;
typedef unsigned long int flag_t;

class FrameHandler
{
public:
	virtual ~FrameHandler(void)
	{
	}
	
	virtual void Print(const char * Buffer, unsigned long int Lenth) = 0;
};

std::string g_sGenres[] = { "Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack", "Eurotechno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk", "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise", "Alternative Rock", "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Jungle", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap", "Pop/Funk", "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Show Tunes", "Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",  "Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing", "Fast-Fusion", "Bebop", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba", "Folklore", "Ballad", "Power Ballad", "Rhytmic Soul", "Freestyle", "Duet", "Punk Rock", "Drum Solo", "Acapella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore", "Terror", "Indie", "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian Gangsta Rap", "Heavy Metal", "Black Metal", "Crossover", "Contemporary Christian", "Christian Rock", "Unknown","Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown",  "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown","Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown" };
std::string g_sEncodings[] = { "ISO-8859-1", "UTF-16 encoded Unicode with Byte Order Mark", "UTF-16BE encoded Unicode in Big Endian", "UTF-8 encoded Unicode" };
std::map< std::string, std::string > g_FrameNames;
std::map< index_t, std::string > g_ID3v2TagFrameFlags;
std::map< std::string, FrameHandler * > g_FrameHandlers;

class TextFrameHandler : public FrameHandler
{
public:
	virtual ~TextFrameHandler(void)
	{
	}
	
	virtual void Print(const char * Buffer, unsigned long int Length)
	{
		unsigned int Encoding(static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0])));
		
		std::cout << "\t\t\tText Encoding: " << g_sEncodings[Encoding] << std::endl;
		if(Encoding == 1)
		{
			std::cout << "\t\t\tByte Order Mark: ";
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
		std::cout << "\t\t\tString: \"";
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
	}
};

class HexFrameHandler : public FrameHandler
{
public:
	virtual ~HexFrameHandler(void)
	{
	}
	
	virtual void Print(const char * Buffer, unsigned long int Length)
	{
		std::cout << "\t\t\t";
		for(unsigned long int Index = 0; Index < Length; ++Index)
		{
			std::cout << GetHexadecimalStringFromCharacter(Buffer[Index]) << ' ';
			
		}
		std::cout << std::endl;
	}
};

class COMMFrameHandler : public FrameHandler
{
public:
	virtual ~COMMFrameHandler(void)
	{
	}
	
	virtual void Print(const char * Buffer, unsigned long int Length)
	{
		std::cout << "\t\t\tText Encoding: " << g_sEncodings[static_cast< unsigned int >(static_cast< unsigned char >(Buffer[0]))] << std::endl;
		std::cout << "\t\t\tLanguage: \"";
		std::cout.write(Buffer + 1, 3);
		std::cout << '"' << std::endl;
		std::cout << "\t\t\tShort content description: \"";
		std::cout.write(Buffer + 4, strlen(Buffer + 4));
		std::cout << '"' << std::endl;
		std::cout << "\t\t\tString: \"";
		std::cout.write(Buffer + 4 + strlen(Buffer + 4) + 1, Length - 4 - strlen(Buffer + 4) - 1);
		std::cout << '"' << std::endl;
	}
};

class MCDIFrameHandler : public FrameHandler
{
public:
	virtual ~MCDIFrameHandler(void)
	{
	}
	
	virtual void Print(const char * Buffer, unsigned long int Length)
	{
		std::cout << "\t\t\t";
		
		unsigned long int Index(0);
		
		if(Index + 4 <= Length)
		{
			std::cout << GetHexadecimalStringFromCharacter(Buffer[Index + 0]) << ' ' << GetHexadecimalStringFromCharacter(Buffer[Index + 1]) << ' ' << GetHexadecimalStringFromCharacter(Buffer[Index + 2]) << ' '<< GetHexadecimalStringFromCharacter(Buffer[Index + 3]) << std::endl;
		}
		std::cout << "\t\t\t";
		Index += 4;
		while(Index < Length)
		{
			std::cout << GetHexadecimalStringFromCharacter(Buffer[Index]) << ' ';
			++Index;
		}
		std::cout << std::endl;
	}
};

class WXXXFrameHandler : public FrameHandler
{
public:
	virtual ~WXXXFrameHandler(void)
	{
	}
	
	virtual void Print(const char * Buffer, unsigned long int Length)
	{
		std::cout << "\t\t\tText Encoding: " << g_sEncodings[static_cast< unsigned long int >(static_cast< unsigned char >(Buffer[0]))] << std::endl;
		std::cout << "\t\t\tDescription: \"";
		
		int Index = 1;
		
		while((Index < Length) && (Buffer[Index] != '\0'))
		{
			std::cout << Buffer[Index];
			++Index;
		}
		std::cout << '"' << std::endl;
		std::cout << "\t\t\tURL: \"";
		while(Index < Length)
		{
			std::cout << Buffer[Index];
			++Index;
		}
		std::cout << '"' << std::endl;
	}
};

class URLFrameHandler : public FrameHandler
{
public:
	virtual void Print(const char * Buffer, unsigned long int Length)
	{
		std::cout << "\t\t\tURL: \"";
		std::cout.write(Buffer, Length);
		std::cout << '"' << std::endl;
	}
};

void vReadFile(const std::string & Path);
void vReadDirectory(const std::string & Path);
void vReadItem(const std::string & Path);

inline void AppendIfFlagsIsSet(flag_t Flags, flag_t Flag, const std::string &sAppend, std::string &sString)
{
	if((Flags & Flag) == Flag)
	{
		AppendSeparated(sString, sAppend, ", ");
	}
}

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

std::string sGetFlags(flag_t Flags, const std::map< index_t, std::string > &Map)
{
	std::map< index_t, std::string >::const_iterator iIterator = Map.begin();
	std::map< index_t, std::string >::const_iterator iEnd = Map.end();
	std::string sFlags = "";

	while(iIterator != iEnd)
	{
		AppendIfFlagsIsSet(Flags, iIterator->first, iIterator->second, sFlags);
		++iIterator;
	}
	if(sFlags.empty() == true)
	{
		sFlags += "None";
	}

	return sFlags;
}

bool bIsIDCharacter(char cCharacter)
{
	return ((cCharacter >= 'A') && (cCharacter <= 'Z')) || ((cCharacter >= '0') && (cCharacter <= '9'));
}

void ReadID3v2Tag(std::ifstream & Stream)
{
	Stream.seekg(0, std::ios::beg);
	
	TagHeader * NewTagHeader(new TagHeader(Stream));
	
	int BufferLength = 1000;
	char * Buffer = new char[BufferLength];
	
	Stream.seekg(0, std::ios::beg);
	Stream.read(Buffer, 10);
	if((Buffer[0] == 'I') && (Buffer[1] == 'D') && (Buffer[2] == '3'))
	{
		std::cout << "ID3v2 TAG:" << std::endl;
		std::cout << "\tFile Identifier: " << NewTagHeader->GetID3Identifier() << std::endl;
		std::cout << "\tVersion: 2." << NewTagHeader->GetMajorVersion() << "." << NewTagHeader->GetRevisionNumber() << std::endl;
		std::cout << "\tFlags: " << NewTagHeader->GetFlagsAsString() << std::endl;
		std::cout << "\tSize: " << NewTagHeader->GetSize() << std::endl;
		std::cout << "\tFrames:" << std::endl;

		count_t u4FrameSize = 0;
		char pcID[5];
		int Size = NewTagHeader->GetSize();

		pcID[4] = '\0';
		while(Size >= 1)
		{
			Stream.read(pcID, 4);
			if((bIsIDCharacter(pcID[0]) == false) || (bIsIDCharacter(pcID[1]) == false) || (bIsIDCharacter(pcID[2]) == false) || (bIsIDCharacter(pcID[3]) == false))
			{
				// this is an invalid identifier
				if((pcID[0] == '\0') && (pcID[1] == '\0') && (pcID[2] == '\0') && (pcID[3] == '\0'))
				{
					// we reached the padding section
					break;
				}
				else
				{
					// this definitely is invalid
					std::cerr << "Invalid identifier: \"" << pcID[0] << pcID[1] << pcID[2] << pcID[3] << "\"" << std::endl;
					
					break;
				}
			}
			std::cout << "\t";
			std::cout << "\tID:\t ";
			std::cout << "\"" << pcID[0] << pcID[1] << pcID[2] << pcID[3] << "\"";
			std::cout << "\n\t\tName:\t " << g_FrameNames[std::string(pcID, pcID + 4)];
			std::cout << "\n\t\tSize:\t ";
			Stream.read(Buffer, 4);
			if(NewTagHeader->GetMajorVersion() > 3)
			{
				u4FrameSize = ((static_cast< u1byte >(Buffer[0]) << 21) + (static_cast< u1byte >(Buffer[1]) << 14) + (static_cast< u1byte >(Buffer[2]) << 7) + static_cast< u1byte >(Buffer[3]));
			}
			else
			{
				u4FrameSize = ((static_cast< u1byte >(Buffer[0]) << 24) + (static_cast< u1byte >(Buffer[1]) << 16) + (static_cast< u1byte >(Buffer[2]) << 8) + static_cast< u1byte >(Buffer[3]));
			}
			std::cout << u4FrameSize;
			std::cout << "\n\t\tFlags:\t ";
			Stream.read(Buffer, 2);
			std::cout << sGetFlags(*(reinterpret_cast< u2byte * >(Buffer)), g_ID3v2TagFrameFlags);
			std::cout << std::endl << "\t\tContent:" << std::endl;
			if(u4FrameSize <= BufferLength)
			{
				Stream.read(Buffer, u4FrameSize);
				
				std::map< std::string, FrameHandler * >::iterator FrameHandler = g_FrameHandlers.find(pcID);
				
				if(FrameHandler == g_FrameHandlers.end())
				{
					std::cerr << "*** WARNING: No handler defined for frame type \"" << pcID << "\"!" << std::endl;
				}
				else
				{
					FrameHandler->second->Print(Buffer, u4FrameSize);
				}
			}
			else
			{
				std::cout << "\t\t\t --- Sorry, too long! --- \"" << std::endl;
				Stream.seekg(u4FrameSize, std::ios::cur);
			}
			if(u4FrameSize != 0)
			{
				Size -= u4FrameSize + 10;
				std::cout<< std::endl;
			}
			else
			{
				Size = 0;
			}
		}
		std::cout << std::endl;
	}
	delete NewTagHeader;
}

void vReadFile(const std::string & Path)
{
	std::vector< char > Buffer;
	count_t u4BufferLength = 1000;
	char *pcBuffer = new char[u4BufferLength];
	count_t u4Track = 0;
	bool bID3v11 = false;

	std::ifstream ReadFile;

	ReadFile.open(Path.c_str(), std::ios::in | std::ios::binary);
	if(ReadFile == false)
	{
		std::cerr << Path << ": Can not be opened." << std::endl;
	}
	ReadFile.seekg(-128, std::ios::end);
	ReadFile.read(pcBuffer, 3);
	pcBuffer[3] = '\0';
	if(strcmp(pcBuffer, "TAG") == 0)
	{
		std::cout << "ID3v1 TAG:" << std::endl;
		pcBuffer[30] = '\0';
		ReadFile.read(pcBuffer, 30);
		std::cout << "\tTitle:\t \"" << pcBuffer << "\"  [length: " << strlen(pcBuffer) << "]" << std::endl;
		ReadFile.read(pcBuffer, 30);
		std::cout << "\tArtist:\t \"" << pcBuffer << "\"  [length: " << strlen(pcBuffer) << "]" << std::endl;
		ReadFile.read(pcBuffer, 30);
		std::cout << "\tAlbum:\t \"" << pcBuffer << "\"  [length: " << strlen(pcBuffer) << "]" << std::endl;
		ReadFile.read(pcBuffer, 4);
		pcBuffer[4] = '\0';
		std::cout << "\tYear:\t \"" << pcBuffer << "\"  [length: " << strlen(pcBuffer) << "]" << std::endl;
		ReadFile.read(pcBuffer, 30);
		std::cout << "\tComment: \"" << pcBuffer << "\"  [length: " << strlen(pcBuffer) << "]" << std::endl;
		bID3v11 = false;
		if(pcBuffer[28] == '\0')
		{
			bID3v11 = true;
			u4Track = static_cast< count_t >(pcBuffer[29]);
		}
		ReadFile.read(pcBuffer, 1);
		std::cout << "\tGenre:\t \"" << g_sGenres[static_cast< u1byte >(*pcBuffer)] << "\"  [number: " << static_cast< count_t >(static_cast< u1byte >(*pcBuffer)) << "]" << std::endl;
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
	count_t u4Arguments = argc;
	count_t u4Argument = 0;

	while(++u4Argument < u4Arguments)
	{
		Paths.push_back(argv[u4Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	g_ID3v2TagFrameFlags[0x0040] = "Discard Frame on Tag Alteration";
	g_ID3v2TagFrameFlags[0x0020] = "Discard Frame on File Alteration";
	g_ID3v2TagFrameFlags[0x0010] = "Read Only";
	g_ID3v2TagFrameFlags[0x4000] = "Grouped Frame";
	g_ID3v2TagFrameFlags[0x8000] = "Compression";
	g_ID3v2TagFrameFlags[0x0400] = "Encryption";
	g_ID3v2TagFrameFlags[0x0200] = "Unsynchronisation";
	g_ID3v2TagFrameFlags[0x0100] = "Data Length Indicator";
	
	// ID3v2.3.0
	g_FrameNames["COMM"] = "Comments";
	g_FrameNames["MCDI"] = "Music CD identifier";
	g_FrameNames["PRIV"] = "Private frame";
	g_FrameNames["TALB"] = "Album/Movie/Show title";
	g_FrameNames["TBPM"] = "BPM (beats per minute)";
	g_FrameNames["TCOM"] = "Composer";
	g_FrameNames["TCON"] = "Content type";
	g_FrameNames["TCOP"] = "Copyright message";
	g_FrameNames["TIT1"] = "Content group description";
	g_FrameNames["TIT2"] = "Title/songname/content description";
	g_FrameNames["TIT3"] = "Subtitle/Description refinement";
	g_FrameNames["TLAN"] = "Language(s)";
	g_FrameNames["TLEN"] = "Length";
	g_FrameNames["TPE1"] = "Lead Performer(s) / Solo Artist(s)";
	g_FrameNames["TPE2"] = "Band / Orchestra / Accompaniment";
	g_FrameNames["TPE3"] = "Conductor / Performer Refinement";
	g_FrameNames["TPE4"] = "Interpreted, Remixed, or otherwise modified by";
	g_FrameNames["TPOS"] = "Part of a set";
	g_FrameNames["TPUB"] = "Publisher";
	g_FrameNames["TRCK"] = "Track number/Position in set";
	g_FrameNames["TSSE"] = "Software/Hardware and settings used for encoding";
	g_FrameNames["TXXX"] = "User defined text information frame";
	g_FrameNames["TYER"] = "Year";
	g_FrameNames["WCOM"] = "Commercial information";
	g_FrameNames["WXXX"] = "User defined URL link frame";
	
	// ID3v2.4.0
	g_FrameNames["TDRC"] = "Recording time";
	
	g_FrameHandlers["COMM"] = new COMMFrameHandler();
	g_FrameHandlers["MCDI"] = new MCDIFrameHandler();
	g_FrameHandlers["PRIV"] = new HexFrameHandler();
	g_FrameHandlers["TALB"] = new TextFrameHandler();
	g_FrameHandlers["TBPM"] = new TextFrameHandler();
	g_FrameHandlers["TCOM"] = new TextFrameHandler();
	g_FrameHandlers["TCON"] = new TextFrameHandler();
	g_FrameHandlers["TCOP"] = new TextFrameHandler();
	g_FrameHandlers["TDAT"] = new TextFrameHandler();
	g_FrameHandlers["TDRC"] = new TextFrameHandler();
	g_FrameHandlers["TENC"] = new TextFrameHandler();
	g_FrameHandlers["TEXT"] = new TextFrameHandler();
	g_FrameHandlers["TFLT"] = new TextFrameHandler();
	g_FrameHandlers["TIT1"] = new TextFrameHandler();
	g_FrameHandlers["TIT2"] = new TextFrameHandler();
	g_FrameHandlers["TIT3"] = new TextFrameHandler();
	g_FrameHandlers["TLAN"] = new TextFrameHandler();
	g_FrameHandlers["TLEN"] = new TextFrameHandler();
	g_FrameHandlers["TPE1"] = new TextFrameHandler();
	g_FrameHandlers["TPE2"] = new TextFrameHandler();
	g_FrameHandlers["TPE3"] = new TextFrameHandler();
	g_FrameHandlers["TPE4"] = new TextFrameHandler();
	g_FrameHandlers["TPOS"] = new TextFrameHandler();
	g_FrameHandlers["TPUB"] = new TextFrameHandler();
	g_FrameHandlers["TRCK"] = new TextFrameHandler();
	g_FrameHandlers["TSSE"] = new TextFrameHandler();
	g_FrameHandlers["TXXX"] = new TextFrameHandler();
	g_FrameHandlers["TYER"] = new TextFrameHandler();
	g_FrameHandlers["WCOM"] = new URLFrameHandler();
	g_FrameHandlers["WXXX"] = new WXXXFrameHandler();
	while(Paths.begin() != Paths.end())
	{
		vReadItem(Paths.front());
		Paths.pop_front();
	}

	return 0;
}
