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
	
	class ValueReferenceDescriptor
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
			
			Inspection::ValueReferenceDescriptor::PartDescriptor::Type Type;
			std::string DetailName;
		};
		
		std::experimental::optional< std::string > CastToType;
		std::vector< Inspection::ValueReferenceDescriptor::PartDescriptor > PartDescriptors;
	};
	
	class GetterReferenceDescriptor
	{
	public:
		std::vector< std::string > Parts;
	};
	
	class ValueDescriptor
	{
	public:
		std::string Type;
		std::experimental::optional< Inspection::GetterReferenceDescriptor > GetterReferenceDescriptor;
		std::experimental::optional< std::string > StringValue;
		std::experimental::optional< std::uint32_t > UnsignedInteger32BitValue;
		std::experimental::optional< Inspection::ValueReferenceDescriptor > ValueReferenceDescriptor;
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
	
	class ValueEqualsDescriptor
	{
	public:
		Inspection::ValueDescriptor ValueDescriptor;
	};
	
	class VerificationDescriptor
	{
	public:
		enum class Type
		{
			ValueEquals
		};
		
		Inspection::VerificationDescriptor::Type Type;
		std::experimental::optional< Inspection::ValueEqualsDescriptor > ValueEqualsDescriptor;
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
		std::vector< Inspection::VerificationDescriptor > VerificationDescriptors;
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
	
	template< typename DataType >
	bool ApplyEnumeration(Inspection::Enumeration * Enumeration, std::shared_ptr< Inspection::Value > Target)
	{
		bool Result{false};
		auto BaseValueString{to_string_cast(std::experimental::any_cast< const DataType & >(Target->GetData()))};
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
	
	const std::experimental::any & GetAnyByReference(Inspection::ValueReferenceDescriptor & ValueReferenceDescriptor, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		std::shared_ptr< Inspection::Value > Value{nullptr};
		
		for(auto & PartDescriptor : ValueReferenceDescriptor.PartDescriptors)
		{
			switch(PartDescriptor.Type)
			{
			case Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Result:
				{
					Value = Result->GetValue();
					
					break;
				}
			case Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Sub:
				{
					Value = Value->GetField(PartDescriptor.DetailName);
					
					break;
				}
			case Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Tag:
				{
					Value = Value->GetTag(PartDescriptor.DetailName);
					
					break;
				}
			}
		}
		assert(Value != nullptr);
		
		return Value->GetData();
	}
	
	template< typename Type >
	Type GetValueFromValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const std::unique_ptr< Inspection::Result > & GetterResult, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		Type Result{};
		
		if(ValueDescriptor.Type == "literal")
		{
			assert(ValueDescriptor.StringValue);
			Result = from_string_cast< Type >(ValueDescriptor.StringValue.value());
		}
		else if(ValueDescriptor.Type == "value-reference")
		{
			assert(ValueDescriptor.ValueReferenceDescriptor);
			
			auto & Any{GetAnyByReference(ValueDescriptor.ValueReferenceDescriptor.value(), GetterResult, Parameters)};
			
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
			assert(ValueDescriptor.StringValue);
			Result = from_string_cast< Type >(ValueDescriptor.StringValue.value());
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
								else if(ActualParameterDescriptor->ValueDescriptor.Type == "value-reference")
								{
									assert(ActualParameterDescriptor->ValueDescriptor.ValueReferenceDescriptor);
									
									auto & ValueAny{GetAnyByReference(ActualParameterDescriptor->ValueDescriptor.ValueReferenceDescriptor.value(), Result, Parameters)};
									
									if(!ActualParameterDescriptor->ValueDescriptor.ValueReferenceDescriptor->CastToType)
									{
										Parameters.emplace(ActualParameterDescriptor->Name, ValueAny);
									}
									else if(ActualParameterDescriptor->ValueDescriptor.ValueReferenceDescriptor->CastToType.value() == "unsigned integer 64bit")
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
									assert(ActualParameterDescriptor->ValueDescriptor.GetterReferenceDescriptor);
									Parameters.emplace(ActualParameterDescriptor->Name, ActualParameterDescriptor->ValueDescriptor.GetterReferenceDescriptor->Parts);
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
									Result->GetValue()->AppendField(PartDescriptor->ValueName, PartResult->GetValue());
									
									break;
								}
							case Inspection::AppendType::AppendSubValues:
								{
									Result->GetValue()->AppendFields(PartResult->GetValue()->GetFields());
									
									break;
								}
							case Inspection::AppendType::Set:
								{
									Result->SetValue(PartResult->GetValue());
									
									break;
								}
							}
							Reader.AdvancePosition(PartReader->GetConsumedLength());
							// verification
							if(Continue == true)
							{
								for(auto & VerificationDescriptor : PartDescriptor->VerificationDescriptors)
								{
									switch(VerificationDescriptor.Type)
									{
									case Inspection::VerificationDescriptor::Type::ValueEquals:
										{
											assert(VerificationDescriptor.ValueEqualsDescriptor);
											if(VerificationDescriptor.ValueEqualsDescriptor->ValueDescriptor.Type == "unsigned integer 32bit")
											{
												assert(VerificationDescriptor.ValueEqualsDescriptor->ValueDescriptor.UnsignedInteger32BitValue);
												Continue = std::experimental::any_cast< std::uint32_t >(PartResult->GetValue()->GetData()) == VerificationDescriptor.ValueEqualsDescriptor->ValueDescriptor.UnsignedInteger32BitValue.value();
												if(Continue == false)
												{
													PartResult->GetValue()->AddTag("error", "The value does not match the required value \"" + to_string_cast(VerificationDescriptor.ValueEqualsDescriptor->ValueDescriptor.UnsignedInteger32BitValue.value()) + "\".");
												}
											}
											else
											{
												assert(false);
											}
											
											break;
										}
									default:
										{
											assert(false);
										}
									}
								}
							}
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
				else if(HardcodedGetterText->GetText() == "Get_Apple_AppleDouble_File")
				{
					_HardcodedGetterWithParameters = Inspection::Get_Apple_AppleDouble_File;
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
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.StringValue = BytesText->GetText();
										}
										else
										{
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.Type = "value-reference";
											PartDescriptor->LengthDescriptor->BytesValueDescriptor.ValueReferenceDescriptor.emplace();
											for(auto PartLengthBytesChildNode : PartLengthChildElement->GetChilds())
											{
												if(PartLengthBytesChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto & ValueReferencePartDescriptors{PartDescriptor->LengthDescriptor->BytesValueDescriptor.ValueReferenceDescriptor->PartDescriptors};
													auto ValueReferencePartDescriptor{ValueReferencePartDescriptors.emplace(ValueReferencePartDescriptors.end())};
													auto PartLengthBytesChildElement{dynamic_cast< const XML::Element * >(PartLengthBytesChildNode)};
													
													if(PartLengthBytesChildElement->GetName() == "result")
													{
														assert(PartLengthBytesChildElement->GetChilds().size() == 0);
														ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Result;
													}
													else if(PartLengthBytesChildElement->GetName() == "sub")
													{
														ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Sub;
														assert(PartLengthBytesChildElement->GetChilds().size() == 1);
														
														auto SubText{dynamic_cast< const XML::Text * >(PartLengthBytesChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ValueReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartLengthBytesChildElement->GetName() == "tag")
													{
														ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Tag;
														assert(PartLengthBytesChildElement->GetChilds().size() == 1);
														
														auto TagText{dynamic_cast< const XML::Text * >(PartLengthBytesChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ValueReferencePartDescriptor->DetailName = TagText->GetText();
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
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.StringValue = BitsText->GetText();
										}
										else
										{
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.Type = "value-reference";
											PartDescriptor->LengthDescriptor->BitsValueDescriptor.ValueReferenceDescriptor.emplace();
											for(auto PartLengthBitsChildNode : PartLengthChildElement->GetChilds())
											{
												if(PartLengthBitsChildNode->GetNodeType() == XML::NodeType::Element)
												{
													auto & ValueReferencePartDescriptors{PartDescriptor->LengthDescriptor->BitsValueDescriptor.ValueReferenceDescriptor->PartDescriptors};
													auto ValueReferencePartDescriptor{ValueReferencePartDescriptors.emplace(ValueReferencePartDescriptors.end())};
													auto PartLengthBitsChildElement{dynamic_cast< const XML::Element * >(PartLengthBitsChildNode)};
													
													if(PartLengthBitsChildElement->GetName() == "result")
													{
														assert(PartLengthBitsChildElement->GetChilds().size() == 0);
														ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Result;
													}
													else if(PartLengthBitsChildElement->GetName() == "sub")
													{
														assert(PartLengthBitsChildElement->GetChilds().size() == 1);
														ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Sub;
														
														auto SubText{dynamic_cast< const XML::Text * >(PartLengthBitsChildElement->GetChild(0))};
														
														assert(SubText != nullptr);
														ValueReferencePartDescriptor->DetailName = SubText->GetText();
													}
													else if(PartLengthBitsChildElement->GetName() == "tag")
													{
														assert(PartLengthBitsChildElement->GetChilds().size() == 1);
														ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Tag;
														
														auto TagText{dynamic_cast< const XML::Text * >(PartLengthBitsChildElement->GetChild(0))};
														
														assert(TagText != nullptr);
														ValueReferencePartDescriptor->DetailName = TagText->GetText();
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
										_LoadValueDescriptor(ActualParameterDescriptor->ValueDescriptor, PartParametersChildElement);
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
						else if(PartChildElement->GetName() == "verification")
						{
							auto VerificationDescriptor{PartDescriptor->VerificationDescriptors.emplace(PartDescriptor->VerificationDescriptors.end())};
							
							for(auto GetterPartVerificationChildNode : PartChildElement->GetChilds())
							{
								if(GetterPartVerificationChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto GetterPartVerificationChildElement{dynamic_cast< const XML::Element * >(GetterPartVerificationChildNode)};
									
									if(GetterPartVerificationChildElement->GetName() == "value-equals")
									{
										VerificationDescriptor->Type = Inspection::VerificationDescriptor::Type::ValueEquals;
										VerificationDescriptor->ValueEqualsDescriptor.emplace();
										_LoadValueDescriptor(VerificationDescriptor->ValueEqualsDescriptor->ValueDescriptor, GetterPartVerificationChildElement);
									}
									else
									{
										throw std::domain_error{"/getter/part/verification/" + GetterPartVerificationChildElement->GetName() + " not allowed."};
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

void Inspection::GetterDescriptor::_LoadValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ParentElement)
{
	for(auto ChildNode : ParentElement->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement{dynamic_cast< const XML::Element * >(ChildNode)};
			
			if(ChildElement->GetName() == "getter-reference")
			{
				assert(ValueDescriptor.Type == "");
				ValueDescriptor.Type = "getter-reference";
				ValueDescriptor.GetterReferenceDescriptor.emplace();
				for(auto GetterReferenceChildNode : ChildElement->GetChilds())
				{
					if(GetterReferenceChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto GetterReferenceChildElement{dynamic_cast< const XML::Element * >(GetterReferenceChildNode)};
						
						if(GetterReferenceChildElement->GetName() == "part")
						{
							assert(GetterReferenceChildElement->GetChilds().size() == 1);
							
							auto PartText{dynamic_cast< const XML::Text * >(GetterReferenceChildElement->GetChild(0))};
							
							assert(PartText != nullptr);
							ValueDescriptor.GetterReferenceDescriptor->Parts.push_back(PartText->GetText());
						}
						else
						{
							throw std::domain_error{GetterReferenceChildElement->GetName() + " not allowed."};
						}
					}
				}
			}
			else if(ChildElement->GetName() == "value-reference")
			{
				assert(ValueDescriptor.Type == "");
				ValueDescriptor.Type = "value-reference";
				ValueDescriptor.ValueReferenceDescriptor.emplace();
				if(ChildElement->HasAttribute("cast-to-type") == true)
				{
					ValueDescriptor.ValueReferenceDescriptor->CastToType = ChildElement->GetAttribute("cast-to-type");
				}
				for(auto ValueReferenceChildNode : ChildElement->GetChilds())
				{
					if(ValueReferenceChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto & ValueReferencePartDescriptors{ValueDescriptor.ValueReferenceDescriptor->PartDescriptors};
						auto ValueReferencePartDescriptor{ValueReferencePartDescriptors.emplace(ValueReferencePartDescriptors.end())};
						auto ValueReferenceChildElement{dynamic_cast< const XML::Element * >(ValueReferenceChildNode)};
						
						if(ValueReferenceChildElement->GetName() == "result")
						{
							assert(ValueReferenceChildElement->GetChilds().size() == 0);
							ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Result;
						}
						else if(ValueReferenceChildElement->GetName() == "sub")
						{
							assert(ValueReferenceChildElement->GetChilds().size() == 1);
							ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Sub;
							
							auto SubText{dynamic_cast< const XML::Text * >(ValueReferenceChildElement->GetChild(0))};
							
							assert(SubText != nullptr);
							ValueReferencePartDescriptor->DetailName = SubText->GetText();
						}
						else if(ValueReferenceChildElement->GetName() == "tag")
						{
							assert(ValueReferenceChildElement->GetChilds().size() == 1);
							ValueReferencePartDescriptor->Type = Inspection::ValueReferenceDescriptor::PartDescriptor::Type::Tag;
							
							auto TagText{dynamic_cast< const XML::Text * >(ValueReferenceChildElement->GetChild(0))};
							
							assert(TagText != nullptr);
							ValueReferencePartDescriptor->DetailName = TagText->GetText();
						}
						else
						{
							throw std::domain_error{ValueReferenceChildElement->GetName() + " not allowed."};
						}
					}
				}
			}
			else if(ChildElement->GetName() == "string")
			{
				assert(ValueDescriptor.Type == "");
				ValueDescriptor.Type = "string";
				assert((ChildElement->GetChilds().size() == 1) && (ChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
				
				auto TextNode{dynamic_cast< const XML::Text * >(ChildElement->GetChild(0))};
				
				assert(TextNode != nullptr);
				ValueDescriptor.StringValue = TextNode->GetText();
			}
			else if(ChildElement->GetName() == "unsigned-integer-32bit")
			{
				assert(ValueDescriptor.Type == "");
				ValueDescriptor.Type = "unsigned integer 32bit";
				assert((ChildElement->GetChilds().size() == 1) && (ChildElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
				
				auto TextNode{dynamic_cast< const XML::Text * >(ChildElement->GetChild(0))};
				
				assert(TextNode != nullptr);
				ValueDescriptor.UnsignedInteger32BitValue = from_string_cast< std::uint32_t >(TextNode->GetText());
			}
			else
			{
				throw std::domain_error{ChildElement->GetName() + " not allowed."};
			}
		}
	}
}
