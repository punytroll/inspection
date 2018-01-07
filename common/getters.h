#ifndef INSPECTION_COMMON_GETTERS_H
#define INSPECTION_COMMON_GETTERS_H

#include "result.h"

namespace Inspection
{
	class Buffer;
	class Reader;
	
	void UpdateState(bool & Continue, std::unique_ptr< Inspection::Result > & FieldResult);
	void UpdateState(bool & Continue, Inspection::Buffer & Buffer, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader);
	
	std::unique_ptr< Inspection::Result > Get_APE_Tags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_Flags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter_VersionNumber(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_APE_Tags_Item(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Buffer & Buffer, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Getter, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByNumberOfElements(Inspection::Buffer & Buffer, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Getter, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_Array_EndedByNumberOfElements_PassArrayIndex(Inspection::Buffer & Buffer, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, std::uint64_t) > Getter, std::uint64_t NumberOfElements);
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
	std::unique_ptr< Inspection::Result > Get_ASF_Boolean_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_Boolean_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_CodecEntry(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_CodecEntryType(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_CodecListObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_CompatibilityObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_ContentDescriptionObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_DataObject(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_DataType(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType, const std::string & Name);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescriptionObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASF_File(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_FilePropertiesFlags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_FilePropertiesObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_GUID(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_HeaderExtensionObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_HeaderObject(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_HeaderObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_IndexPlaceholderObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASF_LanguageIDRecord(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_LanguageListObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Buffer & Buffer, const std::string & Type, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord_DataType(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibraryObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType);
	std::unique_ptr< Inspection::Result > Get_ASF_MetadataObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_Object(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_ObjectHeader(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamBitratePropertiesObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_Flags(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObject(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObjectData(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Bits_Set_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Bits_SetOrUnset_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Bits_Unset_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Bits_Unset_UntilByteAlignment(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Boolean_1Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_FLAC_ApplicationBlock_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame(Inspection::Buffer & Buffer, std::uint8_t NumberOfChannels);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame_Footer(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_Frame_Header(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Header(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlock_Type(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_Data(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_Data(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints);
	std::unique_ptr< Inspection::Result > Get_FLAC_SeekTableBlock_SeekPoint(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer, bool OnlyStreamHeader);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_Data(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock_NumberOfChannels(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_Constant(Inspection::Buffer & Buffer, std::uint8_t BitsPerSample);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_Fixed(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t Order);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Data_LPC(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t Order);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Header(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_CodingMethod(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Buffer & Buffer, std::uint64_t PartitionIndex, std::uint32_t NumberOfSamples, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice2(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Residual_Rice2_Partition(Inspection::Buffer & Buffer, std::uint32_t NumberOfSamples);
	std::unique_ptr< Inspection::Result > Get_FLAC_Subframe_Type(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_FLAC_VorbisCommentBlock_Data(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_GUID_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_1_Tag(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_COM(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_PIC_PictureType(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_T__(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Body_UFI(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Header(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_2_Frame_Header_Identifier(Inspection::Buffer & Buffer);
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
	std::unique_ptr< Inspection::Result > Get_ID3_2_3_Frame_Header_Flags(Inspection::Buffer & Buffer);
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
	std::unique_ptr< Inspection::Result > Get_ID3_2_4_Frame_Header_Identifier(Inspection::Buffer & Buffer);
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
	std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_NameCode(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_ReplayGainAdjustment_SignBit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_ID3_2_Tag(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_Tag_Header(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_ID3_GUID(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Header(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Buffer & Buffer, std::uint8_t FirstTrackNumber, std::uint8_t LastTrackNumber);
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
	std::unique_ptr< Inspection::Result > Get_SignedInteger_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_1Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_5Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_12Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_RiceEncoded(Inspection::Buffer & Buffer, std::uint8_t RiceParameter);
	std::unique_ptr< Inspection::Result > Get_SignedIntegers_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_0Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_1Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_2Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_3Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_4Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_5Bit(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_6Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_7Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_9Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_10Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_11Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_12Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_13Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_14Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_15Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_17Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_20Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_31Bit_UTF_8_Coded(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_AlternativeUnary(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_BigEndian(Inspection::Reader & Reader);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_UTF_8_Coded(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedIntegers_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_UnsignedIntegers_16Bit_BigEndian(Inspection::Buffer & Buffer, std::uint64_t NumberOfElements);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Buffer & Buffer);
}

#endif
