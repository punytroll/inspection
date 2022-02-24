#include <numeric>
#include <sstream>
#include <time.h>
#include <vector>

#include "assertion.h"
#include "explode.h"
#include "helper.h"
#include "string_cast.h"
#include "unknown_value_exception.h"

using namespace std::string_literals;

/// Top-level ASF Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.1
Inspection::GUID Inspection::g_ASF_HeaderObjectGUID{"75b22630-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID Inspection::g_ASF_DataObjectGUID{"75b22636-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID Inspection::g_ASF_SimpleIndexObjectGUID{"33000890-e5b1-11cf-89f4-00a0c90349cb"};
Inspection::GUID Inspection::g_ASF_IndexObjectGUID{"D6E229D3-35DA-11D1-9034-00A0C90349BE"};
Inspection::GUID Inspection::g_ASF_MediaObjectIndexObjectGUID{"feb103f8-12ad-4c64-840f-2a1d2f7ad48c"};
Inspection::GUID Inspection::g_ASF_TimecodeIndexObject{"3cb73fd0-0c4a-4803-953d-edf7b6228f0c"};

/// Header Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.2
Inspection::GUID Inspection::g_ASF_FilePropertiesObjectGUID{"8cabdca1-a947-11cf-8ee4-00c00c205365"};
Inspection::GUID Inspection::g_ASF_StreamPropertiesObjectGUID{"b7dc0791-a9b7-11cf-8ee6-00c00c205365"};
Inspection::GUID Inspection::g_ASF_HeaderExtensionObjectGUID{"5fbf03b5-a92e-11cf-8ee3-00c00c205365"};
Inspection::GUID Inspection::g_ASF_CodecListObjectGUID{"86d15240-311d-11d0-a3a4-00a0c90348f6"};
Inspection::GUID Inspection::g_ASF_ScriptCommandObjectGUID{"1efb1a30-0b62-11d0-a39b-00a0c90348f6"};
Inspection::GUID Inspection::g_ASF_MarkerObjectGUID{"f487cd01-a951-11cf-8ee6-00c00c205365"};
Inspection::GUID Inspection::g_ASF_BitrateMutualExclusionObjectGUID{"d6e229dc-35da-11d1-9034-00a0c90349be"};
Inspection::GUID Inspection::g_ASF_ErrorCorrectionObjectGUID{"75b22635-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID Inspection::g_ASF_ContentDescriptionObjectGUID{"75b22633-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID Inspection::g_ASF_ExtendedContentDescriptionObjectGUID{"d2d0a440-e307-11d2-97f0-00a0c95ea850"};
Inspection::GUID Inspection::g_ASF_ContentBrandingObjectGUID{"2211b3fa-bd23-11d2-b4b7-00a0c955fc6e"};
Inspection::GUID Inspection::g_ASF_StreamBitratePropertiesObjectGUID{"7bf875ce-468d-11d1-8d82-006097c9a2b2"};
Inspection::GUID Inspection::g_ASF_ContentEncryptionObjectGUID{"2211b3fb-bd23-11d2-b4b7-00a0c955fc6e"};
Inspection::GUID Inspection::g_ASF_ExtendedContentEncryptionObjectGUID{"298ae614-2622-4c17-b935-dae07ee9289c"};
Inspection::GUID Inspection::g_ASF_DigitalSignatureObjectGUID{"2211b3fc-bd23-11d2-b4b7-00a0c955fc6e"};
Inspection::GUID Inspection::g_ASF_PaddingObjectGUID{"1806d474-cadf-4509-a4ba-9aabcb96aae8"};

/// Header Extension Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.3
Inspection::GUID Inspection::g_ASF_ExtendedStreamPropertiesObjectGUID{"14e6a5cb-c672-4332-8399-a96952065b5a"};
Inspection::GUID Inspection::g_ASF_AdvancedMutualExclusionObjectGUID{"a08649cf-4775-4670-8a16-6e35357566cd"};
Inspection::GUID Inspection::g_ASF_GroupMutualExclusionObjectGUID{"d1465a40-5a79-4338-b71b-e36b8fd6c249"};
Inspection::GUID Inspection::g_ASF_StreamPrioritizationObjectGUID{"d4fed15b-88d3-454f-81f0-ed5c45999e24"};
Inspection::GUID Inspection::g_ASF_BandwidthSharingObjectGUID{"a69609e6-517b-11d2-b6af-00c04fd908e9"};
Inspection::GUID Inspection::g_ASF_LanguageListObjectGUID{"7c4346a9-efe0-4bfc-b229-393ede415c85"};
Inspection::GUID Inspection::g_ASF_MetadataObjectGUID{"c5f8cbea-5baf-4877-8467-aa8c44fa4cca"};
Inspection::GUID Inspection::g_ASF_MetadataLibraryObjectGUID{"44231c94-9498-49d1-a141-1d134e457054"};
Inspection::GUID Inspection::g_ASF_IndexParametersObjectGUID{"d6e229df-35da-11d1-9034-00a0c90349be"};
Inspection::GUID Inspection::g_ASF_MediaObjectIndexParametersObjectGUID{"6b203bad-3f11-48e4-aca8-d7613de2cfa7"};
Inspection::GUID Inspection::g_ASF_TimecodeIndexParametersObjectGUID{"f55e496d-9797-4b5d-8c8b-604dfe9bfb24"};
Inspection::GUID Inspection::g_ASF_CompatibilityObjectGUID{"26f18b5d-4584-47ec-9f5f-0e651f0452c9"};
Inspection::GUID Inspection::g_ASF_AdvancedContentEncryptionObjectGUID{"43058533-6981-49e6-9b74-ad12cb86d58c"};

/// Stream Properties Object Stream Type GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.4
Inspection::GUID Inspection::g_ASF_AudioMediaGUID{"f8699e40-5b4d-11cf-a8fd-00805f5c442b"};
Inspection::GUID Inspection::g_ASF_VideoMediaGUID{"bc19efc0-5b4d-11cf-a8fd-00805f5c442b"};
Inspection::GUID Inspection::g_ASF_CommandMediaGUID{"59dacfc0-59e6-11d0-a3ac-00a0c90348f6"};
Inspection::GUID Inspection::g_ASF_JFIFMediaGUID{"b61be100-5b4e-11cf-a8fd-00805f5c442b"};
Inspection::GUID Inspection::g_ASF_DegradableJPEGMediaGUID{"35907de0-e415-11cf-a917-00805f5c442b"};
Inspection::GUID Inspection::g_ASF_FileTransferMediaGUID{"91bd222c-f21c-497a-8b6d-5aa86bfc0185"};
Inspection::GUID Inspection::g_ASF_BinaryMediaGUID{"3afb65e2-47ef-40f2-ac2c-70a90d71d343"};

/// Stream Properties Object Error Correction Type GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.5
Inspection::GUID Inspection::g_ASF_NoErrorCorrectionGUID{"20fb5700-5b55-11cf-a8fd-00805f5c442b"};
Inspection::GUID Inspection::g_ASF_AudioSpreadGUID{"bfc3cd50-618f-11cf-8bb2-00aa00b4e220"};

/// Header Extension Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.6
Inspection::GUID Inspection::g_ASF_Reserved1GUID{"abd3d211-a9ba-11cf-8ee6-00c00c205365"};

/// Codec List Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.8
Inspection::GUID Inspection::g_ASF_Reserved2GUID{"86d15241-311d-11d0-a3a4-00a0c90348f6"};

/// Other GUIDs found on the web
Inspection::GUID Inspection::g_ASF_IndexPlaceholderObjectGUID{"d9aade20-7c17-4f9c-bc28-8555dd98e2a2"};

/// RIFF GUIDs
Inspection::GUID Inspection::g_KSDATAFORMAT_SUBTYPE_PCM{"00000001-0000-0010-8000-00aa00389b71"};

/// WM/MediaClassPrimaryID
Inspection::GUID Inspection::g_WM_MediaClassPrimaryID_AudioMusic{"d1607dbc-e323-4be2-86a1-48a42a28441e"};
Inspection::GUID Inspection::g_WM_MediaClassPrimaryID_AudioNoMusic{"01cd0f29-da4e-4157-897b-6275d50c4f11"};
Inspection::GUID Inspection::g_WM_MediaClassPrimaryID_NeitherAudioNorMusic{"fcf24a76-9a57-4036-990d-e35dd8b244e1"};
Inspection::GUID Inspection::g_WM_MediaClassPrimaryID_Video{"db9830bd-3ab3-4fab-8a37-1a995f7ff74b"};

bool Inspection::Is_ASCII_Character_Alphabetic(std::uint8_t Character)
{
    return ((Character >= 0x41) && (Character < 0x5b)) || ((Character >= 0x61) && (Character < 0x7b));
}

bool Inspection::Is_ASCII_Character_Alphabetic_LowerCase(std::uint8_t Character)
{
    return (Character >= 0x61) && (Character < 0x7b);
}

bool Inspection::Is_ASCII_Character_Alphabetic_UpperCase(std::uint8_t Character)
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

bool Inspection::Is_ISO_IEC_8859_1_1998_Character(std::uint8_t Character)
{
    return ((Character >= 0x20) && (Character < 0x7f)) || (Character >= 0xa0);
}

std::string Inspection::Get_CountryName_From_ISO_3166_1_Alpha_2_CountryCode(const std::string & ISO_3166_1_Alpha_2_CountryCode)
{
    if(ISO_3166_1_Alpha_2_CountryCode == "GB")
    {
        return "United Kingdom";
    }
    else if(ISO_3166_1_Alpha_2_CountryCode == "US")
    {
        return "United States of America";
    }
    else if(ISO_3166_1_Alpha_2_CountryCode == "ZA")
    {
        return "South Africa";
    }
    else
    {
        throw Inspection::UnknownValueException(ISO_3166_1_Alpha_2_CountryCode);
    }
}

Inspection::DateTime Inspection::Get_DateTime_FromMicrosoftFileTime(std::uint64_t FileTime)
{
    return Inspection::Get_DateTime_FromUnixTimeStamp(Get_Unix_TimeStamp_FromWindowsFileTime(FileTime));
}

Inspection::DateTime Inspection::Get_DateTime_FromUnixTimeStamp(std::uint64_t UnixTimeStamp)
{
    auto Result = Inspection::DateTime{};
    time_t LocalUnixTimeStamp = UnixTimeStamp;
    auto UnixTime = gmtime(&LocalUnixTimeStamp);
    
    Result.Year = UnixTime->tm_year + 1900;
    Result.Month = UnixTime->tm_mon + 1;
    Result.Day = UnixTime->tm_mday;
    Result.Hour = UnixTime->tm_hour;
    Result.Minute = UnixTime->tm_min;
    Result.Second = UnixTime->tm_sec;
    
    return Result;
}

Inspection::GUID Inspection::Get_GUID_FromString_WithCurlyBraces(const std::string & String)
{
    Inspection::GUID Result;
    
    if((String.length() == 38) && (String[0] == '{') && (String[37] == '}'))
    {
        std::vector< std::string > Fields;
        
        explode(String.substr(1, 36), '-', std::back_inserter(Fields));
        if((Fields.size() == 5) && (Fields[0].length() == 8) && (Fields[1].length() == 4) && (Fields[2].length() == 4) && (Fields[3].length() == 4) && (Fields[4].length() == 12))
        {
            Result.Data1 = Get_UnsignedInteger_32Bit_FromHexadecimalString(Fields[0]);
            Result.Data2 = Get_UnsignedInteger_16Bit_FromHexadecimalString(Fields[1]);
            Result.Data3 = Get_UnsignedInteger_16Bit_FromHexadecimalString(Fields[2]);
            Result.Data4[0] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[3].substr(0, 2));
            Result.Data4[1] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[3].substr(2, 2));
            Result.Data4[2] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(0, 2));
            Result.Data4[3] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(2, 2));
            Result.Data4[4] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(4, 2));
            Result.Data4[5] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(6, 2));
            Result.Data4[6] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(8, 2));
            Result.Data4[7] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(10, 2));
        }
        else
        {
            throw std::invalid_argument("The given string is not a valid GUID with curly braces.");
        }
    }
    else
    {
        throw std::invalid_argument("The given string is not a valid GUID with curly braces.");
    }
    
    return Result;
}

std::string Inspection::Get_GUID_Interpretation(const Inspection::GUID & GUID)
{
    // ASF Top-level Object GUIDs
    if(GUID == g_ASF_HeaderObjectGUID)
    {
        return "ASF_Header_Object";
    }
    else if(GUID == g_ASF_DataObjectGUID)
    {
        return "ASF_Data_Object";
    }
    else if(GUID == g_ASF_SimpleIndexObjectGUID)
    {
        return "ASF_Simple_Index_Object";
    }
    else if(GUID == g_ASF_IndexObjectGUID)
    {
        return "ASF_Index_Object";
    }
    else if(GUID == g_ASF_MediaObjectIndexObjectGUID)
    {
        return "ASF_Media_Object_Index_Object";
    }
    else if(GUID == g_ASF_TimecodeIndexObject)
    {
        return "ASF_Timecode_Index_Object";
    }
    // ASF Header Object GUIDs
    else if(GUID == g_ASF_FilePropertiesObjectGUID)
    {
        return "ASF_File_Properties_Object";
    }
    else if(GUID == g_ASF_StreamPropertiesObjectGUID)
    {
        return "ASF_Stream_Properties_Object";
    }
    else if(GUID == g_ASF_HeaderExtensionObjectGUID)
    {
        return "ASF_Header_Extension_Object";
    }
    else if(GUID == g_ASF_CodecListObjectGUID)
    {
        return "ASF_Codec_List_Object";
    }
    else if(GUID == g_ASF_ScriptCommandObjectGUID)
    {
        return "ASF_Script_Command_Object";
    }
    else if(GUID == g_ASF_MarkerObjectGUID)
    {
        return "ASF_Marker_Object";
    }
    else if(GUID == g_ASF_BitrateMutualExclusionObjectGUID)
    {
        return "ASF_Bitrate_Mutual_Exclusion_Object";
    }
    else if(GUID == g_ASF_ErrorCorrectionObjectGUID)
    {
        return "ASF_Error_Correction_Object";
    }
    else if(GUID == g_ASF_ContentDescriptionObjectGUID)
    {
        return "ASF_Content_Description_Object";
    }
    else if(GUID == g_ASF_ExtendedContentDescriptionObjectGUID)
    {
        return "ASF_Extended_Content_Description_Object";
    }
    else if(GUID == g_ASF_ContentBrandingObjectGUID)
    {
        return "ASF_Content_Branding_Object";
    }
    else if(GUID == g_ASF_StreamBitratePropertiesObjectGUID)
    {
        return "ASF_Stream_Bitrate_Properties_Object";
    }
    else if(GUID == g_ASF_ContentEncryptionObjectGUID)
    {
        return "ASF_Content_Encryption_Object";
    }
    else if(GUID == g_ASF_ExtendedContentEncryptionObjectGUID)
    {
        return "ASF_Extended_Content_Encryption_Object";
    }
    else if(GUID == g_ASF_DigitalSignatureObjectGUID)
    {
        return "ASF_Digital_Signature_Object";
    }
    else if(GUID == g_ASF_PaddingObjectGUID)
    {
        return "ASF_Padding_Object";
    }
    // ASF Header Extension Object GUIDs
    else if(GUID == g_ASF_ExtendedStreamPropertiesObjectGUID)
    {
        return "ASF_Extended_Stream_Properties_Object";
    }
    else if(GUID == g_ASF_AdvancedMutualExclusionObjectGUID)
    {
        return "ASF_Advanced_Mutual_Exclusion_Object";
    }
    else if(GUID == g_ASF_GroupMutualExclusionObjectGUID)
    {
        return "ASF_Group_Mutual_Exclusion_Object";
    }
    else if(GUID == g_ASF_StreamPrioritizationObjectGUID)
    {
        return "ASF_Stream_Prioritization_Object";
    }
    else if(GUID == g_ASF_BandwidthSharingObjectGUID)
    {
        return "ASF_Bandwidth_Sharing_Object";
    }
    else if(GUID == g_ASF_LanguageListObjectGUID)
    {
        return "ASF_Language_List_Object";
    }
    else if(GUID == g_ASF_MetadataObjectGUID)
    {
        return "ASF_Metadata_Object";
    }
    else if(GUID == g_ASF_MetadataLibraryObjectGUID)
    {
        return "ASF_Metadata_Library_Object";
    }
    else if(GUID == g_ASF_IndexParametersObjectGUID)
    {
        return "ASF_Index_Parameters_Object";
    }
    else if(GUID == g_ASF_MediaObjectIndexParametersObjectGUID)
    {
        return "ASF_Media_Object_Index_Parameters_Object";
    }
    else if(GUID == g_ASF_TimecodeIndexParametersObjectGUID)
    {
        return "ASF_Timecode_Index_Parameters_Object";
    }
    else if(GUID == g_ASF_CompatibilityObjectGUID)
    {
        return "ASF_Compatibility_Object";
    }
    else if(GUID == g_ASF_AdvancedContentEncryptionObjectGUID)
    {
        return "ASF_Advanced_Content_Encryption_Object";
    }
    // ASF Stream Properties Object Stream Type GUIDs
    else if(GUID == g_ASF_AudioMediaGUID)
    {
        return "ASF_Audio_Media";
    }
    else if(GUID == g_ASF_VideoMediaGUID)
    {
        return "ASF_Video_Media";
    }
    else if(GUID == g_ASF_CommandMediaGUID)
    {
        return "ASF_Command_Media";
    }
    else if(GUID == g_ASF_JFIFMediaGUID)
    {
        return "ASF_JFIF_Media";
    }
    else if(GUID == g_ASF_DegradableJPEGMediaGUID)
    {
        return "ASF_Degradable_JPEG_Media";
    }
    else if(GUID == g_ASF_FileTransferMediaGUID)
    {
        return "ASF_File_Transfer_Media";
    }
    else if(GUID == g_ASF_BinaryMediaGUID)
    {
        return "ASF_Binary_Media";
    }
    // ASF Stream Properties Object Error Correction Type GUIDs
    else if(GUID == g_ASF_NoErrorCorrectionGUID)
    {
        return "ASF_No_Error_Correction";
    }
    else if(GUID == g_ASF_AudioSpreadGUID)
    {
        return "ASF_Audio_Spread";
    }
    // ASF Header Extension Object GUIDs
    else if(GUID == g_ASF_Reserved1GUID)
    {
        return "ASF_Reserved_1";
    }
    // ASF Codec List Object GUIDs
    else if(GUID == g_ASF_Reserved2GUID)
    {
        return "ASF_Reserved_2";
    }
    // ASF other GUIDs
    else if(GUID == g_ASF_IndexPlaceholderObjectGUID)
    {
        return "ASF_Index_Placeholder_Object";
    }
    // WM/MediaClassPrimaryID
    else if(GUID == g_WM_MediaClassPrimaryID_AudioMusic)
    {
        return "audio, music";
    }
    else if(GUID == g_WM_MediaClassPrimaryID_AudioNoMusic)
    {
        return "audio, no music";
    }
    else if(GUID == g_WM_MediaClassPrimaryID_NeitherAudioNorMusic)
    {
        return "neither audio nor video";
    }
    else if(GUID == g_WM_MediaClassPrimaryID_Video)
    {
        return "video";
    }
    else
    {
        return "<unknown GUID>";
    }
}

std::string Inspection::Get_ID3_1_Genre(std::uint8_t GenreNumber)
{
    switch(GenreNumber)
    {
    case 0: return "Blues";
    case 1: return "Classic Rock";
    case 2: return "Country";
    case 3: return "Dance";
    case 4: return "Disco";
    case 5: return "Funk";
    case 6: return "Grunge";
    case 7: return "Hip-Hop";
    case 8: return "Jazz";
    case 9: return "Metal";
    case 10: return "New Age";
    case 11: return "Oldies";
    case 12: return "Other";
    case 13: return "Pop";
    case 14: return "R&B";
    case 15: return "Rap";
    case 16: return "Reggae";
    case 17: return "Rock";
    case 18: return "Techno";
    case 19: return "Industrial";
    case 20: return "Alternative";
    case 21: return "Ska";
    case 22: return "Death Metal";
    case 23: return "Pranks";
    case 24: return "Soundtrack";
    case 25: return "Euro-Techno";
    case 26: return "Ambient";
    case 27: return "Trip-Hop";
    case 28: return "Vocal";
    case 29: return "Jazz+Funk";
    case 30: return "Fusion";
    case 31: return "Trance";
    case 32: return "Classical";
    case 33: return "Instrumental";
    case 34: return "Acid";
    case 35: return "House";
    case 36: return "Game";
    case 37: return "Sound Clip";
    case 38: return "Gospel";
    case 39: return "Noise";
    case 40: return "AlternRock";
    case 41: return "Bass";
    case 42: return "Soul";
    case 43: return "Punk";
    case 44: return "Space";
    case 45: return "Meditative";
    case 46: return "Instrumental Pop";
    case 47: return "Instrumental Rock";
    case 48: return "Ethnic";
    case 49: return "Gothic";
    case 50: return "Darkwave";
    case 51: return "Techno-Industrial";
    case 52: return "Electronic";
    case 53: return "Pop-Folk";
    case 54: return "Eurodance";
    case 55: return "Dream";
    case 56: return "Southern Rock";
    case 57: return "Comedy";
    case 58: return "Cult";
    case 59: return "Gangsta";
    case 60: return "Top 40";
    case 61: return "Christian Rap";
    case 62: return "Pop/Funk";
    case 63: return "Jungle";
    case 64: return "Native American";
    case 65: return "Cabaret";
    case 66: return "New Wave";
    case 67: return "Psychadelic";
    case 68: return "Rave";
    case 69: return "Showtunes";
    case 70: return "Trailer";
    case 71: return "Lo-Fi";
    case 72: return "Tribal";
    case 73: return "Acid Punk";
    case 74: return "Acid Jazz";
    case 75: return "Polka";
    case 76: return "Retro";
    case 77: return "Musical";
    case 78: return "Rock & Roll";
    case 79: return "Hard Rock";
    default: throw Inspection::UnknownValueException(to_string_cast(GenreNumber));
    }
}

std::string Inspection::Get_ID3_1_Winamp_Genre(std::uint8_t GenreNumber)
{
    switch(GenreNumber)
    {
    case 80: return "Folk";
    case 81: return "Folk-Rock";
    case 82: return "National Folk";
    case 83: return "Swing";
    case 84: return "Fast Fusion";
    case 85: return "Bebob";
    case 86: return "Latin";
    case 87: return "Revival";
    case 88: return "Celtic";
    case 89: return "Bluegrass";
    case 90: return "Avantgarde";
    case 91: return "Gothic Rock";
    case 92: return "Progressive Rock";
    case 93: return "Psychedelic Rock";
    case 94: return "Symphonic Rock";
    case 95: return "Slow Rock";
    case 96: return "Big Band";
    case 97: return "Chorus";
    case 98: return "Easy Listening";
    case 99: return "Acoustic";
    case 100: return "Humour";
    case 101: return "Speech";
    case 102: return "Chanson";
    case 103: return "Opera";
    case 104: return "Chamber Music";
    case 105: return "Sonata";
    case 106: return "Symphony";
    case 107: return "Booty Bass";
    case 108: return "Primus";
    case 109: return "Porn Groove";
    case 110: return "Satire";
    case 111: return "Slow Jam";
    case 112: return "Club";
    case 113: return "Tango";
    case 114: return "Samba";
    case 115: return "Folklore";
    case 116: return "Ballad";
    case 117: return "Power Ballad";
    case 118: return "Rhythmic Soul";
    case 119: return "Freestyle";
    case 120: return "Duet";
    case 121: return "Punk Rock";
    case 122: return "Drum Solo";
    case 123: return "Acapella";
    case 124: return "Euro-House";
    case 125: return "Dance Hall";
    default: throw Inspection::UnknownValueException(to_string_cast(GenreNumber));
    }
}

std::string Inspection::Get_ID3_2_PictureType_Interpretation(std::uint8_t Value)
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

std::string Inspection::Get_ID3_2_3_FileType_Interpretation(const std::string & Value)
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

std::uint64_t Inspection::Get_Unix_TimeStamp_FromWindowsFileTime(std::uint64_t FileTime)
{
    const auto SecondsToUnixEpoch{11644473600ull};
    const auto WindowsTick{10000000ull};
    
    return (FileTime / WindowsTick) - SecondsToUnixEpoch;
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

std::string Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::uint32_t CodePoint)
{
    std::stringstream Result;
    
    if(CodePoint < 0x00000080)
    {
        Result << static_cast<char>(CodePoint & 0x0000007f);
    }
    else if(CodePoint < 0x00000800)
    {
        Result << static_cast<char>(0x00000c0 + ((CodePoint & 0x00000700) >> 6) + ((CodePoint & 0x000000c0) >> 6));
        Result << static_cast<char>(0x0000080 + (CodePoint & 0x0000003f));
    }
    else if(CodePoint < 0x00010000)
    {
        Result << static_cast<char>(0x00000e0 + ((CodePoint & 0x0000f000) >> 12));
        Result << static_cast<char>(0x0000080 + ((CodePoint & 0x00000f00) >> 6) + ((CodePoint & 0x000000c0) >> 6));
        Result << static_cast<char>(0x0000080 + (CodePoint & 0x0000003f));
    }
    else if(CodePoint < 0x00110000)
    {
        Result << static_cast<char>(0x00000f0 + ((CodePoint & 0x001c0000) >> 18));
        Result << static_cast<char>(0x0000080 + ((CodePoint & 0x00030000) >> 12) + ((CodePoint & 0x0000f000) >> 12));
        Result << static_cast<char>(0x0000080 + ((CodePoint & 0x00000f00) >> 6) + ((CodePoint & 0x000000c0) >> 6));
        Result << static_cast<char>(0x0000080 + (CodePoint & 0x0000003f));
    }
    else
    {
        ASSERTION(false);
    }
    
    return Result.str();
}

std::string Inspection::Get_LanguageName_From_ISO_639_2_1998_Code(const std::string & ISO_639_2_1998_Code)
{
    if(ISO_639_2_1998_Code == "bul")
    {
        return "Bulgarian";
    }
    else if(ISO_639_2_1998_Code == "deu")
    {
        return "German";
    }
    else if(ISO_639_2_1998_Code == "dut")
    {
        return "Dutch";
    }
    else if(ISO_639_2_1998_Code == "eng")
    {
        return "English";
    }
    else if(ISO_639_2_1998_Code == "fra")
    {
        return "French";
    }
    else if(ISO_639_2_1998_Code == "fre")
    {
        return "French";
    }
    else if(ISO_639_2_1998_Code == "ger")
    {
        return "German";
    }
    else if(ISO_639_2_1998_Code == "ita")
    {
        return "Italian";
    }
    else if(ISO_639_2_1998_Code == "nld")
    {
        return "Dutch";
    }
    else if(ISO_639_2_1998_Code == "rus")
    {
        return "Russian";
    }
    /// Scope: Special
    else if(ISO_639_2_1998_Code == "mis")
    {
        return "<Uncoded language>";
    }
    else if(ISO_639_2_1998_Code == "mul")
    {
        return "<Multiple languages>";
    }
    else if(ISO_639_2_1998_Code == "und")
    {
        return "<Undetermined>";
    }
    else if(ISO_639_2_1998_Code == "zxx")
    {
        return "<No linguistic content>";
    }
    else
    {
        throw std::invalid_argument("The ISO 639-2:1998 (alpha-3) code '" + ISO_639_2_1998_Code + "' is unknown.");
    }
}

std::string Inspection::JoinWithSeparator(const std::vector<std::string> & Strings, const std::string & Separator)
{
    if(Strings.empty() == true)
    {
        return std::string{};
    }
    else
    {
        return std::accumulate(Strings.begin() + 1, Strings.end(), Strings[0], [&Separator](const auto & Start, const auto & String) { return Start + Separator + String; } );
    }
}
