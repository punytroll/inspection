#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/any_printing.h"
#include "../common/file_handling.h"
#include "../common/getters/4th.h"
#include "../common/guid.h"
#include "../common/results.h"

/// Top-level ASF Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.1
GUID g_ASF_HeaderObjectGUID{"75b22630-668e-11cf-a6d9-00aa0062ce6c"};
GUID g_ASF_DataObjectGUID{"75b22636-668e-11cf-a6d9-00aa0062ce6c"};
GUID g_ASF_SimpleIndexObjectGUID{"33000890-e5b1-11cf-89f4-00a0c90349cb"};
GUID g_ASF_IndexObjectGUID{"D6E229D3-35DA-11D1-9034-00A0C90349BE"};
GUID g_ASF_MediaObjectIndexObjectGUID{"feb103f8-12ad-4c64-840f-2a1d2f7ad48c"};
GUID g_ASF_TimecodeIndexObject{"3cb73fd0-0c4a-4803-953d-edf7b6228f0c"};

/// Header Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.2
GUID g_ASF_FilePropertiesObjectGUID{"8cabdca1-a947-11cf-8ee4-00c00c205365"};
GUID g_ASF_StreamPropertiesObjectGUID{"b7dc0791-a9b7-11cf-8ee6-00c00c205365"};
GUID g_ASF_HeaderExtensionObjectGUID{"5fbf03b5-a92e-11cf-8ee3-00c00c205365"};
GUID g_ASF_CodecListObjectGUID{"86d15240-311d-11d0-a3a4-00a0c90348f6"};
GUID g_ASF_ScriptCommandObjectGUID{"1efb1a30-0b62-11d0-a39b-00a0c90348f6"};
GUID g_ASF_MarkerObjectGUID{"f487cd01-a951-11cf-8ee6-00c00c205365"};
GUID g_ASF_BitrateMutualExclusionObjectGUID{"d6e229dc-35da-11d1-9034-00a0c90349be"};
GUID g_ASF_ErrorCorrectionObjectGUID{"75b22635-668e-11cf-a6d9-00aa0062ce6c"};
GUID g_ASF_ContentDescriptionObjectGUID{"75b22633-668e-11cf-a6d9-00aa0062ce6c"};
GUID g_ASF_ExtendedContentDescriptionObjectGUID{"d2d0a440-e307-11d2-97f0-00a0c95ea850"};
GUID g_ASF_ContentBrandingObjectGUID{"2211b3fa-bd23-11d2-b4b7-00a0c955fc6e"};
GUID g_ASF_StreamBitratePropertiesObjectGUID{"7bf875ce-468d-11d1-8d82-006097c9a2b2"};
GUID g_ASF_ContentEncryptionObjectGUID{"2211b3fb-bd23-11d2-b4b7-00a0c955fc6e"};
GUID g_ASF_ExtendedContentEncryptionObjectGUID{"298ae614-2622-4c17-b935-dae07ee9289c"};
GUID g_ASF_DigitalSignatureObjectGUID{"2211b3fc-bd23-11d2-b4b7-00a0c955fc6e"};
GUID g_ASF_PaddingObjectGUID{"1806d474-cadf-4509-a4ba-9aabcb96aae8"};

/// Stream Properties Object Stream Type GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.4
GUID g_ASF_AudioMediaGUID{"f8699e40-5b4d-11cf-a8fd-00805f5c442b"};
GUID g_ASF_VideoMediaGUID{"bc19efc0-5b4d-11cf-a8fd-00805f5c442b"};
GUID g_ASF_CommandMediaGUID{"59dacfc0-59e6-11d0-a3ac-00a0c90348f6"};
GUID g_ASF_JFIFMediaGUID{"b61be100-5b4e-11cf-a8fd-00805f5c442b"};
GUID g_ASF_DegradableJPEGMediaGUID{"35907de0-e415-11cf-a917-00805f5c442b"};
GUID g_ASF_FileTransferMediaGUID{"91bd222c-f21c-497a-8b6d-5aa86bfc0185"};
GUID g_ASF_BinaryMediaGUID{"3afb65e2-47ef-40f2-ac2c-70a90d71d343"};

/// Stream Properties Object Error Correction Type GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.5
GUID g_ASF_NoErrorCorrection{"20fb5700-5b55-11cf-a8fd-00805f5c442b"};
GUID g_ASF_AudioSpread{"bfc3cd50-618f-11cf-8bb2-00aa00b4e220"};

/// Codec List Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.8
GUID g_ASF_Reserved2{"86d15241-311d-11d0-a3a4-00a0c90348f6"};

///////////////////////////////////////////////////////////////////////////////////////////////////
// 4th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Results::Result > Get_ASF_CodecEntry(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_CodecEntryType(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_CodecListObjectData(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_GUID(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_FilePropertiesFlags(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_FilePropertiesObjectData(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_HeaderObjectData(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_Object(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_StreamPropertiesFlags(const std::uint8_t * Buffer, std::uint64_t Length);
std::unique_ptr< Results::Result > Get_ASF_StreamPropertiesObjectData(const std::uint8_t * Buffer, std::uint64_t Length);

std::unique_ptr< Results::Result > Get_ASF_CodecEntry(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{true};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto Type{Get_ASF_CodecEntryType(Buffer + Index, Length - Index)};
	
	if(Type->GetSuccess() == true)
	{
		Index += Type->GetLength();
		Value->Append("Type", Type->GetValue());
		
		auto CodecNameLength{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
		
		if(CodecNameLength->GetSuccess() == true)
		{
			Index += CodecNameLength->GetLength();
			Value->Append("Codec Name Length", CodecNameLength->GetValue());
			
			auto CodecNameLengthValue{std::experimental::any_cast< std::uint16_t >(CodecNameLength->GetAny())};
			auto CodecName{Get_UTF16LE_String_EndedByTermination(Buffer + Index, Length - Index)};
			
			if((CodecName->GetSuccess() == true) && (CodecName->GetLength() == CodecNameLengthValue * 2))
			{
				Index += CodecName->GetLength();
				Value->Append("Codec Name", CodecName->GetValue());
				
				auto CodecDescriptionLength{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
				
				if(CodecDescriptionLength->GetSuccess() == true)
				{
					Index += CodecDescriptionLength->GetLength();
					Value->Append("Codec Description Length", CodecDescriptionLength->GetValue());
					
					auto CodecDescriptionLengthValue{std::experimental::any_cast< std::uint16_t >(CodecDescriptionLength->GetAny())};
					auto CodecDescription{Get_UTF16LE_String_EndedByTermination(Buffer + Index, Length - Index)};
					
					if((CodecDescription->GetSuccess() == true) && (CodecDescription->GetLength() == CodecDescriptionLengthValue * 2))
					{
						Index += CodecDescription->GetLength();
						Value->Append("Codec Description", CodecDescription->GetValue());
						
						auto CodecInformationLength{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
						
						if(CodecInformationLength->GetSuccess() == true)
						{
							Index += CodecInformationLength->GetLength();
							Value->Append("Codec Information Length", CodecInformationLength->GetValue());
							
							auto CodecInformationLengthValue{std::experimental::any_cast< std::uint16_t >(CodecInformationLength->GetAny())};
							auto CodecInformation{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer + Index, CodecInformationLengthValue)};
							
							if(CodecInformation->GetSuccess() == true)
							{
								Index += CodecInformation->GetLength();
								Value->Append("Codec Information", CodecInformation->GetValue());
							}
						}
					}
				}
			}
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_CodecEntryType(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto Type{Get_UnsignedInteger_16Bit_LittleEndian(Buffer + Index, Length - Index)};
	
	if(Type->GetSuccess() == true)
	{
		Success = true;
		Index += Type->GetLength();
		Value->SetAny(Type->GetAny());
		
		auto TypeValue{std::experimental::any_cast< std::uint16_t >(Type->GetAny())};
		
		if(TypeValue == 0x0001)
		{
			Value->Append("Interpretation", std::string("Video Codec"));
		}
		else if(TypeValue == 0x0002)
		{
			Value->Append("Interpretation", std::string("Audio Codec"));
		}
		else if(TypeValue == 0xffff)
		{
			Value->Append("Interpretation", std::string("Unknown Codec"));
		}
		else
		{
			Value->Append("Interpretation", std::string("<unknown Type value>"));
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_CodecListObjectData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto ReservedGUID{Get_ASF_GUID(Buffer + Index, Length - Index)};
	
	if(ReservedGUID->GetSuccess() == true)
	{
		auto ReservedGUIDValue{std::experimental::any_cast< GUID >(ReservedGUID->GetAny())};
		
		if(ReservedGUIDValue == g_ASF_Reserved2)
		{
			Index += ReservedGUID->GetLength();
			Value->Append("Reserved", ReservedGUID->GetValue());
			
			auto CodecEntriesCount{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
			
			if(CodecEntriesCount->GetSuccess() == true)
			{
				Index += CodecEntriesCount->GetLength();
				Value->Append("Codec Entries Count", CodecEntriesCount->GetValue());
				
				auto CodecEntries(std::make_shared< Results::Value >());
				
				CodecEntries->SetName("Codec Entries");
				
				auto CodecEntriesCountValue{std::experimental::any_cast< std::uint32_t >(CodecEntriesCount->GetAny())};
				
				for(auto CodecEntryIndex = 0ul; CodecEntryIndex < CodecEntriesCountValue; ++CodecEntryIndex)
				{
					auto CodecEntry{Get_ASF_CodecEntry(Buffer + Index, Length - Index)};
					
					if(CodecEntry->GetSuccess() == true)
					{
						Index += CodecEntry->GetLength();
						CodecEntries->Append(CodecEntry->GetValue());
					}
					else
					{
						return Results::MakeFailure();
					}
				}
				Value->Append(CodecEntries);
				Success = true;
			}
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_GUID(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer + Index, Length - Index)};
	
	if(GUIDResult->GetSuccess() == true)
	{
		Success = true;
		Index += GUIDResult->GetLength();
		
		auto GUIDValue{std::experimental::any_cast< GUID >(GUIDResult->GetAny())};
		
		Value->SetAny(GUIDResult->GetAny());
		// Top-level ASF Object GUIDs
		if(GUIDValue == g_ASF_HeaderObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Header_Object"));
		}
		else if(GUIDValue == g_ASF_DataObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Data_Object"));
		}
		else if(GUIDValue == g_ASF_SimpleIndexObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Simple_Index_Object"));
		}
		else if(GUIDValue == g_ASF_IndexObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Index_Object"));
		}
		else if(GUIDValue == g_ASF_MediaObjectIndexObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Media_Object_Index_Object"));
		}
		else if(GUIDValue == g_ASF_TimecodeIndexObject)
		{
			Value->Append("Interpretation", std::string("ASF_Timecode_Index_Object"));
		}
		// Header Object GUIDs
		else if(GUIDValue == g_ASF_FilePropertiesObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_File_Properties_Object"));
		}
		else if(GUIDValue == g_ASF_StreamPropertiesObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Stream_Properties_Object"));
		}
		else if(GUIDValue == g_ASF_HeaderExtensionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Header_Extension_Object"));
		}
		else if(GUIDValue == g_ASF_CodecListObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Codec_List_Object"));
		}
		else if(GUIDValue == g_ASF_ScriptCommandObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Script_Command_Object"));
		}
		else if(GUIDValue == g_ASF_MarkerObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Marker_Object"));
		}
		else if(GUIDValue == g_ASF_BitrateMutualExclusionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Bitrate_Mutual_Exclusion_Object"));
		}
		else if(GUIDValue == g_ASF_ErrorCorrectionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Error_Correction_Object"));
		}
		else if(GUIDValue == g_ASF_ContentDescriptionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Content_Description_Object"));
		}
		else if(GUIDValue == g_ASF_ExtendedContentDescriptionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Extended_Content_Description_Object"));
		}
		else if(GUIDValue == g_ASF_ContentBrandingObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Content_Branding_Object"));
		}
		else if(GUIDValue == g_ASF_StreamBitratePropertiesObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Stream_Bitrate_Properties_Object"));
		}
		else if(GUIDValue == g_ASF_ContentEncryptionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Content_Encryption_Object"));
		}
		else if(GUIDValue == g_ASF_ExtendedContentEncryptionObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Extended_Content_Encryption_Object"));
		}
		else if(GUIDValue == g_ASF_DigitalSignatureObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Digital_Signature_Object"));
		}
		else if(GUIDValue == g_ASF_PaddingObjectGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Padding_Object"));
		}
		// Stream Properties Object Stream Type GUIDs
		else if(GUIDValue == g_ASF_AudioMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Audio_Media"));
		}
		else if(GUIDValue == g_ASF_VideoMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Video_Media"));
		}
		else if(GUIDValue == g_ASF_CommandMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Command_Media"));
		}
		else if(GUIDValue == g_ASF_JFIFMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_JFIF_Media"));
		}
		else if(GUIDValue == g_ASF_DegradableJPEGMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Degradable_JPEG_Media"));
		}
		else if(GUIDValue == g_ASF_FileTransferMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_File_Transfer_Media"));
		}
		else if(GUIDValue == g_ASF_BinaryMediaGUID)
		{
			Value->Append("Interpretation", std::string("ASF_Binary_Media"));
		}
		// Stream Properties Object Error Correction Type GUIDs
		else if(GUIDValue == g_ASF_NoErrorCorrection)
		{
			Value->Append("Interpretation", std::string("ASF_No_Error_Correction"));
		}
		else if(GUIDValue == g_ASF_AudioSpread)
		{
			Value->Append("Interpretation", std::string("ASF_Audio_Spread"));
		}
		// Codec List Object GUIDs
		else if(GUIDValue == g_ASF_Reserved2)
		{
			Value->Append("Interpretation", std::string("ASF_Reserved_2"));
		}
		// unknown
		else
		{
			Value->Append("Interpretation", std::string("<unknown GUID value>"));
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_FilePropertiesFlags(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto Flags{Get_BitSet_32Bit_LittleEndian(Buffer + Index, Length - Index)};
	
	if(Flags->GetSuccess() == true)
	{
		auto FlagsValue{std::experimental::any_cast< std::bitset< 32 > >(Flags->GetAny())};
		
		Value->SetAny(Flags->GetAny());
		Value->Append("[0] Broadcast", bool(FlagsValue[0]));
		Value->Append("[1] Seekable", bool(FlagsValue[1]));
		Value->Append("[2-31] Reserved", false);
		Success = true;
		for(auto Index = 2; Index < 32; ++Index)
		{
			Success &= ~FlagsValue[Index];
		}
		Index += Flags->GetLength();
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_FilePropertiesObjectData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto FileID{Get_GUID_LittleEndian(Buffer + Index, Length - Index)};
	
	if(FileID->GetSuccess() == true)
	{
		Index += FileID->GetLength();
		Value->Append("File ID", FileID->GetValue());
		
		auto FileSize{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
		
		if(FileSize->GetSuccess() == true)
		{
			Index += FileSize->GetLength();
			Value->Append("File Size", FileSize->GetValue());
			
			auto CreationDate{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
			
			if(CreationDate->GetSuccess() == true)
			{
				Index += CreationDate->GetLength();
				Value->Append("Creation Date", CreationDate->GetValue());
				
				auto DataPacketsCount{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
				
				if(DataPacketsCount->GetSuccess() == true)
				{
					Index += DataPacketsCount->GetLength();
					Value->Append("Data Packets Count", DataPacketsCount->GetValue());
					
					auto PlayDuration{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
					
					if(PlayDuration->GetSuccess() == true)
					{
						Index += PlayDuration->GetLength();
						Value->Append("Play Duration", PlayDuration->GetValue());
						
						auto SendDuration{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
						
						if(SendDuration->GetSuccess() == true)
						{
							Index += SendDuration->GetLength();
							Value->Append("Send Duration", SendDuration->GetValue());
							
							auto Preroll{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
							
							if(Preroll->GetSuccess() == true)
							{
								Index += Preroll->GetLength();
								Value->Append("Preroll", Preroll->GetValue());
								
								auto Flags{Get_ASF_FilePropertiesFlags(Buffer + Index, Length - Index)};
								
								if(Flags->GetSuccess() == true)
								{
									Index += Flags->GetLength();
									Value->Append("Flags", Flags->GetValue());
									
									auto MinimumDataPacketSize{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
									
									if(MinimumDataPacketSize->GetSuccess() == true)
									{
										Index += MinimumDataPacketSize->GetLength();
										Value->Append("Minimum Data Packet Size", MinimumDataPacketSize->GetValue());
										
										auto MaximumDataPacketSize{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
										
										if(MaximumDataPacketSize->GetSuccess() == true)
										{
											Index += MaximumDataPacketSize->GetLength();
											Value->Append("Maximum Data Packet Size", MaximumDataPacketSize->GetValue());
											
											auto MaximumBitrate{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
											
											if(MaximumBitrate->GetSuccess() == true)
											{
												Index += MaximumBitrate->GetLength();
												Value->Append("Maximum Bitrate", MaximumBitrate->GetValue());
												Success = true;
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
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_HeaderObjectData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto NumberOfHeaderObjects{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
	
	if(NumberOfHeaderObjects->GetSuccess() == true)
	{
		Index += NumberOfHeaderObjects->GetLength();
		Value->Append("Number of header objects", NumberOfHeaderObjects->GetValue());
		
		auto Reserved1{Get_UnsignedInteger_8Bit(Buffer + Index, Length - Index)};
		
		if((Reserved1->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(Reserved1->GetAny()) == 0x01))
		{
			Index += Reserved1->GetLength();
			Value->Append("Reserved1", Reserved1->GetValue());
			
			auto Reserved2{Get_UnsignedInteger_8Bit(Buffer + Index, Length - Index)};
			
			if((Reserved2->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(Reserved2->GetAny()) == 0x02))
			{
				Index += Reserved2->GetLength();
				Value->Append("Reserved2", Reserved2->GetValue());
				
				auto HeaderObjects(std::make_shared< Results::Value >());
				
				HeaderObjects->SetName("Header objects");
				
				auto NumberOfHeaderObjectsValue{std::experimental::any_cast< std::uint32_t >(NumberOfHeaderObjects->GetAny())};
				
				for(auto ObjectIndex = 0ul; ObjectIndex < NumberOfHeaderObjectsValue; ++ObjectIndex)
				{
					auto HeaderObject{Get_ASF_Object(Buffer + Index, Length - Index)};
					
					if(HeaderObject->GetSuccess() == true)
					{
						Index += HeaderObject->GetLength();
						HeaderObjects->Append(HeaderObject->GetValue());
					}
					else
					{
						return Results::MakeFailure();
					}
				}
				Value->Append(HeaderObjects);
				Success = true;
			}
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_Object(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto ObjectGUID{Get_ASF_GUID(Buffer + Index, Length - Index)};
	
	if(ObjectGUID->GetSuccess() == true)
	{
		Value->SetName("ASF Object");
		Index += ObjectGUID->GetLength();
		Value->Append("GUID", ObjectGUID->GetValue());
		
		auto ObjectSize{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
		
		if(ObjectSize->GetSuccess() == true)
		{
			Index += ObjectSize->GetLength();
			Value->Append("Size", ObjectSize->GetValue());
			
			auto DataSize{std::experimental::any_cast< std::uint64_t >(ObjectSize->GetAny())};
			
			if(DataSize <= Length)
			{
				DataSize -= Index;
				
				auto GUIDValue{std::experimental::any_cast< GUID >(ObjectGUID->GetAny())};
				std::unique_ptr< Results::Result > ObjectData;
				
				if(GUIDValue == g_ASF_HeaderObjectGUID)
				{
					ObjectData = Get_ASF_HeaderObjectData(Buffer + Index, DataSize);
				}
				else if(GUIDValue == g_ASF_FilePropertiesObjectGUID)
				{
					ObjectData = Get_ASF_FilePropertiesObjectData(Buffer + Index, DataSize);
				}
				else if(GUIDValue == g_ASF_StreamPropertiesObjectGUID)
				{
					ObjectData = Get_ASF_StreamPropertiesObjectData(Buffer + Index, DataSize);
				}
				else if(GUIDValue == g_ASF_CodecListObjectGUID)
				{
					ObjectData = Get_ASF_CodecListObjectData(Buffer + Index, DataSize);
				}
				else
				{
					ObjectData = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer + Index, DataSize);
				}
				if(ObjectData->GetSuccess() == true)
				{
					Index += ObjectData->GetLength();
					Value->Append("Data", ObjectData->GetValue());
					Success = true;
				}
			}
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_StreamPropertiesFlags(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto Flags{Get_BitSet_16Bit_LittleEndian(Buffer + Index, Length - Index)};
	
	if(Flags->GetSuccess() == true)
	{
		Value->SetAny(Flags->GetAny());
		
		auto FlagsValue{std::experimental::any_cast< std::bitset< 16 > >(Flags->GetAny())};
		std::uint8_t StreamNumber{0};
		
		if(FlagsValue[0] == true)
		{
			StreamNumber += 1;
		}
		if(FlagsValue[1] == true)
		{
			StreamNumber += 2;
		}
		if(FlagsValue[2] == true)
		{
			StreamNumber += 4;
		}
		if(FlagsValue[3] == true)
		{
			StreamNumber += 8;
		}
		if(FlagsValue[4] == true)
		{
			StreamNumber += 16;
		}
		if(FlagsValue[5] == true)
		{
			StreamNumber += 32;
		}
		if(FlagsValue[6] == true)
		{
			StreamNumber += 64;
		}
		Value->Append("[0-6] Stream Number", StreamNumber);
		Value->Append("[7-14] Reserved", false);
		Value->Append("[15] Encrypted Content Flag", bool(FlagsValue[15]));
		Success = true;
		for(auto Index = 7; Index < 15; ++Index)
		{
			Success &= ~FlagsValue[Index];
		}
		Index += Flags->GetLength();
	}
	
	return Results::MakeResult(Success, Index, Value);
}

std::unique_ptr< Results::Result > Get_ASF_StreamPropertiesObjectData(const std::uint8_t * Buffer, std::uint64_t Length)
{
	auto Success{false};
	auto Index{0ull};
	auto Value{std::make_shared< Results::Value >()};
	auto StreamType{Get_ASF_GUID(Buffer + Index, Length - Index)};
	
	if(StreamType->GetSuccess() == true)
	{
		Index += StreamType->GetLength();
		Value->Append("Stream Type", StreamType->GetValue());
		
		auto ErrorCorrectionType{Get_ASF_GUID(Buffer + Index, Length - Index)};
		
		if(ErrorCorrectionType->GetSuccess() == true)
		{
			Index += ErrorCorrectionType->GetLength();
			Value->Append("Error Correction Type", ErrorCorrectionType->GetValue());
			
			auto TimeOffset{Get_UnsignedInteger_64Bit_LittleEndian(Buffer + Index, Length - Index)};
			
			if(TimeOffset->GetSuccess() == true)
			{
				Index += TimeOffset->GetLength();
				Value->Append("Time Offset", TimeOffset->GetValue());
				
				auto TypeSpecificDataLength{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
				
				if(TypeSpecificDataLength->GetSuccess() == true)
				{
					Index += TypeSpecificDataLength->GetLength();
					Value->Append("Type-Specific Data Length", TypeSpecificDataLength->GetValue());
					
					auto ErrorCorrectionDataLength{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
					
					if(ErrorCorrectionDataLength->GetSuccess() == true)
					{
						Index += ErrorCorrectionDataLength->GetLength();
						Value->Append("Error Correction Data Length", ErrorCorrectionDataLength->GetValue());
						
						auto Flags{Get_ASF_StreamPropertiesFlags(Buffer + Index, Length - Index)};
						
						if(Flags->GetSuccess() == true)
						{
							Index += Flags->GetLength();
							Value->Append("Flags", Flags->GetValue());
							
							auto Reserved{Get_UnsignedInteger_32Bit_LittleEndian(Buffer + Index, Length - Index)};
							
							if(Reserved->GetSuccess() == true)
							{
								Index += Reserved->GetLength();
								Value->Append("Reserved", Reserved->GetValue());
								
								auto TypeSpecificData{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer + Index, std::experimental::any_cast< std::uint32_t >(TypeSpecificDataLength->GetAny()))};
								
								if(TypeSpecificData->GetSuccess() == true)
								{
									Index += TypeSpecificData->GetLength();
									Value->Append("Type-Specific Data", TypeSpecificData->GetValue());
									
									auto ErrorCorrectionData{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer + Index, std::experimental::any_cast< std::uint32_t >(ErrorCorrectionDataLength->GetAny()))};
									
									if(ErrorCorrectionData->GetSuccess() == true)
									{
										Index += ErrorCorrectionData->GetLength();
										Value->Append("Error Correction Data", ErrorCorrectionData->GetValue());
										Success = true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	return Results::MakeResult(Success, Index, Value);
}

void PrintValue(const std::string & Indentation, std::shared_ptr< Results::Value > Value)
{
	auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetAny().empty() == false)};
	
	if(HeaderLine == true)
	{
		std::cout << Indentation;
	}
	if(Value->GetName().empty() == false)
	{
		std::cout << Value->GetName() << ": ";
	}
	if(Value->GetAny().empty() == false)
	{
		std::cout << Value->GetAny();
	}
	
	auto SubIndentation{Indentation};
	
	if(HeaderLine == true)
	{
		std::cout << std::endl;
		SubIndentation += "    ";
	}
	if(Value->GetCount() > 0)
	{
		for(auto & SubValue : Value->GetValues())
		{
			PrintValue(SubIndentation, SubValue);
		}
	}
}

void PrintValue(std::shared_ptr< Results::Value > Value)
{
	PrintValue("", Value);
}

void ReadFile(const std::string & Path)
{
	auto FileDescriptor{open(Path.c_str(), O_RDONLY)};
	
	if(FileDescriptor == -1)
	{
		std::cerr << "Could not open the file \"" << Path << "\"." << std::endl;
	}
	else
	{
		std::int64_t FileSize{GetFileSize(Path)};
		
		if(FileSize != -1)
		{
			auto Address{reinterpret_cast< std::uint8_t * >(mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0))};
			
			if(Address == MAP_FAILED)
			{
				std::cerr << "Could not map the file \"" + Path + "\" into memory." << std::endl;
			}
			else
			{
				std::int64_t Index{0};
				std::int64_t Skipping{0};
				
				while(Index < FileSize)
				{
					auto ASFObject{Get_ASF_Object(Address + Index, FileSize - Index)};
					
					if(ASFObject->GetSuccess() == true)
					{
						if(Skipping > 0)
						{
							std::cerr << "Skipping " << Skipping << " bytes of unrecognized data." << std::endl;
							Skipping = 0;
						}
						Index += ASFObject->GetLength();
						PrintValue(ASFObject->GetValue());
					}
					else
					{
						Skipping += 1;
						Index += 1;
					}
				}
				if(Skipping > 0)
				{
					std::cerr << "Skipping " << Skipping << " bytes of unrecognized data." << std::endl;
					Skipping = 0;
				}
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
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
		ReadItem(Paths.front());
		Paths.pop_front();
	}
	
	return 0;
}
