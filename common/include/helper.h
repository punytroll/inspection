#ifndef INSPECTION_COMMON_HELPER_H
#define INSPECTION_COMMON_HELPER_H

#include <cstdint>
#include <string>
#include <vector>

#include "date_time.h"
#include "guid.h"

namespace Inspection
{
    /// Top-level ASF Object GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.1
    extern Inspection::GUID g_ASF_HeaderObjectGUID;
    extern Inspection::GUID g_ASF_DataObjectGUID;
    extern Inspection::GUID g_ASF_SimpleIndexObjectGUID;
    extern Inspection::GUID g_ASF_IndexObjectGUID;
    extern Inspection::GUID g_ASF_MediaObjectIndexObjectGUID;
    extern Inspection::GUID g_ASF_TimecodeIndexObject;

    /// ASF Header Object GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.2
    extern Inspection::GUID g_ASF_FilePropertiesObjectGUID;
    extern Inspection::GUID g_ASF_StreamPropertiesObjectGUID;
    extern Inspection::GUID g_ASF_HeaderExtensionObjectGUID;
    extern Inspection::GUID g_ASF_CodecListObjectGUID;
    extern Inspection::GUID g_ASF_ScriptCommandObjectGUID;
    extern Inspection::GUID g_ASF_MarkerObjectGUID;
    extern Inspection::GUID g_ASF_BitrateMutualExclusionObjectGUID;
    extern Inspection::GUID g_ASF_ErrorCorrectionObjectGUID;
    extern Inspection::GUID g_ASF_ContentDescriptionObjectGUID;
    extern Inspection::GUID g_ASF_ExtendedContentDescriptionObjectGUID;
    extern Inspection::GUID g_ASF_ContentBrandingObjectGUID;
    extern Inspection::GUID g_ASF_StreamBitratePropertiesObjectGUID;
    extern Inspection::GUID g_ASF_ContentEncryptionObjectGUID;
    extern Inspection::GUID g_ASF_ExtendedContentEncryptionObjectGUID;
    extern Inspection::GUID g_ASF_DigitalSignatureObjectGUID;
    extern Inspection::GUID g_ASF_PaddingObjectGUID;

    /// ASF Header Extension Object GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.3
    extern Inspection::GUID g_ASF_ExtendedStreamPropertiesObjectGUID;
    extern Inspection::GUID g_ASF_AdvancedMutualExclusionObjectGUID;
    extern Inspection::GUID g_ASF_GroupMutualExclusionObjectGUID;
    extern Inspection::GUID g_ASF_StreamPrioritizationObjectGUID;
    extern Inspection::GUID g_ASF_BandwidthSharingObjectGUID;
    extern Inspection::GUID g_ASF_LanguageListObjectGUID;
    extern Inspection::GUID g_ASF_MetadataObjectGUID;
    extern Inspection::GUID g_ASF_MetadataLibraryObjectGUID;
    extern Inspection::GUID g_ASF_IndexParametersObjectGUID;
    extern Inspection::GUID g_ASF_MediaObjectIndexParametersObjectGUID;
    extern Inspection::GUID g_ASF_TimecodeIndexParametersObjectGUID;
    extern Inspection::GUID g_ASF_CompatibilityObjectGUID;
    extern Inspection::GUID g_ASF_AdvancedContentEncryptionObjectGUID;

    /// ASF Stream Properties Object Stream Type GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.4
    extern Inspection::GUID g_ASF_AudioMediaGUID;
    extern Inspection::GUID g_ASF_VideoMediaGUID;
    extern Inspection::GUID g_ASF_CommandMediaGUID;
    extern Inspection::GUID g_ASF_JFIFMediaGUID;
    extern Inspection::GUID g_ASF_DegradableJPEGMediaGUID;
    extern Inspection::GUID g_ASF_FileTransferMediaGUID;
    extern Inspection::GUID g_ASF_BinaryMediaGUID;

    /// ASF Stream Properties Object Error Correction Type GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.5
    extern Inspection::GUID g_ASF_NoErrorCorrectionGUID;
    extern Inspection::GUID g_ASF_AudioSpreadGUID;

    /// ASF Header Extension Object GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.6
    extern Inspection::GUID g_ASF_Reserved1GUID;

    /// ASF Codec List Object GUIDs
    /// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.8
    extern Inspection::GUID g_ASF_Reserved2GUID;

    /// Other ASF GUIDs found on the web
    extern Inspection::GUID g_ASF_IndexPlaceholderObjectGUID;
    
    /// RIFF GUIDs
    extern Inspection::GUID g_KSDATAFORMAT_SUBTYPE_PCM;
    
    /// WM/MediaClassPrimaryID
    extern Inspection::GUID g_WM_MediaClassPrimaryID_AudioNoMusic;
    extern Inspection::GUID g_WM_MediaClassPrimaryID_AudioMusic;
    extern Inspection::GUID g_WM_MediaClassPrimaryID_Video;
    extern Inspection::GUID g_WM_MediaClassPrimaryID_NeitherAudioNorMusic;

    // validators
    bool Is_ASCII_Character_Alphabetic(std::uint8_t Character);
    bool Is_ASCII_Character_Alphabetic_LowerCase(std::uint8_t Character);
    bool Is_ASCII_Character_Alphabetic_UpperCase(std::uint8_t Character);
    bool Is_ASCII_Character_DecimalDigit(std::uint8_t Character);
    bool Is_ASCII_Character_HexadecimalDigit(std::uint8_t Character);
    bool Is_ASCII_Character_Printable(std::uint8_t Character);
    bool Is_ASCII_Character_Space(std::uint8_t Character);
    bool Is_ISO_IEC_8859_1_1998_Character(std::uint8_t Character);
    // getters
    std::string Get_CountryName_From_ISO_3166_1_Alpha_2_CountryCode(const std::string & ISO_3166_1_Alpha_2_CountryCode);
    Inspection::DateTime Get_DateTime_FromMicrosoftFileTime(std::uint64_t FileTime);
    Inspection::DateTime Get_DateTime_FromUnixTimeStamp(std::uint64_t UnixTimeStamp);
    Inspection::GUID Get_GUID_FromString_WithCurlyBraces(const std::string & String);
    std::string Get_GUID_Interpretation(const Inspection::GUID & GUID);
    std::string Get_ID3_1_Genre(std::uint8_t GenreNumber);
    std::string Get_ID3_1_Winamp_Genre(std::uint8_t GenreNumber);
    std::string Get_ID3_2_PictureType_Interpretation(std::uint8_t Value);
    std::string Get_ID3_2_3_FileType_Interpretation(const std::string & Value);
    std::string Get_LanguageName_From_ISO_639_2_1998_Code(const std::string & ISO_639_2_1998_Code);
    std::uint64_t Get_Unix_TimeStamp_FromWindowsFileTime(std::uint64_t FileTime);
    std::uint8_t Get_UnsignedInteger_8Bit_FromHexadecimalDigit(char HexadecimalDigit);
    std::uint8_t Get_UnsignedInteger_8Bit_FromHexadecimalString(const std::string & HexadecimalString);
    std::uint16_t Get_UnsignedInteger_16Bit_FromHexadecimalString(const std::string & HexadecimalString);
    std::uint32_t Get_UnsignedInteger_32Bit_FromHexadecimalString(const std::string & HexadecimalString);
    std::string Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::uint32_t CodePoint);
    
    // helpers for the engine
    std::string JoinWithSeparator(const std::vector<std::string> & Strings, const std::string & Separator);
}

#endif
