#include <deque>
#include <map>
#include <string>

#include "../common/common.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// global variables and configuration                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::map< unsigned int, std::string > g_NumericGenresID3_1;
std::map< unsigned int, std::string > g_NumericGenresWinamp;
bool g_PrintBytes(false);

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

std::string Get_ID3_2_3_FrameIdentifier_Interpretation(const std::string & Value)
{
	if(Value == "AENC")
	{
		return "Audio encryption";
	}
	else if(Value == "APIC")
	{
		return "Attached Picture";
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
	else if(Value == "EQUA")
	{
		return "Equalisation";
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
	else if(Value == "IPLS")
	{
		return "Involved people list";
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
	else if(Value == "RVAD")
	{
		return "Relative volume adjustment";
	}
	else if(Value == "RVRB")
	{
		return "Reverb";
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
	else if(Value == "TBPM")
	{
		return "BPM (beats per minute)";
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
	else if(Value == "TDAT")
	{
		return "Date";
	}
	else if(Value == "TDLY")
	{
		return "Playlist delay";
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
	else if(Value == "TIME")
	{
		return "Time";
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
	else if(Value == "TMED")
	{
		return "Media type";
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
	else if(Value == "TORY")
	{
		return "Original release year";
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
	else if(Value == "TPUB")
	{
		return "Publisher";
	}
	else if(Value == "TRCK")
	{
		return "Track number/Position in set";
	}
	else if(Value == "TRDA")
	{
		return "Recording dates";
	}
	else if(Value == "TRSN")
	{
		return "Internet radio station name";
	}
	else if(Value == "TRSO")
	{
		return "Internet radio station owner";
	}
	else if(Value == "TSIZ")
	{
		return "Size";
	}
	else if(Value == "TSRC")
	{
		return "ISRC (international standard recording code)";
	}
	else if(Value == "TSSE")
	{
		return "Software/Hardware and settings used for encoding";
	}
	else if(Value == "TXXX")
	{
		return "User defined text information frame";
	}
	else if(Value == "TYER")
	{
		return "Year";
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
	else if(Value == "TBPM")
	{
		return "BPM (beats per minute)";
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
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PCNT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PRIV(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_RGAD(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCMP(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCON(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TFLT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TLAN(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TSRC(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header_Identifier(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length);
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
std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_NameCode(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_SignBit(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ID3_2_Tag(Inspection::Buffer & Buffer);
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

std::unique_ptr< Inspection::Result > Get_ID3_GUID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->SetValue(GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const Inspection::GUID & GUID{std::experimental::any_cast< const Inspection::GUID & >(GUIDResult->GetAny())};
		
		try
		{
			Result->GetValue()->PrependTag("interpretation", Inspection::Get_GUID_Interpretation(GUID));
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Result->GetValue()->PrependTag("interpretation", "<unknown GUID>"s);
		}
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

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_3_Frame_Header(Buffer)};
	
	Result->SetValue(HeaderResult->GetValue());
	if(HeaderResult->GetSuccess() == true)
	{
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(HeaderResult->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(HeaderResult->GetAny("Size")), 0)};
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > BodyHandler;
		
		if(Identifier == "APIC")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_APIC;
		}
		else if(Identifier == "COMM")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_COMM;
		}
		else if(Identifier == "GEOB")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_GEOB;
		}
		else if(Identifier == "MCDI")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_MCDI;
		}
		else if(Identifier == "PCNT")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_PCNT;
		}
		else if(Identifier == "POPM")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_POPM;
		}
		else if(Identifier == "PRIV")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_PRIV;
		}
		else if(Identifier == "RGAD")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_RGAD;
		}
		else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCOP") || (Identifier == "TDAT") || (Identifier == "TDRC") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIME") || (Identifier == "TIT1") || (Identifier == "TIT2") || (Identifier == "TIT3") || (Identifier == "TLEN") || (Identifier == "TMED") || (Identifier == "TOAL") || (Identifier == "TOFN") || (Identifier == "TOPE") || (Identifier == "TOWN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPE3") || (Identifier == "TPE4") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TRDA") || (Identifier == "TSIZ") || (Identifier == "TSO2") || (Identifier == "TSOA") || (Identifier == "TSOP") || (Identifier == "TSSE") || (Identifier == "TSST") || (Identifier == "TYER"))
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_T___;
		}
		else if(Identifier == "TCMP")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TCMP;
		}
		else if(Identifier == "TCON")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TCON;
		}
		else if(Identifier == "TFLT")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TFLT;
		}
		else if(Identifier == "TLAN")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TLAN;
		}
		else if(Identifier == "TSRC")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TSRC;
		}
		else if(Identifier == "TXXX")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TXXX;
		}
		else if(Identifier == "UFID")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_UFID;
		}
		else if(Identifier == "USLT")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_USLT;
		}
		else if(Identifier == "WCOM")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_W___;
		}
		else if(Identifier == "WOAF")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_W___;
		}
		else if(Identifier == "WOAR")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_W___;
		}
		else if(Identifier == "WXXX")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_WXXX;
		}
		if(BodyHandler != nullptr)
		{
			auto BodyResult{BodyHandler(Buffer, Size)};
			
			if(Size > BodyResult->GetLength())
			{
				Result->GetValue()->PrependTag("handled size", to_string_cast(BodyResult->GetLength()));
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated larger than the actually handled size."s);
			}
			else if(BodyResult->GetLength() < Size)
			{
				Result->GetValue()->PrependTag("handled size", to_string_cast(BodyResult->GetLength()));
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated smaller than the actually handled size."s);
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
	
	if(TableOfContentsResult->GetSuccess() == true)
	{
		Result->SetValue(TableOfContentsResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Result->GetOffset());
		
		auto MCDIStringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Buffer, Length)};
		
		if(MCDIStringResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("String", MCDIStringResult->GetValue());
			Result->GetValue("String")->PrependTag("error", "The content of an \"MCDI\" frame should be a binary compact disc table of contents, but is a unicode string encoded with UCS-2 in little endian."s);
			Result->SetSuccess(true);
		}
	}
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

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PRIV(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
	
	Result->GetValue()->Append("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		const std::string & OwnerIdentifier{std::experimental::any_cast< const std::string & >(OwnerIdentifierResult->GetAny())};
		std::string PRIVDataName;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > PRIVDataHandler;
		
		if(OwnerIdentifier == "AverageLevel")
		{
			PRIVDataHandler = std::bind(Inspection::Get_UnsignedInteger_32Bit_LittleEndian, std::placeholders::_1);
			PRIVDataName = "AverageLevel";
		}
		else if(OwnerIdentifier == "PeakValue")
		{
			PRIVDataHandler = std::bind(Inspection::Get_UnsignedInteger_32Bit_LittleEndian, std::placeholders::_1);
			PRIVDataName = "PeakValue";
		}
		else if(OwnerIdentifier == "WM/MediaClassPrimaryID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "MediaClassPrimaryID";
		}
		else if(OwnerIdentifier == "WM/MediaClassSecondaryID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "MediaClassSecondaryID";
		}
		else if(OwnerIdentifier == "WM/Provider")
		{
			PRIVDataHandler = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
			PRIVDataName = "Provider";
		}
		else if(OwnerIdentifier == "WM/UniqueFileIdentifier")
		{
			PRIVDataHandler = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
			PRIVDataName = "UniqueFileIdentifier";
		}
		else if(OwnerIdentifier == "WM/WMCollectionGroupID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "CollectionGroupID";
		}
		else if(OwnerIdentifier == "WM/WMCollectionID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "CollectionID";
		}
		else if(OwnerIdentifier == "WM/WMContentID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ContentID";
		}
		else if(OwnerIdentifier == "ZuneAlbumArtistMediaID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneAlbumArtistMediaID";
		}
		else if(OwnerIdentifier == "ZuneAlbumMediaID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneAlbumArtistMediaID";
		}
		else if(OwnerIdentifier == "ZuneCollectionID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneAlbumArtistMediaID";
		}
		else if(OwnerIdentifier == "ZuneMediaID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneMediaID";
		}
		else
		{
			PRIVDataHandler = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
			PRIVDataName = "PrivateData";
		}
		
		auto PRIVDataResult{PRIVDataHandler(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->Append(PRIVDataName, PRIVDataResult->GetValue());
		Result->SetSuccess(PRIVDataResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_RGAD(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PeakAmplitudeResult{Get_ISO_IEC_IEEE_60559_2011_binary32(Buffer)};
	
	Result->GetValue()->Append("PeakAmplitude", PeakAmplitudeResult->GetValue());
	if(PeakAmplitudeResult->GetSuccess() == true)
	{
		auto TrackReplayGainAdjustmentResult{Get_ID3_2_ReplayGainAdjustment(Buffer)};
		
		Result->GetValue()->Append("TrackReplayGainAdjustment", TrackReplayGainAdjustmentResult->GetValue());
		if(TrackReplayGainAdjustmentResult->GetSuccess() == true)
		{
			auto AlbumReplayGainAdjustmentResult{Get_ID3_2_ReplayGainAdjustment(Buffer)};
			
			Result->GetValue()->Append("AlbumReplayGainAdjustment", AlbumReplayGainAdjustmentResult->GetValue());
			Result->SetSuccess(AlbumReplayGainAdjustmentResult->GetSuccess());
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

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TSRC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		
		if(Information.length() == 12)
		{
			Result->GetValue("Information")->PrependTag("standard", "ISRC Bulletin 2015/01"s);
			Result->GetValue("Information")->PrependTag("DesignationCode", Information.substr(7, 5));
			Result->GetValue("Information")->PrependTag("YearOfReference", Information.substr(5, 2));
			Result->GetValue("Information")->PrependTag("RegistrantCode", Information.substr(2, 3));
			
			std::string CountryCode{Information.substr(0, 2)};
			auto CountryCodeValue{Result->GetValue("Information")->PrependTag("CountryCode", CountryCode)};
			
			try
			{
				CountryCodeValue->PrependTag("standard", "ISO 3166-1 alpha-2"s);
				CountryCodeValue->PrependTag("interpretation", Inspection::Get_CountryName_From_ISO_3166_1_Alpha_2_CountryCode(CountryCode));
				Result->SetSuccess(true);
			}
			catch(Inspection::UnknownValueException & Exception)
			{
				CountryCodeValue->PrependTag("standard", "ISRC Bulletin 2015/01"s);
				CountryCodeValue->PrependTag("error", "The ISRC string needs to contain a two letter country code from ISO 3166-1 alpha-2."s);
				CountryCodeValue->PrependTag("interpretation", "<unkown>"s);
			}
		}
		else
		{
			Result->GetValue("Information")->PrependTag("standard", "ID3 2.3"s);
			Result->GetValue("Information")->PrependTag("error", "The TSRC frame needs to contain a twelve letter ISRC code from ISRC Bulletin 2015/01."s);
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

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ID3_2_3_Frame_Header_Identifier(Buffer)};
	
	Result->GetValue()->Append("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		auto SizeResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->Append("Size", SizeResult->GetValue());
		if(SizeResult->GetSuccess() == true)
		{
			auto FlagsResult{Get_BitSet_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->Append("Flags", FlagsResult->GetValue());
			Result->SetSuccess(FlagsResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByLength(Buffer, Inspection::Length(4ull, 0))};
	
	Result->SetValue(IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(IdentifierResult->GetAny())};
		
		Result->GetValue()->PrependTag("standard", "ID3 2.3"s);
		try
		{
			Result->GetValue()->PrependTag("interpretation", Get_ID3_2_3_FrameIdentifier_Interpretation(Identifier));
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Identifier == "RGAD")
			{
				Result->GetValue()->PrependTag("new standard", "Hydrogenaudio ReplayGain"s);
				Result->GetValue()->PrependTag("interpretation", "Replay gain adjustment"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It is a non-standard frame which is acknowledged as an 'in the wild' tag by id3.org."s);
			}
			else if(Identifier == "TCMP")
			{
				Result->GetValue()->PrependTag("new standard", "iTunes"s);
				Result->GetValue()->PrependTag("interpretation", "Part of a compilation"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate whether a title is a part of a compilation."s);
			}
			else if(Identifier == "TDRC")
			{
				Result->GetValue()->PrependTag("new standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "Recording time"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TDTG")
			{
				Result->GetValue()->PrependTag("new standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "Tagging time"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSST")
			{
				Result->GetValue()->PrependTag("new standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "Set subtitle"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSOA")
			{
				Result->GetValue()->PrependTag("new standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "Album sort order"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSOP")
			{
				Result->GetValue()->PrependTag("new standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "Performer sort order"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSO2")
			{
				Result->GetValue()->PrependTag("new standard", "iTunes"s);
				Result->GetValue()->PrependTag("interpretation", "Album artist sort order"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate the album artist sort order."s);
			}
			else
			{
				Result->GetValue()->PrependTag("error", "Unkown frame identifier \"" + Identifier + "\"."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_3_Frame(Buffer)};
		
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
		else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCON") || (Identifier == "TCOP") || (Identifier == "TDRC") || (Identifier == "TDRL") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIT2") || (Identifier == "TLAN") || (Identifier == "TLEN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TSSE") || (Identifier == "TYER"))
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
		auto Continue{true};
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
				Continue = false;
			}
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
		if(Continue == true)
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
			else
			{
				Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->PrependTag("error", "The language code \"" + Code + "\" is unknown."s);
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

std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NameCodeResult{Get_ID3_2_ReplayGainAdjustment_NameCode(Buffer)};
	
	Result->GetValue()->Append("NameCode", NameCodeResult->GetValue());
	if(NameCodeResult->GetSuccess() == true)
	{
		auto OriginatorCodeResult{Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Buffer)};
		
		Result->GetValue()->Append("OriginatorCode", OriginatorCodeResult->GetValue());
		if(OriginatorCodeResult->GetSuccess() == true)
		{
			auto SignBitResult{Get_ID3_2_ReplayGainAdjustment_SignBit(Buffer)};
			
			Result->GetValue()->Append("SignBit", SignBitResult->GetValue());
			if(SignBitResult->GetSuccess() == true)
			{
				auto ReplayGainAdjustmentResult{Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Buffer)};
				
				Result->GetValue()->Append("ReplayGainAdjustment", ReplayGainAdjustmentResult->GetValue());
				if(ReplayGainAdjustmentResult->GetSuccess() == true)
				{
					auto SignBit{std::experimental::any_cast< std::uint8_t >(SignBitResult->GetAny())};
					auto ReplayGainAdjustment{std::experimental::any_cast< float >(ReplayGainAdjustmentResult->GetValue()->GetTagAny("Interpretation"))};
					
					if(SignBit == 0x01)
					{
						ReplayGainAdjustment *= -1.0f;
					}
					Result->GetValue()->PrependTag("Standard", "Hydrogenaudio ReplayGain"s);
					Result->GetValue()->PrependTag("Interpretation", to_string_cast(ReplayGainAdjustment) + " dB");
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_NameCode(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NameCodeResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	Result->SetValue(NameCodeResult->GetValue());
	if(NameCodeResult->GetSuccess() == true)
	{
		auto NameCode{std::experimental::any_cast< std::uint8_t >(NameCodeResult->GetAny())};
		
		if(NameCode == 0x00)
		{
			Result->GetValue()->PrependTag("Interpretation", "not set"s);
			Result->SetSuccess(true);
		}
		else if(NameCode == 0x01)
		{
			Result->GetValue()->PrependTag("Interpretation", "track gain adjustment"s);
			Result->SetSuccess(true);
		}
		else if(NameCode == 0x02)
		{
			Result->GetValue()->PrependTag("Interpretation", "album gain adjustment"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("Interpretation", "<unknown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OriginatorCodeResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	Result->SetValue(OriginatorCodeResult->GetValue());
	if(OriginatorCodeResult->GetSuccess() == true)
	{
		auto OriginatorCode{std::experimental::any_cast< std::uint8_t >(OriginatorCodeResult->GetAny())};
		
		if(OriginatorCode == 0x00)
		{
			Result->GetValue()->PrependTag("Interpretation", "unspecified"s);
			Result->SetSuccess(true);
		}
		else if(OriginatorCode == 0x01)
		{
			Result->GetValue()->PrependTag("Interpretation", "pre-set by artist/producer/mastering engineer"s);
			Result->SetSuccess(true);
		}
		else if(OriginatorCode == 0x02)
		{
			Result->GetValue()->PrependTag("Interpretation", "set by user"s);
			Result->SetSuccess(true);
		}
		else if(OriginatorCode == 0x03)
		{
			Result->GetValue()->PrependTag("Interpretation", "determined automatically"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("Interpretation", "<unknown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReplayGainAdjustmentResult{Get_UnsignedInteger_9Bit_BigEndian(Buffer)};
	
	Result->SetValue(ReplayGainAdjustmentResult->GetValue());
	if(ReplayGainAdjustmentResult->GetSuccess() == true)
	{
		float ReplayGainAdjustment{static_cast< float >(std::experimental::any_cast< std::uint16_t >(ReplayGainAdjustmentResult->GetAny()))};
		
		Result->GetValue()->PrependTag("Interpretation", ReplayGainAdjustment / 10.0f);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_SignBit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SignBitResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(SignBitResult->GetValue());
	if(SignBitResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto SignBit{std::experimental::any_cast< std::uint8_t >(SignBitResult->GetAny())};
		
		if(SignBit == 0x00)
		{
			Result->GetValue()->PrependTag("Interpretation", "positive gain (boost)"s);
		}
		else
		{
			Result->GetValue()->PrependTag("Interpretation", "negative gain (attenuation)"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ID3_2_Tag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TagHeaderResult{Get_ID3_2_Tag_Header(Buffer)};
	
	Result->GetValue()->Append(TagHeaderResult->GetValue()->GetValues());
	if(TagHeaderResult->GetSuccess() == true)
	{
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(TagHeaderResult->GetAny("MajorVersion"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(TagHeaderResult->GetAny("Size")), 0)};
		
		if(MajorVersion == 0x02)
		{
			auto FramesResult{Get_ID3_2_2_Frames(Buffer, Size)};
			
			Result->GetValue()->Append(FramesResult->GetValue()->GetValues());
			Result->SetSuccess(FramesResult->GetSuccess());
		}
		else if(MajorVersion == 0x03)
		{
			if((TagHeaderResult->GetValue("Flags")->HasValue("ExtendedHeader") == true) && (std::experimental::any_cast< bool >(TagHeaderResult->GetValue("Flags")->GetValueAny("ExtendedHeader")) == true))
			{
				throw Inspection::NotImplementedException("ID3 2.3 extended header");
			}
			
			auto FramesResult{Get_ID3_2_3_Frames(Buffer, Size)};
			
			Result->GetValue()->Append(FramesResult->GetValue()->GetValues());
			Result->SetSuccess(FramesResult->GetSuccess());
		}
		else if(MajorVersion == 0x04)
		{
			std::unique_ptr< Inspection::Result > ExtendedHeaderResult;
			
			if((TagHeaderResult->GetValue("Flags")->HasValue("ExtendedHeader") == true) && (std::experimental::any_cast< bool >(TagHeaderResult->GetValue("Flags")->GetValueAny("ExtendedHeader")) == true))
			{
				ExtendedHeaderResult = Get_ID3_2_4_Tag_ExtendedHeader(Buffer);
				Result->GetValue()->Append("ExtendedHeader", ExtendedHeaderResult->GetValue());
				Result->SetSuccess(ExtendedHeaderResult->GetSuccess());
				Size -= ExtendedHeaderResult->GetLength();
			}
			if((ExtendedHeaderResult == nullptr) || (ExtendedHeaderResult->GetSuccess() == true))
			{
				auto FramesResult{Get_ID3_2_4_Frames(Buffer, Size)};
				
				Result->GetValue()->Append(FramesResult->GetValue()->GetValues());
				Result->SetSuccess(FramesResult->GetSuccess());
			}
		}
		else
		{
			Result->GetValue()->PrependTag("error", "Unknown major version \"" + to_string_cast(MajorVersion) + "\".");
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
// application                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->SetName("ID3Tags");
	
	std::unique_ptr< Inspection::Result > ID3v1TagResult;
	
	if(Buffer.GetLength() >= Inspection::Length(128ull, 0))
	{
		Buffer.SetPosition(Buffer.GetLength() - Inspection::Length(128ull, 0));
		ID3v1TagResult = Get_ID3_1_Tag(Buffer);
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
	}
	Buffer.SetPosition(Inspection::Length(0ull, 0));
	
	auto ID3v2TagResult{Get_ID3_2_Tag(Buffer)};
	
	if(ID3v2TagResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("ID3v2", ID3v2TagResult->GetValue());
	}
	Result->SetSuccess(((ID3v1TagResult != nullptr) && (ID3v1TagResult->GetSuccess() == true)) || ID3v2TagResult->GetSuccess());
	Buffer.SetPosition(Buffer.GetLength());
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
	// processing
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}

	return 0;
}
