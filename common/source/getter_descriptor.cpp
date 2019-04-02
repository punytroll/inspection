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
	
	class ReferenceDescriptor
	{
	public:
		class PartDescriptor
		{
		public:
			enum class Type
			{
				Result,
				Sub,
				Tag
			};
			
			Inspection::ReferenceDescriptor::PartDescriptor::Type Type;
			std::string DetailName;
		};
		
		std::experimental::optional< std::string > CastToType;
		std::vector< Inspection::ReferenceDescriptor::PartDescriptor > PartDescriptors;
	};
	
	class GetterReferenceDescriptor
	{
	public:
		std::vector< std::string > Parts;
	};
	
	class ValueDescriptor
	{
	public:
		ValueDescriptor(void)
		{
		}
		
		~ValueDescriptor(void)
		{
		}
		
		std::string Type;
		union
		{
			Inspection::GetterReferenceDescriptor GetterReferenceDescriptor;
			std::experimental::optional< std::string > LiteralValue;
			std::experimental::optional< Inspection::ReferenceDescriptor > ReferenceDescriptor;
		};
	};
	
	class ActualParameterDescriptor
	{
	public:
		std::string Name;
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

	class PartDescriptor
	{
	public:
		~PartDescriptor(void)
		{
			for(auto ActualParameterDescriptor : ActualParameterDescriptors)
			{
				delete ActualParameterDescriptor;
			}
		}
		
		std::vector< Inspection::ActualParameterDescriptor * > ActualParameterDescriptors;
		std::experimental::optional< Inspection::LengthDescriptor > LengthDescriptor;
		std::vector< std::string > PathParts;
		Inspection::AppendType ValueAppendType;
		std::string ValueName;
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
	
	const std::experimental::any & GetAnyByReference(Inspection::ReferenceDescriptor & ReferenceDescriptor, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		std::shared_ptr< Inspection::Value > Value{nullptr};
		
		for(auto & PartDescriptor : ReferenceDescriptor.PartDescriptors)
		{
			switch(PartDescriptor.Type)
			{
			case Inspection::ReferenceDescriptor::PartDescriptor::Type::Result:
				{
					Value = Result->GetValue();
					
					break;
				}
			case Inspection::ReferenceDescriptor::PartDescriptor::Type::Sub:
				{
					Value = Value->GetValue(PartDescriptor.DetailName);
					
					break;
				}
			case Inspection::ReferenceDescriptor::PartDescriptor::Type::Tag:
				{
					Value = Value->GetTag(PartDescriptor.DetailName);
					
					break;
				}
			}
		}
		assert(Value != nullptr);
		
		return Value->GetAny();
	}
	
	template< typename Type >
	Type GetValueFromValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const std::unique_ptr< Inspection::Result > & GetterResult, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		Type Result{};
		
		if(ValueDescriptor.Type == "literal")
		{
			assert(ValueDescriptor.LiteralValue);
			Result = from_string_cast< Type >(ValueDescriptor.LiteralValue.value());
		}
		else if(ValueDescriptor.Type == "reference")
		{
			assert(ValueDescriptor.ReferenceDescriptor);
			
			auto & Any{GetAnyByReference(ValueDescriptor.ReferenceDescriptor.value(), GetterResult, Parameters)};
			
			if(Any.type() == typeid(std::uint8_t))
			{
				Result = std::experimental::any_cast< std::uint8_t >(Any);
			}
			else if(Any.type() == typeid(std::uint16_t))
			{
				Result = std::experimental::any_cast< std::uint16_t >(Any);
			}
			else if(Any.type() == typeid(std::uint32_t))
			{
				Result = std::experimental::any_cast< std::uint32_t >(Any);
			}
			else
			{
				assert(false);
			}
		}
		else if(ValueDescriptor.Type == "string")
		{
			assert(ValueDescriptor.LiteralValue);
			Result = from_string_cast< Type >(ValueDescriptor.LiteralValue.value());
		}
		else
		{
			assert(false);
		}
		
		return Result;
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
						
						if(PartDescriptor->LengthDescriptor)
						{
							auto Bytes{GetValueFromValueDescriptor< std::uint64_t >(PartDescriptor->LengthDescriptor->BytesValueDescriptor, Result, Parameters)};
							auto Bits{GetValueFromValueDescriptor< std::uint64_t >(PartDescriptor->LengthDescriptor->BitsValueDescriptor, Result, Parameters)};
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
							
							for(auto ActualParameterDescriptor : PartDescriptor->ActualParameterDescriptors)
							{
								if(ActualParameterDescriptor->ValueDescriptor.Type == "string")
								{
									Parameters.emplace(ActualParameterDescriptor->Name, GetValueFromValueDescriptor< std::string >(ActualParameterDescriptor->ValueDescriptor, Result, Parameters));
								}
								else if(ActualParameterDescriptor->ValueDescriptor.Type == "reference")
								{
									assert(ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor);
									
									auto & ValueAny{GetAnyByReference(ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor.value(), Result, Parameters)};
									
									if(!ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor->CastToType)
									{
										Parameters.emplace(ActualParameterDescriptor->Name, ValueAny);
									}
									else if(ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor->CastToType.value() == "unsigned integer 64bit")
									{
										if(ValueAny.type() == typeid(std::uint16_t))
										{
											Parameters.emplace(ActualParameterDescriptor->Name, static_cast< std::uint64_t >(std::experimental::any_cast< std::uint16_t >(ValueAny)));
										}
										else if(ValueAny.type() == typeid(std::uint32_t))
										{
											Parameters.emplace(ActualParameterDescriptor->Name, static_cast< std::uint64_t >(std::experimental::any_cast< std::uint32_t >(ValueAny)));
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
								else if(ActualParameterDescriptor->ValueDescriptor.Type == "getter-reference")
								{
									Parameters.emplace(ActualParameterDescriptor->Name, ActualParameterDescriptor->ValueDescriptor.GetterReferenceDescriptor.Parts);
								}
								else
								{
									assert(false);
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
				else if(HardcodedGetterText->GetText() == "Get_Array_EndedByNumberOfElements")
				{
					_HardcodedGetterWithParameters = Inspection::Get_Array_EndedByNumberOfElements;
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
										throw std::domain_error{"/getter/interpret/apply-enumeration/" + ApplyEnumerationChildElement->GetName() + " not allowed."};
									}
								}
							}
						}
						else
						{
							throw std::domain_error{"/getter/interpret/" + InterpretChildElement->GetName() + " not allowed."};
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
										throw std::domain_error{"/getter/part/getter/" + PartGetterChildElement->GetName() + " not allowed."};
									}
								}
							}
						}
						else if(PartChildElement->GetName() == "length")
						{
							PartDescriptor->LengthDescriptor.emplace();
							for(auto PartLengthChildNode : PartChildElement->GetChilds())
							{
								if(PartLengthChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartLengthChildElement{dynamic_cast< const XML::Element * >(PartLengthChildNode)};
									
									if(PartLengthChildElement->GetName() == "bytes")
									{
										if((PartLengthChildElement->GetChilds().size() == 1) && (PartLengthChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text))
										{
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.Type = "literal";
											assert(PartLengthChildElement->GetChilds().size() == 1);
											
											auto BytesText{dynamic_cast< const XML::Text * >(PartLengthChildElement->GetChild(0))};
											
											assert(BytesText != nullptr);
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.LiteralValue = BytesText->GetText();
										}
										else
										{
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.Type = "reference";
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.ReferenceDescriptor.emplace();
											for(auto PartLengthBytesChildNode : PartLengthChildElement->GetChilds())
											{
												if(PartLengthBytesChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto & ReferencePartDescriptors{PartDescriptor->LengthDescriptor->BytesValueDescriptor.ReferenceDescriptor->PartDescriptors};
													auto ReferencePartDescriptor{ReferencePartDescriptors.emplace(ReferencePartDescriptors.end())};
													auto PartLengthBytesChildElement{dynamic_cast< const XML::Element * >(PartLengthBytesChildNode)};
													
													if(PartLengthBytesChildElement->GetName() == "result")
													{
														assert(PartLengthBytesChildElement->GetChilds().size() == 0);
														ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Result;
													}
													else if(PartLengthBytesChildElement->GetName() == "sub")
													{
														ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Sub;
														assert(PartLengthBytesChildElement->GetChilds().size() == 1);
														
														auto SubText{dynamic_cast< const XML::Text * >(PartLengthBytesChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartLengthBytesChildElement->GetName() == "tag")
													{
														ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Tag;
														assert(PartLengthBytesChildElement->GetChilds().size() == 1);
														
														auto TagText{dynamic_cast< const XML::Text * >(PartLengthBytesChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ReferencePartDescriptor->DetailName = TagText->GetText();
													}
													else
													{
														throw std::domain_error{"/getter/part/length/bytes/" + PartLengthBytesChildElement->GetName() + " not allowed."};
													}
												}
											}
										}
									}
									else if(PartLengthChildElement->GetName() == "bits")
									{
										if((PartLengthChildElement->GetChilds().size() == 1) && (PartLengthChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text))
										{
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.Type = "literal";
											assert(PartLengthChildElement->GetChilds().size() == 1);
											
											auto BitsText{dynamic_cast< const XML::Text * >(PartLengthChildElement->GetChild(0))};
											
											assert(BitsText != nullptr);
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.LiteralValue = BitsText->GetText();
										}
										else
										{
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.Type = "reference";
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.ReferenceDescriptor.emplace();
											for(auto PartLengthBitsChildNode : PartLengthChildElement->GetChilds())
											{
												if(PartLengthBitsChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto & ReferencePartDescriptors{PartDescriptor->LengthDescriptor->BitsValueDescriptor.ReferenceDescriptor->PartDescriptors};
													auto ReferencePartDescriptor{ReferencePartDescriptors.emplace(ReferencePartDescriptors.end())};
													auto PartLengthBitsChildElement{dynamic_cast< const XML::Element * >(PartLengthBitsChildNode)};
													
													if(PartLengthBitsChildElement->GetName() == "result")
													{
														assert(PartLengthBitsChildElement->GetChilds().size() == 0);
														ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Result;
													}
													else if(PartLengthBitsChildElement->GetName() == "sub")
													{
														ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Sub;
														assert(PartLengthBitsChildElement->GetChilds().size() == 1);
														
														auto SubText{dynamic_cast< const XML::Text * >(PartLengthBitsChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartLengthBitsChildElement->GetName() == "tag")
													{
														ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Tag;
														assert(PartLengthBitsChildElement->GetChilds().size() == 1);
														
														auto TagText{dynamic_cast< const XML::Text * >(PartLengthBitsChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ReferencePartDescriptor->DetailName = TagText->GetText();
													}
													else
													{
														throw std::domain_error{"/getter/part/length/bits/" + PartLengthBitsChildElement->GetName() + " not allowed."};
													}
												}
											}
										}
									}
									else
									{
										throw std::domain_error{"/getter/part/length/" + PartLengthChildElement->GetName() + " not allowed."};
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
										auto ActualParameterDescriptor{new Inspection::ActualParameterDescriptor{}};
										
										assert(PartParametersChildElement->HasAttribute("name") == true);
										ActualParameterDescriptor->Name = PartParametersChildElement->GetAttribute("name");
										for(auto GetterPartParametersParameterChildNode : PartParametersChildElement->GetChilds())
										{
											if(GetterPartParametersParameterChildNode->GetNodeType() == XML::NodeType::Element)
											{
												auto GetterPartParametersParameterChildElement{dynamic_cast< const XML::Element * >(GetterPartParametersParameterChildNode)};
												
												if(GetterPartParametersParameterChildElement->GetName() == "string")
												{
													assert(ActualParameterDescriptor->ValueDescriptor.Type == "");
													ActualParameterDescriptor->ValueDescriptor.Type = "string";
													assert((GetterPartParametersParameterChildElement->GetChilds().size() == 1) && (GetterPartParametersParameterChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
													
													auto ParameterText{dynamic_cast< const XML::Text * >(GetterPartParametersParameterChildElement->GetChild(0))};
													
													assert(ParameterText != nullptr);
													ActualParameterDescriptor->ValueDescriptor.LiteralValue = ParameterText->GetText();
												}
												else if(GetterPartParametersParameterChildElement->GetName() == "reference")
												{
													assert(ActualParameterDescriptor->ValueDescriptor.Type == "");
													ActualParameterDescriptor->ValueDescriptor.Type = "reference";
													ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor.emplace();
													if(GetterPartParametersParameterChildElement->HasAttribute("cast-to-type") == true)
													{
														ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor->CastToType = GetterPartParametersParameterChildElement->GetAttribute("cast-to-type");
													}
													for(auto PartParametersParameterChildNode : GetterPartParametersParameterChildElement->GetChilds())
													{
														if(PartParametersParameterChildNode->GetNodeType() == XML::NodeType::Element)
														{
															auto & ReferencePartDescriptors{ActualParameterDescriptor->ValueDescriptor.ReferenceDescriptor->PartDescriptors};
															auto ReferencePartDescriptor{ReferencePartDescriptors.emplace(ReferencePartDescriptors.end())};
															auto PartParametersParameterChildElement{dynamic_cast< const XML::Element * >(PartParametersParameterChildNode)};
															
															if(PartParametersParameterChildElement->GetName() == "result")
															{
																assert(PartParametersParameterChildElement->GetChilds().size() == 0);
																ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Result;
															}
															else if(PartParametersParameterChildElement->GetName() == "sub")
															{
																ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Sub;
																assert(PartParametersParameterChildElement->GetChilds().size() == 1);
																
																auto SubText{dynamic_cast< const XML::Text * >(PartParametersParameterChildElement->GetChild(0))};
																
																assert(SubText != nullptr);
																ReferencePartDescriptor->DetailName = SubText->GetText();
															}
															else if(PartParametersParameterChildElement->GetName() == "tag")
															{
																ReferencePartDescriptor->Type = Inspection::ReferenceDescriptor::PartDescriptor::Type::Tag;
																assert(PartParametersParameterChildElement->GetChilds().size() == 1);
																
																auto TagText{dynamic_cast< const XML::Text * >(PartParametersParameterChildElement->GetChild(0))};
																
																assert(TagText != nullptr);
																ReferencePartDescriptor->DetailName = TagText->GetText();
															}
															else
															{
																throw std::domain_error{"/getter/part/parameters/parameter/reference/" + PartParametersParameterChildElement->GetName() + " not allowed."};
															}
														}
													}
												}
												else if(GetterPartParametersParameterChildElement->GetName() == "getter-reference")
												{
													assert(ActualParameterDescriptor->ValueDescriptor.Type == "");
													ActualParameterDescriptor->ValueDescriptor.Type = "getter-reference";
													for(auto GetterPartParametersParameterGetterChildNode : GetterPartParametersParameterChildElement->GetChilds())
													{
														if(GetterPartParametersParameterGetterChildNode->GetNodeType() == XML::NodeType::Element)
														{
															auto GetterPartParametersParameterGetterChildElement{dynamic_cast< const XML::Element * >(GetterPartParametersParameterGetterChildNode)};
															
															if(GetterPartParametersParameterGetterChildElement->GetName() == "part")
															{
																assert(GetterPartParametersParameterGetterChildElement->GetChilds().size() == 1);
																
																auto PartText{dynamic_cast< const XML::Text * >(GetterPartParametersParameterGetterChildElement->GetChild(0))};
																
																assert(PartText != nullptr);
																ActualParameterDescriptor->ValueDescriptor.GetterReferenceDescriptor.Parts.push_back(PartText->GetText());
															}
															else
															{
																throw std::domain_error{"/getter/part/parameters/parameter/getter/" + GetterPartParametersParameterGetterChildElement->GetName() + " not allowed."};
															}
														}
													}
												}
												else
												{
													throw std::domain_error{"/getter/part/parameters/parameter/" + GetterPartParametersParameterChildElement->GetName() + " not allowed."};
												}
											}
										}
										PartDescriptor->ActualParameterDescriptors.push_back(ActualParameterDescriptor);
									}
									else
									{
										throw std::domain_error{"/getter/part/parameters/" + PartParametersChildElement->GetName() + " not allowed."};
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
													throw std::domain_error{"/getter/part/value/append-value/" + PartValueAppendChildElement->GetName() + " not allowed."};
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
										throw std::domain_error{"/getter/part/value/" + PartValueChildElement->GetName() + " not allowed."};
									}
								}
							}
						}
						else
						{
							throw std::domain_error{"/getter/part/" + PartChildElement->GetName() + " not allowed."};
						}
					}
				}
				_Actions.push_back(std::make_pair(Inspection::ActionType::Read, _PartDescriptors.size()));
				_PartDescriptors.push_back(PartDescriptor);
			}
			else
			{
				throw std::domain_error{"/getter/" + GetterChildElement->GetName() + " not allowed."};
			}
		}
	}
}
