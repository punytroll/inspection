#include <experimental/optional>
#include <fstream>

#include "enumeration.h"
#include "getter_descriptor.h"
#include "getter_repository.h"
#include "getters.h"
#include "not_implemented_exception.h"
#include "result.h"
#include "xml_puny_dom.h"

namespace Inspection
{
	class EvaluationResult
	{
	public:
		std::experimental::optional< bool > AbortEvaluation;
		std::experimental::optional< bool > DataIsValid;
		std::experimental::optional< bool > EngineError;
		std::experimental::optional< bool > StructureIsValid;
	};
	
	class ReferencePartDescriptor
	{
	public:
		enum class Type
		{
			Result,
			Sub,
			Tag
		};
		
		Inspection::ReferencePartDescriptor::Type Type;
		std::string DetailName;
	};
	
	class ValueDescriptor
	{
	public:
		~ValueDescriptor(void)
		{
			for(auto ReferencePartDescriptor : ReferencePartDescriptors)
			{
				delete ReferencePartDescriptor;
			}
		}
		
		std::experimental::optional< std::string > LiteralValue;
		std::vector< Inspection::ReferencePartDescriptor * > ReferencePartDescriptors;
	};
	
	class ParameterDescriptor
	{
	public:
		std::string Name;
		std::experimental::optional< std::string > Type;
		Inspection::ValueDescriptor ValueDescriptor;
	};
	
	enum class AppendType
	{
		AppendValue,
		AppendSubValues,
		Set
	};
	
	enum class InterpretType
	{
		ApplyEnumeration
	};
	
	class LengthDescriptor
	{
	public:
		Inspection::ValueDescriptor BytesValueDescriptor;
		Inspection::ValueDescriptor BitsValueDescriptor;
	};

	class PartDescriptor
	{
	public:
		PartDescriptor(void) :
			LengthDescriptor{nullptr}
		{
		}
		
		~PartDescriptor(void)
		{
			for(auto ParameterDescriptor : ParameterDescriptors)
			{
				delete ParameterDescriptor;
			}
			delete LengthDescriptor;
		}
		
		Inspection::LengthDescriptor * LengthDescriptor;
		std::vector< Inspection::ParameterDescriptor * > ParameterDescriptors;
		std::vector< std::string > PathParts;
		Inspection::AppendType ValueAppendType;
		std::string ValueName;
	};

	class InterpretDescriptor
	{
	public:
		InterpretDescriptor(void) :
			Enumeration{nullptr}
		{
		}
		
		~InterpretDescriptor(void)
		{
			delete Enumeration;
			Enumeration = nullptr;
		}
		
		std::vector< std::string > PathParts;
		Inspection::InterpretType Type;
		Inspection::Enumeration * Enumeration;
	};
	
	void ApplyTags(const std::vector< Inspection::Enumeration::Element::Tag * > & Tags, std::shared_ptr< Inspection::Value > Target)
	{
		for(auto Tag : Tags)
		{
			if(Tag->Type)
			{
				if(Tag->Type.value() == "string")
				{
					assert(Tag->Value);
					Target->AddTag(Tag->Name, Tag->Value.value());
				}
				else if(Tag->Type.value() == "boolean")
				{
					assert(Tag->Value);
					Target->AddTag(Tag->Name, from_string_cast< bool >(Tag->Value.value()));
				}
				else if(Tag->Type.value() == "nothing")
				{
					assert(!Tag->Value);
					Target->AddTag(Tag->Name, nullptr);
				}
				else if(Tag->Type.value() == "single precision real")
				{
					assert(Tag->Value);
					Target->AddTag(Tag->Name, from_string_cast< float >(Tag->Value.value()));
				}
				else if(Tag->Type.value() == "unsigned integer 8bit")
				{
					assert(Tag->Value);
					Target->AddTag(Tag->Name, from_string_cast< std::uint8_t >(Tag->Value.value()));
				}
				else if(Tag->Type.value() == "unsigned integer 32bit")
				{
					assert(Tag->Value);
					Target->AddTag(Tag->Name, from_string_cast< std::uint32_t >(Tag->Value.value()));
				}
				else
				{
					assert(false);
				}
			}
			else
			{
				Target->AddTag(Tag->Name);
			}
		}
	}
	
	template< typename Type >
	bool ApplyEnumeration(Inspection::Enumeration * Enumeration, std::shared_ptr< Inspection::Value > Target)
	{
		bool Result{false};
		auto BaseValueString{to_string_cast(std::experimental::any_cast< const Type & >(Target->GetAny()))};
		auto ElementIterator{std::find_if(Enumeration->Elements.begin(), Enumeration->Elements.end(), [BaseValueString](auto Element){ return Element->BaseValue == BaseValueString; })};
		
		if(ElementIterator != Enumeration->Elements.end())
		{
			ApplyTags((*ElementIterator)->Tags, Target);
			Result = (*ElementIterator)->Valid;
		}
		else
		{
			if(Enumeration->FallbackElement != nullptr)
			{
				ApplyTags(Enumeration->FallbackElement->Tags, Target);
				Target->AddTag("error", "Could find no enumeration element for the base value \"" + BaseValueString + "\".");
				Result = Enumeration->FallbackElement->Valid;
			}
			else
			{
				Target->AddTag("error", "Could find neither an enumarion element nor an enumeration fallback element for the base value \"" + BaseValueString + "\".");
				Result = false;
			}
		}
		
		return Result;
	}
	
	const std::experimental::any & GetAnyByReference(const std::vector< Inspection::ReferencePartDescriptor * > & ReferencePartDescriptors, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		std::shared_ptr< Inspection::Value > Value{nullptr};
		
		for(auto ReferencePartDescriptor : ReferencePartDescriptors)
		{
			switch(ReferencePartDescriptor->Type)
			{
			case Inspection::ReferencePartDescriptor::Type::Result:
				{
					Value = Result->GetValue();
					
					break;
				}
			case Inspection::ReferencePartDescriptor::Type::Sub:
				{
					Value = Value->GetValue(ReferencePartDescriptor->DetailName);
					
					break;
				}
			case Inspection::ReferencePartDescriptor::Type::Tag:
				{
					Value = Value->GetTag(ReferencePartDescriptor->DetailName);
					
					break;
				}
			}
		}
		assert(Value != nullptr);
		
		return Value->GetAny();
	}
}

Inspection::GetterDescriptor::GetterDescriptor(Inspection::GetterRepository * GetterRepository) :
	_GetterRepository{GetterRepository}
{
}

Inspection::GetterDescriptor::~GetterDescriptor(void)
{
	for(auto InterpretDescriptor : _InterpretDescriptors)
	{
		delete InterpretDescriptor;
	}
	for(auto PartDescriptor : _PartDescriptors)
	{
		delete PartDescriptor;
	}
}

std::unique_ptr< Inspection::Result > Inspection::GetterDescriptor::Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(_HardcodedGetter != nullptr)
		{
			auto HardcodedResult{_HardcodedGetter(Reader)};
			
			Continue = HardcodedResult->GetSuccess();
			Result->SetValue(HardcodedResult->GetValue());
		}
		else if(_HardcodedGetterWithParameters != nullptr)
		{
			auto HardcodedResult{_HardcodedGetterWithParameters(Reader, Parameters)};
			
			Continue = HardcodedResult->GetSuccess();
			Result->SetValue(HardcodedResult->GetValue());
		}
		else
		{
			for(auto ActionIndex = 0ul; (Continue == true) && (ActionIndex < _Actions.size()); ++ActionIndex)
			{
				auto Action{_Actions[ActionIndex]};
				
				switch(Action.first)
				{
				case Inspection::ActionType::Interpret:
					{
						auto InterpretDescriptor{_InterpretDescriptors[Action.second]};
						
						switch(InterpretDescriptor->Type)
						{
						case Inspection::InterpretType::ApplyEnumeration:
							{
								Inspection::Enumeration * Enumeration{nullptr};
								
								if(InterpretDescriptor->Enumeration != nullptr)
								{
									Enumeration = InterpretDescriptor->Enumeration;
								}
								else
								{
									Enumeration = _GetterRepository->GetEnumeration(InterpretDescriptor->PathParts);
								}
								
								auto Target{Result->GetValue()};
								auto EvaluationResult{_ApplyEnumeration(Enumeration, Target)};
								
								if(EvaluationResult.AbortEvaluation)
								{
									if(EvaluationResult.AbortEvaluation.value() == true)
									{
										Continue = false;
									}
								}
								
								break;
							}
						}
						
						break;
					}
				case Inspection::ActionType::Read:
					{
						auto PartDescriptor{_PartDescriptors[Action.second]};
						Inspection::Reader * PartReader{nullptr};
						
						if(PartDescriptor->LengthDescriptor != nullptr)
						{
							std::uint64_t Bytes{0};
							
							if(PartDescriptor->LengthDescriptor->BytesValueDescriptor.LiteralValue)
							{
								Bytes = from_string_cast< std::uint64_t >(PartDescriptor->LengthDescriptor->BytesValueDescriptor.LiteralValue.value());
							}
							else if(PartDescriptor->LengthDescriptor->BytesValueDescriptor.ReferencePartDescriptors.size() > 0)
							{
								auto & BytesAny{GetAnyByReference(PartDescriptor->LengthDescriptor->BytesValueDescriptor.ReferencePartDescriptors, Result, Parameters)};
								
								if(BytesAny.type() == typeid(std::uint8_t))
								{
									Bytes = std::experimental::any_cast< std::uint8_t >(BytesAny);
								}
								else if(BytesAny.type() == typeid(std::uint16_t))
								{
									Bytes = std::experimental::any_cast< std::uint16_t >(BytesAny);
								}
								else if(BytesAny.type() == typeid(std::uint32_t))
								{
									Bytes = std::experimental::any_cast< std::uint32_t >(BytesAny);
								}
								else
								{
									assert(false);
								}
							}
							
							std::uint64_t Bits{0};
							
							if(PartDescriptor->LengthDescriptor->BitsValueDescriptor.LiteralValue)
							{
								Bits = from_string_cast< std::uint64_t >(PartDescriptor->LengthDescriptor->BitsValueDescriptor.LiteralValue.value());
							}
							else if(PartDescriptor->LengthDescriptor->BitsValueDescriptor.ReferencePartDescriptors.size() > 0)
							{
								auto & BitsAny{GetAnyByReference(PartDescriptor->LengthDescriptor->BitsValueDescriptor.ReferencePartDescriptors, Result, Parameters)};
								
								if(BitsAny.type() == typeid(std::uint8_t))
								{
									Bits = std::experimental::any_cast< std::uint8_t >(BitsAny);
								}
								else if(BitsAny.type() == typeid(std::uint16_t))
								{
									Bits = std::experimental::any_cast< std::uint16_t >(BitsAny);
								}
								else if(BitsAny.type() == typeid(std::uint32_t))
								{
									Bits = std::experimental::any_cast< std::uint32_t >(BitsAny);
								}
								else
								{
									assert(false);
								}
							}
							
							Inspection::Length Length{Bytes, Bits};
							
							if(Reader.Has(Length) == true)
							{
								PartReader = new Inspection::Reader{Reader, Length};
							}
							else
							{
								Result->GetValue()->AddTag("error", "At least " + to_string_cast(Length) + " bytes and bits are necessary to read this part.");
								Continue = false;
							}
						}
						else
						{
							PartReader = new Inspection::Reader{Reader};
						}
						if(PartReader != nullptr)
						{
							std::unordered_map< std::string, std::experimental::any > Parameters;
							
							for(auto ParameterDescriptor : PartDescriptor->ParameterDescriptors)
							{
								if(ParameterDescriptor->ValueDescriptor.LiteralValue)
								{
									assert(ParameterDescriptor->Type);
									if(ParameterDescriptor->Type.value() == "string")
									{
										Parameters.emplace(ParameterDescriptor->Name, ParameterDescriptor->ValueDescriptor.LiteralValue.value());
									}
									else
									{
										assert(false);
									}
								}
								else
								{
									assert(ParameterDescriptor->ValueDescriptor.ReferencePartDescriptors.empty() == false);
									
									auto & ValueAny{GetAnyByReference(ParameterDescriptor->ValueDescriptor.ReferencePartDescriptors, Result, Parameters)};
									
									if(!ParameterDescriptor->Type)
									{
										Parameters.emplace(ParameterDescriptor->Name, ValueAny);
									}
									else if(ParameterDescriptor->Type.value() == "unsigned integer 64bit")
									{
										if(ValueAny.type() == typeid(std::uint16_t))
										{
											Parameters.emplace(ParameterDescriptor->Name, static_cast< std::uint64_t >(std::experimental::any_cast< std::uint16_t >(ValueAny)));
										}
										else
										{
											assert(false);
										}
									}
									else
									{
										assert(false);
									}
								}
							}
							
							auto PartResult{g_GetterRepository.Get(PartDescriptor->PathParts, *PartReader, Parameters)};
							
							Continue = PartResult->GetSuccess();
							switch(PartDescriptor->ValueAppendType)
							{
							case Inspection::AppendType::AppendValue:
								{
									Result->GetValue()->AppendValue(PartDescriptor->ValueName, PartResult->GetValue());
									
									break;
								}
							case Inspection::AppendType::AppendSubValues:
								{
									Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
									
									break;
								}
							case Inspection::AppendType::Set:
								{
									Result->SetValue(PartResult->GetValue());
									
									break;
								}
							}
							Reader.AdvancePosition(PartReader->GetConsumedLength());
						}
						delete PartReader;
						
						break;
					}
				}
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

Inspection::EvaluationResult Inspection::GetterDescriptor::_ApplyEnumeration(Inspection::Enumeration * Enumeration, std::shared_ptr< Inspection::Value > Target)
{
	assert(Enumeration != nullptr);
	
	Inspection::EvaluationResult Result;
	
	Result.StructureIsValid = true;
	if(Enumeration->BaseType == "string")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::string >(Enumeration, Target);
	}
	else if(Enumeration->BaseType == "unsigned integer 8bit")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint8_t >(Enumeration, Target);
	}
	else if(Enumeration->BaseType == "unsigned integer 16bit")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint16_t >(Enumeration, Target);
	}
	else if(Enumeration->BaseType == "unsigned integer 32bit")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint32_t >(Enumeration, Target);
	}
	else
	{
		Target->AddTag("error", "Could not handle the enumeration base type \"" + Enumeration->BaseType + "\".");
		Result.EngineError = true;
	}
	
	return Result;
}

void Inspection::GetterDescriptor::LoadGetterDescription(const std::string & GetterPath)
{
	std::ifstream InputFileStream{GetterPath};
	XML::Document Document{InputFileStream};
	auto DocumentElement{Document.GetDocumentElement()};
	
	assert(DocumentElement != nullptr);
	assert(DocumentElement->GetName() == "getter");
	for(auto GetterChildNode : DocumentElement->GetChilds())
	{
		if(GetterChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto GetterChildElement{dynamic_cast< const XML::Element * >(GetterChildNode)};
			
			if(GetterChildElement->GetName() == "hardcoded-getter")
			{
				assert(GetterChildElement->GetChilds().size() == 1);
				
				auto HardcodedGetterText{dynamic_cast< const XML::Text * >(GetterChildElement->GetChild(0))};
				
				assert(HardcodedGetterText != nullptr);
				if(HardcodedGetterText->GetText() == "Get_APE_Flags")
				{
					_HardcodedGetterWithParameters = Inspection::Get_APE_Flags;
				}
				else if(HardcodedGetterText->GetText() == "Get_APE_Item")
				{
					_HardcodedGetterWithParameters = Inspection::Get_APE_Item;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASCII_String_AlphaNumeric_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASCII_String_AlphaNumericOrSpace_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASCII_String_Printable_EndedByLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ASCII_String_Printable_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASCII_String_Printable_EndedByTermination")
				{
					_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByTermination;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_ExtendedContentDescription_ContentDescriptor")
				{
					_HardcodedGetterWithParameters = Get_ASF_ExtendedContentDescription_ContentDescriptor;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_Metadata_DescriptionRecord_Data")
				{
					_HardcodedGetterWithParameters = Get_ASF_Metadata_DescriptionRecord_Data;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_MetadataLibrary_DescriptionRecord_Data")
				{
					_HardcodedGetterWithParameters = Get_ASF_MetadataLibrary_DescriptionRecord_Data;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_Object")
				{
					_HardcodedGetterWithParameters = Get_ASF_Object;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_StreamBitrateProperties_BitrateRecord_Flags")
				{
					_HardcodedGetter = Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_CreationDate")
				{
					_HardcodedGetter = Inspection::Get_ASF_CreationDate;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_GUID")
				{
					_HardcodedGetter = Inspection::Get_ASF_GUID;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_FileProperties_Flags")
				{
					_HardcodedGetter = Inspection::Get_ASF_FileProperties_Flags;
				}
				else if(HardcodedGetterText->GetText() == "Get_BitSet_16Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_BitSet_16Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_Boolean_1Bit")
				{
					_HardcodedGetter = Inspection::Get_Boolean_1Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_Data_SetOrUnset_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_Data_SetOrUnset_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_Data_Unset_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_Data_Unset_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_FLAC_StreamInfoBlock_BitsPerSample")
				{
					_HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_BitsPerSample;
				}
				else if(HardcodedGetterText->GetText() == "Get_FLAC_StreamInfoBlock_NumberOfChannels")
				{
					_HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_NumberOfChannels;
				}
				else if(HardcodedGetterText->GetText() == "Get_GUID_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_GUID_LittleEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_Language")
				{
					_HardcodedGetter = Inspection::Get_ID3_2_2_Language;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_Language")
				{
					_HardcodedGetter = Inspection::Get_ID3_2_3_Language;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_Frame_Header_Flags")
				{
					_HardcodedGetter = Inspection::Get_ID3_2_3_Frame_Header_Flags;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_Language")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_4_Language;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_ReplayGainAdjustment")
				{
					_HardcodedGetter = Inspection::Get_ID3_ReplayGainAdjustment;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit")
				{
					_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_IEC_60908_1999_TableOfContents_Track")
				{
					_HardcodedGetterWithParameters = Inspection::Get_IEC_60908_1999_TableOfContents_Track;
				}
				else if(HardcodedGetterText->GetText() == "Get_IEC_60908_1999_TableOfContents_Tracks")
				{
					_HardcodedGetterWithParameters = Inspection::Get_IEC_60908_1999_TableOfContents_Tracks;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTermination")
				{
					_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength")
				{
					_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength")
				{
					_HardcodedGetterWithParameters = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_IEEE_60559_2011_binary32")
				{
					_HardcodedGetter = Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32;
				}
				else if(HardcodedGetterText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask")
				{
					_HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask;
				}
				else if(HardcodedGetterText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat")
				{
					_HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat;
				}
				else if(HardcodedGetterText->GetText() == "Get_SignedInteger_8Bit")
				{
					_HardcodedGetter = Inspection::Get_SignedInteger_8Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_SignedInteger_32Bit_RiceEncoded")
				{
					_HardcodedGetterWithParameters = Inspection::Get_SignedInteger_32Bit_RiceEncoded;
				}
				else if(HardcodedGetterText->GetText() == "Get_SignedInteger_BigEndian")
				{
					_HardcodedGetterWithParameters = Inspection::Get_SignedInteger_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_String_ASCII_Alphabetic_ByTemplate")
				{
					_HardcodedGetterWithParameters = Inspection::Get_String_ASCII_Alphabetic_ByTemplate;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_1Bit")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_1Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_2Bit")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_2Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_3Bit")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_3Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_4Bit")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_4Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_7Bit")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_7Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_8Bit")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_8Bit;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_16Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_16Bit_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_LittleEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_20Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_20Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_24Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_24Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_32Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_32Bit_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_LittleEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_36Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_36Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_64Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_64Bit_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_LittleEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_BigEndian")
				{
					_HardcodedGetterWithParameters = Inspection::Get_UnsignedInteger_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_Vorbis_CommentHeader_UserComment")
				{
					_HardcodedGetterWithParameters = Inspection::Get_Vorbis_CommentHeader_UserComment;
				}
				else
				{
					throw std::domain_error{HardcodedGetterText->GetText()};
				}
			}
			else if(GetterChildElement->GetName() == "interpret")
			{
				auto InterpretDescriptor{new Inspection::InterpretDescriptor{}};
				
				for(auto InterpretChildNode : GetterChildElement->GetChilds())
				{
					if(InterpretChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto InterpretChildElement{dynamic_cast< const XML::Element * >(InterpretChildNode)};
						
						if(InterpretChildElement->GetName() == "apply-enumeration")
						{
							InterpretDescriptor->Type = Inspection::InterpretType::ApplyEnumeration;
							for(auto ApplyEnumerationChildNode : InterpretChildElement->GetChilds())
							{
								if(ApplyEnumerationChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto ApplyEnumerationChildElement{dynamic_cast< const XML::Element * >(ApplyEnumerationChildNode)};
									
									if(ApplyEnumerationChildElement->GetName() == "part")
									{
										assert(ApplyEnumerationChildElement->GetChilds().size() == 1);
										
										auto PartText{dynamic_cast< const XML::Text * >(ApplyEnumerationChildElement->GetChild(0))};
										
										assert(PartText != nullptr);
										InterpretDescriptor->PathParts.push_back(PartText->GetText());
									}
									else if(ApplyEnumerationChildElement->GetName() == "enumeration")
									{
										InterpretDescriptor->Enumeration = new Enumeration{};
										InterpretDescriptor->Enumeration->Load(ApplyEnumerationChildElement);
									}
									else
									{
										throw std::domain_error{ApplyEnumerationChildElement->GetName()};
									}
								}
							}
						}
						else
						{
							throw std::domain_error{InterpretChildElement->GetName()};
						}
					}
				}
				_Actions.push_back(std::make_pair(Inspection::ActionType::Interpret, _InterpretDescriptors.size()));
				_InterpretDescriptors.push_back(InterpretDescriptor);
			}
			else if(GetterChildElement->GetName() == "part")
			{
				auto PartDescriptor{new Inspection::PartDescriptor{}};
				
				for(auto PartChildNode : GetterChildElement->GetChilds())
				{
					if(PartChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto PartChildElement{dynamic_cast< const XML::Element * >(PartChildNode)};
						
						if(PartChildElement->GetName() == "getter")
						{
							for(auto PartGetterChildNode : PartChildElement->GetChilds())
							{
								if(PartGetterChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartGetterChildElement{dynamic_cast< const XML::Element * >(PartGetterChildNode)};
									
									if(PartGetterChildElement->GetName() == "part")
									{
										assert(PartGetterChildElement->GetChilds().size() == 1);
										
										auto PartText{dynamic_cast< const XML::Text * >(PartGetterChildElement->GetChild(0))};
										
										assert(PartText != nullptr);
										PartDescriptor->PathParts.push_back(PartText->GetText());
									}
									else
									{
										throw std::domain_error{PartGetterChildElement->GetName()};
									}
								}
							}
						}
						else if(PartChildElement->GetName() == "length")
						{
							PartDescriptor->LengthDescriptor = new Inspection::LengthDescriptor{};
							for(auto PartLengthChildNode : PartChildElement->GetChilds())
							{
								if(PartLengthChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartLengthChildElement{dynamic_cast< const XML::Element * >(PartLengthChildNode)};
									
									if(PartLengthChildElement->GetName() == "bytes")
									{
										if((PartLengthChildElement->GetChilds().size() == 1) && (PartLengthChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text))
										{
											assert(PartLengthChildElement->GetChilds().size() == 1);
											
											auto BytesText{dynamic_cast< const XML::Text * >(PartLengthChildElement->GetChild(0))};
											
											assert(BytesText != nullptr);
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.LiteralValue = BytesText->GetText();
										}
										else
										{
											for(auto PartLengthBytesChildNode : PartLengthChildElement->GetChilds())
											{
												if(PartLengthBytesChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto ReferencePartDescriptor{new Inspection::ReferencePartDescriptor{}};
													auto PartLengthBytesChildElement{dynamic_cast< const XML::Element * >(PartLengthBytesChildNode)};
													
													if(PartLengthBytesChildElement->GetName() == "result")
													{
														assert(PartLengthBytesChildElement->GetChilds().size() == 0);
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Result;
													}
													else if(PartLengthBytesChildElement->GetName() == "sub")
													{
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Sub;
														assert(PartLengthBytesChildElement->GetChilds().size() == 1);
														
														auto SubText{dynamic_cast< const XML::Text * >(PartLengthBytesChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartLengthBytesChildElement->GetName() == "tag")
													{
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Tag;
														assert(PartLengthBytesChildElement->GetChilds().size() == 1);
														
														auto TagText{dynamic_cast< const XML::Text * >(PartLengthBytesChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ReferencePartDescriptor->DetailName = TagText->GetText();
													}
													else
													{
														throw std::domain_error{PartLengthBytesChildElement->GetName()};
													}
													PartDescriptor->LengthDescriptor->BytesValueDescriptor.ReferencePartDescriptors.push_back(ReferencePartDescriptor);
												}
											}
										}
									}
									else if(PartLengthChildElement->GetName() == "bits")
									{
										if((PartLengthChildElement->GetChilds().size() == 1) && (PartLengthChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text))
										{
											assert(PartLengthChildElement->GetChilds().size() == 1);
											
											auto BitsText{dynamic_cast< const XML::Text * >(PartLengthChildElement->GetChild(0))};
											
											assert(BitsText != nullptr);
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.LiteralValue = BitsText->GetText();
										}
										else
										{
											for(auto PartLengthBitsChildNode : PartLengthChildElement->GetChilds())
											{
												if(PartLengthBitsChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto ReferencePartDescriptor{new Inspection::ReferencePartDescriptor{}};
													auto PartLengthBitsChildElement{dynamic_cast< const XML::Element * >(PartLengthBitsChildNode)};
													
													if(PartLengthBitsChildElement->GetName() == "result")
													{
														assert(PartLengthBitsChildElement->GetChilds().size() == 0);
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Result;
													}
													else if(PartLengthBitsChildElement->GetName() == "sub")
													{
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Sub;
														assert(PartLengthBitsChildElement->GetChilds().size() == 1);
														
														auto SubText{dynamic_cast< const XML::Text * >(PartLengthBitsChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartLengthBitsChildElement->GetName() == "tag")
													{
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Tag;
														assert(PartLengthBitsChildElement->GetChilds().size() == 1);
														
														auto TagText{dynamic_cast< const XML::Text * >(PartLengthBitsChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ReferencePartDescriptor->DetailName = TagText->GetText();
													}
													else
													{
														throw std::domain_error{PartLengthBitsChildElement->GetName()};
													}
													PartDescriptor->LengthDescriptor->BitsValueDescriptor.ReferencePartDescriptors.push_back(ReferencePartDescriptor);
												}
											}
										}
									}
									else
									{
										throw std::domain_error{PartLengthChildElement->GetName()};
									}
								}
							}
						}
						else if(PartChildElement->GetName() == "parameters")
						{
							for(auto PartParametersChildNode : PartChildElement->GetChilds())
							{
								if(PartParametersChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartParametersChildElement{dynamic_cast< const XML::Element * >(PartParametersChildNode)};
									
									if(PartParametersChildElement->GetName() == "parameter")
									{
										auto ParameterDescriptor{new Inspection::ParameterDescriptor{}};
										
										assert(PartParametersChildElement->HasAttribute("name") == true);
										ParameterDescriptor->Name = PartParametersChildElement->GetAttribute("name");
										if((PartParametersChildElement->GetChilds().size() == 1) && (PartParametersChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text))
										{
											assert(PartParametersChildElement->HasAttribute("type") == true);
											ParameterDescriptor->Type = PartParametersChildElement->GetAttribute("type");
											
											auto ParameterText{dynamic_cast< const XML::Text * >(PartParametersChildElement->GetChild(0))};
											
											assert(ParameterText != nullptr);
											ParameterDescriptor->ValueDescriptor.LiteralValue = ParameterText->GetText();
										}
										else
										{
											if(PartParametersChildElement->HasAttribute("type") == true)
											{
												ParameterDescriptor->Type = PartParametersChildElement->GetAttribute("type");
											}
											for(auto PartParametersParameterChildNode : PartParametersChildElement->GetChilds())
											{
												if(PartParametersParameterChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto ReferencePartDescriptor{new Inspection::ReferencePartDescriptor{}};
													auto PartParametersParameterChildElement{dynamic_cast< const XML::Element * >(PartParametersParameterChildNode)};
													
													if(PartParametersParameterChildElement->GetName() == "result")
													{
														assert(PartParametersParameterChildElement->GetChilds().size() == 0);
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Result;
													}
													else if(PartParametersParameterChildElement->GetName() == "sub")
													{
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Sub;
														assert(PartParametersParameterChildElement->GetChilds().size() == 1);
														
														auto SubText{dynamic_cast< const XML::Text * >(PartParametersParameterChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartParametersParameterChildElement->GetName() == "tag")
													{
														ReferencePartDescriptor->Type = Inspection::ReferencePartDescriptor::Type::Tag;
														assert(PartParametersParameterChildElement->GetChilds().size() == 1);
														
														auto TagText{dynamic_cast< const XML::Text * >(PartParametersParameterChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ReferencePartDescriptor->DetailName = TagText->GetText();
													}
													else
													{
														throw std::domain_error{PartParametersParameterChildElement->GetName()};
													}
													ParameterDescriptor->ValueDescriptor.ReferencePartDescriptors.push_back(ReferencePartDescriptor);
												}
											}
										}
										PartDescriptor->ParameterDescriptors.push_back(ParameterDescriptor);
									}
									else
									{
										throw std::domain_error{PartParametersChildElement->GetName()};
									}
								}
							}
						}
						else if(PartChildElement->GetName() == "value")
						{
							for(auto PartValueChildNode : PartChildElement->GetChilds())
							{
								if(PartValueChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartValueChildElement{dynamic_cast< const XML::Element * >(PartValueChildNode)};
									
									if(PartValueChildElement->GetName() == "append-value")
									{
										PartDescriptor->ValueAppendType = Inspection::AppendType::AppendValue;
										for(auto PartValueAppendChildNode : PartValueChildElement->GetChilds())
										{
											if(PartValueAppendChildNode->GetNodeType() == XML::NodeType::Element)
											{
												auto PartValueAppendChildElement{dynamic_cast< const XML::Element * >(PartValueAppendChildNode)};
												
												if(PartValueAppendChildElement->GetName() == "name")
												{
													assert(PartValueAppendChildElement->GetChilds().size() == 1);
													
													auto NameText{dynamic_cast< const XML::Text * >(PartValueAppendChildElement->GetChild(0))};
													
													assert(NameText != nullptr);
													PartDescriptor->ValueName = NameText->GetText();
												}
												else
												{
													throw std::domain_error{PartValueAppendChildElement->GetName()};
												}
											}
										}
									}
									else if(PartValueChildElement->GetName() == "append-sub-values")
									{
										assert(PartValueChildElement->GetChilds().size() == 0);
										PartDescriptor->ValueAppendType = Inspection::AppendType::AppendSubValues;
									}
									else if(PartValueChildElement->GetName() == "set")
									{
										assert(PartValueChildElement->GetChilds().size() == 0);
										PartDescriptor->ValueAppendType = Inspection::AppendType::Set;
									}
									else
									{
										throw std::domain_error{PartValueChildElement->GetName()};
									}
								}
							}
						}
						else
						{
							throw std::domain_error{PartChildElement->GetName()};
						}
					}
				}
				_Actions.push_back(std::make_pair(Inspection::ActionType::Read, _PartDescriptors.size()));
				_PartDescriptors.push_back(PartDescriptor);
			}
			else
			{
				throw std::domain_error{GetterChildElement->GetName()};
			}
		}
	}
}
