#ifndef INSPECTION_COMMON_GETTERS_H
#define INSPECTION_COMMON_GETTERS_H

#include "result.h"

namespace Inspection
{
	class Buffer;
	
	std::unique_ptr< Inspection::Result > Get_APE_Tags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_Flags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter_VersionNumber(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_Item(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_Alphabetical(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_AlphaNumeric(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_AlphaNumericOrSpace(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Alphabetical_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Alphabetical_EndedByTemplateLength(Inspection::Buffer & Buffer, const std::string & TemplateString);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumeric_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Inspection::Buffer & Buffer, const std::string & TemplateString);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByInvalidOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Bits_Set_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Bits_SetOrUnset_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Bits_Unset_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Boolean_1Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_GUID_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_639_2_1998_Code(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_Character(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_Character(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_Character(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_Character(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Buffer & Buffer, std::uint64_t NumberOfCodePoints);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Microsoft_WaveFormat_FormatTag(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_Frame(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Buffer & Buffer, std::uint8_t LayerDescription);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Copyright(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Emphasis(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Mode(Inspection::Buffer & Buffer, std::uint8_t LayerDescription);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Buffer & Buffer, std::uint8_t LayerDescription, std::uint8_t Mode);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_PaddingBit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ProtectionBit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_Stream(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_1Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_2Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_3Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_4Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_5Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_6Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_7Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_9Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_20Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Buffer & Buffer);
}

#endif
