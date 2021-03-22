#ifndef INSPECTION_COMMON_GETTERS_H
#define INSPECTION_COMMON_GETTERS_H

#include <any>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "result.h"

extern bool g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples;

namespace Inspection
{
	class Buffer;
	class Reader;
	
	void UpdateState(bool & Continue, std::unique_ptr< Inspection::Result > & FieldResult);
	void UpdateState(bool & Continue, Inspection::Buffer & Buffer, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader);
	void UpdateState(bool & Continue, Inspection::Reader & Reader, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader);
	
	std::unique_ptr< Inspection::Result > Get_APE_Flags(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_APE_Item(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Apple_AppleDouble_File(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByNumberOfElements(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByPredicate(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_Alphabetic(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_AlphaNumeric(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_AlphaNumericOrSpace(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Alphabetic_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumeric_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Inspection::Reader & Reader, const std::string & TemplateString);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByInvalidOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_CreationDate(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_FileProperties_Flags(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_GUID(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_IndexPlaceholderObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_Metadata_DescriptionRecord_Data(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_Object(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObjectData(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_BitSet_8Bit_LeastSignificantBitFirst(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_LittleEndian_LeastSignificantBitFirstPerByte(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_BitSet_32Bit_LittleEndian_LeastSignificantBitFirstPerByte(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Boolean_1Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Data_Set_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Data_SetOrUnset_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Data_SetOrUnset_Until16BitAlignment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Data_Unset_Until16BitAlignment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Data_Unset_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Data_Unset_Until8BitAlignment(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_ApplicationBlock_Data(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame_Header(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_Stream_Header(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_NumberOfChannels(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_CalculateBitsPerSample(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_LPC(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t Order);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice2(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice2_Partition(Inspection::Reader & Reader, std::uint32_t NumberOfSamples);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Type(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_GUID_LittleEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_1_Genre(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_TCO(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_UFI(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Tag_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_MCDI(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PCNT(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_POPM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PRIV(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCMP(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCON(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TFLT(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TLAN(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TSRC(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header_Flags(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Tag_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_POPM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_TCMP(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_Tag(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_2_Tag_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_ReplayGainAdjustment(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ID3_GUID(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_639_2_1998_Code(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_Character(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByNumberOfCodePoints(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_Frame(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Mode(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Packet(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Page(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Page_HeaderType(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Stream(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Vorbis_AudioPacket(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Vorbis_CommentHeader(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Vorbis_HeaderPacket(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_Ogg_Vorbis_IdentificationHeader(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_RIFF_Chunk(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_RIFF_ChunkData_fmt_(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_1Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_5Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_8Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_12Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_RiceEncoded(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_SignedIntegers_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_String_ASCII_ByTemplate(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_0Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_1Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_2Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_3Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_4Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_5Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_6Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_7Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_9Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_10Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_11Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_12Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_13Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_14Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_15Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_17Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_20Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_31Bit_UTF_8_Coded(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_AlternativeUnary(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_UTF_8_Coded(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
}

#endif
