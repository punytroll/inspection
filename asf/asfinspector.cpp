#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/common.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
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
std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObject(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObjectData(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_ASF_Boolean_16Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(UnsignedInteger16BitResult->GetValue());
	if(UnsignedInteger16BitResult->GetSuccess() == true)
	{
		auto UnsignedInteger16Bit{std::experimental::any_cast< std::uint16_t >(UnsignedInteger16BitResult->GetAny())};
		
		if(UnsignedInteger16Bit == 0x0000)
		{
			Result->GetValue()->PrependTag("value", false);
			Result->SetSuccess(true);
		}
		else if(UnsignedInteger16Bit == 0x0001)
		{
			Result->GetValue()->PrependTag("value", true);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("value", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_Boolean_32Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(UnsignedInteger32BitResult->GetValue());
	if(UnsignedInteger32BitResult->GetSuccess() == true)
	{
		auto UnsignedInteger32Bit{std::experimental::any_cast< std::uint32_t >(UnsignedInteger32BitResult->GetAny())};
		
		if(UnsignedInteger32Bit == 0x00000000)
		{
			Result->GetValue()->PrependTag("value", false);
			Result->SetSuccess(true);
		}
		else if(UnsignedInteger32Bit == 0x00000001)
		{
			Result->GetValue()->PrependTag("value", true);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("value", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_CodecEntry(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TypeResult{Get_ASF_CodecEntryType(Buffer)};
	
	TypeResult->GetValue()->Append("Type", TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto CodecNameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("CodecNameLength", CodecNameLengthResult->GetValue());
		if(CodecNameLengthResult->GetSuccess() == true)
		{
			auto CodecNameLength{std::experimental::any_cast< std::uint16_t >(CodecNameLengthResult->GetAny())};
			auto CodecNameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecNameLength)};
			
			Result->GetValue()->Append("CodecName", CodecNameResult->GetValue());
			if(CodecNameResult->GetSuccess() == true)
			{
				auto CodecDescriptionLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("CodecDescriptionLength", CodecDescriptionLengthResult->GetValue());
				if(CodecDescriptionLengthResult->GetSuccess() == true)
				{
					auto CodecDescriptionLength{std::experimental::any_cast< std::uint16_t >(CodecDescriptionLengthResult->GetAny())};
					auto CodecDescriptionResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecDescriptionLength)};
					
					Result->GetValue()->Append("CodecDescription", CodecDescriptionResult->GetValue());
					if(CodecDescriptionResult->GetSuccess() == true)
					{
						auto CodecInformationLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->Append("CodecInformation Length", CodecInformationLengthResult->GetValue());
						if(CodecInformationLengthResult->GetSuccess() == true)
						{
							auto CodecInformationLength{std::experimental::any_cast< std::uint16_t >(CodecInformationLengthResult->GetAny())};
							auto CodecInformationResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, CodecInformationLength)};
							
							Result->GetValue()->Append("CodecInformation", CodecInformationResult->GetValue());
							if(CodecInformationResult->GetSuccess() == true)
							{
								Result->SetSuccess(true);
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_CodecEntryType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto Type{std::experimental::any_cast< std::uint16_t >(TypeResult->GetAny())};
		
		if(Type == 0x0001)
		{
			Result->GetValue()->PrependTag("interpretation", "Video Codec"s);
		}
		else if(Type == 0x0002)
		{
			Result->GetValue()->PrependTag("interpretation", "Audio Codec"s);
		}
		else if(Type == 0xffff)
		{
			Result->GetValue()->PrependTag("interpretation", "Unknown Codec"s);
		}
		else
		{
			Result->GetValue()->PrependTag("interpretation", "<no interpretation>"s);
		}
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_CodecListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReservedGUIDResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->Append("Reserved", ReservedGUIDResult->GetValue());
	if((ReservedGUIDResult->GetSuccess() == true) && (std::experimental::any_cast< Inspection::GUID >(ReservedGUIDResult->GetAny()) == Inspection::g_ASF_Reserved2GUID))
	{
		auto CodecEntriesCountResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("CodecEntriesCount", CodecEntriesCountResult->GetValue());
		if(CodecEntriesCountResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
			
			auto CodecEntries(std::make_shared< Inspection::Value >());
			
			Result->GetValue()->Append("CodecEntries", CodecEntries);
			
			auto CodecEntriesCount{std::experimental::any_cast< std::uint32_t >(CodecEntriesCountResult->GetAny())};
			
			for(auto CodecEntryIndex = 0ul; CodecEntryIndex < CodecEntriesCount; ++CodecEntryIndex)
			{
				auto CodecEntryResult{Get_ASF_CodecEntry(Buffer)};
				
				CodecEntries->AppendValue(CodecEntryResult->GetValue());
				if(CodecEntryResult->GetSuccess() == false)
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_CompatibilityObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ProfileResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->GetValue()->Append("Profile", ProfileResult->GetValue());
	if(ProfileResult->GetSuccess() == true)
	{
		auto Profile{std::experimental::any_cast< std::uint8_t >(ProfileResult->GetAny())};
		
		if(Profile == 0x02)
		{
			auto ModeResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->Append("Mode", ModeResult->GetValue());
			if(ModeResult->GetSuccess() == true)
			{
				auto Mode{std::experimental::any_cast< std::uint8_t >(ModeResult->GetAny())};
				
				if(Mode == 0x01)
				{
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ContentDescriptionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TitleLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("TitleLength", TitleLengthResult->GetValue());
	if(TitleLengthResult->GetSuccess() == true)
	{
		auto AuthorLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("AuthorLength", AuthorLengthResult->GetValue());
		if(AuthorLengthResult->GetSuccess() == true)
		{
			auto CopyrightLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("CopyrightLength", CopyrightLengthResult->GetValue());
			if(CopyrightLengthResult->GetSuccess() == true)
			{
				auto DescriptionLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("DescriptionLength", DescriptionLengthResult->GetValue());
				if(DescriptionLengthResult->GetSuccess() == true)
				{
					auto RatingLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("RatingLength", RatingLengthResult->GetValue());
					if(RatingLengthResult->GetSuccess() == true)
					{
						auto TitleLength{std::experimental::any_cast< std::uint16_t >(TitleLengthResult->GetAny())};
						auto TitleResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, TitleLength)};
						
						Result->GetValue()->Append("Title", TitleResult->GetValue());
						if(TitleResult->GetSuccess() == true)
						{
							auto AuthorLength{std::experimental::any_cast< std::uint16_t >(AuthorLengthResult->GetAny())};
							auto AuthorResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, AuthorLength)};
							
							Result->GetValue()->Append("Author", AuthorResult->GetValue());
							if(AuthorResult->GetSuccess() == true)
							{
								auto CopyrightLength{std::experimental::any_cast< std::uint16_t >(CopyrightLengthResult->GetAny())};
								auto CopyrightResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, CopyrightLength)};
								
								Result->GetValue()->Append("Copyright", CopyrightResult->GetValue());
								if(CopyrightResult->GetSuccess() == true)
								{
									auto DescriptionLength{std::experimental::any_cast< std::uint16_t >(DescriptionLengthResult->GetAny())};
									auto DescriptionResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, DescriptionLength)};
									
									Result->GetValue()->Append("Description", DescriptionResult->GetValue());
									if(DescriptionResult->GetSuccess() == true)
									{
										auto RatingLength{std::experimental::any_cast< std::uint16_t >(RatingLengthResult->GetAny())};
										auto RatingResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, RatingLength)};
										
										Result->GetValue()->Append("Rating", RatingResult->GetValue());
										Result->SetSuccess(RatingResult->GetSuccess());
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_DataObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		auto Size{std::experimental::any_cast< std::uint64_t >(ObjectHeaderResult->GetAny("Size"))};
		
		if(GUID == Inspection::g_ASF_DataObjectGUID)
		{
			auto DataObjectDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Inspection::Length(Size, 0) - ObjectHeaderResult->GetLength())};
			
			Result->GetValue()->Append("Data", DataObjectDataResult->GetValue());
			Result->SetSuccess(DataObjectDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_DataType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataTypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(DataTypeResult->GetValue());
	if(DataTypeResult->GetSuccess() == true)
	{
		auto DataType{std::experimental::any_cast< std::uint16_t >(DataTypeResult->GetAny())};
		
		if(DataType == 0x0000)
		{
			Result->GetValue()->PrependTag("interpretation", "Unicode string"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0001)
		{
			Result->GetValue()->PrependTag("interpretation", "Byte array"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0002)
		{
			Result->GetValue()->PrependTag("interpretation", "Boolean"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0003)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 32bit"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0004)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 64bit"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0005)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 16bit"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("interpretation", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("NameLength", NameLengthResult->GetValue());
	if(NameLengthResult->GetSuccess() == true)
	{
		auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
		auto NameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, NameLength)};
		
		Result->GetValue()->Append("Name", NameResult->GetValue());
		if(NameResult->GetSuccess() == true)
		{
			auto ValueDataTypeResult{Get_ASF_DataType(Buffer)};
			
			Result->GetValue()->Append("ValueDataType", ValueDataTypeResult->GetValue());
			if(ValueDataTypeResult->GetSuccess() == true)
			{
				auto ValueLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("ValueLength", ValueLengthResult->GetValue());
				if(ValueLengthResult->GetSuccess() == true)
				{
					auto ValueLength{std::experimental::any_cast< std::uint16_t >(ValueLengthResult->GetAny())};
					auto ValueDataType{std::experimental::any_cast< const std::string & >(ValueDataTypeResult->GetValue()->GetTagAny("interpretation"))};
					auto Name{std::experimental::any_cast< const std::string & >(NameResult->GetAny())};
					auto DataValueResult{Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Buffer, ValueLength, ValueDataType, Name)};
					
					Result->GetValue()->Append("Value", DataValueResult->GetValue());
					Result->SetSuccess(DataValueResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType, const std::string & Name)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(DataType == "Unicode string")
	{
		auto UnicodeStringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
		
		Result->SetValue(UnicodeStringResult->GetValue());
		Result->SetSuccess(UnicodeStringResult->GetSuccess());
		if(Name == "WM/MediaPrimaryClassID")
		{
			auto String{std::experimental::any_cast< const std::string & >(Result->GetAny())};
			auto GUID{Inspection::Get_GUID_FromString_WithCurlyBraces(String)};
			
			Result->GetValue()->Append("GUID", GUID);
			Result->GetValue("GUID")->AppendTag("guid"s);
			Result->GetValue("GUID")->AppendTag("string"s);
			
			auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
			
			Result->GetValue("GUID")->AppendTag("interpretation", GUIDInterpretation);
		}
	}
	else if(DataType == "Byte array")
	{
		auto ByteArrayResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
		
		Result->SetValue(ByteArrayResult->GetValue());
		Result->SetSuccess(ByteArrayResult->GetSuccess());
	}
	else if(DataType == "Boolean")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto BooleanResult{Get_ASF_Boolean_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(BooleanResult->GetValue());
		Result->SetSuccess(BooleanResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger32BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger32BitResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		
		auto UnsignedInteger64BitResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger64BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger64BitResult->GetSuccess());
		if(Name == "WM/EncodingTime")
		{
			auto UnsignedInteger64Bit{std::experimental::any_cast< std::uint64_t >(Result->GetAny())};
			auto DateTime{Inspection::Get_DateTime_FromMicrosoftFileTime(UnsignedInteger64Bit)};
			
			Result->GetValue()->Append("DateTime", DateTime);
			Result->GetValue("DateTime")->AppendTag("date and time"s);
			Result->GetValue("DateTime")->AppendTag("from Microsoft filetime"s);
		}
	}
	else if(DataType == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger16BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger16BitResult->GetSuccess());
	}
	else
	{
		Result->GetValue()->SetAny("<unknown type>"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ExtendedContentDescriptionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ContentDescriptorsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("ContentDescriptorsCount", ContentDescriptorsCountResult->GetValue());
	if(ContentDescriptorsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto ContentDescriptorsCount{std::experimental::any_cast< std::uint16_t >(ContentDescriptorsCountResult->GetAny())};
		
		for(auto ContentDescriptorsIndex = 0; ContentDescriptorsIndex < ContentDescriptorsCount; ++ContentDescriptorsIndex)
		{
			auto ContentDescriptorResult{Get_ASF_ExtendedContentDescription_ContentDescriptor(Buffer)};
			
			Result->GetValue()->Append("ContentDescriptor", ContentDescriptorResult->GetValue());
			if(ContentDescriptorResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(FlagsResult->GetAny())};
		
		Result->GetValue()->Append("[0] Reliable", Flags[0]);
		Result->GetValue()->Append("[1] Seekable", Flags[1]);
		Result->GetValue()->Append("[2] No Cleanpoints", Flags[2]);
		Result->GetValue()->Append("[3] Resend Live Cleanpoints", Flags[3]);
		Result->GetValue()->Append("[4-31] Reserved", false);
		Result->SetSuccess(true);
		for(auto Index = 4; Index < 32; ++Index)
		{
			Result->SetSuccess(Result->GetSuccess() & ~Flags[Index]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto StartTimeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("StartTime", StartTimeResult->GetValue());
	Result->GetValue("StartTime")->PrependTag("milliseconds"s);
	if(StartTimeResult->GetSuccess() == true)
	{
		auto EndTimeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("EndTime", EndTimeResult->GetValue());
		Result->GetValue("EndTime")->PrependTag("milliseconds"s);
		if(EndTimeResult->GetSuccess() == true)
		{
			auto DataBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("DataBitrate", DataBitrateResult->GetValue());
			Result->GetValue("DataBitrate")->PrependTag("bits per second"s);
			if(DataBitrateResult->GetSuccess() == true)
			{
				auto BufferSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("BufferSize", BufferSizeResult->GetValue());
				Result->GetValue("BufferSize")->PrependTag("milliseconds"s);
				if(BufferSizeResult->GetSuccess() == true)
				{
					auto InitialBufferFullnessResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("InitialBufferFullness", InitialBufferFullnessResult->GetValue());
					Result->GetValue("InitialBufferFullness")->PrependTag("milliseconds"s);
					if(InitialBufferFullnessResult->GetSuccess() == true)
					{
						auto AlternateDataBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->Append("AlternateDataBitrate", AlternateDataBitrateResult->GetValue());
						Result->GetValue("AlternateDataBitrate")->PrependTag("bits per second"s);
						if(AlternateDataBitrateResult->GetSuccess() == true)
						{
							auto AlternateBufferSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->Append("AlternateBufferSize", AlternateBufferSizeResult->GetValue());
							Result->GetValue("AlternateBufferSize")->PrependTag("milliseconds"s);
							if(AlternateBufferSizeResult->GetSuccess() == true)
							{
								auto AlternateInitialBufferFullnessResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
								
								Result->GetValue()->Append("AlternateInitialBufferFullness", AlternateInitialBufferFullnessResult->GetValue());
								Result->GetValue("AlternateInitialBufferFullness")->PrependTag("milliseconds"s);
								if(AlternateInitialBufferFullnessResult->GetSuccess() == true)
								{
									auto MaximumObjectSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
									
									Result->GetValue()->Append("MaximumObjectSize", MaximumObjectSizeResult->GetValue());
									if(MaximumObjectSizeResult->GetSuccess() == true)
									{
										auto FlagsResult{Get_ASF_ExtendedStreamPropertiesObject_Flags(Buffer)};
										
										Result->GetValue()->Append("Flags", FlagsResult->GetValue());
										if(FlagsResult->GetSuccess() == true)
										{
											auto StreamNumberResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
											
											Result->GetValue()->Append("StreamNumber", StreamNumberResult->GetValue());
											if(StreamNumberResult->GetSuccess() == true)
											{
												auto StreamLanguageIndexResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
												
												Result->GetValue()->Append("StreamLanguageIndex", StreamLanguageIndexResult->GetValue());
												if(StreamLanguageIndexResult->GetSuccess() == true)
												{
													auto AverageTimePerFrameResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
													
													Result->GetValue()->Append("AverageTimePerFrame", AverageTimePerFrameResult->GetValue());
													if(AverageTimePerFrameResult->GetSuccess() == true)
													{
														auto StreamNameCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
														
														Result->GetValue()->Append("StreamNameCount", StreamNameCountResult->GetValue());
														if(StreamNameCountResult->GetSuccess() == true)
														{
															auto PayloadExtensionSystemCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
															
															Result->GetValue()->Append("PayloadExtensionSystemCount", PayloadExtensionSystemCountResult->GetValue());
															if(PayloadExtensionSystemCountResult->GetSuccess() == true)
															{
																auto StreamNameCount{std::experimental::any_cast< std::uint16_t >(StreamNameCountResult->GetAny())};
																auto PayloadExtensionSystemCount{std::experimental::any_cast< std::uint16_t >(PayloadExtensionSystemCountResult->GetAny())};
																
																if((StreamNameCount == 0) && (PayloadExtensionSystemCount == 0))
																{
																	if(Buffer.GetPosition() < Boundary)
																	{
																		auto StreamPropertiesObjectResult{Get_ASF_StreamPropertiesObject(Buffer)};
																		
																		Result->GetValue()->Append("StreamPropertiesObject", StreamPropertiesObjectResult->GetValue());
																		Result->SetSuccess(StreamPropertiesObjectResult->GetSuccess());
																	}
																	else
																	{
																		Result->SetSuccess(true);
																	}
																}
																else
																{
																	throw std::runtime_error("Not implemented.");
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_GUID(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->SetValue(GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		auto GUIDInterpretation{Get_GUID_Interpretation(std::experimental::any_cast< Inspection::GUID >(GUIDResult->GetAny()))};
		
		Result->GetValue()->PrependTag("interpretation", GUIDInterpretation);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_HeaderExtensionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReservedField1Result{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->Append("ReservedField1", ReservedField1Result->GetValue());
	if(ReservedField1Result->GetSuccess() == true)
	{
		auto Reserved1Field{std::experimental::any_cast< Inspection::GUID >(ReservedField1Result->GetAny())};
		
		if(Reserved1Field == Inspection::g_ASF_Reserved1GUID)
		{
			auto ReservedField2Result{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("ReservedField2", ReservedField2Result->GetValue());
			if(ReservedField2Result->GetSuccess() == true)
			{
				auto ReservedField2{std::experimental::any_cast< std::uint16_t >(ReservedField2Result->GetAny())};
				
				if(ReservedField2 == 0x0006)
				{
					auto HeaderExtensionDataSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("HeaderExtensionDataSize", HeaderExtensionDataSizeResult->GetValue());
					if(HeaderExtensionDataSizeResult->GetSuccess() == true)
					{
						auto HeaderExtensionDataSize{std::experimental::any_cast< std::uint32_t >(HeaderExtensionDataSizeResult->GetAny())};
						Inspection::Length Boundary{Buffer.GetPosition() + HeaderExtensionDataSize};
						
						Result->SetSuccess(true);
						while(Buffer.GetPosition() < Boundary)
						{
							auto AdditionalExtendedHeaderObjectResult{Get_ASF_Object(Buffer)};
							
							Result->GetValue()->Append("AdditionalExtendedHeader", AdditionalExtendedHeaderObjectResult->GetValue());
							if(AdditionalExtendedHeaderObjectResult->GetSuccess() == false)
							{
								Result->SetSuccess(false);
								
								break;
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_File(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderObjectResult{Get_ASF_HeaderObject(Buffer)};
	
	Result->GetValue()->Append("HeaderObject", HeaderObjectResult->GetValue());
	if(HeaderObjectResult->GetSuccess() == true)
	{
		auto DataObjectResult{Get_ASF_DataObject(Buffer)};
		
		Result->GetValue()->Append("DataObject", DataObjectResult->GetValue());
		if(DataObjectResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
			while(Buffer.GetPosition() < Buffer.GetLength())
			{
				auto ObjectResult{Get_ASF_Object(Buffer)};
				
				Result->GetValue()->Append("Object", ObjectResult->GetValue());
				if(ObjectResult->GetSuccess() == false)
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_FilePropertiesFlags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(FlagsResult->GetAny())};
		
		Result->GetValue()->Append("[0] Broadcast", Flags[0]);
		Result->GetValue()->Append("[1] Seekable", Flags[1]);
		Result->GetValue()->Append("[2-31] Reserved", false);
		Result->SetSuccess(true);
		for(auto Index = 2; Index < 32; ++Index)
		{
			Result->SetSuccess(Result->GetSuccess() & ~Flags[Index]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_FilePropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FileIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("FileID", FileIDResult->GetValue());
	if(FileIDResult->GetSuccess() == true)
	{
		auto FileSizeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("FileSize", FileSizeResult->GetValue());
		if(FileSizeResult->GetSuccess() == true)
		{
			auto CreationDateResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("CreationDate", CreationDateResult->GetValue());
			if(CreationDateResult->GetSuccess() == true)
			{
				auto CreationDate{std::experimental::any_cast< std::uint64_t >(CreationDateResult->GetAny())};
				
				Result->GetValue("CreationDate")->Append("DateTime", Inspection::Get_DateTime_FromMicrosoftFileTime(CreationDate));
				Result->GetValue("CreationDate")->GetValue("DateTime")->AppendTag("date and time"s);
				Result->GetValue("CreationDate")->GetValue("DateTime")->AppendTag("from Microsoft filetime"s);
				
				auto DataPacketsCountResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("DataPacketsCount", DataPacketsCountResult->GetValue());
				if(DataPacketsCountResult->GetSuccess() == true)
				{
					auto PlayDurationResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("PlayDuration", PlayDurationResult->GetValue());
					if(PlayDurationResult->GetSuccess() == true)
					{
						auto SendDurationResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->Append("SendDuration", SendDurationResult->GetValue());
						if(SendDurationResult->GetSuccess() == true)
						{
							auto PrerollResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->Append("Preroll", PrerollResult->GetValue());
							if(PrerollResult->GetSuccess() == true)
							{
								auto FlagsResult{Get_ASF_FilePropertiesFlags(Buffer)};
								
								Result->GetValue()->Append("Flags", FlagsResult->GetValue());
								if(FlagsResult->GetSuccess() == true)
								{
									auto MinimumDataPacketSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
									
									Result->GetValue()->Append("MinimumDataPacketSize", MinimumDataPacketSizeResult->GetValue());
									if(MinimumDataPacketSizeResult->GetSuccess() == true)
									{
										auto MaximumDataPacketSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
										
										Result->GetValue()->Append("MaximumDataPacketSize", MaximumDataPacketSizeResult->GetValue());
										if(MaximumDataPacketSizeResult->GetSuccess() == true)
										{
											auto MaximumBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
											
											Result->GetValue()->Append("MaximumBitrate", MaximumBitrateResult->GetValue());
											if(MaximumBitrateResult->GetSuccess() == true)
											{
												Result->SetSuccess(true);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_HeaderObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		
		if(GUID == Inspection::g_ASF_HeaderObjectGUID)
		{
			auto HeaderObjectDataResult{Get_ASF_HeaderObjectData(Buffer)};
			
			Result->GetValue()->AppendValues(HeaderObjectDataResult->GetValue()->GetValues());
			if(HeaderObjectDataResult->GetSuccess() ==true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_HeaderObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NumberOfHeaderObjects{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("Number of header objects", NumberOfHeaderObjects->GetValue());
	if(NumberOfHeaderObjects->GetSuccess() == true)
	{
		auto Reserved1{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->Append("Reserved1", Reserved1->GetValue());
		if((Reserved1->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(Reserved1->GetAny()) == 0x01))
		{
			auto Reserved2{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->Append("Reserved2", Reserved2->GetValue());
			if((Reserved2->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(Reserved2->GetAny()) == 0x02))
			{
				Result->SetSuccess(true);
				
				auto HeaderObjects(std::make_shared< Inspection::Value >());
				
				Result->GetValue()->Append("HeaderObjects", HeaderObjects);
				
				auto NumberOfHeaderObjectsValue{std::experimental::any_cast< std::uint32_t >(NumberOfHeaderObjects->GetAny())};
				
				for(auto ObjectIndex = 0ul; ObjectIndex < NumberOfHeaderObjectsValue; ++ObjectIndex)
				{
					auto HeaderObject{Get_ASF_Object(Buffer)};
					
					HeaderObjects->Append("Object", HeaderObject->GetValue());
					if(HeaderObject->GetSuccess() == false)
					{
						Result->SetSuccess(false);
						
						break;
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_IndexPlaceholderObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Length == Inspection::Length(10ull, 0))
	{
		auto DataResult{Get_Bits_Unset_EndedByLength(Buffer, Length)};
		
		if(DataResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("Data", DataResult->GetValue());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_LanguageIDRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LanguageIDLengthResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->GetValue()->Append("LanguageIDLength", LanguageIDLengthResult->GetValue());
	if(LanguageIDLengthResult->GetSuccess() == true)
	{
		auto LanguageIDLength{std::experimental::any_cast< std::uint8_t >(LanguageIDLengthResult->GetAny())};
		auto LanguageIDResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, LanguageIDLength)};
		
		Result->GetValue()->Append("LanguageID", LanguageIDResult->GetValue());
		Result->SetSuccess(LanguageIDResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_LanguageListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LanguageIDRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("LanguageIDRecordsCount", LanguageIDRecordsCountResult->GetValue());
	if(LanguageIDRecordsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto LanguageIDRecordsCount{std::experimental::any_cast< std::uint16_t >(LanguageIDRecordsCountResult->GetAny())};
		
		for(auto LanguageIDRecordIndex = 0; LanguageIDRecordIndex < LanguageIDRecordsCount; ++LanguageIDRecordIndex)
		{
			auto LanguageIDRecordResult{Get_ASF_LanguageIDRecord(Buffer)};
			
			Result->GetValue()->Append("LanguageIDRecord", LanguageIDRecordResult->GetValue());
			if(LanguageIDRecordResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LanguageListIndexResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("LanguageListIndex", LanguageListIndexResult->GetValue());
	if(LanguageListIndexResult->GetSuccess() == true)
	{
		auto StreamNumberResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto NameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("NameLength", NameLengthResult->GetValue());
			if(NameLengthResult->GetSuccess() == true)
			{
				auto DataTypeResult{Get_ASF_MetadataLibrary_DescriptionRecord_DataType(Buffer)};
				
				Result->GetValue()->Append("DataType", DataTypeResult->GetValue());
				if(DataTypeResult->GetSuccess() == true)
				{
					auto DataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("DataLength", DataLengthResult->GetValue());
					if(DataLengthResult->GetSuccess() == true)
					{
						auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
						auto NameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Inspection::Length(NameLength, 0))};
						
						Result->GetValue()->Append("Name", NameResult->GetValue());
						if(NameResult->GetSuccess() == true)
						{
							auto DataLength{std::experimental::any_cast< std::uint32_t >(DataLengthResult->GetAny())};
							auto DataType{std::experimental::any_cast< const std::string & >(DataTypeResult->GetValue()->GetTagAny("interpretation"))};
							auto DataResult{Get_ASF_MetadataLibrary_DescriptionRecord_Data(Buffer, DataType, Inspection::Length(DataLength, 0))};
							
							Result->GetValue()->Append("Data", DataResult->GetValue());
							Result->SetSuccess(DataResult->GetSuccess());
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Buffer & Buffer, const std::string & Type, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Type == "Unicode string")
	{
		auto UnicodeStringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
		
		Result->SetValue(UnicodeStringResult->GetValue());
		Result->SetSuccess(UnicodeStringResult->GetSuccess());
	}
	else if(Type == "Byte array")
	{
		auto ByteArrayResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
		
		Result->SetValue(ByteArrayResult->GetValue());
		Result->SetSuccess(ByteArrayResult->GetSuccess());
	}
	else if(Type == "Boolean")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto BooleanResult{Get_ASF_Boolean_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(BooleanResult->GetValue());
		Result->SetSuccess(BooleanResult->GetSuccess());
	}
	else if(Type == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger32BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger32BitResult->GetSuccess());
	}
	else if(Type == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		
		auto UnsignedInteger64BitResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger64BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger64BitResult->GetSuccess());
	}
	else if(Type == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger16BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger16BitResult->GetSuccess());
	}
	else if(Type == "GUID")
	{
		assert(Length == Inspection::Length(16ull, 0));
		
		auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
		
		Result->SetValue(GUIDResult->GetValue());
		Result->SetSuccess(GUIDResult->GetSuccess());
		
		auto GUID{std::experimental::any_cast< const Inspection::GUID & >(GUIDResult->GetAny())};
		auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
		
		Result->GetValue()->AppendTag("interpretation", GUIDInterpretation);
	}
	else
	{
		Result->GetValue()->SetAny("<unknown type>"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibrary_DescriptionRecord_DataType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataTypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(DataTypeResult->GetValue());
	if(DataTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DataType{std::experimental::any_cast< std::uint16_t >(DataTypeResult->GetAny())};
		
		if(DataType == 0x0000)
		{
			Result->GetValue()->AppendTag("interpretation", "Unicode string"s);
		}
		else if(DataType == 0x0001)
		{
			Result->GetValue()->AppendTag("interpretation", "Byte array"s);
		}
		else if(DataType == 0x0002)
		{
			Result->GetValue()->AppendTag("interpretation", "Boolean"s);
		}
		else if(DataType == 0x0003)
		{
			Result->GetValue()->AppendTag("interpretation", "Unsigned integer 32bit"s);
		}
		else if(DataType == 0x0004)
		{
			Result->GetValue()->AppendTag("interpretation", "Unsigned integer 64bit"s);
		}
		else if(DataType == 0x0005)
		{
			Result->GetValue()->AppendTag("interpretation", "Unsigned integer 16bit"s);
		}
		else if(DataType == 0x0006)
		{
			Result->GetValue()->AppendTag("interpretation", "GUID"s);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataLibraryObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DescriptionRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("DescriptionRecordsCount", DescriptionRecordsCountResult->GetValue());
	if(DescriptionRecordsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(DescriptionRecordsCountResult->GetAny())};
		
		for(auto DescriptionRecordsIndex = 0; DescriptionRecordsIndex < DescriptionRecordsCount; ++DescriptionRecordsIndex)
		{
			auto DescriptionRecordResult{Get_ASF_MetadataLibrary_DescriptionRecord(Buffer)};
			
			Result->GetValue()->Append("DescriptionRecord", DescriptionRecordResult->GetValue());
			if(DescriptionRecordResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(2ull, 0))};
	
	Result->GetValue()->Append("Reserved", ReservedResult->GetValue());
	if(ReservedResult->GetSuccess() == true)
	{
		auto StreamNumberResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto NameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("NameLength", NameLengthResult->GetValue());
			if(NameLengthResult->GetSuccess() == true)
			{
				auto DataTypeResult{Get_ASF_DataType(Buffer)};
				
				Result->GetValue()->Append("DataType", DataTypeResult->GetValue());
				if(DataTypeResult->GetSuccess() == true)
				{
					auto DataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("DataLength", DataLengthResult->GetValue());
					if(DataLengthResult->GetSuccess() == true)
					{
						auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
						auto NameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, NameLength)};
						
						Result->GetValue()->Append("Name", NameResult->GetValue());
						if(NameResult->GetSuccess() == true)
						{
							auto DataLength{std::experimental::any_cast< std::uint32_t >(DataLengthResult->GetAny())};
							auto DataType{std::experimental::any_cast< std::string >(DataTypeResult->GetValue()->GetTagAny("interpretation"))};
							auto DataValueResult{Get_ASF_MetadataObject_DescriptionRecord_Data(Buffer, DataLength, DataType)};
							
							Result->GetValue()->Append("Data", DataValueResult->GetValue());
							Result->SetSuccess(DataValueResult->GetSuccess());
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(DataType == "Unicode string")
	{
		auto UnicodeStringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
		
		Result->SetValue(UnicodeStringResult->GetValue());
		Result->SetSuccess(UnicodeStringResult->GetSuccess());
	}
	else if(DataType == "Byte array")
	{
		auto ByteArrayResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
		
		Result->SetValue(ByteArrayResult->GetValue());
		Result->SetSuccess(ByteArrayResult->GetSuccess());
	}
	else if(DataType == "Boolean")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto BooleanResult{Get_ASF_Boolean_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(BooleanResult->GetValue());
		Result->SetSuccess(BooleanResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger32BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger32BitResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		
		auto UnsignedInteger64BitResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger64BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger64BitResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger16BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger16BitResult->GetSuccess());
	}
	else
	{
		Result->GetValue()->SetAny("<unknown type>"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DescriptionRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("DescriptionRecordsCount", DescriptionRecordsCountResult->GetValue());
	if(DescriptionRecordsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(DescriptionRecordsCountResult->GetAny())};
		
		for(auto DescriptionRecordsIndex = 0; DescriptionRecordsIndex < DescriptionRecordsCount; ++DescriptionRecordsIndex)
		{
			auto DescriptionRecordResult{Get_ASF_MetadataObject_DescriptionRecord(Buffer)};
			
			Result->GetValue()->Append("DescriptionRecord", DescriptionRecordResult->GetValue());
			if(DescriptionRecordResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_Object(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->SetValue(ObjectHeaderResult->GetValue());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto Size{std::experimental::any_cast< std::uint64_t >(ObjectHeaderResult->GetAny("Size"))};
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		std::unique_ptr< Inspection::Result > ObjectDataResult;
		
		if(GUID == Inspection::g_ASF_HeaderObjectGUID)
		{
			ObjectDataResult = Get_ASF_HeaderObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_FilePropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_FilePropertiesObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_StreamPropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_StreamPropertiesObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_CodecListObjectGUID)
		{
			ObjectDataResult = Get_ASF_CodecListObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_HeaderExtensionObjectGUID)
		{
			ObjectDataResult = Get_ASF_HeaderExtensionObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_LanguageListObjectGUID)
		{
			ObjectDataResult = Get_ASF_LanguageListObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_ExtendedStreamPropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_ExtendedStreamPropertiesObjectData(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_MetadataObjectGUID)
		{
			ObjectDataResult = Get_ASF_MetadataObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_CompatibilityObjectGUID)
		{
			ObjectDataResult = Get_ASF_CompatibilityObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_IndexPlaceholderObjectGUID)
		{
			ObjectDataResult = Get_ASF_IndexPlaceholderObjectData(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_PaddingObjectGUID)
		{
			auto Length{Inspection::Length(Size) - ObjectHeaderResult->GetValue()->GetLength()};
			
			ObjectDataResult = Get_Bits_Unset_EndedByLength(Buffer, Length);
			Result->GetValue()->Append("Data", ObjectDataResult->GetValue());
		}
		else if(GUID == Inspection::g_ASF_ExtendedContentDescriptionObjectGUID)
		{
			ObjectDataResult = Get_ASF_ExtendedContentDescriptionObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_StreamBitratePropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_StreamBitratePropertiesObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_ContentDescriptionObjectGUID)
		{
			ObjectDataResult = Get_ASF_ContentDescriptionObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_MetadataLibraryObjectGUID)
		{
			ObjectDataResult = Get_ASF_MetadataLibraryObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else
		{
			ObjectDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->Append("Data", ObjectDataResult->GetValue());
		}
		if(ObjectDataResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ObjectHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->Append("GUID", GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		auto SizeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("Size", SizeResult->GetValue());
		if(SizeResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Buffer)};
	
	Result->GetValue()->Append("Flags", FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		auto AverageBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("AverageBitrate", AverageBitrateResult->GetValue());
		Result->SetSuccess(AverageBitrateResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Position{Buffer.GetPosition()};
	auto FlagsResult{Get_BitSet_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Buffer.SetPosition(Position);
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
		
		auto StreamNumberResult{Get_UnsignedInteger_7Bit(Buffer)};
		
		Result->GetValue()->Append("[0-6] StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0ull, 9))};
			
			Result->GetValue()->Append("[7-15] Reserved", ReservedResult->GetValue());
			Result->SetSuccess(ReservedResult->GetSuccess());
		}
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamBitratePropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitrateRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("BitrateRecordsCount", BitrateRecordsCountResult->GetValue());
	if(BitrateRecordsCountResult->GetSuccess() == true)
	{
		auto BitrateRecordsCount{std::experimental::any_cast< std::uint16_t >(BitrateRecordsCountResult->GetAny())};
		
		for(auto BitrateRecordsIndex = 0; BitrateRecordsIndex < BitrateRecordsCount; ++BitrateRecordsIndex)
		{
			auto BitrateRecordResult{Get_ASF_StreamBitrateProperties_BitrateRecord(Buffer)};
			
			Result->GetValue()->Append("BitrateRecord", BitrateRecordResult->GetValue());
			Result->SetSuccess(BitrateRecordResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Position{Buffer.GetPosition()};
	auto FlagsResult{Get_BitSet_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Buffer.SetPosition(Position);
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
		
		auto StreamNumberResult{Get_UnsignedInteger_7Bit(Buffer)};
		
		Result->GetValue()->Append("[0-6] StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto ReservedResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->Append("[7-14] Reserved", ReservedResult->GetValue());
			if((ReservedResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(ReservedResult->GetAny()) == 0x00))
			{
				auto EncryptedContentFlagResult{Get_Boolean_1Bit(Buffer)};
				
				Result->GetValue()->Append("[15] EncryptedContentFlag", EncryptedContentFlagResult->GetValue());
				if(EncryptedContentFlagResult->GetSuccess() == true)
				{
					Result->SetSuccess(true);
				}
			}
		}
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FormatTagResult{Get_Microsoft_WaveFormat_FormatTag(Buffer)};
	
	Result->GetValue()->Append("FormatTag", FormatTagResult->GetValue());
	if(FormatTagResult->GetSuccess() == true)
	{
		auto NumberOfChannelsResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("NumberOfChannels", NumberOfChannelsResult->GetValue());
		if(NumberOfChannelsResult->GetSuccess() == true)
		{
			auto SamplesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("SamplesPerSecond", SamplesPerSecondResult->GetValue());
			if(SamplesPerSecondResult->GetSuccess() == true)
			{
				auto AverageNumberOfBytesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("AverageNumberOfBytesPerSecond", AverageNumberOfBytesPerSecondResult->GetValue());
				if(AverageNumberOfBytesPerSecondResult->GetSuccess() == true)
				{
					auto BlockAlignmentResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("BlockAlignment", BlockAlignmentResult->GetValue());
					Result->GetValue("BlockAlignment")->PrependTag("block size in bytes"s);
					if(BlockAlignmentResult->GetSuccess() == true)
					{
						auto BitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->Append("BitsPerSample", BitsPerSampleResult->GetValue());
						if(BitsPerSampleResult->GetSuccess() == true)
						{
							auto CodecSpecificDataSizeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->Append("CodecSpecificDataSize", CodecSpecificDataSizeResult->GetValue());
							Result->GetValue("CodecSpecificDataSize")->PrependTag("bytes"s);
							if(CodecSpecificDataSizeResult->GetSuccess() == true)
							{
								auto FormatTag{std::experimental::any_cast< const std::string & >(FormatTagResult->GetValue()->GetTagAny("constant name"))};
								auto CodecSpecificDataSize{std::experimental::any_cast< std::uint16_t >(CodecSpecificDataSizeResult->GetAny())};
								std::unique_ptr< Inspection::Result > CodecSpecificDataResult;
								
								if(FormatTag == "WAVE_FORMAT_WMAUDIO2")
								{
									CodecSpecificDataResult = Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Buffer, CodecSpecificDataSize);
								}
								else
								{
									CodecSpecificDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length(CodecSpecificDataSize));
								}
								Result->GetValue()->Append("CodecSpecificData", CodecSpecificDataResult->GetValue());
								Result->SetSuccess(CodecSpecificDataResult->GetSuccess());
								Result->GetValue()->AppendTag("AudioMedia"s);
								Result->GetValue()->AppendTag("WAVEFORMATEX"s);
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Length == Inspection::Length(10ull, 0))
	{
		auto SamplesPerBlockResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("SamplesPerBlock", SamplesPerBlockResult->GetValue());
		if(SamplesPerBlockResult->GetSuccess() == true)
		{
			auto EncodeOptionsResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("EncodeOptions", EncodeOptionsResult->GetValue());
			if(EncodeOptionsResult->GetSuccess() == true)
			{
				auto SuperBlockAlignResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("SuperBlockAlign", SuperBlockAlignResult->GetValue());
				Result->SetSuccess(SuperBlockAlignResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		
		if(GUID == Inspection::g_ASF_StreamPropertiesObjectGUID)
		{
			auto HeaderObjectDataResult{Get_ASF_StreamPropertiesObjectData(Buffer)};
			
			Result->GetValue()->AppendValues(HeaderObjectDataResult->GetValue()->GetValues());
			if(HeaderObjectDataResult->GetSuccess() ==true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto StreamTypeResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->Append("StreamType", StreamTypeResult->GetValue());
	if(StreamTypeResult->GetSuccess() == true)
	{
		auto ErrorCorrectionTypeResult{Get_ASF_GUID(Buffer)};
		
		Result->GetValue()->Append("ErrorCorrectionType", ErrorCorrectionTypeResult->GetValue());
		if(ErrorCorrectionTypeResult->GetSuccess() == true)
		{
			auto TimeOffsetResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("TimeOffset", TimeOffsetResult->GetValue());
			if(TimeOffsetResult->GetSuccess() == true)
			{
				auto TypeSpecificDataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("TypeSpecificDataLength", TypeSpecificDataLengthResult->GetValue());
				if(TypeSpecificDataLengthResult->GetSuccess() == true)
				{
					auto ErrorCorrectionDataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("ErrorCorrectionDataLength", ErrorCorrectionDataLengthResult->GetValue());
					if(ErrorCorrectionDataLengthResult->GetSuccess() == true)
					{
						auto FlagsResult{Get_ASF_StreamProperties_Flags(Buffer)};
						
						Result->GetValue()->Append("Flags", FlagsResult->GetValue());
						if(FlagsResult->GetSuccess() == true)
						{
							auto ReservedResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->Append("Reserved", ReservedResult->GetValue());
							if(ReservedResult->GetSuccess() == true)
							{
								auto TypeSpecificDataLength{std::experimental::any_cast< std::uint32_t >(TypeSpecificDataLengthResult->GetAny())};
								auto StreamType{std::experimental::any_cast< Inspection::GUID >(StreamTypeResult->GetAny())};
								std::unique_ptr< Inspection::Result > TypeSpecificDataResult;
								
								if(StreamType == Inspection::g_ASF_AudioMediaGUID)
								{
									TypeSpecificDataResult = Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Buffer, TypeSpecificDataLength);
								}
								else
								{
									TypeSpecificDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, TypeSpecificDataLength);
								}
								Result->GetValue()->Append("TypeSpecificData", TypeSpecificDataResult->GetValue());
								if(TypeSpecificDataResult->GetSuccess() == true)
								{
									auto ErrorCorrectionDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, std::experimental::any_cast< std::uint32_t >(ErrorCorrectionDataLengthResult->GetAny()))};
									
									Result->GetValue()->Append("ErrorCorrectionData", ErrorCorrectionDataResult->GetValue());
									if(ErrorCorrectionDataResult->GetSuccess() == true)
									{
										Result->SetSuccess(true);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto ASFFileResult{Get_ASF_File(Buffer)};
	
	ASFFileResult->GetValue()->SetName("ASFFile");
	
	return ASFFileResult;
}

int main(int argc, char ** argv)
{
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto Argument{0};
	
	while(++Argument < Arguments)
	{
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}
	
	return 0;
}
