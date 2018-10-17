#ifndef INSPECTION_COMMON_GETTERS_H
#define INSPECTION_COMMON_GETTERS_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "result.h"

extern bool g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples;

namespace Inspection
{
	class Buffer;
	class Reader;
	
	void UpdateState(bool & Continue, std::unique_ptr< Inspection::Result > & FieldResult);
	void UpdateState(bool & Continue, Inspection::Buffer & Buffer, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader);
	void UpdateState(bool & Continue, Inspection::Reader & Reader, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader);
	
	std::unique_ptr< Inspection::Result > Get_APE_Tags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_Item(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByLength(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByNumberOfElements(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByNumberOfElements_PassArrayIndex(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &, std::uint64_t) > Getter, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByPredicate(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter, std::function< bool (std::shared_ptr< Inspection::Value > PartValue) > EndedPredicate);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_Alphabetic(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_AlphaNumeric(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_Character_AlphaNumericOrSpace(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Alphabetic_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Alphabetic_EndedByTemplateLength(Inspection::Reader & Reader, const std::string & TemplateString);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumeric_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Inspection::Reader & Reader, const std::string & TemplateString);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByInvalidOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_CodecEntry(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_CodecEntryType(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_CodecListObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_CompatibilityObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_ContentDescriptionObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_CreationDate(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_DataObject(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Reader & Reader, const std::string & DataType, const std::string & Name);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescriptionObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_File(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_FileProperties_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_GUID(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_HeaderExtensionObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_HeaderObject(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_HeaderObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_IndexPlaceholderObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_LanguageIDRecord(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_LanguageListObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Reader & Reader, const std::string & DataType);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibraryObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Reader & Reader, const std::string & DataType);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_Object(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitratePropertiesObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObject(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObjectData(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Bits_Set_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Bits_SetOrUnset_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Bits_Unset_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Bits_Unset_UntilByteAlignment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_BitSet_8Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_BitSet_32Bit_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Boolean_1Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_ApplicationBlock_Data(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame(Inspection::Reader & Reader, std::uint8_t NumberOfChannels);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame_Footer(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame_Header_SampleRate(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Type(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_Data(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_Data(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Stream_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_NumberOfChannels(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_CalculateBitsPerSample(Inspection::Reader & Reader, std::uint64_t SubFrameIndex, std::uint16_t BlockSize, std::uint8_t BitsPerSample, std::uint8_t ChannelAssignment);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_Constant(Inspection::Reader & Reader, std::uint8_t BitsPerSample);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_Fixed(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t Order);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_LPC(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t Order);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_CodingMethod(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Reader & Reader, std::uint64_t PartitionIndex, std::uint32_t NumberOfSamples, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice2(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice2_Partition(Inspection::Reader & Reader, std::uint32_t NumberOfSamples);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Type(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlock_Data(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_GUID_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_1_Tag(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_1_Genre(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_COM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_T__(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_TCO(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_UFI(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Language(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Tag_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Inspection::Reader & Reader, std::uint8_t TextEncoding);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, std::uint8_t TextEncoding);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_APIC(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_COMM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_MCDI(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PCNT(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_POPM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_PRIV(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_T___(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCMP(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TCON(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TFLT(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TLAN(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TSRC(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_TXXX(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_USLT(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_W___(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Body_WXXX(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Language(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Tag_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Inspection::Reader & Reader, std::uint8_t TextEncoding);
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, std::uint8_t TextEncoding);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_APIC_MIMEType(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_COMM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_MCDI(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_POPM(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_T___(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_TCMP(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_TXXX(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_USLT(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_W___(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Body_WXXX(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Language(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_Header_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Inspection::Reader & Reader, std::uint8_t TextEncoding);
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, std::uint8_t TextEncoding);
	std::unique_ptr< Inspection::Result > Get_ID3_2_Tag(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_Tag_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_ReplayGainAdjustment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_GUID(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Header(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Reader & Reader, std::uint8_t FirstTrackNumber, std::uint8_t LastTrackNumber);
	std::unique_ptr< Inspection::Result > Get_ISO_639_2_1998_Code(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_Character(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Inspection::Reader & Reader);
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
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Reader & Reader, std::uint64_t NumberOfCodePoints);
	std::unique_ptr< Inspection::Result > Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Microsoft_WaveFormat_FormatTag(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_Frame(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Reader & Reader, std::uint8_t LayerDescription);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Copyright(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Emphasis(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_Mode(Inspection::Reader & Reader, std::uint8_t LayerDescription);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Reader & Reader, std::uint8_t LayerDescription, std::uint8_t Mode);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_PaddingBit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_ProtectionBit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_MPEG_1_Stream(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_1Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_5Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_12Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_RiceEncoded(Inspection::Reader & Reader, std::uint8_t RiceParameter);
	std::unique_ptr< Inspection::Result > Get_SignedIntegers_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_0Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_1Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_2Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_3Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_4Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_5Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_6Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_7Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_9Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_10Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_11Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_12Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_13Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_14Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_15Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_17Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_20Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_31Bit_UTF_8_Coded(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_AlternativeUnary(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_UTF_8_Coded(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedIntegers_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_UnsignedIntegers_16Bit_BigEndian(Inspection::Reader & Reader, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Reader & Reader);
}

#endif
