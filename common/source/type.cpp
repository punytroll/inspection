#include <experimental/optional>
#include <fstream>

#include "execution_context.h"
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
	
	namespace Algorithms
	{
		std::experimental::any Divide(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Statement & Dividend, const Inspection::TypeDefinition::Statement & Divisor);
		std::experimental::any GetAnyFromCast(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Cast & Cast);
		std::experimental::any GetAnyFromStatement(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Statement & Statement);
		const std::experimental::any & GetAnyReferenceFromDataReference(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::DataReference & DataReference);
		
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
				std::cout << Any.type().name() << std::endl;
				assert(false);
			}
		}
		
		template< typename Type >
		Type GetDataFromCast(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Cast & Cast)
		{
			return Inspection::Algorithms::Cast< Type >(Inspection::Algorithms::GetAnyFromStatement(ExecutionContext, Cast.Statement));
		}
		
		template< typename Type >
		Type GetDataFromValue(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Value & Value)
		{
			Type Result{};
			
			if(Value.DataType == Inspection::TypeDefinition::DataType::DataReference)
			{
				assert(Value.DataReference);
				
				auto & Any{Inspection::Algorithms::GetAnyReferenceFromDataReference(ExecutionContext, Value.DataReference.value())};
				
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
		
		template< typename Type >
		Type GetDataFromStatement(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Statement & Statement)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Cast:
				{
					assert(Statement.Cast != nullptr);
					
					return Inspection::Algorithms::GetDataFromCast< Type >(ExecutionContext, *(Statement.Cast));
				}
			case Inspection::TypeDefinition::Statement::Type::Value:
				{
					assert(Statement.Value != nullptr);
					
					return Inspection::Algorithms::GetDataFromValue< Type >(ExecutionContext, *(Statement.Value));
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		const std::experimental::any & GetAnyReferenceFromDataReference(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::DataReference & DataReference)
		{
			auto Value{ExecutionContext.GetValueFromDataReference(DataReference)};
			
			assert(Value != nullptr);
			
			return Value->GetData();
		}
		
		const std::experimental::any & GetAnyReferenceFromParameterReference(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::ParameterReference & ParameterReference)
		{
			return ExecutionContext.GetAnyReferenceFromParameterReference(ParameterReference);
		}
		
		std::experimental::any GetAnyFromValue(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Value & Value)
		{
			switch(Value.DataType)
			{
			case Inspection::TypeDefinition::DataType::Boolean:
				{
					assert(Value.Boolean);
					
					return Value.Boolean.value();
				}
			case Inspection::TypeDefinition::DataType::DataReference:
				{
					assert(Value.DataReference);
					
					return Inspection::Algorithms::GetAnyReferenceFromDataReference(ExecutionContext, Value.DataReference.value());
				}
			case Inspection::TypeDefinition::DataType::Length:
				{
					assert(Value.Length);
					
					return Inspection::Length{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, Value.Length->Bytes), Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, Value.Length->Bits)};
				}
			case Inspection::TypeDefinition::DataType::Nothing:
				{
					return nullptr;
				}
			case Inspection::TypeDefinition::DataType::ParameterReference:
				{
					assert(Value.ParameterReference);
					
					return Inspection::Algorithms::GetAnyReferenceFromParameterReference(ExecutionContext, Value.ParameterReference.value());
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
			case Inspection::TypeDefinition::DataType::TypeReference:
				{
					assert(Value.TypeReference);
					
					return Inspection::g_TypeRepository.GetType(Value.TypeReference->Parts);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
				{
					assert(Value.UnsignedInteger8Bit);
					
					return Value.UnsignedInteger8Bit.value();
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
				{
					assert(Value.UnsignedInteger16Bit);
					
					return Value.UnsignedInteger16Bit.value();
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
				{
					assert(Value.UnsignedInteger32Bit);
					
					return Value.UnsignedInteger32Bit.value();
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
				{
					assert(Value.UnsignedInteger64Bit);
					
					return Value.UnsignedInteger64Bit.value();
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		void ApplyTags(Inspection::ExecutionContext & ExecutionContext, const std::vector< Inspection::TypeDefinition::Tag > & Tags, std::shared_ptr< Inspection::Value > Target)
		{
			for(auto & Tag : Tags)
			{
				assert((Tag.Statement.Type == Inspection::TypeDefinition::Statement::Type::Value) && (Tag.Statement.Value != nullptr));
				Target->AddTag(Tag.Name, GetAnyFromValue(ExecutionContext, *(Tag.Statement.Value)));
			}
		}
		
		template< typename DataType >
		bool ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target)
		{
			bool Result{false};
			auto BaseValueString{to_string_cast(std::experimental::any_cast< const DataType & >(Target->GetData()))};
			auto ElementIterator{std::find_if(Enumeration.Elements.begin(), Enumeration.Elements.end(), [BaseValueString](auto & Element){ return Element.BaseValue == BaseValueString; })};
			
			if(ElementIterator != Enumeration.Elements.end())
			{
				ApplyTags(ExecutionContext, ElementIterator->Tags, Target);
				Result = ElementIterator->Valid;
			}
			else
			{
				if(Enumeration.FallbackElement)
				{
					ApplyTags(ExecutionContext, Enumeration.FallbackElement->Tags, Target);
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
		
		std::experimental::any GetAnyFromStatement(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Statement & Statement)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Cast:
				{
					assert(Statement.Cast != nullptr);
					
					return Inspection::Algorithms::GetAnyFromCast(ExecutionContext, *(Statement.Cast));
				}
			case Inspection::TypeDefinition::Statement::Type::Divide:
				{
					assert(Statement.Divide != nullptr);
					
					return Inspection::Algorithms::Divide(ExecutionContext, Statement.Divide->Dividend, Statement.Divide->Divisor);
				}
			case Inspection::TypeDefinition::Statement::Type::Value:
				{
					assert(Statement.Value != nullptr);
					
					return Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *(Statement.Value));
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		std::experimental::any GetAnyFromCast(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Cast & Cast)
		{
			switch(Cast.DataType)
			{
			case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
				{
					return Inspection::Algorithms::GetDataFromCast< float >(ExecutionContext, Cast);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
				{
					return Inspection::Algorithms::GetDataFromCast< std::uint64_t >(ExecutionContext, Cast);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		bool Equals(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Value & Value1, const Inspection::TypeDefinition::Value & Value2)
		{
			auto Any1{Inspection::Algorithms::GetAnyFromValue(ExecutionContext, Value1)};
			auto Any2{Inspection::Algorithms::GetAnyFromValue(ExecutionContext, Value2)};
			
			if((Any1.empty() == false) && (Any2.empty() == false))
			{
				if(Any1.type() == Any2.type())
				{
					if(Any1.type() == typeid(std::uint8_t))
					{
						return std::experimental::any_cast< std::uint8_t >(Any1) == std::experimental::any_cast< std::uint8_t >(Any2);
					}
					else if(Any1.type() == typeid(std::uint16_t))
					{
						return std::experimental::any_cast< std::uint16_t >(Any1) == std::experimental::any_cast< std::uint16_t >(Any2);
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
		
		bool Equals(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Statement & Statement1, const Inspection::TypeDefinition::Statement & Statement2)
		{
			if(Statement1.Type == Inspection::TypeDefinition::Statement::Type::Value)
			{
				assert(Statement1.Value != nullptr);
				if(Statement2.Type == Inspection::TypeDefinition::Statement::Type::Value)
				{
					assert(Statement2.Value != nullptr);
					
					return Inspection::Algorithms::Equals(ExecutionContext, *(Statement1.Value), *(Statement2.Value));
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
		
		std::experimental::any Divide(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Statement & Dividend, const Inspection::TypeDefinition::Statement & Divisor)
		{
			auto AnyDivident{Inspection::Algorithms::GetAnyFromStatement(ExecutionContext, Dividend)};
			auto AnyDivisor{Inspection::Algorithms::GetAnyFromStatement(ExecutionContext, Divisor)};
			
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
	
	void FillNewParameters(ExecutionContext & ExecutionContext, std::unordered_map< std::string, std::experimental::any > & NewParameters, const Inspection::TypeDefinition::Parameters & ParameterDefinitions)
	{
		for(auto & ParameterDefinition : ParameterDefinitions.Parameters)
		{
			switch(ParameterDefinition.Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Cast:
				{
					assert(ParameterDefinition.Statement.Cast != nullptr);
					NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromCast(ExecutionContext, *(ParameterDefinition.Statement.Cast)));
					
					break;
				}
			case Inspection::TypeDefinition::Statement::Type::Value:
				{
					assert(ParameterDefinition.Statement.Value != nullptr);
					if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::String)
					{
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *(ParameterDefinition.Statement.Value)));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::DataReference)
					{
						assert(ParameterDefinition.Statement.Value->DataReference);
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *(ParameterDefinition.Statement.Value)));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::TypeReference)
					{
						assert(ParameterDefinition.Statement.Value->TypeReference);
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *(ParameterDefinition.Statement.Value)));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::ParameterReference)
					{
						assert(ParameterDefinition.Statement.Value->ParameterReference);
						NewParameters.emplace(ParameterDefinition.Name, Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *(ParameterDefinition.Statement.Value)));
					}
					else if(ParameterDefinition.Statement.Value->DataType == Inspection::TypeDefinition::DataType::Parameters)
					{
						assert(ParameterDefinition.Statement.Value->Parameters);
						
						std::unordered_map< std::string, std::experimental::any > InnerParameters;
						
						FillNewParameters(ExecutionContext, InnerParameters, ParameterDefinition.Statement.Value->Parameters.value());
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

Inspection::TypeDefinition::Type::Type(Inspection::TypeRepository * TypeRepository) :
	_Part{nullptr},
	_TypeRepository{TypeRepository}
{
}

Inspection::TypeDefinition::Type::~Type(void)
{
	delete _Part;
}

std::unique_ptr< Inspection::Result > Inspection::TypeDefinition::Type::Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const
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
			Inspection::ExecutionContext ExecutionContext;
			
			ExecutionContext.Push(Result, Parameters);
			
			Inspection::Reader * PartReader{nullptr};
			
			if(_Part->Length)
			{
				auto Bytes{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, _Part->Length->Bytes)};
				auto Bits{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, _Part->Length->Bits)};
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
				std::unordered_map< std::string, std::experimental::any > PartParameters;
				
				if(_Part->Parameters)
				{
					FillNewParameters(ExecutionContext, PartParameters, _Part->Parameters.value());
				}
				switch(_Part->Type)
				{
				case Inspection::TypeDefinition::Part::Type::Sequence:
					{
						auto SequenceResult{_GetSequence(ExecutionContext, *_Part, *PartReader, PartParameters)};
						
						Continue = SequenceResult->GetSuccess();
						Result->SetValue(SequenceResult->GetValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Field:
					{
						auto FieldResult{_GetField(ExecutionContext, *_Part, *PartReader, PartParameters)};
						
						Continue = FieldResult->GetSuccess();
						Result->GetValue()->AppendField(_Part->FieldName.value(), FieldResult->GetValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Fields:
					{
						auto FieldsResult{_GetFields(ExecutionContext, *_Part, *PartReader, PartParameters)};
						
						Continue = FieldsResult->GetSuccess();
						Result->GetValue()->AppendFields(FieldsResult->GetValue()->GetFields());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Forward:
					{
						auto ForwardResult{_GetForward(ExecutionContext, *_Part, *PartReader, PartParameters)};
						
						Continue = ForwardResult->GetSuccess();
						Result->SetValue(ForwardResult->GetValue());
						
						break;
					}
				default:
					{
						assert(false);
						
						break;
					}
				}
				Reader.AdvancePosition(PartReader->GetConsumedLength());
			}
			ExecutionContext.Pop();
			assert(ExecutionContext._ExecutionStack.size() == 0);
		}
		else
		{
			assert(false);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::TypeDefinition::Type::_GetArray(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Array, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	ExecutionContext.Push(Array, Result, Parameters);
	assert(Array.Array);
	
	auto IterateField{ExecutionContext.GetFieldFromFieldReference(Array.Array->IterateField)};
	
	assert(IterateField != nullptr);
	
	std::vector< std::pair< Inspection::Length, Inspection::Length > > ElementProperties;
	
	for(auto Field : IterateField->GetFields())
	{
		ElementProperties.emplace_back(std::experimental::any_cast< const Inspection::Length & >(Field->GetTag("position")->GetData()), std::experimental::any_cast< const Inspection::Length & >(Field->GetTag("length")->GetData()));
	}
	std::sort(std::begin(ElementProperties), std::end(ElementProperties));
	for(auto ElementPropertiesIndex = 0ul; (Continue == true) && (ElementPropertiesIndex < ElementProperties.size()); ++ElementPropertiesIndex)
	{
		auto & Properties{ElementProperties[ElementPropertiesIndex]};
		
		assert(Reader.GetPositionInBuffer() == Properties.first);
		
		Inspection::Reader ElementReader{Reader, Properties.second};
		auto ElementResult{Inspection::g_TypeRepository.GetType(Array.Array->ElementType.Parts)->Get(ElementReader, ExecutionContext.GetAllParameters())};
		
		Continue = ElementResult->GetSuccess();
		Result->GetValue()->AppendField(Array.Array->ElementName, ElementResult->GetValue());
		Reader.AdvancePosition(ElementReader.GetConsumedLength());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::TypeDefinition::Type::_GetField(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Field, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	ExecutionContext.Push(Field, Result, Parameters);
	if(Field.TypeReference)
	{
		auto FieldResult{Inspection::g_TypeRepository.GetType(Field.TypeReference->Parts)->Get(Reader, ExecutionContext.GetAllParameters())};
		
		Continue = FieldResult->GetSuccess();
		Result->SetValue(FieldResult->GetValue());
	}
	else if(Field.Parts)
	{
		assert(Field.Parts->size() == 1);
		
		auto & FieldPart{Field.Parts->front()};
		Inspection::Reader * FieldPartReader{nullptr};
		
		if(FieldPart.Length)
		{
			auto Bytes{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, FieldPart.Length->Bytes)};
			auto Bits{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, FieldPart.Length->Bits)};
			Inspection::Length Length{Bytes, Bits};
			
			if(Reader.Has(Length) == true)
			{
				FieldPartReader = new Inspection::Reader{Reader, Length};
			}
			else
			{
				Result->GetValue()->AddTag("error", "At least " + to_string_cast(Length) + " bytes and bits are necessary to read this part.");
				Continue = false;
			}
		}
		else
		{
			FieldPartReader = new Inspection::Reader{Reader};
		}
		if(FieldPartReader != nullptr)
		{
			std::unordered_map< std::string, std::experimental::any > FieldPartParameters;
			
			if(FieldPart.Parameters)
			{
				FillNewParameters(ExecutionContext, FieldPartParameters, FieldPart.Parameters.value());
			}
			switch(FieldPart.Type)
			{
			case Inspection::TypeDefinition::Part::Type::Field:
				{
					auto FieldResult{_GetField(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters)};
					
					Continue = FieldResult->GetSuccess();
					assert(FieldPart.FieldName);
					Result->GetValue()->AppendField(FieldPart.FieldName.value(), FieldResult->GetValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Fields:
				{
					auto FieldsResult{_GetFields(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters)};
					
					Continue = FieldsResult->GetSuccess();
					Result->GetValue()->AppendFields(FieldsResult->GetValue()->GetFields());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Forward:
				{
					auto ForwardResult{_GetForward(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters)};
					
					Continue = ForwardResult->GetSuccess();
					Result->SetValue(ForwardResult->GetValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Sequence:
				{
					auto SequenceResult{_GetSequence(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters)};
					
					Continue = SequenceResult->GetSuccess();
					Result->GetValue()->AppendFields(SequenceResult->GetValue()->GetFields());
					
					break;
				}
			default:
				{
					assert(false);
				}
			}
			Reader.AdvancePosition(FieldPartReader->GetConsumedLength());
		}
	}
	// tags
	if(Continue == true)
	{
		if(Field.Tags.empty() == false)
		{
			assert((Field.Type == Inspection::TypeDefinition::Part::Type::Field) || (Field.Type == Inspection::TypeDefinition::Part::Type::Forward));
			for(auto & Tag : Field.Tags)
			{
				Result->GetValue()->AddTag(Tag.Name, Inspection::Algorithms::GetAnyFromStatement(ExecutionContext, Tag.Statement));
			}
		}
	}
	// interpretation
	if(Continue == true)
	{
		if(Field.Interpretation)
		{
			auto EvaluationResult{_ApplyInterpretation(ExecutionContext, Field.Interpretation.value(), Result->GetValue())};
			
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
		for(auto & Statement : Field.Verifications)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Equals:
				{
					assert(Statement.Equals);
					Continue = Inspection::Algorithms::Equals(ExecutionContext, Statement.Equals->Statement1, Statement.Equals->Statement2);
					if(Continue == false)
					{
						Result->GetValue()->AddTag("error", "The value failed to verify."s);
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
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::TypeDefinition::Type::_GetFields(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Fields, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	ExecutionContext.Push(Fields, Result, Parameters);
	assert(Fields.TypeReference);
	
	auto FieldsResult{Inspection::g_TypeRepository.GetType(Fields.TypeReference->Parts)->Get(Reader, ExecutionContext.GetAllParameters())};
	
	Continue = FieldsResult->GetSuccess();
	Result->SetValue(FieldsResult->GetValue());
	// interpretation
	if(Continue == true)
	{
		if(Fields.Interpretation)
		{
			auto EvaluationResult{_ApplyInterpretation(ExecutionContext, Fields.Interpretation.value(), Result->GetValue())};
			
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
		for(auto & Statement : Fields.Verifications)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Equals:
				{
					assert(Statement.Equals);
					Continue = Inspection::Algorithms::Equals(ExecutionContext, Statement.Equals->Statement1, Statement.Equals->Statement2);
					if(Continue == false)
					{
						Result->GetValue()->AddTag("error", "The value failed to verify."s);
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
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::TypeDefinition::Type::_GetForward(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Forward, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	ExecutionContext.Push(Forward, Result, Parameters);
	assert(Forward.TypeReference);
	
	auto ForwardResult{Inspection::g_TypeRepository.GetType(Forward.TypeReference->Parts)->Get(Reader, ExecutionContext.GetAllParameters())};
	
	Continue = ForwardResult->GetSuccess();
	Result->SetValue(ForwardResult->GetValue());
	// tags
	if(Continue == true)
	{
		if(Forward.Tags.empty() == false)
		{
			for(auto & Tag : Forward.Tags)
			{
				Result->GetValue()->AddTag(Tag.Name, Inspection::Algorithms::GetAnyFromStatement(ExecutionContext, Tag.Statement));
			}
		}
	}
	// interpretation
	if(Continue == true)
	{
		if(Forward.Interpretation)
		{
			auto EvaluationResult{_ApplyInterpretation(ExecutionContext, Forward.Interpretation.value(), Result->GetValue())};
			
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
		for(auto & Statement : Forward.Verifications)
		{
			switch(Statement.Type)
			{
			case Inspection::TypeDefinition::Statement::Type::Equals:
				{
					assert(Statement.Equals);
					Continue = Inspection::Algorithms::Equals(ExecutionContext, Statement.Equals->Statement1, Statement.Equals->Statement2);
					if(Continue == false)
					{
						Result->GetValue()->AddTag("error", "The value failed to verify."s);
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
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::TypeDefinition::Type::_GetSequence(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Sequence, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	ExecutionContext.Push(Sequence, Result, Parameters);
	assert(Sequence.Parts);
	for(auto SequencePartIterator = std::begin(Sequence.Parts.value()); ((Continue == true) && (SequencePartIterator != std::end(Sequence.Parts.value()))); ++SequencePartIterator)
	{
		auto & SequencePart{*SequencePartIterator};
		Inspection::Reader * SequencePartReader{nullptr};
		
		if(SequencePart.Length)
		{
			auto Bytes{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, SequencePart.Length->Bytes)};
			auto Bits{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(ExecutionContext, SequencePart.Length->Bits)};
			Inspection::Length Length{Bytes, Bits};
			
			if(Reader.Has(Length) == true)
			{
				SequencePartReader = new Inspection::Reader{Reader, Length};
			}
			else
			{
				Result->GetValue()->AddTag("error", "At least " + to_string_cast(Length) + " bytes and bits are necessary to read this part.");
				Continue = false;
			}
		}
		else
		{
			SequencePartReader = new Inspection::Reader{Reader};
		}
		if(SequencePartReader != nullptr)
		{
			std::unordered_map< std::string, std::experimental::any > SequencePartParameters;
			
			if(SequencePart.Parameters)
			{
				FillNewParameters(ExecutionContext, SequencePartParameters, SequencePart.Parameters.value());
			}
			switch(SequencePart.Type)
			{
			case Inspection::TypeDefinition::Part::Type::Array:
				{
					auto ArrayResult{_GetArray(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters)};
					
					Continue = ArrayResult->GetSuccess();
					assert(SequencePart.FieldName);
					Result->GetValue()->AppendField(SequencePart.FieldName.value(), ArrayResult->GetValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Field:
				{
					auto FieldResult{_GetField(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters)};
					
					Continue = FieldResult->GetSuccess();
					assert(SequencePart.FieldName);
					Result->GetValue()->AppendField(SequencePart.FieldName.value(), FieldResult->GetValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Fields:
				{
					auto FieldsResult{_GetFields(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters)};
					
					Continue = FieldsResult->GetSuccess();
					Result->GetValue()->AppendFields(FieldsResult->GetValue()->GetFields());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Forward:
				{
					auto ForwardResult{_GetForward(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters)};
					
					Continue = ForwardResult->GetSuccess();
					Result->SetValue(ForwardResult->GetValue());
					
					break;
				}
			default:
				{
					assert(false);
				}
			}
			Reader.AdvancePosition(SequencePartReader->GetConsumedLength());
		}
	}
	// tags
	if(Continue == true)
	{
		for(auto & Tag : Sequence.Tags)
		{
			Result->GetValue()->AddTag(Tag.Name, Inspection::Algorithms::GetAnyFromStatement(ExecutionContext, Tag.Statement));
		}
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

Inspection::EvaluationResult Inspection::TypeDefinition::Type::_ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target) const
{
	Inspection::EvaluationResult Result;
	
	if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::String)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration< std::string >(ExecutionContext, Enumeration, Target);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger8Bit)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration< std::uint8_t >(ExecutionContext, Enumeration, Target);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger16Bit)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration< std::uint16_t >(ExecutionContext, Enumeration, Target);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger32Bit)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration< std::uint32_t >(ExecutionContext, Enumeration, Target);
	}
	else
	{
		Target->AddTag("error", "Could not handle the enumeration base type.");
		Result.AbortEvaluation = true;
	}
	
	return Result;
}
	
Inspection::EvaluationResult Inspection::TypeDefinition::Type::_ApplyInterpretation(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Interpretation & Interpretation, std::shared_ptr< Inspection::Value > Target) const
{
	Inspection::EvaluationResult Result;
	
	switch(Interpretation.Type)
	{
	case Inspection::TypeDefinition::Interpretation::Type::ApplyEnumeration:
		{
			assert(Interpretation.ApplyEnumeration);
			
			auto EvaluationResult{_ApplyEnumeration(ExecutionContext, Interpretation.ApplyEnumeration->Enumeration, Target)};
			
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

void Inspection::TypeDefinition::Type::Load(const std::string & TypePath)
{
	try
	{
		std::ifstream InputFileStream{TypePath};
		XML::Document Document{InputFileStream};
		auto DocumentElement{Document.GetDocumentElement()};
		
		assert(DocumentElement != nullptr);
		assert(DocumentElement->GetName() == "type");
		_LoadType(*this, DocumentElement);
	}
	catch(std::domain_error & Exception)
	{
		std::throw_with_nested(std::runtime_error("Type path: " + TypePath));
	}
}

void Inspection::TypeDefinition::Type::_LoadCast(Inspection::TypeDefinition::Cast & Cast, const XML::Element * CastElement)
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

void Inspection::TypeDefinition::Type::_LoadDivide(Inspection::TypeDefinition::Divide & Divide, const XML::Element * DivideElement)
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

void Inspection::TypeDefinition::Type::_LoadInterpretation(Inspection::TypeDefinition::Interpretation & Interpretation, const XML::Element * InterpretationElement)
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

void Inspection::TypeDefinition::Type::_LoadEnumeration(Inspection::TypeDefinition::Enumeration & Enumeration, const XML::Element * EnumerationElement)
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

void Inspection::TypeDefinition::Type::_LoadEquals(Inspection::TypeDefinition::Equals & Equals, const XML::Element * EqualsElement)
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

void Inspection::TypeDefinition::Type::_LoadFieldReference(Inspection::TypeDefinition::FieldReference & FieldReference, const XML::Element * FieldReferenceElement)
{
	assert(FieldReferenceElement->HasAttribute("root") == true);
	if(FieldReferenceElement->GetAttribute("root") == "current")
	{
		FieldReference.Root = Inspection::TypeDefinition::FieldReference::Root::Current;
	}
	else
	{
		FieldReference.Root = Inspection::TypeDefinition::FieldReference::Root::Type;
	}
	for(auto FieldReferenceChildNode : FieldReferenceElement->GetChilds())
	{
		if(FieldReferenceChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto FieldReferenceChildElement{dynamic_cast< const XML::Element * >(FieldReferenceChildNode)};
			
			assert(FieldReferenceChildElement->GetName() == "field");
			assert(FieldReferenceChildElement->GetChilds().size() == 1);
			
			auto FieldReferenceFieldText{dynamic_cast< const XML::Text * >(FieldReferenceChildElement->GetChild(0))};
			
			assert(FieldReferenceFieldText != nullptr);
			FieldReference.Parts.emplace_back();
			FieldReference.Parts.back().FieldName = FieldReferenceFieldText->GetText();
		}
	}
}

void Inspection::TypeDefinition::Type::_LoadLength(Inspection::TypeDefinition::Length & Length, const XML::Element * LengthElement)
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

void Inspection::TypeDefinition::Type::_LoadParameter(Inspection::TypeDefinition::Parameter & Parameter, const XML::Element * ParameterElement)
{
	assert(ParameterElement->HasAttribute("name") == true);
	Parameter.Name = ParameterElement->GetAttribute("name");
	_LoadStatementFromWithin(Parameter.Statement, ParameterElement);
}

void Inspection::TypeDefinition::Type::_LoadParameters(Inspection::TypeDefinition::Parameters & Parameters, const XML::Element * ParametersElement)
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

void Inspection::TypeDefinition::Type::_LoadPart(Inspection::TypeDefinition::Part & Part, const XML::Element * PartElement)
{
	if(PartElement->GetName() == "array")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Array;
		Part.FieldName = PartElement->GetAttribute("name");
		Part.Array.emplace();
	}
	else if(PartElement->GetName() == "sequence")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Sequence;
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
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward) || (Part.Type == Inspection::TypeDefinition::Part::Type::Sequence));
				Part.Tags.emplace_back();
				
				auto & Tag{Part.Tags.back()};
				
				_LoadTag(Tag, PartChildElement);
			}
			else if((PartChildElement->GetName() == "sequence") || (PartChildElement->GetName() == "field") || (PartChildElement->GetName() == "fields") || (PartChildElement->GetName() == "forward") || (PartChildElement->GetName() == "array"))
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Sequence) || ((Part.Type == Inspection::TypeDefinition::Part::Type::Field) && (!Part.Parts)));
				if(!Part.Parts)
				{
					Part.Parts.emplace();
				}
				Part.Parts->emplace_back();
				
				auto & ContainedPart{Part.Parts->back()};
				
				_LoadPart(ContainedPart, PartChildElement);
			}
			else if(PartChildElement->GetName() == "iterate")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				
				XML::Element * FieldReferenceElement{nullptr};
				
				for(auto PartIterateChildNode : PartChildElement->GetChilds())
				{
					if(PartIterateChildNode->GetNodeType() == XML::NodeType::Element)
					{
						assert(FieldReferenceElement == nullptr);
						FieldReferenceElement = dynamic_cast< XML::Element * >(PartIterateChildNode);
					}
				}
				assert(FieldReferenceElement != nullptr);
				_LoadFieldReference(Part.Array->IterateField, FieldReferenceElement);
			}
			else if(PartChildElement->GetName() == "element-name")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				assert(PartChildElement->GetChilds().size() == 1);
				
				auto ElementNameText{dynamic_cast< const XML::Text * >(PartChildElement->GetChild(0))};
				
				assert(ElementNameText != nullptr);
				Part.Array->ElementName = ElementNameText->GetText();
			}
			else if(PartChildElement->GetName() == "element-type")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				_LoadTypeReference(Part.Array->ElementType, PartChildElement);
			}
			else
			{
				std::cout << PartChildElement->GetName() << std::endl;
				assert(false);
			}
		}
	}
}

void Inspection::TypeDefinition::Type::_LoadStatement(Inspection::TypeDefinition::Statement & Statement, const XML::Element * StatementElement)
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

void Inspection::TypeDefinition::Type::_LoadStatementFromWithin(Inspection::TypeDefinition::Statement & Statement, const XML::Element * ParentElement)
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

void Inspection::TypeDefinition::Type::_LoadTag(Inspection::TypeDefinition::Tag & Tag, const XML::Element * TagElement)
{
	assert(TagElement->HasAttribute("name") == true);
	Tag.Name = TagElement->GetAttribute("name");
	_LoadStatementFromWithin(Tag.Statement, TagElement);
}

void Inspection::TypeDefinition::Type::_LoadType(Inspection::TypeDefinition::Type & Type, const XML::Element * TypeElement)
{
	for(auto TypeChildNode : TypeElement->GetChilds())
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
					Type._HardcodedGetter = Inspection::Get_APE_Flags;
				}
				else if(HardcodedText->GetText() == "Get_APE_Item")
				{
					Type._HardcodedGetter = Inspection::Get_APE_Item;
				}
				else if(HardcodedText->GetText() == "Get_Apple_AppleDouble_File")
				{
					Type._HardcodedGetter = Inspection::Get_Apple_AppleDouble_File;
				}
				else if(HardcodedText->GetText() == "Get_Array_EndedByFailureOrLength_ResetPositionOnFailure")
				{
					Type._HardcodedGetter = Inspection::Get_Array_EndedByFailureOrLength_ResetPositionOnFailure;
				}
				else if(HardcodedText->GetText() == "Get_Array_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Array_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_Array_EndedByNumberOfElements")
				{
					Type._HardcodedGetter = Inspection::Get_Array_EndedByNumberOfElements;
				}
				else if(HardcodedText->GetText() == "Get_ASCII_String_AlphaNumeric_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_ASCII_String_AlphaNumericOrSpace_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_ASCII_String_Printable_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_ASCII_String_Printable_EndedByTermination")
				{
					Type._HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByTermination;
				}
				else if(HardcodedText->GetText() == "Get_ASF_DataObject")
				{
					Type._HardcodedGetter = Get_ASF_DataObject;
				}
				else if(HardcodedText->GetText() == "Get_ASF_ExtendedContentDescription_ContentDescriptor_Data")
				{
					Type._HardcodedGetter = Get_ASF_ExtendedContentDescription_ContentDescriptor_Data;
				}
				else if(HardcodedText->GetText() == "Get_ASF_HeaderObject")
				{
					Type._HardcodedGetter = Get_ASF_HeaderObject;
				}
				else if(HardcodedText->GetText() == "Get_ASF_Metadata_DescriptionRecord_Data")
				{
					Type._HardcodedGetter = Get_ASF_Metadata_DescriptionRecord_Data;
				}
				else if(HardcodedText->GetText() == "Get_ASF_MetadataLibrary_DescriptionRecord_Data")
				{
					Type._HardcodedGetter = Get_ASF_MetadataLibrary_DescriptionRecord_Data;
				}
				else if(HardcodedText->GetText() == "Get_ASF_Object")
				{
					Type._HardcodedGetter = Get_ASF_Object;
				}
				else if(HardcodedText->GetText() == "Get_ASF_StreamBitrateProperties_BitrateRecord_Flags")
				{
					Type._HardcodedGetter = Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags;
				}
				else if(HardcodedText->GetText() == "Get_ASF_CreationDate")
				{
					Type._HardcodedGetter = Inspection::Get_ASF_CreationDate;
				}
				else if(HardcodedText->GetText() == "Get_ASF_GUID")
				{
					Type._HardcodedGetter = Inspection::Get_ASF_GUID;
				}
				else if(HardcodedText->GetText() == "Get_ASF_FileProperties_Flags")
				{
					Type._HardcodedGetter = Inspection::Get_ASF_FileProperties_Flags;
				}
				else if(HardcodedText->GetText() == "Get_BitSet_16Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_BitSet_16Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_Boolean_1Bit")
				{
					Type._HardcodedGetter = Inspection::Get_Boolean_1Bit;
				}
				else if(HardcodedText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_Data_Set_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Data_Set_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_Data_SetOrUnset_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Data_SetOrUnset_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_Data_Unset_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Data_Unset_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Frame")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Frame;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_MetaDataBlock")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_MetaDataBlock;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Stream_Header")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Stream_Header;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_StreamInfoBlock_BitsPerSample")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_BitsPerSample;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_StreamInfoBlock_NumberOfChannels")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_NumberOfChannels;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Subframe_CalculateBitsPerSample")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Subframe_CalculateBitsPerSample;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual_Rice_Partition")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual_Rice_Partition;
				}
				else if(HardcodedText->GetText() == "Get_GUID_LittleEndian")
				{
					Type._HardcodedGetter = Inspection::Get_GUID_LittleEndian;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_2_Frame")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_2_Frame;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_2_Language")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_2_Language;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_3_Frame")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_3_Frame;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_3_Language")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_3_Language;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_3_Frame_Header_Flags")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_3_Frame_Header_Flags;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_4_Frame")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_4_Frame;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_4_Language")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_4_Language;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ID3_ReplayGainAdjustment")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_ReplayGainAdjustment;
				}
				else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit;
				}
				else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Track")
				{
					Type._HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Track;
				}
				else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Tracks")
				{
					Type._HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Tracks;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTermination")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_IEEE_60559_2011_binary32")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32;
				}
				else if(HardcodedText->GetText() == "Get_MPEG_1_Frame")
				{
					Type._HardcodedGetter = Inspection::Get_MPEG_1_Frame;
				}
				else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_BitRateIndex")
				{
					Type._HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_BitRateIndex;
				}
				else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_Mode")
				{
					Type._HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_Mode;
				}
				else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_ModeExtension")
				{
					Type._HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_ModeExtension;
				}
				else if(HardcodedText->GetText() == "Get_Ogg_Page")
				{
					Type._HardcodedGetter = Inspection::Get_Ogg_Page;
				}
				else if(HardcodedText->GetText() == "Get_RIFF_Chunk")
				{
					Type._HardcodedGetter = Inspection::Get_RIFF_Chunk;
				}
				else if(HardcodedText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask")
				{
					Type._HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask;
				}
				else if(HardcodedText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat")
				{
					Type._HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat;
				}
				else if(HardcodedText->GetText() == "Get_SignedInteger_8Bit")
				{
					Type._HardcodedGetter = Inspection::Get_SignedInteger_8Bit;
				}
				else if(HardcodedText->GetText() == "Get_SignedInteger_32Bit_RiceEncoded")
				{
					Type._HardcodedGetter = Inspection::Get_SignedInteger_32Bit_RiceEncoded;
				}
				else if(HardcodedText->GetText() == "Get_SignedInteger_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_SignedInteger_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_String_ASCII_Alphabetic_ByTemplate")
				{
					Type._HardcodedGetter = Inspection::Get_String_ASCII_Alphabetic_ByTemplate;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_1Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_1Bit;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_2Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_2Bit;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_3Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_3Bit;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_4Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_4Bit;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_7Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_7Bit;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_8Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_8Bit;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_9Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_9Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_16Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_16Bit_LittleEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_LittleEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_20Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_20Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_24Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_24Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_32Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_32Bit_LittleEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_LittleEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_36Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_36Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_64Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_64Bit_LittleEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_LittleEndian;
				}
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_BigEndian;
				}
				else
				{
					throw std::domain_error{"Invalid reference to hardcoded getter \"" + HardcodedText->GetText() + "\"."};
				}
			}
			else if((TypeChildElement->GetName() == "sequence") || (TypeChildElement->GetName() == "field") || (TypeChildElement->GetName() == "fields") || (TypeChildElement->GetName() == "forward"))
			{
				assert(Type._Part == nullptr);
				Type._Part = new Inspection::TypeDefinition::Part{};
				_LoadPart(*(Type._Part), TypeChildElement);
			}
			else
			{
				throw std::domain_error{"\"type/" + TypeChildElement->GetName() + "\" not allowed."};
			}
		}
	}
}

void Inspection::TypeDefinition::Type::_LoadTypeReference(Inspection::TypeDefinition::TypeReference & TypeReference, const XML::Element * TypeReferenceElement)
{
	for(auto TypeReferenceChildNode : TypeReferenceElement->GetChilds())
	{
		if(TypeReferenceChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto TypeReferenceChildElement{dynamic_cast< const XML::Element * >(TypeReferenceChildNode)};
			
			assert(TypeReferenceChildElement->GetName() == "part");
			assert(TypeReferenceChildElement->GetChilds().size() == 1);
			
			auto TypeReferencePartText{dynamic_cast< const XML::Text * >(TypeReferenceChildElement->GetChild(0))};
			
			assert(TypeReferencePartText != nullptr);
			TypeReference.Parts.push_back(TypeReferencePartText->GetText());
		}
	}
}

void Inspection::TypeDefinition::Type::_LoadValueFromWithin(Inspection::TypeDefinition::Value & Value, const XML::Element * ParentElement)
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

void Inspection::TypeDefinition::Type::_LoadValue(Inspection::TypeDefinition::Value & Value, const XML::Element * ValueElement)
{
	assert(Value.DataType == Inspection::TypeDefinition::DataType::Unknown);
	if(ValueElement == nullptr)
	{
		Value.DataType = Inspection::TypeDefinition::DataType::Nothing;
	}
	else if(ValueElement->GetName() == "boolean")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::Boolean;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.Boolean = from_string_cast< bool >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "data-reference")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::DataReference;
		Value.DataReference.emplace();
		assert(ValueElement->HasAttribute("root") == true);
		if(ValueElement->GetAttribute("root") == "current")
		{
			Value.DataReference->Root = Inspection::TypeDefinition::DataReference::Root::Current;
		}
		else if(ValueElement->GetAttribute("root") == "type")
		{
			Value.DataReference->Root = Inspection::TypeDefinition::DataReference::Root::Type;
		}
		else
		{
			assert(false);
		}
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
	else if(ValueElement->GetName() == "length")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::Length;
		Value.Length.emplace();
		_LoadLength(Value.Length.value(), ValueElement);
	}
	else if(ValueElement->GetName() == "parameter-reference")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::ParameterReference;
		Value.ParameterReference.emplace();
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.ParameterReference->Name = TextNode->GetText();
	}
	else if(ValueElement->GetName() == "parameters")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::Parameters;
		Value.Parameters.emplace();
		_LoadParameters(Value.Parameters.value(), ValueElement);
	}
	else if(ValueElement->GetName() == "single-precision-real")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::SinglePrecisionReal;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.SinglePrecisionReal = from_string_cast< float >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "string")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::String;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.String = TextNode->GetText();
	}
	else if(ValueElement->GetName() == "type-reference")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::TypeReference;
		Value.TypeReference.emplace();
		_LoadTypeReference(Value.TypeReference.value(), ValueElement);
	}
	else if(ValueElement->GetName() == "unsigned-integer-8bit")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger8Bit = from_string_cast< std::uint8_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-16bit")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger16Bit = from_string_cast< std::uint16_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-32bit")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger32Bit = from_string_cast< std::uint32_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-64bit")
	{
		Value.DataType = Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		Value.UnsignedInteger64Bit = from_string_cast< std::uint64_t >(TextNode->GetText());
	}
	else
	{
		throw std::domain_error{ValueElement->GetName() + " not allowed."};
	}
	assert(Value.DataType != Inspection::TypeDefinition::DataType::Unknown);
}
