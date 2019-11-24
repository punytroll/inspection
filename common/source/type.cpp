#include <experimental/optional>
#include <fstream>

#include "getters.h"
#include "not_implemented_exception.h"
#include "result.h"
#include "type.h"
#include "type_definition.h"
#include "type_repository.h"
#include "xml_helper.h"
#include "xml_puny_dom.h"

using namespace std::string_literals;

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
	
	const std::experimental::any & GetAnyReferenceByDataReference(const Inspection::TypeDefinition::DataReference & DataReference, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		std::shared_ptr< Inspection::Value > Value{CurrentValue};
		
		for(auto & PartDescriptor : DataReference.PartDescriptors)
		{
			switch(PartDescriptor.Type)
			{
			case Inspection::TypeDefinition::DataReference::PartDescriptor::Type::Field:
				{
					Value = Value->GetField(PartDescriptor.DetailName);
					
					break;
				}
			case Inspection::TypeDefinition::DataReference::PartDescriptor::Type::Tag:
				{
					Value = Value->GetTag(PartDescriptor.DetailName);
					
					break;
				}
			}
		}
		assert(Value != nullptr);
		
		return Value->GetData();
	}
	
	std::experimental::any GetAnyFromValueDescriptor(const Inspection::TypeDefinition::Value & Value, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		if(Value.DataType == Inspection::TypeDefinition::DataType::Boolean)
		{
			assert(Value.Boolean);
			
			return Value.Boolean.value();
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::DataReference)
		{
			assert(Value.DataReference);
			
			return GetAnyReferenceByDataReference(Value.DataReference.value(), CurrentValue, Parameters);
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::Nothing)
		{
			return nullptr;
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::SinglePrecisionReal)
		{
			assert(Value.SinglePrecisionReal);
			
			return Value.SinglePrecisionReal.value();
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::String)
		{
			assert(Value.String);
			
			return Value.String.value();
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::UnsignedInteger8Bit)
		{
			assert(Value.UnsignedInteger8Bit);
			
			return Value.UnsignedInteger8Bit.value();
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::UnsignedInteger16Bit)
		{
			assert(Value.UnsignedInteger16Bit);
			
			return Value.UnsignedInteger16Bit.value();
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::UnsignedInteger32Bit)
		{
			assert(Value.UnsignedInteger32Bit);
			
			return Value.UnsignedInteger32Bit.value();
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::UnsignedInteger64Bit)
		{
			assert(Value.UnsignedInteger64Bit);
			
			return Value.UnsignedInteger64Bit.value();
		}
		else
		{
			assert(false);
		}
	}
	
	template< typename Type >
	Type GetDataFromValueDescriptor(const Inspection::TypeDefinition::Value & Value, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		Type Result{};
		
		if(Value.DataType == Inspection::TypeDefinition::DataType::DataReference)
		{
			assert(Value.DataReference);
			
			auto & Any{GetAnyReferenceByDataReference(Value.DataReference.value(), CurrentValue, Parameters)};
			
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
		else if(Value.DataType == Inspection::TypeDefinition::DataType::String)
		{
			assert(Value.String);
			Result = from_string_cast< Type >(Value.String.value());
		}
		else if(Value.DataType == Inspection::TypeDefinition::DataType::UnsignedInteger64Bit)
		{
			assert(Value.UnsignedInteger64Bit);
			Result = Value.UnsignedInteger64Bit.value();
		}
		else
		{
			assert(false);
		}
		
		return Result;
	}
	
	void ApplyTags(const std::vector< Inspection::TypeDefinition::Tag > & Tags, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		for(auto & Tag : Tags)
		{
			assert((Tag.Statement.Type == Inspection::TypeDefinition::Statement::Type::Value) && (Tag.Statement.Value != nullptr));
			Target->AddTag(Tag.Name, GetAnyFromValueDescriptor(*(Tag.Statement.Value), CurrentValue, Parameters));
		}
	}
	
	template< typename DataType >
	bool ApplyEnumeration(const Inspection::TypeDefinition::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		bool Result{false};
		auto BaseValueString{to_string_cast(std::experimental::any_cast< const DataType & >(Target->GetData()))};
		auto ElementIterator{std::find_if(Enumeration.Elements.begin(), Enumeration.Elements.end(), [BaseValueString](auto & Element){ return Element.BaseValue == BaseValueString; })};
		
		if(ElementIterator != Enumeration.Elements.end())
		{
			ApplyTags(ElementIterator->Tags, Target, CurrentValue, Parameters);
			Result = ElementIterator->Valid;
		}
		else
		{
			if(Enumeration.FallbackElement)
			{
				ApplyTags(Enumeration.FallbackElement->Tags, Target, CurrentValue, Parameters);
				Target->AddTag("error", "Could find no enumeration element for the base value \"" + BaseValueString + "\".");
				Result = Enumeration.FallbackElement->Valid;
			}
			else
			{
				Target->AddTag("error", "Could find neither an enumarion element nor an enumeration fallback element for the base value \"" + BaseValueString + "\".");
				Result = false;
			}
		}
		
		return Result;
	}
	
	namespace Algorithms
	{
		std::experimental::any Divide(const Inspection::TypeDefinition::Statement & Dividend, const Inspection::TypeDefinition::Statement & Divisor, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		std::experimental::any GetAnyFromCast(const Inspection::TypeDefinition::Cast & Cast, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		
		template< typename Type >
		Type Cast(const std::experimental::any & Any)
		{
			if(Any.type() == typeid(float))
			{
				return static_cast< Type >(std::experimental::any_cast< float >(Any));
			}
			else if(Any.type() == typeid(std::uint8_t))
			{
				return static_cast< Type >(std::experimental::any_cast< std::uint8_t >(Any));
			}
			else if(Any.type() == typeid(std::uint16_t))
			{
				return static_cast< Type >(std::experimental::any_cast< std::uint16_t >(Any));
			}
			else if(Any.type() == typeid(std::uint32_t))
			{
				return static_cast< Type >(std::experimental::any_cast< std::uint32_t >(Any));
			}
			else if(Any.type() == typeid(std::uint64_t))
			{
				return static_cast< Type >(std::experimental::any_cast< std::uint64_t >(Any));
			}
			else
			{
				assert(false);
			}
		}
		
		std::experimental::any GetAnyFromValue(const Inspection::TypeDefinition::Value & Value, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			switch(Value.DataType)
			{
			case Inspection::TypeDefinition::DataType::DataReference:
				{
					assert(Value.DataReference);
					
					return GetAnyReferenceByDataReference(Value.DataReference.value(), CurrentValue, Parameters);
				}
			case Inspection::TypeDefinition::DataType::ParameterReference:
				{
					assert(Value.ParameterReference);
					
					return Parameters.at(Value.ParameterReference->Name);
				}
			case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
				{
					assert(Value.SinglePrecisionReal);
					
					return Value.SinglePrecisionReal.value();
				}
			case Inspection::TypeDefinition::DataType::String:
				{
					assert(Value.String);
					
					return Value.String.value();
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		std::experimental::any GetAnyFromStatement(const Inspection::TypeDefinition::Statement & Statement, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Cast:
				{
					assert(Statement.Cast != nullptr);
					
					return Inspection::Algorithms::GetAnyFromCast(*(Statement.Cast), CurrentValue, Parameters);
				}
			case Inspection::TypeDefinition::Statement::Type::Divide:
				{
					assert(Statement.Divide != nullptr);
					
					return Inspection::Algorithms::Divide(Statement.Divide->Dividend, Statement.Divide->Divisor, CurrentValue, Parameters);
				}
			case Inspection::TypeDefinition::Statement::Type::Value:
				{
					assert(Statement.Value != nullptr);
					
					return GetAnyFromValue(*(Statement.Value), CurrentValue, Parameters);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		template< typename Type >
		Type GetDataFromCast(const Inspection::TypeDefinition::Cast & Cast, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			return Inspection::Algorithms::Cast< Type >(GetAnyFromStatement(Cast.Statement, CurrentValue, Parameters));
		}
		
		template< typename Type >
		Type GetDataFromStatement(const Inspection::TypeDefinition::Statement & Statement, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Cast:
				{
					assert(Statement.Cast != nullptr);
					
					return GetDataFromCast< Type >(*(Statement.Cast), CurrentValue, Parameters);
				}
			case Inspection::TypeDefinition::Statement::Type::Value:
				{
					assert(Statement.Value != nullptr);
					
					return GetDataFromValueDescriptor< Type >(*(Statement.Value), CurrentValue, Parameters);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		std::experimental::any GetAnyFromCast(const Inspection::TypeDefinition::Cast & Cast, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			switch(Cast.DataType)
			{
			case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
				{
					return Inspection::Algorithms::GetDataFromCast< float >(Cast, CurrentValue, Parameters);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
				{
					return Inspection::Algorithms::GetDataFromCast< std::uint64_t >(Cast, CurrentValue, Parameters);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		bool Equals(const Inspection::TypeDefinition::Value & Value1, const Inspection::TypeDefinition::Value & Value2, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			auto Any1{GetAnyFromValueDescriptor(Value1, CurrentValue, Parameters)};
			auto Any2{GetAnyFromValueDescriptor(Value2, CurrentValue, Parameters)};
			
			if((Any1.empty() == false) && (Any2.empty() == false))
			{
				if(Any1.type() == Any2.type())
				{
					if(Any1.type() == typeid(std::uint8_t))
					{
						return std::experimental::any_cast< std::uint8_t >(Any1) == std::experimental::any_cast< std::uint8_t >(Any2);
					}
					else if(Any1.type() == typeid(std::uint32_t))
					{
						return std::experimental::any_cast< std::uint32_t >(Any1) == std::experimental::any_cast< std::uint32_t >(Any2);
					}
					else
					{
						assert(false);
					}
				}
			}
			
			return false;
		}
		
		bool Equals(const Inspection::TypeDefinition::Statement & Statement1, const Inspection::TypeDefinition::Statement & Statement2, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			if(Statement1.Type == Inspection::TypeDefinition::Statement::Type::Value)
			{
				assert(Statement1.Value != nullptr);
				if(Statement2.Type == Inspection::TypeDefinition::Statement::Type::Value)
				{
					assert(Statement2.Value != nullptr);
					
					return Inspection::Algorithms::Equals(*(Statement1.Value), *(Statement2.Value), CurrentValue, Parameters);
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
		
		std::experimental::any Divide(const Inspection::TypeDefinition::Statement & Dividend, const Inspection::TypeDefinition::Statement & Divisor, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			auto AnyDivident{Inspection::Algorithms::GetAnyFromStatement(Dividend, CurrentValue, Parameters)};
			auto AnyDivisor{Inspection::Algorithms::GetAnyFromStatement(Divisor, CurrentValue, Parameters)};
			
			if((AnyDivident.type() == typeid(float)) && (AnyDivisor.type() == typeid(float)))
			{
				return std::experimental::any_cast< float >(AnyDivident) / std::experimental::any_cast< float >(AnyDivisor);
			}
			else
			{
				assert(false);
			}
			
			return nullptr;
		}
	}
	
	void FillNewParameters(std::unordered_map< std::string, std::experimental::any > & NewParameters, const Inspection::TypeDefinition::Parameters & ParameterDefinitions, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		for(auto & ParameterDefinition : ParameterDefinitions.Parameters)
		{
			switch(ParameterDefinition.Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Cast:
				{
					assert(ParameterDefinition.Statement.Cast != nullptr);
					NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromCast(*(ParameterDefinition.Statement.Cast), CurrentValue, Parameters));
					
					break;
				}
			case Inspection::TypeDefinition::Statement::Type::Value:
				{
					assert(ParameterDefinition.Statement.Value != nullptr);
					if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::String)
					{
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(*(ParameterDefinition.Statement.Value), CurrentValue, Parameters));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::DataReference)
					{
						assert(ParameterDefinition.Statement.Value->DataReference);
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(*(ParameterDefinition.Statement.Value), CurrentValue, Parameters));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::TypeReference)
					{
						assert(ParameterDefinition.Statement.Value->TypeReference);
						NewParameters.emplace(ParameterDefinition.Name, ParameterDefinition.Statement.Value->TypeReference->Parts);
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::ParameterReference)
					{
						assert(ParameterDefinition.Statement.Value->ParameterReference);
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(*(ParameterDefinition.Statement.Value), CurrentValue, Parameters));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::Parameters)
					{
						assert(ParameterDefinition.Statement.Value->Parameters);
						
						std::unordered_map< std::string, std::experimental::any > InnerParameters;
						
						FillNewParameters(InnerParameters, ParameterDefinition.Statement.Value->Parameters.value(), CurrentValue, Parameters);
						NewParameters.emplace(ParameterDefinition.Name, InnerParameters);
						
						break;
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

Inspection::Type::Type(Inspection::TypeRepository * TypeRepository) :
	_Part{nullptr},
	_TypeRepository{TypeRepository}
{
}

Inspection::Type::~Type(void)
{
	delete _Part;
}

std::unique_ptr< Inspection::Result > Inspection::Type::Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(_HardcodedGetter != nullptr)
		{
			auto HardcodedResult{_HardcodedGetter(Reader, Parameters)};
			
			Continue = HardcodedResult->GetSuccess();
			Result->SetValue(HardcodedResult->GetValue());
		}
		else if(_Part != nullptr)
		{
			Continue = _GetPart(*_Part, Result, Result->GetValue(), Reader, Parameters);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

bool Inspection::Type::_GetPart(const Inspection::TypeDefinition::Part & Part, std::unique_ptr< Inspection::Result > & TopLevelResult, std::shared_ptr< Inspection::Value > & Target, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Continue{true};
	Inspection::Reader * PartReader{nullptr};
	
	if(Part.Length)
	{
		auto Bytes{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(Part.Length->Bytes, TopLevelResult->GetValue(), Parameters)};
		auto Bits{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(Part.Length->Bits, TopLevelResult->GetValue(), Parameters)};
		Inspection::Length Length{Bytes, Bits};
		
		if(Reader.Has(Length) == true)
		{
			PartReader = new Inspection::Reader{Reader, Length};
		}
		else
		{
			Target->AddTag("error", "At least " + to_string_cast(Length) + " bytes and bits are necessary to read this part.");
			Continue = false;
		}
	}
	else
	{
		PartReader = new Inspection::Reader{Reader};
	}
	if(PartReader != nullptr)
	{
		switch(Part.Type)
		{
		case Inspection::TypeDefinition::Part::Type::Sequence:
			{
				assert(Part.Parts);
				for(auto & SequencePart : Part.Parts.value())
				{
					auto PartResult{_GetPart(SequencePart, TopLevelResult, Target, *PartReader, Parameters)};
					
					Continue = PartResult;
					if(Continue == false)
					{
						break;
					}
				}
				Reader.AdvancePosition(PartReader->GetConsumedLength());
				
				break;
			}
		case Inspection::TypeDefinition::Part::Type::Field:
		case Inspection::TypeDefinition::Part::Type::Fields:
		case Inspection::TypeDefinition::Part::Type::Forward:
			{
				std::unordered_map< std::string, std::experimental::any > NewParameters;
				
				if(Part.Parameters)
				{
					FillNewParameters(NewParameters, Part.Parameters.value(), TopLevelResult->GetValue(), Parameters);
				}
				
				auto PartResult{g_TypeRepository.Get(Part.TypeReference->Parts, *PartReader, NewParameters)};
				
				Continue = PartResult->GetSuccess();
				switch(Part.Type)
				{
				case Inspection::TypeDefinition::Part::Type::Field:
					{
						assert(Part.FieldName);
						Target->AppendField(Part.FieldName.value(), PartResult->GetValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Fields:
					{
						Target->AppendFields(PartResult->GetValue()->GetFields());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Forward:
					{
						Target = PartResult->GetValue();
						
						break;
					}
				default:
					{
						assert(false);
					}
				}
				Reader.AdvancePosition(PartReader->GetConsumedLength());
				// tags
				if(Continue == true)
				{
					if(Part.Tags.empty() == false)
					{
						assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward));
						for(auto & Tag : Part.Tags)
						{
							PartResult->GetValue()->AddTag(Tag.Name, Inspection::Algorithms::GetAnyFromStatement(Tag.Statement, PartResult->GetValue(), Parameters));
						}
					}
				}
				// interpretation
				if(Continue == true)
				{
					if(Part.Interpretation)
					{
						auto EvaluationResult{_ApplyInterpretation(Part.Interpretation.value(), PartResult->GetValue(), PartResult->GetValue(), Parameters)};
						
						if(EvaluationResult.AbortEvaluation)
						{
							if(EvaluationResult.AbortEvaluation.value() == true)
							{
								Continue = false;
							}
						}
					}
				}
				// verification
				if(Continue == true)
				{
					for(auto & Statement : Part.Verifications)
					{
						switch(Statement.Type)
						{
						case Inspection::TypeDefinition::Statement::Type::Equals:
							{
								assert(Statement.Equals);
								Continue = Inspection::Algorithms::Equals(Statement.Equals->Statement1, Statement.Equals->Statement2, PartResult->GetValue(), Parameters);
								if(Continue == false)
								{
									PartResult->GetValue()->AddTag("error", "The value failed to verify."s);
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
				
				break;
			}
		}
		delete PartReader;
	}
	
	return Continue;
}

Inspection::EvaluationResult Inspection::Type::_ApplyEnumeration(const Inspection::TypeDefinition::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	Inspection::EvaluationResult Result;
	
	if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::String)
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::string >(Enumeration, Target, CurrentValue, Parameters);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger8Bit)
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint8_t >(Enumeration, Target, CurrentValue, Parameters);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger16Bit)
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint16_t >(Enumeration, Target, CurrentValue, Parameters);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger32Bit)
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint32_t >(Enumeration, Target, CurrentValue, Parameters);
	}
	else
	{
		Target->AddTag("error", "Could not handle the enumeration base type.");
		Result.AbortEvaluation = true;
	}
	
	return Result;
}
	
Inspection::EvaluationResult Inspection::Type::_ApplyInterpretation(const Inspection::TypeDefinition::Interpretation & Interpretation, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	Inspection::EvaluationResult Result;
	
	switch(Interpretation.Type)
	{
	case Inspection::TypeDefinition::Interpretation::Type::ApplyEnumeration:
		{
			assert(Interpretation.ApplyEnumeration);
			
			auto EvaluationResult{_ApplyEnumeration(Interpretation.ApplyEnumeration->Enumeration, Target, CurrentValue, Parameters)};
			
			if(EvaluationResult.AbortEvaluation)
			{
				Result.AbortEvaluation = EvaluationResult.AbortEvaluation.value();
			}
			
			break;
		}
	default:
		{
			assert(false);
		}
	}
	
	return Result;
}

void Inspection::Type::Load(const std::string & TypePath)
{
	try
	{
		std::ifstream InputFileStream{TypePath};
		XML::Document Document{InputFileStream};
		auto DocumentElement{Document.GetDocumentElement()};
		
		assert(DocumentElement != nullptr);
		assert(DocumentElement->GetName() == "type");
		for(auto TypeChildNode : DocumentElement->GetChilds())
		{
			if(TypeChildNode->GetNodeType() == XML::NodeType::Element)
			{
				auto TypeChildElement{dynamic_cast< const XML::Element * >(TypeChildNode)};
				
				if(TypeChildElement->GetName() == "hardcoded")
				{
					assert(TypeChildElement->GetChilds().size() == 1);
					
					auto HardcodedText{dynamic_cast< const XML::Text * >(TypeChildElement->GetChild(0))};
					
					assert(HardcodedText != nullptr);
					if(HardcodedText->GetText() == "Get_APE_Flags")
					{
						_HardcodedGetter = Inspection::Get_APE_Flags;
					}
					else if(HardcodedText->GetText() == "Get_APE_Item")
					{
						_HardcodedGetter = Inspection::Get_APE_Item;
					}
					else if(HardcodedText->GetText() == "Get_Apple_AppleDouble_File")
					{
						_HardcodedGetter = Inspection::Get_Apple_AppleDouble_File;
					}
					else if(HardcodedText->GetText() == "Get_Array_EndedByFailureOrLength_ResetPositionOnFailure")
					{
						_HardcodedGetter = Inspection::Get_Array_EndedByFailureOrLength_ResetPositionOnFailure;
					}
					else if(HardcodedText->GetText() == "Get_Array_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_Array_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_Array_EndedByNumberOfElements")
					{
						_HardcodedGetter = Inspection::Get_Array_EndedByNumberOfElements;
					}
					else if(HardcodedText->GetText() == "Get_ASCII_String_AlphaNumeric_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_ASCII_String_AlphaNumericOrSpace_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_ASCII_String_Printable_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_ASCII_String_Printable_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByTermination;
					}
					else if(HardcodedText->GetText() == "Get_ASF_DataObject")
					{
						_HardcodedGetter = Get_ASF_DataObject;
					}
					else if(HardcodedText->GetText() == "Get_ASF_ExtendedContentDescription_ContentDescriptor_Data")
					{
						_HardcodedGetter = Get_ASF_ExtendedContentDescription_ContentDescriptor_Data;
					}
					else if(HardcodedText->GetText() == "Get_ASF_HeaderObject")
					{
						_HardcodedGetter = Get_ASF_HeaderObject;
					}
					else if(HardcodedText->GetText() == "Get_ASF_Metadata_DescriptionRecord_Data")
					{
						_HardcodedGetter = Get_ASF_Metadata_DescriptionRecord_Data;
					}
					else if(HardcodedText->GetText() == "Get_ASF_MetadataLibrary_DescriptionRecord_Data")
					{
						_HardcodedGetter = Get_ASF_MetadataLibrary_DescriptionRecord_Data;
					}
					else if(HardcodedText->GetText() == "Get_ASF_Object")
					{
						_HardcodedGetter = Get_ASF_Object;
					}
					else if(HardcodedText->GetText() == "Get_ASF_StreamBitrateProperties_BitrateRecord_Flags")
					{
						_HardcodedGetter = Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags;
					}
					else if(HardcodedText->GetText() == "Get_ASF_CreationDate")
					{
						_HardcodedGetter = Inspection::Get_ASF_CreationDate;
					}
					else if(HardcodedText->GetText() == "Get_ASF_GUID")
					{
						_HardcodedGetter = Inspection::Get_ASF_GUID;
					}
					else if(HardcodedText->GetText() == "Get_ASF_FileProperties_Flags")
					{
						_HardcodedGetter = Inspection::Get_ASF_FileProperties_Flags;
					}
					else if(HardcodedText->GetText() == "Get_BitSet_16Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_BitSet_16Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_Boolean_1Bit")
					{
						_HardcodedGetter = Inspection::Get_Boolean_1Bit;
					}
					else if(HardcodedText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_Data_Set_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_Data_Set_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_Data_SetOrUnset_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_Data_SetOrUnset_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_Data_Unset_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_Data_Unset_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_Frame")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Frame;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_MetaDataBlock")
					{
						_HardcodedGetter = Inspection::Get_FLAC_MetaDataBlock;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_Stream_Header")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Stream_Header;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_StreamInfoBlock_BitsPerSample")
					{
						_HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_BitsPerSample;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_StreamInfoBlock_NumberOfChannels")
					{
						_HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_NumberOfChannels;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_Subframe_CalculateBitsPerSample")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Subframe_CalculateBitsPerSample;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual;
					}
					else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual_Rice_Partition")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual_Rice_Partition;
					}
					else if(HardcodedText->GetText() == "Get_GUID_LittleEndian")
					{
						_HardcodedGetter = Inspection::Get_GUID_LittleEndian;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_2_Frame")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_Frame;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_2_Language")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_Language;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_3_Frame")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_Frame;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_3_Language")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_Language;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_3_Frame_Header_Flags")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_Frame_Header_Flags;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_4_Frame")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_Frame;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_4_Language")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_Language;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination;
					}
					else if(HardcodedText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength;
					}
					else if(HardcodedText->GetText() == "Get_ID3_ReplayGainAdjustment")
					{
						_HardcodedGetter = Inspection::Get_ID3_ReplayGainAdjustment;
					}
					else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit")
					{
						_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit;
					}
					else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Track")
					{
						_HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Track;
					}
					else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Tracks")
					{
						_HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Tracks;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength;
					}
					else if(HardcodedText->GetText() == "Get_ISO_IEC_IEEE_60559_2011_binary32")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32;
					}
					else if(HardcodedText->GetText() == "Get_MPEG_1_Frame")
					{
						_HardcodedGetter = Inspection::Get_MPEG_1_Frame;
					}
					else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_BitRateIndex")
					{
						_HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_BitRateIndex;
					}
					else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_Mode")
					{
						_HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_Mode;
					}
					else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_ModeExtension")
					{
						_HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_ModeExtension;
					}
					else if(HardcodedText->GetText() == "Get_Ogg_Page")
					{
						_HardcodedGetter = Inspection::Get_Ogg_Page;
					}
					else if(HardcodedText->GetText() == "Get_RIFF_Chunk")
					{
						_HardcodedGetter = Inspection::Get_RIFF_Chunk;
					}
					else if(HardcodedText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask")
					{
						_HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask;
					}
					else if(HardcodedText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat")
					{
						_HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat;
					}
					else if(HardcodedText->GetText() == "Get_SignedInteger_8Bit")
					{
						_HardcodedGetter = Inspection::Get_SignedInteger_8Bit;
					}
					else if(HardcodedText->GetText() == "Get_SignedInteger_32Bit_RiceEncoded")
					{
						_HardcodedGetter = Inspection::Get_SignedInteger_32Bit_RiceEncoded;
					}
					else if(HardcodedText->GetText() == "Get_SignedInteger_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_SignedInteger_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_String_ASCII_Alphabetic_ByTemplate")
					{
						_HardcodedGetter = Inspection::Get_String_ASCII_Alphabetic_ByTemplate;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_1Bit")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_1Bit;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_2Bit")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_2Bit;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_3Bit")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_3Bit;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_4Bit")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_4Bit;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_7Bit")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_7Bit;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_8Bit")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_8Bit;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_9Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_9Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_16Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_16Bit_LittleEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_LittleEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_20Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_20Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_24Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_24Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_32Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_32Bit_LittleEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_LittleEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_36Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_36Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_64Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_BigEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_64Bit_LittleEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_LittleEndian;
					}
					else if(HardcodedText->GetText() == "Get_UnsignedInteger_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_UnsignedInteger_BigEndian;
					}
					else
					{
						throw std::domain_error{"Invalid reference to hardcoded getter \"" + HardcodedText->GetText() + "\"."};
					}
				}
				else if((TypeChildElement->GetName() == "sequence") || (TypeChildElement->GetName() == "field") || (TypeChildElement->GetName() == "fields") || (TypeChildElement->GetName() == "forward"))
				{
					assert(_Part == nullptr);
					_Part = new Inspection::TypeDefinition::Part{};
					_LoadPart(*_Part, TypeChildElement);
				}
				else
				{
					throw std::domain_error{"\"type/" + TypeChildElement->GetName() + "\" not allowed."};
				}
			}
		}
	}
	catch(std::domain_error & Exception)
	{
		std::throw_with_nested(std::runtime_error("Type path: " + TypePath));
	}
}

void Inspection::Type::_LoadCast(Inspection::TypeDefinition::Cast & Cast, const XML::Element * CastElement)
{
	assert(Cast.DataType == Inspection::TypeDefinition::DataType::Unknown);
	for(auto CastChildNode : CastElement->GetChilds())
	{
		if(CastChildNode->GetNodeType() == XML::NodeType::Element)
		{
			assert(Cast.DataType == Inspection::TypeDefinition::DataType::Unknown);
			
			auto CastChildElement{dynamic_cast< const XML::Element * >(CastChildNode)};
			
			Cast.DataType = Inspection::TypeDefinition::GetDataTypeFromString(CastElement->GetName());
			_LoadStatement(Cast.Statement, CastChildElement);
		}
	}
	assert(Cast.DataType != Inspection::TypeDefinition::DataType::Unknown);
}

void Inspection::Type::_LoadDivide(Inspection::TypeDefinition::Divide & Divide, const XML::Element * DivideElement)
{
	bool First{true};
	
	for(auto DivideChildNode : DivideElement->GetChilds())
	{
		if(DivideChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto DivideChildElement{dynamic_cast< const XML::Element * >(DivideChildNode)};
			
			if(First == true)
			{
				_LoadStatement(Divide.Dividend, DivideChildElement);
				First = false;
			}
			else
			{
				_LoadStatement(Divide.Divisor, DivideChildElement);
			}
		}
	}
}

void Inspection::Type::_LoadInterpretation(Inspection::TypeDefinition::Interpretation & Interpretation, const XML::Element * InterpretationElement)
{
	assert(InterpretationElement->GetName() == "interpretation");
	for(auto InterpretationChildNode : InterpretationElement->GetChilds())
	{
		if(InterpretationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto InterpretationChildElement{dynamic_cast< const XML::Element * >(InterpretationChildNode)};
			
			if(InterpretationChildElement->GetName() == "apply-enumeration")
			{
				Interpretation.Type = Inspection::TypeDefinition::Interpretation::Type::ApplyEnumeration;
				Interpretation.ApplyEnumeration.emplace();
				for(auto InterpretationApplyEnumerationChildNode : InterpretationChildElement->GetChilds())
				{
					if(InterpretationApplyEnumerationChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto InterpretationApplyEnumerationChildElement{dynamic_cast< const XML::Element * >(InterpretationApplyEnumerationChildNode)};
						
						if(InterpretationApplyEnumerationChildElement->GetName() == "enumeration")
						{
							_LoadEnumeration(Interpretation.ApplyEnumeration->Enumeration, InterpretationApplyEnumerationChildElement);
						}
						else
						{
							throw std::domain_error{"interpretation/apply-enumeration/" + InterpretationApplyEnumerationChildElement->GetName() + " not allowed."};
						}
					}
				}
			}
			else
			{
				throw std::domain_error{"interpretation/" + InterpretationChildElement->GetName() + " not allowed."};
			}
		}
	}
}

void Inspection::Type::_LoadEnumeration(Inspection::TypeDefinition::Enumeration & Enumeration, const XML::Element * EnumerationElement)
{
	assert(EnumerationElement != nullptr);
	assert(EnumerationElement->GetName() == "enumeration");
	Enumeration.BaseDataType = Inspection::TypeDefinition::GetDataTypeFromString(EnumerationElement->GetAttribute("base-data-type"));
	for(auto EnumerationChildNode : EnumerationElement->GetChilds())
	{
		if(EnumerationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto EnumerationChildElement{dynamic_cast< const XML::Element * >(EnumerationChildNode)};
			
			if(EnumerationChildElement->GetName() == "element")
			{
				Enumeration.Elements.emplace_back();
				
				auto & Element{Enumeration.Elements.back()};
				
				assert(EnumerationChildElement->HasAttribute("base-value") == true);
				Element.BaseValue = EnumerationChildElement->GetAttribute("base-value");
				assert(EnumerationChildElement->HasAttribute("valid") == true);
				Element.Valid = from_string_cast< bool >(EnumerationChildElement->GetAttribute("valid"));
				for(auto EnumerationElementChildNode : EnumerationChildElement->GetChilds())
				{
					if(EnumerationElementChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto EnumerationElementChildElement{dynamic_cast< const XML::Element * >(EnumerationElementChildNode)};
						
						if(EnumerationElementChildElement->GetName() == "tag")
						{
							Element.Tags.emplace_back();
							
							auto & Tag{Element.Tags.back()};
							
							_LoadTag(Tag, EnumerationElementChildElement);
						}
						else
						{
							throw std::domain_error{EnumerationElementChildElement->GetName()};
						}
					}
				}
			}
			else if(EnumerationChildElement->GetName() == "fallback-element")
			{
				assert(!Enumeration.FallbackElement);
				Enumeration.FallbackElement.emplace();
				Enumeration.FallbackElement->Valid = from_string_cast< bool >(EnumerationChildElement->GetAttribute("valid"));
				for(auto EnumerationFallbackElementChildNode : EnumerationChildElement->GetChilds())
				{
					if(EnumerationFallbackElementChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto EnumerationFallbackElementChildElement{dynamic_cast< const XML::Element * >(EnumerationFallbackElementChildNode)};
						
						if(EnumerationFallbackElementChildElement->GetName() == "tag")
						{
							Enumeration.FallbackElement->Tags.emplace_back();
							
							auto & Tag{Enumeration.FallbackElement->Tags.back()};
							
							_LoadTag(Tag, EnumerationFallbackElementChildElement);
						}
						else
						{
							throw std::domain_error{EnumerationFallbackElementChildElement->GetName()};
						}
					}
				}
			}
			else
			{
				throw std::domain_error{EnumerationChildElement->GetName()};
			}
		}
	}
}

void Inspection::Type::_LoadEquals(Inspection::TypeDefinition::Equals & Equals, const XML::Element * EqualsElement)
{
	bool First{true};
	
	for(auto EqualsChildNode : EqualsElement->GetChilds())
	{
		if(EqualsChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto EqualsChildElement{dynamic_cast< const XML::Element * >(EqualsChildNode)};
			
			if(First == true)
			{
				_LoadStatement(Equals.Statement1, EqualsChildElement);
				First = false;
			}
			else
			{
				_LoadStatement(Equals.Statement2, EqualsChildElement);
			}
		}
	}
}

void Inspection::Type::_LoadLength(Inspection::TypeDefinition::Length & Length, const XML::Element * LengthElement)
{
	for(auto LengthChildNode : LengthElement->GetChilds())
	{
		if(LengthChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto LengthChildElement{dynamic_cast< const XML::Element * >(LengthChildNode)};
			
			if(LengthChildElement->GetName() == "bytes")
			{
				_LoadStatementFromWithin(Length.Bytes, LengthChildElement);
			}
			else if(LengthChildElement->GetName() == "bits")
			{
				_LoadStatementFromWithin(Length.Bits, LengthChildElement);
			}
			else
			{
				throw std::domain_error{"length/" + LengthChildElement->GetName() + " not allowed."};
			}
		}
	}
}

void Inspection::Type::_LoadParameter(Inspection::TypeDefinition::Parameter & Parameter, const XML::Element * ParameterElement)
{
	assert(ParameterElement->HasAttribute("name") == true);
	Parameter.Name = ParameterElement->GetAttribute("name");
	_LoadStatementFromWithin(Parameter.Statement, ParameterElement);
}

void Inspection::Type::_LoadParameters(Inspection::TypeDefinition::Parameters & Parameters, const XML::Element * ParametersElement)
{
	for(auto ParametersChildNode : ParametersElement->GetChilds())
	{
		if(ParametersChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ParametersChildElement{dynamic_cast< const XML::Element * >(ParametersChildNode)};
			
			if(ParametersChildElement->GetName() == "parameter")
			{
				Parameters.Parameters.emplace_back();
				
				auto & Parameter{Parameters.Parameters.back()};
				
				_LoadParameter(Parameter, ParametersChildElement);
			}
			else
			{
				throw std::domain_error{ParametersChildElement->GetName() + " not allowed."};
			}
		}
	}
}

void Inspection::Type::_LoadPart(Inspection::TypeDefinition::Part & Part, const XML::Element * PartElement)
{
	if(PartElement->GetName() == "sequence")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Sequence;
		Part.Parts.emplace();
	}
	else if(PartElement->GetName() == "field")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Field;
		Part.FieldName = PartElement->GetAttribute("name");
	}
	else if(PartElement->GetName() == "fields")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Fields;
	}
	else if(PartElement->GetName() == "forward")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Forward;
	}
	else
	{
		throw std::domain_error{"\"" + PartElement->GetName() + "\" not allowed."};
	}
	for(auto PartChildNode : PartElement->GetChilds())
	{
		if(PartChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto PartChildElement{dynamic_cast< const XML::Element * >(PartChildNode)};
			
			if(PartChildElement->GetName() == "type-reference")
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Fields) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward));
				Part.TypeReference.emplace();
				_LoadTypeReference(Part.TypeReference.value(), PartChildElement);
			}
			else if(PartChildElement->GetName() == "interpretation")
			{
				Part.Interpretation.emplace();
				_LoadInterpretation(Part.Interpretation.value(), PartChildElement);
			}
			else if(PartChildElement->GetName() == "length")
			{
				Part.Length.emplace();
				_LoadLength(Part.Length.value(), PartChildElement);
			}
			else if(PartChildElement->GetName() == "parameters")
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Fields) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward));
				Part.Parameters.emplace();
				_LoadParameters(Part.Parameters.value(), PartChildElement);
			}
			else if(PartChildElement->GetName() == "verification")
			{
				Part.Verifications.emplace_back();
				
				auto & Verification{Part.Verifications.back()};
				
				for(auto GetterPartVerificationChildNode : PartChildElement->GetChilds())
				{
					if(GetterPartVerificationChildNode->GetNodeType() == XML::NodeType::Element)
					{
						_LoadStatement(Verification, dynamic_cast< XML::Element * >(GetterPartVerificationChildNode));
					}
				}
			}
			else if(PartChildElement->GetName() == "tag")
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward));
				Part.Tags.emplace_back();
				
				auto & Tag{Part.Tags.back()};
				
				_LoadTag(Tag, PartChildElement);
			}
			else if((PartChildElement->GetName() == "sequence") || (PartChildElement->GetName() == "field") || (PartChildElement->GetName() == "fields") || (PartChildElement->GetName() == "forward"))
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Sequence);
				Part.Parts->emplace_back();
				
				auto & ContainedPart{Part.Parts->back()};
				
				_LoadPart(ContainedPart, PartChildElement);
			}
		}
	}
}

void Inspection::Type::_LoadStatement(Inspection::TypeDefinition::Statement & Statement, const XML::Element * StatementElement)
{
	assert(Statement.Type == Inspection::TypeDefinition::Statement::Type::Unknown);
	// statement element may be nullptr, if it represents the "nothing" value
	if((StatementElement != nullptr) && (StatementElement->GetName() == "divide"))
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Divide;
		Statement.Divide = new Inspection::TypeDefinition::Divide{};
		_LoadDivide(*(Statement.Divide), StatementElement);
	}
	else if((StatementElement != nullptr) && (StatementElement->GetName() == "equals"))
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Equals;
		Statement.Equals = new Inspection::TypeDefinition::Equals{};
		_LoadEquals(*(Statement.Equals), StatementElement);
	}
	else if((StatementElement != nullptr) && (StatementElement->GetName() == "unsigned-integer-64bit") && (XML::HasChildElements(StatementElement) == true))
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Cast;
		Statement.Cast = new Inspection::TypeDefinition::Cast{};
		_LoadCast(*(Statement.Cast), StatementElement);
	}
	else if((StatementElement != nullptr) && (StatementElement->GetName() == "single-precision-real") && (XML::HasChildElements(StatementElement) == true))
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Cast;
		Statement.Cast = new Inspection::TypeDefinition::Cast{};
		_LoadCast(*(Statement.Cast), StatementElement);
	}
	else
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Value;
		Statement.Value = new Inspection::TypeDefinition::Value{};
		_LoadValue(*(Statement.Value), StatementElement);
	}
	assert(Statement.Type != Inspection::TypeDefinition::Statement::Type::Unknown);
}

void Inspection::Type::_LoadStatementFromWithin(Inspection::TypeDefinition::Statement & Statement, const XML::Element * ParentElement)
{
	XML::Element * StatementElement{nullptr};
	
	if(ParentElement->GetChilds().size() > 0)
	{
		for(auto ChildNode : ParentElement->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				StatementElement = dynamic_cast< XML::Element * >(ChildNode);
				
				break;
			}
		}
		if(StatementElement == nullptr)
		{
			throw std::domain_error{"To read a statment from an element with childs, at least one of them needs to be an element."};
		}
	}
	// the statement element may still be nullptr if it represents the "nothing" value
	_LoadStatement(Statement, StatementElement);
}

void Inspection::Type::_LoadTag(Inspection::TypeDefinition::Tag & Tag, const XML::Element * TagElement)
{
	assert(TagElement->HasAttribute("name") == true);
	Tag.Name = TagElement->GetAttribute("name");
	_LoadStatementFromWithin(Tag.Statement, TagElement);
}

void Inspection::Type::_LoadTypeReference(Inspection::TypeDefinition::TypeReference & TypeReference, const XML::Element * TypeReferenceElement)
{
	for(auto TypeReferenceChildNode : TypeReferenceElement->GetChilds())
	{
		if(TypeReferenceChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto TypeReferenceChildElement{dynamic_cast< const XML::Element * >(TypeReferenceChildNode)};
			
			if(TypeReferenceChildElement->GetName() == "part")
			{
				assert(TypeReferenceChildElement->GetChilds().size() == 1);
				
				auto TypeReferencePartText{dynamic_cast< const XML::Text * >(TypeReferenceChildElement->GetChild(0))};
				
				assert(TypeReferencePartText != nullptr);
				TypeReference.Parts.push_back(TypeReferencePartText->GetText());
			}
			else
			{
				throw std::domain_error{"type-reference/" + TypeReferenceChildElement->GetName() + " not allowed."};
			}
		}
	}
}

void Inspection::Type::_LoadValueFromWithin(Inspection::TypeDefinition::Value & Value, const XML::Element * ParentElement)
{
	if(ParentElement->GetChilds().size() == 0)
	{
		_LoadValue(Value, nullptr);
	}
	else
	{
		for(auto ChildNode : ParentElement->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				_LoadValue(Value, dynamic_cast< XML::Element * >(ChildNode));
			}
		}
	}
	if(Value.DataType == Inspection::TypeDefinition::DataType::Unknown)
	{
		throw std::domain_error{"No valid value description."};
	}
}

void Inspection::Type::_LoadValue(Inspection::TypeDefinition::Value & Value, const XML::Element * ValueElement)
{
	if(ValueElement == nullptr)
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::Nothing;
	}
	else if(ValueElement->GetName() == "data-reference")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::DataReference;
		Value.DataReference.emplace();
		for(auto DataReferenceChildNode : ValueElement->GetChilds())
		{
			if(DataReferenceChildNode->GetNodeType() == XML::NodeType::Element)
			{
				auto & DataReferencePartDescriptors{Value.DataReference->PartDescriptors};
				auto DataReferencePartDescriptor{DataReferencePartDescriptors.emplace(DataReferencePartDescriptors.end())};
				auto DataReferenceChildElement{dynamic_cast< const XML::Element * >(DataReferenceChildNode)};
				
				if(DataReferenceChildElement->GetName() == "field")
				{
					assert(DataReferenceChildElement->GetChilds().size() == 1);
					DataReferencePartDescriptor->Type = Inspection::TypeDefinition::DataReference::PartDescriptor::Type::Field;
					
					auto SubText{dynamic_cast< const XML::Text * >(DataReferenceChildElement->GetChild(0))};
					
					assert(SubText != nullptr);
					DataReferencePartDescriptor->DetailName = SubText->GetText();
				}
				else if(DataReferenceChildElement->GetName() == "tag")
				{
					assert(DataReferenceChildElement->GetChilds().size() == 1);
					DataReferencePartDescriptor->Type = Inspection::TypeDefinition::DataReference::PartDescriptor::Type::Tag;
					
					auto TagText{dynamic_cast< const XML::Text * >(DataReferenceChildElement->GetChild(0))};
					
					assert(TagText != nullptr);
					DataReferencePartDescriptor->DetailName = TagText->GetText();
				}
				else
				{
					throw std::domain_error{DataReferenceChildElement->GetName() + " not allowed."};
				}
			}
		}
	}
	else if(ValueElement->GetName() == "parameter-reference")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::ParameterReference;
		Value.ParameterReference.emplace();
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.ParameterReference->Name = TextNode->GetText();
	}
	else if(ValueElement->GetName() == "boolean")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::Boolean;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.Boolean = from_string_cast< bool >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "string")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::String;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.String = TextNode->GetText();
	}
	else if(ValueElement->GetName() == "type-reference")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::TypeReference;
		Value.TypeReference.emplace();
		_LoadTypeReference(Value.TypeReference.value(), ValueElement);
	}
	else if(ValueElement->GetName() == "unsigned-integer-8bit")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger8Bit = from_string_cast< std::uint8_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-16bit")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger8Bit = from_string_cast< std::uint16_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-32bit")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger32Bit = from_string_cast< std::uint32_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-64bit")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger64Bit = from_string_cast< std::uint64_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "single-precision-real")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::SinglePrecisionReal;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.SinglePrecisionReal = from_string_cast< float >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "parameters")
	{
		assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
		Value.DataType = Inspection::TypeDefinition::DataType::Parameters;
		Value.Parameters.emplace();
		_LoadParameters(Value.Parameters.value(), ValueElement);
	}
	else
	{
		throw std::domain_error{ValueElement->GetName() + " not allowed."};
	}
}
