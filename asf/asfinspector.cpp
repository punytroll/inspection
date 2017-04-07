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

#include "../common/5th.h"
#include "../common/file_handling.h"
#include "../common/guid.h"
#include "../common/string_cast.h"

using namespace std::string_literals;

/// Top-level ASF Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.1
Inspection::GUID g_ASF_HeaderObjectGUID{"75b22630-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID g_ASF_DataObjectGUID{"75b22636-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID g_ASF_SimpleIndexObjectGUID{"33000890-e5b1-11cf-89f4-00a0c90349cb"};
Inspection::GUID g_ASF_IndexObjectGUID{"D6E229D3-35DA-11D1-9034-00A0C90349BE"};
Inspection::GUID g_ASF_MediaObjectIndexObjectGUID{"feb103f8-12ad-4c64-840f-2a1d2f7ad48c"};
Inspection::GUID g_ASF_TimecodeIndexObject{"3cb73fd0-0c4a-4803-953d-edf7b6228f0c"};

/// Header Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.2
Inspection::GUID g_ASF_FilePropertiesObjectGUID{"8cabdca1-a947-11cf-8ee4-00c00c205365"};
Inspection::GUID g_ASF_StreamPropertiesObjectGUID{"b7dc0791-a9b7-11cf-8ee6-00c00c205365"};
Inspection::GUID g_ASF_HeaderExtensionObjectGUID{"5fbf03b5-a92e-11cf-8ee3-00c00c205365"};
Inspection::GUID g_ASF_CodecListObjectGUID{"86d15240-311d-11d0-a3a4-00a0c90348f6"};
Inspection::GUID g_ASF_ScriptCommandObjectGUID{"1efb1a30-0b62-11d0-a39b-00a0c90348f6"};
Inspection::GUID g_ASF_MarkerObjectGUID{"f487cd01-a951-11cf-8ee6-00c00c205365"};
Inspection::GUID g_ASF_BitrateMutualExclusionObjectGUID{"d6e229dc-35da-11d1-9034-00a0c90349be"};
Inspection::GUID g_ASF_ErrorCorrectionObjectGUID{"75b22635-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID g_ASF_ContentDescriptionObjectGUID{"75b22633-668e-11cf-a6d9-00aa0062ce6c"};
Inspection::GUID g_ASF_ExtendedContentDescriptionObjectGUID{"d2d0a440-e307-11d2-97f0-00a0c95ea850"};
Inspection::GUID g_ASF_ContentBrandingObjectGUID{"2211b3fa-bd23-11d2-b4b7-00a0c955fc6e"};
Inspection::GUID g_ASF_StreamBitratePropertiesObjectGUID{"7bf875ce-468d-11d1-8d82-006097c9a2b2"};
Inspection::GUID g_ASF_ContentEncryptionObjectGUID{"2211b3fb-bd23-11d2-b4b7-00a0c955fc6e"};
Inspection::GUID g_ASF_ExtendedContentEncryptionObjectGUID{"298ae614-2622-4c17-b935-dae07ee9289c"};
Inspection::GUID g_ASF_DigitalSignatureObjectGUID{"2211b3fc-bd23-11d2-b4b7-00a0c955fc6e"};
Inspection::GUID g_ASF_PaddingObjectGUID{"1806d474-cadf-4509-a4ba-9aabcb96aae8"};

/// Header Extension Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.3
Inspection::GUID g_ASF_ExtendedStreamPropertiesObjectGUID{"14e6a5cb-c672-4332-8399-a96952065b5a"};
Inspection::GUID g_ASF_AdvancedMutualExclusionObjectGUID{"a08649cf-4775-4670-8a16-6e35357566cd"};
Inspection::GUID g_ASF_GroupMutualExclusionObjectGUID{"d1465a40-5a79-4338-b71b-e36b8fd6c249"};
Inspection::GUID g_ASF_StreamPrioritizationObjectGUID{"d4fed15b-88d3-454f-81f0-ed5c45999e24"};
Inspection::GUID g_ASF_BandwidthSharingObjectGUID{"a69609e6-517b-11d2-b6af-00c04fd908e9"};
Inspection::GUID g_ASF_LanguageListObjectGUID{"7c4346a9-efe0-4bfc-b229-393ede415c85"};
Inspection::GUID g_ASF_MetadataObjectGUID{"c5f8cbea-5baf-4877-8467-aa8c44fa4cca"};
Inspection::GUID g_ASF_MetadataLibraryObjectGUID{"44231c94-9498-49d1-a141-1d134e457054"};
Inspection::GUID g_ASF_IndexParametersObjectGUID{"d6e229df-35da-11d1-9034-00a0c90349be"};
Inspection::GUID g_ASF_MediaObjectIndexParametersObjectGUID{"6b203bad-3f11-48e4-aca8-d7613de2cfa7"};
Inspection::GUID g_ASF_TimecodeIndexParametersObjectGUID{"f55e496d-9797-4b5d-8c8b-604dfe9bfb24"};
Inspection::GUID g_ASF_CompatibilityObjectGUID{"26f18b5d-4584-47ec-9f5f-0e651f0452c9"};
Inspection::GUID g_ASF_AdvancedContentEncryptionObjectGUID{"43058533-6981-49e6-9b74-ad12cb86d58c"};

/// Stream Properties Object Stream Type GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.4
Inspection::GUID g_ASF_AudioMediaGUID{"f8699e40-5b4d-11cf-a8fd-00805f5c442b"};
Inspection::GUID g_ASF_VideoMediaGUID{"bc19efc0-5b4d-11cf-a8fd-00805f5c442b"};
Inspection::GUID g_ASF_CommandMediaGUID{"59dacfc0-59e6-11d0-a3ac-00a0c90348f6"};
Inspection::GUID g_ASF_JFIFMediaGUID{"b61be100-5b4e-11cf-a8fd-00805f5c442b"};
Inspection::GUID g_ASF_DegradableJPEGMediaGUID{"35907de0-e415-11cf-a917-00805f5c442b"};
Inspection::GUID g_ASF_FileTransferMediaGUID{"91bd222c-f21c-497a-8b6d-5aa86bfc0185"};
Inspection::GUID g_ASF_BinaryMediaGUID{"3afb65e2-47ef-40f2-ac2c-70a90d71d343"};

/// Stream Properties Object Error Correction Type GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.5
Inspection::GUID g_ASF_NoErrorCorrection{"20fb5700-5b55-11cf-a8fd-00805f5c442b"};
Inspection::GUID g_ASF_AudioSpread{"bfc3cd50-618f-11cf-8bb2-00aa00b4e220"};

/// Header Extension Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.6
Inspection::GUID g_ASF_Reserved1{"abd3d211-a9ba-11cf-8ee6-00c00c205365"};

/// Codec List Object GUIDs
/// Taken from "Advanced Systems Format (ASF) Specification", Revision 01.20.05, Microsoft Corporation, June 2010, chapter 10.8
Inspection::GUID g_ASF_Reserved2{"86d15241-311d-11d0-a3a4-00a0c90348f6"};


///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_ASF_Boolean_16Bit_LittleEndian(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_CodecEntry(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_CodecEntryType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_CodecListObjectData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_DataObject(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length);
std::unique_ptr< Inspection::Result > Get_ASF_File(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_FilePropertiesFlags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_FilePropertiesObjectData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_GUID(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_HeaderExtensionObjectData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_HeaderObject(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_HeaderObjectData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_LanguageIDRecord(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_LanguageListObjectData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType);
std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_DataType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_MetadataObjectData(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_Object(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_ObjectHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesFlags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObject(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObjectData(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_ASF_Boolean_16Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(UnsignedInteger16BitResult->GetValue());
	if(UnsignedInteger16BitResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto UnsignedInteger16Bit{std::experimental::any_cast< std::uint16_t >(UnsignedInteger16BitResult->GetAny())};
		
		if(UnsignedInteger16Bit == 0x0000)
		{
			Result->GetValue()->Append("Interpretation", false);
		}
		else if(UnsignedInteger16Bit == 0x0001)
		{
			Result->GetValue()->Append("Interpretation", true);
		}
		else
		{
			Result->GetValue()->Append("Interpretation", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_CodecEntry(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto TypeResult{Get_ASF_CodecEntryType(Buffer)};
	
	TypeResult->GetValue()->Append("Type", TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto CodecNameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("CodecNameLength", CodecNameLengthResult->GetValue());
		if(CodecNameLengthResult->GetSuccess() == true)
		{
			auto CodecNameLength{std::experimental::any_cast< std::uint16_t >(CodecNameLengthResult->GetAny())};
			auto CodecNameResult{Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecNameLength)};
			
			Result->GetValue()->Append("CodecName", CodecNameResult->GetValue());
			if(CodecNameResult->GetSuccess() == true)
			{
				auto CodecDescriptionLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("CodecDescriptionLength", CodecDescriptionLengthResult->GetValue());
				if(CodecDescriptionLengthResult->GetSuccess() == true)
				{
					auto CodecDescriptionLength{std::experimental::any_cast< std::uint16_t >(CodecDescriptionLengthResult->GetAny())};
					auto CodecDescriptionResult{Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecDescriptionLength)};
					
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto TypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto Type{std::experimental::any_cast< std::uint16_t >(TypeResult->GetAny())};
		
		if(Type == 0x0001)
		{
			Result->GetValue()->Append("Interpretation", std::string("Video Codec"));
		}
		else if(Type == 0x0002)
		{
			Result->GetValue()->Append("Interpretation", std::string("Audio Codec"));
		}
		else if(Type == 0xffff)
		{
			Result->GetValue()->Append("Interpretation", std::string("Unknown Codec"));
		}
		else
		{
			Result->GetValue()->Append("Interpretation", std::string("<no interpretation>"));
		}
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_CodecListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ReservedGUIDResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->Append("Reserved", ReservedGUIDResult->GetValue());
	if((ReservedGUIDResult->GetSuccess() == true) && (std::experimental::any_cast< Inspection::GUID >(ReservedGUIDResult->GetAny()) == g_ASF_Reserved2))
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
				
				CodecEntries->Append(CodecEntryResult->GetValue());
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

std::unique_ptr< Inspection::Result > Get_ASF_DataObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->Append(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		auto Size{std::experimental::any_cast< std::uint64_t >(ObjectHeaderResult->GetAny("Size"))};
		
		if(GUID == g_ASF_DataObjectGUID)
		{
			auto DataObjectDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size - ObjectHeaderResult->GetValue()->GetLength().GetBytes())};
			
			Result->GetValue()->Append("Data", DataObjectDataResult->GetValue());
			if(DataObjectDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto StartTimeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->Append("StartTime", StartTimeResult->GetValue());
	Result->GetValue("StartTime")->AppendTag("milliseconds"s);
	if(StartTimeResult->GetSuccess() == true)
	{
		auto EndTimeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->Append("EndTime", EndTimeResult->GetValue());
		Result->GetValue("EndTime")->AppendTag("milliseconds"s);
		if(EndTimeResult->GetSuccess() == true)
		{
			auto DataBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->Append("DataBitrate", DataBitrateResult->GetValue());
			Result->GetValue("DataBitrate")->AppendTag("bits per second"s);
			if(DataBitrateResult->GetSuccess() == true)
			{
				auto BufferSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->Append("BufferSize", BufferSizeResult->GetValue());
				Result->GetValue("BufferSize")->AppendTag("milliseconds"s);
				if(BufferSizeResult->GetSuccess() == true)
				{
					auto InitialBufferFullnessResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("InitialBufferFullness", InitialBufferFullnessResult->GetValue());
					Result->GetValue("InitialBufferFullness")->AppendTag("milliseconds"s);
					if(InitialBufferFullnessResult->GetSuccess() == true)
					{
						auto AlternateDataBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->Append("AlternateDataBitrate", AlternateDataBitrateResult->GetValue());
						Result->GetValue("AlternateDataBitrate")->AppendTag("bits per second"s);
						if(AlternateDataBitrateResult->GetSuccess() == true)
						{
							auto AlternateBufferSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->Append("AlternateBufferSize", AlternateBufferSizeResult->GetValue());
							Result->GetValue("AlternateBufferSize")->AppendTag("milliseconds"s);
							if(AlternateBufferSizeResult->GetSuccess() == true)
							{
								auto AlternateInitialBufferFullnessResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
								
								Result->GetValue()->Append("AlternateInitialBufferFullness", AlternateInitialBufferFullnessResult->GetValue());
								Result->GetValue("AlternateInitialBufferFullness")->AppendTag("milliseconds"s);
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->SetValue(GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(GUIDResult->GetAny())};
		
		// Top-level ASF Object GUIDs
		if(GUID == g_ASF_HeaderObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Header_Object"));
		}
		else if(GUID == g_ASF_DataObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Data_Object"));
		}
		else if(GUID == g_ASF_SimpleIndexObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Simple_Index_Object"));
		}
		else if(GUID == g_ASF_IndexObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Index_Object"));
		}
		else if(GUID == g_ASF_MediaObjectIndexObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Media_Object_Index_Object"));
		}
		else if(GUID == g_ASF_TimecodeIndexObject)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Timecode_Index_Object"));
		}
		// Header Object GUIDs
		else if(GUID == g_ASF_FilePropertiesObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_File_Properties_Object"));
		}
		else if(GUID == g_ASF_StreamPropertiesObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Stream_Properties_Object"));
		}
		else if(GUID == g_ASF_HeaderExtensionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Header_Extension_Object"));
		}
		else if(GUID == g_ASF_CodecListObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Codec_List_Object"));
		}
		else if(GUID == g_ASF_ScriptCommandObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Script_Command_Object"));
		}
		else if(GUID == g_ASF_MarkerObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Marker_Object"));
		}
		else if(GUID == g_ASF_BitrateMutualExclusionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Bitrate_Mutual_Exclusion_Object"));
		}
		else if(GUID == g_ASF_ErrorCorrectionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Error_Correction_Object"));
		}
		else if(GUID == g_ASF_ContentDescriptionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Content_Description_Object"));
		}
		else if(GUID == g_ASF_ExtendedContentDescriptionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Extended_Content_Description_Object"));
		}
		else if(GUID == g_ASF_ContentBrandingObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Content_Branding_Object"));
		}
		else if(GUID == g_ASF_StreamBitratePropertiesObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Stream_Bitrate_Properties_Object"));
		}
		else if(GUID == g_ASF_ContentEncryptionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Content_Encryption_Object"));
		}
		else if(GUID == g_ASF_ExtendedContentEncryptionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Extended_Content_Encryption_Object"));
		}
		else if(GUID == g_ASF_DigitalSignatureObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Digital_Signature_Object"));
		}
		else if(GUID == g_ASF_PaddingObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Padding_Object"));
		}
		// Header Extension Object GUIDs
		else if(GUID == g_ASF_ExtendedStreamPropertiesObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Extended_Stream_Properties_Object"s);
		}
		else if(GUID == g_ASF_AdvancedMutualExclusionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Advanced_Mutual_Exclusion_Object"s);
		}
		else if(GUID == g_ASF_GroupMutualExclusionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Group_Mutual_Exclusion_Object"s);
		}
		else if(GUID == g_ASF_StreamPrioritizationObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Stream_Prioritization_Object"s);
		}
		else if(GUID == g_ASF_BandwidthSharingObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Bandwidth_Sharing_Object"s);
		}
		else if(GUID == g_ASF_LanguageListObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Language_List_Object"s);
		}
		else if(GUID == g_ASF_MetadataObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Metadata_Object"s);
		}
		else if(GUID == g_ASF_MetadataLibraryObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Metadata_Library_Object"s);
		}
		else if(GUID == g_ASF_IndexParametersObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Index_Parameters_Object"s);
		}
		else if(GUID == g_ASF_MediaObjectIndexParametersObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Media_Object_Index_Parameters_Object"s);
		}
		else if(GUID == g_ASF_TimecodeIndexParametersObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Timecode_Index_Parameters_Object"s);
		}
		else if(GUID == g_ASF_CompatibilityObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Compatibility_Object"s);
		}
		else if(GUID == g_ASF_AdvancedContentEncryptionObjectGUID)
		{
			Result->GetValue()->Append("Interpretation", "ASF_Advanced_Content_Encryption_Object"s);
		}
		// Stream Properties Object Stream Type GUIDs
		else if(GUID == g_ASF_AudioMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Audio_Media"));
		}
		else if(GUID == g_ASF_VideoMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Video_Media"));
		}
		else if(GUID == g_ASF_CommandMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Command_Media"));
		}
		else if(GUID == g_ASF_JFIFMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_JFIF_Media"));
		}
		else if(GUID == g_ASF_DegradableJPEGMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Degradable_JPEG_Media"));
		}
		else if(GUID == g_ASF_FileTransferMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_File_Transfer_Media"));
		}
		else if(GUID == g_ASF_BinaryMediaGUID)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Binary_Media"));
		}
		// Stream Properties Object Error Correction Type GUIDs
		else if(GUID == g_ASF_NoErrorCorrection)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_No_Error_Correction"));
		}
		else if(GUID == g_ASF_AudioSpread)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Audio_Spread"));
		}
		// Header Extension Object GUIDs
		else if(GUID == g_ASF_Reserved1)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Reserved_1"));
		}
		// Codec List Object GUIDs
		else if(GUID == g_ASF_Reserved2)
		{
			Result->GetValue()->Append("Interpretation", std::string("ASF_Reserved_2"));
		}
		// unknown
		else
		{
			Result->GetValue()->Append("Interpretation", std::string("<no interpretation>"));
		}
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_HeaderExtensionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ReservedField1Result{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->Append("ReservedField1", ReservedField1Result->GetValue());
	if(ReservedField1Result->GetSuccess() == true)
	{
		auto Reserved1Field{std::experimental::any_cast< Inspection::GUID >(ReservedField1Result->GetAny())};
		
		if(Reserved1Field == g_ASF_Reserved1)
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->Append(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		
		if(GUID == g_ASF_HeaderObjectGUID)
		{
			auto HeaderObjectDataResult{Get_ASF_HeaderObjectData(Buffer)};
			
			Result->GetValue()->Append(HeaderObjectDataResult->GetValue()->GetValues());
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
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

std::unique_ptr< Inspection::Result > Get_ASF_LanguageIDRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto LanguageIDLengthResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->GetValue()->Append("LanguageIDLength", LanguageIDLengthResult->GetValue());
	if(LanguageIDLengthResult->GetSuccess() == true)
	{
		auto LanguageIDLength{std::experimental::any_cast< std::uint8_t >(LanguageIDLengthResult->GetAny())};
		auto LanguageIDResult{Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, LanguageIDLength)};
		
		Result->GetValue()->Append("LanguageID", LanguageIDResult->GetValue());
		Result->SetSuccess(LanguageIDResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_LanguageListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
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

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(2ull, 0))};
	
	Result->GetValue()->Append("Reserved", to_string_cast(ReservedResult->GetLength()) + " bytes of zeroed data");
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
				auto DataTypeResult{Get_ASF_MetadataObject_DescriptionRecord_DataType(Buffer)};
				
				Result->GetValue()->Append("DataType", DataTypeResult->GetValue());
				if(DataTypeResult->GetSuccess() == true)
				{
					auto DataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->Append("DataLength", DataLengthResult->GetValue());
					if(DataLengthResult->GetSuccess() == true)
					{
						auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
						auto NameResult{Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, NameLength)};
						
						Result->GetValue()->Append("Name", NameResult->GetValue());
						if(NameResult->GetSuccess() == true)
						{
							auto DataLength{std::experimental::any_cast< std::uint32_t >(DataLengthResult->GetAny())};
							auto DataType{std::experimental::any_cast< std::string >(DataTypeResult->GetAny("Interpretation"))};
							auto DataResult{Get_ASF_MetadataObject_DescriptionRecord_Data(Buffer, DataLength, DataType)};
							
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

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::unique_ptr< Inspection::Result > DataResult;
	
	if(DataType == "Unicode string")
	{
		DataResult = Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length);
	}
	else if(DataType == "Byte array")
	{
		DataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length.GetBytes());
	}
	else if(DataType == "Boolean")
	{
		assert(Length == Inspection::Length(2ull, 0));
		DataResult = Get_ASF_Boolean_16Bit_LittleEndian(Buffer);
	}
	else if(DataType == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		DataResult = Get_UnsignedInteger_32Bit_LittleEndian(Buffer);
	}
	else if(DataType == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		DataResult = Get_UnsignedInteger_64Bit_LittleEndian(Buffer);
	}
	else if(DataType == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		DataResult = Get_UnsignedInteger_16Bit_LittleEndian(Buffer);
	}
	Result->SetValue(DataResult->GetValue());
	Result->SetSuccess(DataResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObject_DescriptionRecord_DataType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto DataTypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(DataTypeResult->GetValue());
	if(DataTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DataType{std::experimental::any_cast< std::uint16_t >(DataTypeResult->GetAny())};
		
		if(DataType == 0x0000)
		{
			Result->GetValue()->Append("Interpretation", "Unicode string"s);
		}
		else if(DataType == 0x0001)
		{
			Result->GetValue()->Append("Interpretation", "Byte array"s);
		}
		else if(DataType == 0x0002)
		{
			Result->GetValue()->Append("Interpretation", "Boolean"s);
		}
		else if(DataType == 0x0003)
		{
			Result->GetValue()->Append("Interpretation", "Unsigned integer 32bit"s);
		}
		else if(DataType == 0x0004)
		{
			Result->GetValue()->Append("Interpretation", "Unsigned integer 64bit"s);
		}
		else if(DataType == 0x0005)
		{
			Result->GetValue()->Append("Interpretation", "Unsigned integer 16bit"s);
		}
		else
		{
			Result->GetValue()->Append("Interpretation", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Get_ASF_MetadataObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->SetValue(ObjectHeaderResult->GetValue());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto Size{std::experimental::any_cast< std::uint64_t >(ObjectHeaderResult->GetAny("Size"))};
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		std::unique_ptr< Inspection::Result > ObjectDataResult;
		
		if(GUID == g_ASF_HeaderObjectGUID)
		{
			ObjectDataResult = Get_ASF_HeaderObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_FilePropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_FilePropertiesObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_StreamPropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_StreamPropertiesObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_CodecListObjectGUID)
		{
			ObjectDataResult = Get_ASF_CodecListObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_HeaderExtensionObjectGUID)
		{
			ObjectDataResult = Get_ASF_HeaderExtensionObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_LanguageListObjectGUID)
		{
			ObjectDataResult = Get_ASF_LanguageListObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_ExtendedStreamPropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_ExtendedStreamPropertiesObjectData(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_MetadataObjectGUID)
		{
			ObjectDataResult = Get_ASF_MetadataObjectData(Buffer);
			Result->GetValue()->Append(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == g_ASF_PaddingObjectGUID)
		{
			auto Length{Inspection::Length(Size) - ObjectHeaderResult->GetValue()->GetLength()};
			
			ObjectDataResult = Get_Bits_Unset_EndedByLength(Buffer, Length);
			Result->GetValue()->Append("Data", to_string_cast(ObjectDataResult->GetLength().GetBytes()) + '.' + to_string_cast(ObjectDataResult->GetLength().GetBits()) + " bytes of zeroed data");
		}
		else
		{
			ObjectDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size - ObjectHeaderResult->GetValue()->GetLength().GetBytes());
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
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

std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesFlags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
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

std::unique_ptr< Inspection::Result > Get_ASF_StreamPropertiesObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->Append(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		
		if(GUID == g_ASF_StreamPropertiesObjectGUID)
		{
			auto HeaderObjectDataResult{Get_ASF_StreamPropertiesObjectData(Buffer)};
			
			Result->GetValue()->Append(HeaderObjectDataResult->GetValue()->GetValues());
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
	auto Result{Inspection::InitializeResult(false, Buffer)};
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
						auto FlagsResult{Get_ASF_StreamPropertiesFlags(Buffer)};
						
						Result->GetValue()->Append("Flags", FlagsResult->GetValue());
						if(FlagsResult->GetSuccess() == true)
						{
							auto ReservedResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->Append("Reserved", ReservedResult->GetValue());
							if(ReservedResult->GetSuccess() == true)
							{
								auto TypeSpecificDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, std::experimental::any_cast< std::uint32_t >(TypeSpecificDataLengthResult->GetAny()))};
								
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
				Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
				auto ASFFileResult{Get_ASF_File(Buffer)};
				
				ASFFileResult->GetValue()->SetName("ASFFile");
				PrintValue(ASFFileResult->GetValue());
				if(ASFFileResult->GetSuccess() == false)
				{
					std::cerr << "The file is not an ASF file." << std::endl;
				}
				else
				{
					auto Rest{Buffer.GetLength() - Buffer.GetPosition()};
					
					if(Rest > 0ull)
					{
						std::cerr << "There are " << Rest.GetBytes() << "." << Rest.GetBits() << " bytes and bits after the data." << std::endl;
					}
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
