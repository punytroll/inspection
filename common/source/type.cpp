#include <any>
#include <optional>

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
		std::optional<bool> AbortEvaluation;
		std::optional<bool> DataIsValid;
		std::optional<bool> EngineError;
		std::optional<bool> StructureIsValid;
	};
	
	namespace Algorithms
	{
		std::any Add(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Summand1, const Inspection::TypeDefinition::Expression & Summand2);
		std::any Divide(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Dividend, const Inspection::TypeDefinition::Expression & Divisor);
		std::any GetAnyFromCast(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Cast & Cast);
		std::any GetAnyFromExpression(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Expression);
		const std::any & GetAnyReferenceFromDataReference(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::DataReference & DataReference);
		std::any Subtract(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Minuend, const Inspection::TypeDefinition::Expression & Subtrahend);
		std::unordered_map<std::string, std::any> GetParameters(ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Parameters * Parameters);
		
		template<typename Type>
		Type Cast(const std::any & Any)
		{
			if(Any.type() == typeid(float))
			{
				return static_cast<Type>(std::any_cast<float>(Any));
			}
			else if(Any.type() == typeid(std::uint8_t))
			{
				return static_cast<Type>(std::any_cast<std::uint8_t>(Any));
			}
			else if(Any.type() == typeid(std::uint16_t))
			{
				return static_cast<Type>(std::any_cast<std::uint16_t>(Any));
			}
			else if(Any.type() == typeid(std::uint32_t))
			{
				return static_cast<Type>(std::any_cast<std::uint32_t>(Any));
			}
			else if(Any.type() == typeid(std::uint64_t))
			{
				return static_cast<Type>(std::any_cast<std::uint64_t>(Any));
			}
			else
			{
				std::cout << Any.type().name() << std::endl;
				assert(false);
			}
		}
		
		template<typename Type>
		Type GetDataFromCast(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Cast & Cast)
		{
			return Inspection::Algorithms::Cast<Type>(Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(Cast.Expression)));
		}
		
		template<typename Type>
		Type GetDataFromValue(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Value & Value)
		{
			switch(Value.GetDataType())
			{
			case Inspection::TypeDefinition::DataType::String:
				{
					assert(std::holds_alternative<std::string>(Value.Data) == true);
					
					return from_string_cast<Type>(std::get<std::string>(Value.Data));
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
				{
					assert(std::holds_alternative<std::uint64_t>(Value.Data) == true);
					
					return std::get<std::uint64_t>(Value.Data);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		template<typename Type>
		Type GetDataFromExpression(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Expression)
		{
			switch(Expression.GetExpressionType())
			{
			case Inspection::TypeDefinition::Expression::Type::Cast:
				{
					auto Cast = dynamic_cast<const Inspection::TypeDefinition::Cast *>(&Expression);
					
					assert(Cast != nullptr);
					
					return Inspection::Algorithms::GetDataFromCast<Type>(ExecutionContext, *Cast);
				}
			case Inspection::TypeDefinition::Expression::Type::DataReference:
				{
					auto DataReference = dynamic_cast<const Inspection::TypeDefinition::DataReference *>(&Expression);
					
					assert(DataReference != nullptr);
					
					auto & Any = Inspection::Algorithms::GetAnyReferenceFromDataReference(ExecutionContext, *DataReference);
					
					if(Any.type() == typeid(std::uint8_t))
					{
						return std::any_cast<std::uint8_t>(Any);
					}
					else if(Any.type() == typeid(std::uint16_t))
					{
						return std::any_cast<std::uint16_t>(Any);
					}
					else if(Any.type() == typeid(std::uint32_t))
					{
						return std::any_cast<std::uint32_t>(Any);
					}
					else
					{
						assert(false);
					}
					
					break;
				}
			case Inspection::TypeDefinition::Expression::Type::Value:
				{
					auto Value = dynamic_cast<const Inspection::TypeDefinition::Value *>(&Expression);
					
					assert(Value != nullptr);
					
					return Inspection::Algorithms::GetDataFromValue<Type>(ExecutionContext, *Value);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		const std::any & GetAnyReferenceFromDataReference(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::DataReference & DataReference)
		{
			auto Value = ExecutionContext.GetValueFromDataReference(DataReference);
			
			assert(Value != nullptr);
			
			return Value->GetData();
		}
		
		const std::any & GetAnyReferenceFromParameterReference(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::ParameterReference & ParameterReference)
		{
			return ExecutionContext.GetAnyReferenceFromParameterReference(ParameterReference);
		}
		
		std::any GetAnyFromValue(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Value & Value)
		{
			switch(Value.GetDataType())
			{
			case Inspection::TypeDefinition::DataType::Boolean:
				{
					assert(std::holds_alternative<bool>(Value.Data) == true);
					
					return std::get<bool>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::GUID:
				{
					assert(std::holds_alternative<Inspection::GUID>(Value.Data) == true);
					
					return std::get<Inspection::GUID>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::Length:
				{
					assert(std::holds_alternative<std::unique_ptr<Inspection::TypeDefinition::Length>>(Value.Data) == true);
					
					return Inspection::Length{Inspection::Algorithms::GetDataFromExpression<std::uint64_t>(ExecutionContext, *(std::get<std::unique_ptr<Inspection::TypeDefinition::Length>>(Value.Data)->Bytes)), Inspection::Algorithms::GetDataFromExpression<std::uint64_t>(ExecutionContext, *(std::get<std::unique_ptr<Inspection::TypeDefinition::Length>>(Value.Data)->Bits))};
				}
			case Inspection::TypeDefinition::DataType::Nothing:
				{
					return nullptr;
				}
			case Inspection::TypeDefinition::DataType::Parameters:
				{
					assert(std::holds_alternative<std::unique_ptr<Inspection::TypeDefinition::Parameters>>(Value.Data) == true);
					
					return Inspection::Algorithms::GetParameters(ExecutionContext, std::get<std::unique_ptr<Inspection::TypeDefinition::Parameters>>(Value.Data).get());
				}
			case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
				{
					assert(std::holds_alternative<float>(Value.Data) == true);
					
					return std::get<float>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::String:
				{
					assert(std::holds_alternative<std::string>(Value.Data) == true);
					
					return std::get<std::string>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
				{
					assert(std::holds_alternative<std::uint8_t>(Value.Data) == true);
					
					return std::get<std::uint8_t>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
				{
					assert(std::holds_alternative<std::uint16_t>(Value.Data) == true);
					
					return std::get<std::uint16_t>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
				{
					assert(std::holds_alternative<std::uint32_t>(Value.Data) == true);
					
					return std::get<std::uint32_t>(Value.Data);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
				{
					assert(std::holds_alternative<std::uint64_t>(Value.Data) == true);
					
					return std::get<std::uint64_t>(Value.Data);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		void ApplyTags(Inspection::ExecutionContext & ExecutionContext, const std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> & Tags, Inspection::Value * Target)
		{
			for(auto & Tag : Tags)
			{
				if(Tag->Expression)
				{
					assert(Tag->Expression->GetExpressionType() == Inspection::TypeDefinition::Expression::Type::Value);
					
					auto Value = dynamic_cast<const Inspection::TypeDefinition::Value *>(Tag->Expression.get());
					
					assert(Value != nullptr);
					Target->AddTag(Tag->Name, Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *Value));
				}
				else
				{
					Target->AddTag(Tag->Name);
				}
			}
		}
		
		template<typename DataType>
		bool ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, Inspection::Value * Target)
		{
			bool Result = false;
			auto BaseValueString = to_string_cast(std::any_cast<const DataType &>(Target->GetData()));
			auto ElementIterator = std::find_if(Enumeration.Elements.begin(), Enumeration.Elements.end(), [BaseValueString](auto & Element){ return Element.BaseValue == BaseValueString; });
			
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
		
		std::any GetAnyFromExpression(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Expression)
		{
			switch(Expression.GetExpressionType())
			{
			case Inspection::TypeDefinition::Expression::Type::Add:
				{
					auto Add = dynamic_cast<const Inspection::TypeDefinition::Add *>(&Expression);
					
					assert(Add != nullptr);
					
					return Inspection::Algorithms::Add(ExecutionContext, *(Add->Summand1), *(Add->Summand2));
				}
			case Inspection::TypeDefinition::Expression::Type::Cast:
				{
					auto Cast = dynamic_cast<const Inspection::TypeDefinition::Cast *>(&Expression);
					
					assert(Cast != nullptr);
					
					return Inspection::Algorithms::GetAnyFromCast(ExecutionContext, *Cast);
				}
			case Inspection::TypeDefinition::Expression::Type::DataReference:
				{
					auto DataReference = dynamic_cast<const Inspection::TypeDefinition::DataReference *>(&Expression);
					
					assert(DataReference != nullptr);
					
					return Inspection::Algorithms::GetAnyReferenceFromDataReference(ExecutionContext, *DataReference);
				}
			case Inspection::TypeDefinition::Expression::Type::Divide:
				{
					auto Divide = dynamic_cast<const Inspection::TypeDefinition::Divide *>(&Expression);
					
					assert(Divide != nullptr);
					
					return Inspection::Algorithms::Divide(ExecutionContext, *(Divide->Dividend), *(Divide->Divisor));
				}
			case Inspection::TypeDefinition::Expression::Type::LengthReference:
				{
					auto LengthReference = dynamic_cast<const Inspection::TypeDefinition::LengthReference *>(&Expression);
					
					assert(LengthReference != nullptr);
					
					return ExecutionContext.CalculateLengthFromReference(*LengthReference);
				}
			case Inspection::TypeDefinition::Expression::Type::ParameterReference:
				{
					auto ParameterReference = dynamic_cast<const Inspection::TypeDefinition::ParameterReference *>(&Expression);
					
					assert(ParameterReference != nullptr);
					
					return Inspection::Algorithms::GetAnyReferenceFromParameterReference(ExecutionContext, *ParameterReference);
				}
			case Inspection::TypeDefinition::Expression::Type::Subtract:
				{
					auto Subtract = dynamic_cast<const Inspection::TypeDefinition::Subtract *>(&Expression);
					
					assert(Subtract != nullptr);
					
					return Inspection::Algorithms::Subtract(ExecutionContext, *(Subtract->Minuend), *(Subtract->Subtrahend));
				}
			case Inspection::TypeDefinition::Expression::Type::TypeReference:
				{
					auto TypeReference = dynamic_cast<const Inspection::TypeDefinition::TypeReference *>(&Expression);
					
					assert(TypeReference != nullptr);
					
					return Inspection::g_TypeRepository.GetType(TypeReference->Parts);
				}
			case Inspection::TypeDefinition::Expression::Type::Value:
				{
					auto Value = dynamic_cast<const Inspection::TypeDefinition::Value *>(&Expression);
					
					assert(Value != nullptr);
					
					return Inspection::Algorithms::GetAnyFromValue(ExecutionContext, *Value);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		std::any GetAnyFromCast(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Cast & Cast)
		{
			switch(Cast.DataType)
			{
			case Inspection::TypeDefinition::DataType::Length:
				{
					auto Any = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(Cast.Expression));
					
					assert(Any.type() == typeid(Inspection::Length));
					
					return Any;
				}
			case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
				{
					return Inspection::Algorithms::GetDataFromCast<float>(ExecutionContext, Cast);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
				{
					return Inspection::Algorithms::GetDataFromCast<std::uint8_t>(ExecutionContext, Cast);
				}
			case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
				{
					return Inspection::Algorithms::GetDataFromCast<std::uint64_t>(ExecutionContext, Cast);
				}
			default:
				{
					assert(false);
				}
			}
		}
		
		std::any Add(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Summand1, const Inspection::TypeDefinition::Expression & Summand2)
		{
			auto AnySummand1 = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Summand1);
			auto AnySummand2 = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Summand2);
			
			if((AnySummand1.type() == typeid(std::uint8_t)) && (AnySummand2.type() == typeid(std::uint8_t)))
			{
				return static_cast<std::uint8_t>(std::any_cast<std::uint8_t>(AnySummand1) + std::any_cast<std::uint8_t>(AnySummand2));
			}
			else
			{
				assert(false);
			}
			
			return nullptr;
		}
		
		template<typename Type>
		bool Equals(const std::any & One, const std::any & Two)
		{
			return std::any_cast<const Type &>(One) == std::any_cast<const Type &>(Two);
		}
		
		bool Equals(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Expression1, const Inspection::TypeDefinition::Expression & Expression2)
		{
			auto Any1 = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Expression1);
			auto Any2 = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Expression2);
			
			if((Any1.has_value() == true) && (Any2.has_value() == true))
			{
				if(Any1.type() == Any2.type())
				{
					if(Any1.type() == typeid(std::uint8_t))
					{
						return Inspection::Algorithms::Equals<std::uint8_t>(Any1, Any2);
					}
					else if(Any1.type() == typeid(std::uint16_t))
					{
						return Inspection::Algorithms::Equals<std::uint16_t>(Any1, Any2);
					}
					else if(Any1.type() == typeid(std::uint32_t))
					{
						return Inspection::Algorithms::Equals<std::uint32_t>(Any1, Any2);
					}
					else if(Any1.type() == typeid(Inspection::GUID))
					{
						return Inspection::Algorithms::Equals<Inspection::GUID>(Any1, Any2);
					}
					else if(Any1.type() == typeid(std::string))
					{
						return Inspection::Algorithms::Equals<std::string>(Any1, Any2);
					}
					else
					{
						assert(false);
					}
				}
			}
			
			return false;
		}
		
		std::any Divide(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Dividend, const Inspection::TypeDefinition::Expression & Divisor)
		{
			auto AnyDivident = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Dividend);
			auto AnyDivisor = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Divisor);
			
			if((AnyDivident.type() == typeid(float)) && (AnyDivisor.type() == typeid(float)))
			{
				return std::any_cast<float>(AnyDivident) / std::any_cast<float>(AnyDivisor);
			}
			else
			{
				assert(false);
			}
			
			return nullptr;
		}
		
		std::any Subtract(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Expression & Minuend, const Inspection::TypeDefinition::Expression & Subtrahend)
		{
			auto AnyMinuend = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Minuend);
			auto AnySubtrahend = Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, Subtrahend);
			
			if((AnyMinuend.type() == typeid(Inspection::Length)) && (AnySubtrahend.type() == typeid(Inspection::Length)))
			{
				return std::any_cast<const Inspection::Length &>(AnyMinuend) - std::any_cast<const Inspection::Length &>(AnySubtrahend);
			}
			else
			{
				assert(false);
			}
			
			return nullptr;
		}
		
		std::unordered_map<std::string, std::any> GetParameters(ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Parameters * Parameters)
		{
			auto Result = std::unordered_map<std::string, std::any>{};
			
			if(Parameters != nullptr)
			{
				for(auto & Parameter : Parameters->GetParameters())
				{
					Result.emplace(Parameter->Name, Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(Parameter->Expression)));
				}
			}
			
			return Result;
		}
	}
}

Inspection::TypeDefinition::Type::Type(const std::vector<std::string> & PathParts, Inspection::TypeRepository * TypeRepository) :
	_Part{nullptr},
	_PathParts{PathParts},
	_TypeRepository{TypeRepository}
{
}

Inspection::TypeDefinition::Type::~Type(void)
{
	delete _Part;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::Get(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	// reading
	if(Continue == true)
	{
		if(_HardcodedGetter != nullptr)
		{
			auto HardcodedResult = _HardcodedGetter(Reader, Parameters);
			
			Continue = HardcodedResult->GetSuccess();
			Result->SetValue(HardcodedResult->ExtractValue());
		}
		else if(_Part != nullptr)
		{
			auto ExecutionContext = Inspection::ExecutionContext{*this};
			auto TypePart = Inspection::TypeDefinition::Part{};
			
			TypePart.Type = Inspection::TypeDefinition::Part::Type::Type;
			ExecutionContext.Push(TypePart, *Result, Reader, Parameters);
			
			auto PartReader = std::unique_ptr<Inspection::Reader>{};
			
			if(_Part->Length != nullptr)
			{
				PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(_Part->Length))));
			}
			else
			{
				PartReader = std::make_unique<Inspection::Reader>(Reader);
			}
			if(PartReader != nullptr)
			{
				auto PartParameters = Inspection::Algorithms::GetParameters(ExecutionContext, _Part->Parameters.get());
				
				switch(_Part->Type)
				{
				case Inspection::TypeDefinition::Part::Type::Alternative:
					{
						auto SequenceResult = _GetAlternative(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = SequenceResult->GetSuccess();
						Result->SetValue(SequenceResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Array:
					{
						auto ArrayResult = _GetArray(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = ArrayResult->GetSuccess();
						Result->GetValue()->AppendField(_Part->FieldName.value(), ArrayResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Sequence:
					{
						auto SequenceResult = _GetSequence(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = SequenceResult->GetSuccess();
						Result->SetValue(SequenceResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Field:
					{
						auto FieldResult = _GetField(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = FieldResult->GetSuccess();
						Result->GetValue()->AppendField(_Part->FieldName.value(), FieldResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Fields:
					{
						auto FieldsResult = _GetFields(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = FieldsResult->GetSuccess();
						Result->GetValue()->AppendFields(FieldsResult->GetValue()->ExtractFields());
						
						break;
					}
				case Inspection::TypeDefinition::Part::Type::Forward:
					{
						auto ForwardResult = _GetForward(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = ForwardResult->GetSuccess();
						Result->SetValue(ForwardResult->ExtractValue());
						
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
			assert(ExecutionContext.GetExecutionStackSize() == 0);
		}
		else
		{
			assert(false);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

const std::vector<std::string> Inspection::TypeDefinition::Type::GetPathParts(void) const
{
	return _PathParts;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetAlternative(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Alternative, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	assert(Alternative.Type == Inspection::TypeDefinition::Part::Type::Alternative);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto FoundAlternative = false;
	
	ExecutionContext.Push(Alternative, *Result, Reader, Parameters);
	assert(Alternative.Parts.has_value() == true);
	for(auto AlternativePartIterator = std::begin(Alternative.Parts.value()); ((FoundAlternative == false) && (AlternativePartIterator != std::end(Alternative.Parts.value()))); ++AlternativePartIterator)
	{
		auto & AlternativePart = *AlternativePartIterator;
		auto AlternativePartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(AlternativePart.Length != nullptr)
		{
			AlternativePartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(AlternativePart.Length))));
		}
		else
		{
			AlternativePartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		if(AlternativePartReader != nullptr)
		{
			auto AlternativePartParameters = Inspection::Algorithms::GetParameters(ExecutionContext, AlternativePart.Parameters.get());
			
			switch(AlternativePart.Type)
			{
			case Inspection::TypeDefinition::Part::Type::Array:
				{
					auto ArrayResult = _GetArray(ExecutionContext, AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = ArrayResult->GetSuccess();
					if(FoundAlternative == true)
					{
						assert(AlternativePart.FieldName);
						Result->GetValue()->AppendField(AlternativePart.FieldName.value(), ArrayResult->ExtractValue());
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Field:
				{
					auto FieldResult = _GetField(ExecutionContext, AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = FieldResult->GetSuccess();
					if(FoundAlternative == true)
					{
						assert(AlternativePart.FieldName);
						Result->GetValue()->AppendField(AlternativePart.FieldName.value(), FieldResult->ExtractValue());
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Fields:
				{
					auto FieldsResult = _GetFields(ExecutionContext, AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = FieldsResult->GetSuccess();
					if(FoundAlternative == true)
					{
						Result->GetValue()->AppendFields(FieldsResult->GetValue()->ExtractFields());
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Forward:
				{
					auto ForwardResult = _GetForward(ExecutionContext, AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = ForwardResult->GetSuccess();
					if(FoundAlternative == true)
					{
						Result->SetValue(ForwardResult->ExtractValue());
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Sequence:
				{
					auto SequenceResult = _GetSequence(ExecutionContext, AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = SequenceResult->GetSuccess();
					if(FoundAlternative == true)
					{
						Result->SetValue(SequenceResult->ExtractValue());
					}
					
					break;
				}
			default:
				{
					assert(false);
				}
			}
			if(FoundAlternative == true)
			{
				Reader.AdvancePosition(AlternativePartReader->GetConsumedLength());
			}
		}
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(FoundAlternative);
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetArray(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Array, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	assert(Array.Type == Inspection::TypeDefinition::Part::Type::Array);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Array, *Result, Reader, Parameters);
	Result->GetValue()->AddTag("array");
	assert(Array.Array.has_value() == true);
	
	switch(Array.Array->IterateType)
	{
	case Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength:
		{
			auto ElementParameters = Inspection::Algorithms::GetParameters(ExecutionContext, Array.Array->ElementParameters.get());
			
			assert(Array.Array->ElementType != nullptr);
			
			auto ElementType = Inspection::g_TypeRepository.GetType(Array.Array->ElementType->Parts);
			auto ElementIndexInArray = static_cast<std::uint64_t>(0);
			
			while((Continue == true) && (Reader.HasRemaining() == true))
			{
				auto ElementReader = Inspection::Reader{Reader};
				
				ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
				
				auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
				
				Continue = ElementResult->GetSuccess();
				if(Continue == true)
				{
					ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
					if(Array.Array->ElementName)
					{
						Result->GetValue()->AppendField(Array.Array->ElementName.value(), ElementResult->ExtractValue());
					}
					else
					{
						Result->GetValue()->AppendField(ElementResult->ExtractValue());
					}
					Reader.AdvancePosition(ElementReader.GetConsumedLength());
				}
				else
				{
					break;
				}
			}
			if(Reader.IsAtEnd() == true)
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by failure"s);
			}
			if(ElementIndexInArray > 0)
			{
				Result->GetValue()->AddTag("at least one element"s);
				Continue = true;
			}
			else
			{
				Result->GetValue()->AddTag("error", "At least one element was expected."s);
				Continue = false;
			}
			Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
			
			break;
		}
	case Inspection::TypeDefinition::Array::IterateType::ForEachField:
		{
			auto ElementParameters = Inspection::Algorithms::GetParameters(ExecutionContext, Array.Array->ElementParameters.get());
			
			assert(Array.Array->IterateForEachField != nullptr);
			
			auto IterateField = ExecutionContext.GetFieldFromFieldReference(*(Array.Array->IterateForEachField));
			
			assert(IterateField != nullptr);
			
			auto ElementProperties = std::vector<std::pair<Inspection::Length, Inspection::Length>>{};
			
			for(auto & Field : IterateField->GetFields())
			{
				ElementProperties.emplace_back(std::any_cast<const Inspection::Length &>(Field->GetTag("position")->GetData()), std::any_cast<const Inspection::Length &>(Field->GetTag("length")->GetData()));
			}
			std::sort(std::begin(ElementProperties), std::end(ElementProperties));
			assert(Array.Array->ElementType != nullptr);
			
			auto ElementType = Inspection::g_TypeRepository.GetType(Array.Array->ElementType->Parts);
			auto NumberOfAppendedElements = static_cast<std::uint64_t>(0);
			
			for(auto ElementPropertiesIndex = static_cast<std::uint64_t>(0); (Continue == true) && (ElementPropertiesIndex < ElementProperties.size()); ++ElementPropertiesIndex)
			{
				auto & Properties = ElementProperties[ElementPropertiesIndex];
				
				assert(Reader.GetReadPositionInInput() == Properties.first);
				
				auto ElementReader = Inspection::Reader{Reader, Properties.second};
				auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
				
				Continue = ElementResult->GetSuccess();
				Result->GetValue()->AppendField(Array.Array->ElementName.value(), ElementResult->ExtractValue());
				Reader.AdvancePosition(ElementReader.GetConsumedLength());
				++NumberOfAppendedElements;
			}
			Result->GetValue()->AddTag("number of elements", NumberOfAppendedElements);
			
			break;
		}
	case Inspection::TypeDefinition::Array::IterateType::NumberOfElements:
		{
			auto ElementParameters = Inspection::Algorithms::GetParameters(ExecutionContext, Array.Array->ElementParameters.get());
			
			assert(Array.Array->ElementType != nullptr);
			
			auto ElementType = Inspection::g_TypeRepository.GetType(Array.Array->ElementType->Parts);
			auto NumberOfRequiredElements = Inspection::Algorithms::GetDataFromExpression<std::uint64_t>(ExecutionContext, *(Array.Array->IterateNumberOfElements));
			auto ElementIndexInArray = static_cast<std::uint64_t>(0);
			
			while(true)
			{
				if(ElementIndexInArray < NumberOfRequiredElements)
				{
					auto ElementReader = Inspection::Reader{Reader};
					
					ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
					
					auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
					
					Continue = ElementResult->GetSuccess();
					ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
					if(Array.Array->ElementName)
					{
						Result->GetValue()->AppendField(Array.Array->ElementName.value(), ElementResult->ExtractValue());
					}
					else
					{
						Result->GetValue()->AppendField(ElementResult->ExtractValue());
					}
					Reader.AdvancePosition(ElementReader.GetConsumedLength());
					if(Continue == false)
					{
						Result->GetValue()->AddTag("ended by failure"s);
						
						break;
					}
				}
				else
				{
					Result->GetValue()->AddTag("ended by number of elements"s);
					
					break;
				}
			}
			Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
			
			break;
		}
	case Inspection::TypeDefinition::Array::IterateType::UntilFailureOrLength:
		{
			auto ElementParameters = Inspection::Algorithms::GetParameters(ExecutionContext, Array.Array->ElementParameters.get());
			
			assert(Array.Array->ElementType != nullptr);
			
			auto ElementType = Inspection::g_TypeRepository.GetType(Array.Array->ElementType->Parts);
			auto ElementIndexInArray = static_cast<std::uint64_t>(0);
			
			while((Continue == true) && (Reader.HasRemaining() == true))
			{
				auto ElementReader = Inspection::Reader{Reader};
				
				ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
				
				auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
				
				Continue = ElementResult->GetSuccess();
				if(Continue == true)
				{
					ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
					if(Array.Array->ElementName)
					{
						Result->GetValue()->AppendField(Array.Array->ElementName.value(), ElementResult->ExtractValue());
					}
					else
					{
						Result->GetValue()->AppendField(ElementResult->ExtractValue());
					}
					Reader.AdvancePosition(ElementReader.GetConsumedLength());
				}
				else
				{
					Continue = true;
					
					break;
				}
			}
			if(Reader.IsAtEnd() == true)
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by failure"s);
			}
			Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
			
			break;
		}
	default:
		{
			assert(false);
		}
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetField(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Field, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	assert(Field.Type == Inspection::TypeDefinition::Part::Type::Field);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Field, *Result, Reader, Parameters);
	if(Field.TypeReference)
	{
		auto FieldResult = Inspection::g_TypeRepository.GetType(Field.TypeReference->Parts)->Get(Reader, ExecutionContext.GetAllParameters());
		
		Continue = FieldResult->GetSuccess();
		Result->SetValue(FieldResult->ExtractValue());
	}
	else if(Field.Parts)
	{
		assert(Field.Parts->size() == 1);
		
		auto & FieldPart = Field.Parts->front();
		auto FieldPartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(FieldPart.Length != nullptr)
		{
			FieldPartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(FieldPart.Length))));
		}
		else
		{
			FieldPartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		if(FieldPartReader != nullptr)
		{
			auto FieldPartParameters = Inspection::Algorithms::GetParameters(ExecutionContext, FieldPart.Parameters.get());
			
			switch(FieldPart.Type)
			{
			case Inspection::TypeDefinition::Part::Type::Alternative:
				{
					auto AlternativeResult = _GetAlternative(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = AlternativeResult->GetSuccess();
					Result->SetValue(AlternativeResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Field:
				{
					auto FieldResult = _GetField(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = FieldResult->GetSuccess();
					assert(FieldPart.FieldName);
					Result->GetValue()->AppendField(FieldPart.FieldName.value(), FieldResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Fields:
				{
					auto FieldsResult = _GetFields(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = FieldsResult->GetSuccess();
					Result->SetValue(FieldsResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Forward:
				{
					auto ForwardResult = _GetForward(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = ForwardResult->GetSuccess();
					Result->SetValue(ForwardResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Sequence:
				{
					auto SequenceResult = _GetSequence(ExecutionContext, FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = SequenceResult->GetSuccess();
					Result->SetValue(SequenceResult->ExtractValue());
					
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
		for(auto & Tag : Field.Tags)
		{
			if(Tag->Expression)
			{
				Result->GetValue()->AddTag(Tag->Name, Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(Tag->Expression)));
			}
			else
			{
				Result->GetValue()->AddTag(Tag->Name);
			}
		}
	}
	// interpretation
	if(Continue == true)
	{
		if(Field.Interpretation)
		{
			auto EvaluationResult = _ApplyInterpretation(ExecutionContext, *(Field.Interpretation), Result->GetValue());
			
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
		for(auto & Expression : Field.Verifications)
		{
			switch(Expression->GetExpressionType())
			{
			case Inspection::TypeDefinition::Expression::Type::Equals:
				{
					auto Equals = dynamic_cast<Inspection::TypeDefinition::Equals *>(Expression.get());
					
					assert(Equals != nullptr);
					Continue = Inspection::Algorithms::Equals(ExecutionContext, *(Equals->Expression1), *(Equals->Expression2));
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
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetFields(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Fields, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	assert(Fields.Type == Inspection::TypeDefinition::Part::Type::Fields);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Fields, *Result, Reader, Parameters);
	assert(Fields.TypeReference != nullptr);
	
	auto FieldsResult = Inspection::g_TypeRepository.GetType(Fields.TypeReference->Parts)->Get(Reader, ExecutionContext.GetAllParameters());
	
	Continue = FieldsResult->GetSuccess();
	Result->SetValue(FieldsResult->ExtractValue());
	// interpretation
	if(Continue == true)
	{
		if(Fields.Interpretation)
		{
			auto EvaluationResult = _ApplyInterpretation(ExecutionContext, *(Fields.Interpretation), Result->GetValue());
			
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
		for(auto & Expression : Fields.Verifications)
		{
			switch(Expression->GetExpressionType())
			{
			case Inspection::TypeDefinition::Expression::Type::Equals:
				{
					auto Equals = dynamic_cast<Inspection::TypeDefinition::Equals *>(Expression.get());
					
					assert(Equals != nullptr);
					Continue = Inspection::Algorithms::Equals(ExecutionContext, *(Equals->Expression1), *(Equals->Expression2));
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
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetForward(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Forward, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	assert(Forward.Type == Inspection::TypeDefinition::Part::Type::Forward);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Forward, *Result, Reader, Parameters);
	assert(Forward.TypeReference != nullptr);
	
	auto ForwardResult = Inspection::g_TypeRepository.GetType(Forward.TypeReference->Parts)->Get(Reader, ExecutionContext.GetAllParameters());
	
	Continue = ForwardResult->GetSuccess();
	Result->SetValue(ForwardResult->ExtractValue());
	// tags
	if(Continue == true)
	{
		for(auto & Tag : Forward.Tags)
		{
			if(Tag->Expression)
			{
				Result->GetValue()->AddTag(Tag->Name, Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(Tag->Expression)));
			}
			else
			{
				Result->GetValue()->AddTag(Tag->Name);
			}
		}
	}
	// interpretation
	if(Continue == true)
	{
		if(Forward.Interpretation)
		{
			auto EvaluationResult = _ApplyInterpretation(ExecutionContext, *(Forward.Interpretation), Result->GetValue());
			
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
		for(auto & Expression : Forward.Verifications)
		{
			switch(Expression->GetExpressionType())
			{
			case Inspection::TypeDefinition::Expression::Type::Equals:
				{
					auto Equals = dynamic_cast<Inspection::TypeDefinition::Equals *>(Expression.get());
					
					assert(Equals != nullptr);
					Continue = Inspection::Algorithms::Equals(ExecutionContext, *(Equals->Expression1), *(Equals->Expression2));
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
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetSequence(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Sequence, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	assert(Sequence.Type == Inspection::TypeDefinition::Part::Type::Sequence);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Sequence, *Result, Reader, Parameters);
	assert(Sequence.Parts.has_value() == true);
	for(auto SequencePartIterator = std::begin(Sequence.Parts.value()); ((Continue == true) && (SequencePartIterator != std::end(Sequence.Parts.value()))); ++SequencePartIterator)
	{
		auto & SequencePart = *SequencePartIterator;
		auto SequencePartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(SequencePart.Length != nullptr)
		{
			SequencePartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(SequencePart.Length))));
		}
		else
		{
			SequencePartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		if(SequencePartReader != nullptr)
		{
			auto SequencePartParameters = Inspection::Algorithms::GetParameters(ExecutionContext, SequencePart.Parameters.get());
			
			switch(SequencePart.Type)
			{
			case Inspection::TypeDefinition::Part::Type::Alternative:
				{
					auto AlternativeResult = _GetAlternative(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = AlternativeResult->GetSuccess();
					Result->GetValue()->AppendFields(AlternativeResult->GetValue()->ExtractFields());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Array:
				{
					auto ArrayResult = _GetArray(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = ArrayResult->GetSuccess();
					assert(SequencePart.FieldName);
					Result->GetValue()->AppendField(SequencePart.FieldName.value(), ArrayResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Field:
				{
					auto FieldResult = _GetField(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = FieldResult->GetSuccess();
					assert(SequencePart.FieldName);
					Result->GetValue()->AppendField(SequencePart.FieldName.value(), FieldResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Fields:
				{
					auto FieldsResult = _GetFields(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = FieldsResult->GetSuccess();
					Result->GetValue()->AppendFields(FieldsResult->GetValue()->ExtractFields());
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Forward:
				{
					auto ForwardResult = _GetForward(ExecutionContext, SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = ForwardResult->GetSuccess();
					Result->SetValue(ForwardResult->ExtractValue());
					
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
			if(Tag->Expression)
			{
				Result->GetValue()->AddTag(Tag->Name, Inspection::Algorithms::GetAnyFromExpression(ExecutionContext, *(Tag->Expression)));
			}
			else
			{
				Result->GetValue()->AddTag(Tag->Name);
			}
		}
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

Inspection::EvaluationResult Inspection::TypeDefinition::Type::_ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, Inspection::Value * Target) const
{
	auto Result = Inspection::EvaluationResult{};
	
	if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::String)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration<std::string>(ExecutionContext, Enumeration, Target);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger8Bit)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration<std::uint8_t>(ExecutionContext, Enumeration, Target);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger16Bit)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration<std::uint16_t>(ExecutionContext, Enumeration, Target);
	}
	else if(Enumeration.BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger32Bit)
	{
		Result.DataIsValid = Inspection::Algorithms::ApplyEnumeration<std::uint32_t>(ExecutionContext, Enumeration, Target);
	}
	else
	{
		Target->AddTag("error", "Could not handle the enumeration base type.");
		Result.AbortEvaluation = true;
	}
	
	return Result;
}
	
Inspection::EvaluationResult Inspection::TypeDefinition::Type::_ApplyInterpretation(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Interpretation & Interpretation, Inspection::Value * Target) const
{
	auto Result = Inspection::EvaluationResult{};
	
	switch(Interpretation.Type)
	{
	case Inspection::TypeDefinition::Interpretation::Type::ApplyEnumeration:
		{
			assert(Interpretation.ApplyEnumeration.has_value() == true);
			
			auto EvaluationResult = _ApplyEnumeration(ExecutionContext, *(Interpretation.ApplyEnumeration->Enumeration), Target);
			
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

void Inspection::TypeDefinition::Type::Load(std::istream & InputStream)
{
	auto Document = XML::Document{InputStream};
	auto DocumentElement = Document.GetDocumentElement();
	
	assert(DocumentElement != nullptr);
	assert(DocumentElement->GetName() == "type");
	_LoadType(*this, DocumentElement);
}

void Inspection::TypeDefinition::Type::_LoadPart(Inspection::TypeDefinition::Part & Part, const XML::Element * PartElement)
{
	if(PartElement->GetName() == "alternative")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Alternative;
	}
	else if(PartElement->GetName() == "array")
	{
		Part.Type = Inspection::TypeDefinition::Part::Type::Array;
		assert(PartElement->HasAttribute("name") == true);
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
		assert(PartElement->HasAttribute("name") == true);
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
			auto PartChildElement = dynamic_cast<const XML::Element *>(PartChildNode);
			
			assert(PartChildElement != nullptr);
			if(PartChildElement->GetName() == "type-reference")
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Fields) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward));
				Part.TypeReference = Inspection::TypeDefinition::TypeReference::Load(PartChildElement);
			}
			else if(PartChildElement->GetName() == "interpretation")
			{
				Part.Interpretation = Inspection::TypeDefinition::Interpretation::Load(PartChildElement);
			}
			else if(PartChildElement->GetName() == "length")
			{
				Part.Length = Inspection::TypeDefinition::Expression::Load(PartChildElement);
			}
			else if(PartChildElement->GetName() == "parameters")
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Fields) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward));
				Part.Parameters = Inspection::TypeDefinition::Parameters::Load(PartChildElement);
			}
			else if(PartChildElement->GetName() == "verification")
			{
				for(auto GetterPartVerificationChildNode : PartChildElement->GetChilds())
				{
					if(GetterPartVerificationChildNode->GetNodeType() == XML::NodeType::Element)
					{
						Part.Verifications.push_back(Inspection::TypeDefinition::Expression::Load(dynamic_cast<const XML::Element *>(GetterPartVerificationChildNode)));
					}
				}
			}
			else if(PartChildElement->GetName() == "tag")
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Field) || (Part.Type == Inspection::TypeDefinition::Part::Type::Forward) || (Part.Type == Inspection::TypeDefinition::Part::Type::Sequence));
				Part.Tags.push_back(Inspection::TypeDefinition::Tag::Load(PartChildElement));
			}
			else if((PartChildElement->GetName() == "alternative") || (PartChildElement->GetName() == "sequence") || (PartChildElement->GetName() == "field") || (PartChildElement->GetName() == "fields") || (PartChildElement->GetName() == "forward") || (PartChildElement->GetName() == "array"))
			{
				assert((Part.Type == Inspection::TypeDefinition::Part::Type::Sequence) || ((Part.Type == Inspection::TypeDefinition::Part::Type::Field) && (!Part.Parts)) || (Part.Type == Inspection::TypeDefinition::Part::Type::Alternative));
				if(!Part.Parts)
				{
					Part.Parts.emplace();
				}
				
				auto & ContainedPart = Part.Parts->emplace_back();
				
				_LoadPart(ContainedPart, PartChildElement);
			}
			else if(PartChildElement->GetName() == "iterate")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				assert(Part.Array.has_value() == true);
				assert(PartChildElement->HasAttribute("type") == true);
				if(PartChildElement->GetAttribute("type") == "at-least-one-until-failure-or-length")
				{
					Part.Array->IterateType = Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength;
					assert(XML::HasChildNodes(PartChildElement) == false);
				}
				else if(PartChildElement->GetAttribute("type") == "for-each-field")
				{
					Part.Array->IterateType = Inspection::TypeDefinition::Array::IterateType::ForEachField;
					
					auto FieldReferenceElement = static_cast<const XML::Element *>(nullptr);
					
					for(auto PartIterateChildNode : PartChildElement->GetChilds())
					{
						if(PartIterateChildNode->GetNodeType() == XML::NodeType::Element)
						{
							assert(FieldReferenceElement == nullptr);
							FieldReferenceElement = dynamic_cast<const XML::Element *>(PartIterateChildNode);
						}
					}
					assert(FieldReferenceElement != nullptr);
					Part.Array->IterateForEachField = Inspection::TypeDefinition::FieldReference::Load(FieldReferenceElement);
				}
				else if(PartChildElement->GetAttribute("type") == "number-of-elements")
				{
					Part.Array->IterateType = Inspection::TypeDefinition::Array::IterateType::NumberOfElements;
					Part.Array->IterateNumberOfElements = Inspection::TypeDefinition::Expression::LoadFromWithin(PartChildElement);
				}
				else if(PartChildElement->GetAttribute("type") == "until-failure-or-length")
				{
					Part.Array->IterateType = Inspection::TypeDefinition::Array::IterateType::UntilFailureOrLength;
					assert(XML::HasChildNodes(PartChildElement) == false);
				}
				else
				{
					assert(false);
				}
			}
			else if(PartChildElement->GetName() == "element-name")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				assert(Part.Array.has_value() == true);
				assert(PartChildElement->GetChilds().size() == 1);
				
				auto ElementNameText = dynamic_cast<const XML::Text *>(PartChildElement->GetChild(0));
				
				assert(ElementNameText != nullptr);
				Part.Array->ElementName = ElementNameText->GetText();
			}
			else if(PartChildElement->GetName() == "element-type")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				assert(Part.Array.has_value() == true);
				Part.Array->ElementType = Inspection::TypeDefinition::TypeReference::Load(PartChildElement);
			}
			else if(PartChildElement->GetName() == "element-parameters")
			{
				assert(Part.Type == Inspection::TypeDefinition::Part::Type::Array);
				assert(Part.Array.has_value() == true);
				Part.Array->ElementParameters = Inspection::TypeDefinition::Parameters::Load(PartChildElement);
			}
			else
			{
				std::cout << PartChildElement->GetName() << std::endl;
				assert(false);
			}
		}
	}
}

void Inspection::TypeDefinition::Type::_LoadType(Inspection::TypeDefinition::Type & Type, const XML::Element * TypeElement)
{
	for(auto TypeChildNode : TypeElement->GetChilds())
	{
		if(TypeChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto TypeChildElement = dynamic_cast<const XML::Element *>(TypeChildNode);
			
			assert(TypeChildElement != nullptr);
			if(TypeChildElement->GetName() == "hardcoded")
			{
				assert(TypeChildElement->GetChilds().size() == 1);
				
				auto HardcodedText = dynamic_cast<const XML::Text *>(TypeChildElement->GetChild(0));
				
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
				else if(HardcodedText->GetText() == "Get_ASF_ExtendedContentDescription_ContentDescriptor_Data")
				{
					Type._HardcodedGetter = Get_ASF_ExtendedContentDescription_ContentDescriptor_Data;
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
				else if(HardcodedText->GetText() == "Get_ASF_StreamPropertiesObjectData")
				{
					Type._HardcodedGetter = Inspection::Get_ASF_StreamPropertiesObjectData;
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
				else if(HardcodedText->GetText() == "Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte")
				{
					Type._HardcodedGetter = Inspection::Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte;
				}
				else if(HardcodedText->GetText() == "Get_Boolean_1Bit")
				{
					Type._HardcodedGetter = Inspection::Get_Boolean_1Bit;
				}
				else if(HardcodedText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength;
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
				else if(HardcodedText->GetText() == "Get_Data_Unset_Until8BitAlignment")
				{
					Type._HardcodedGetter = Inspection::Get_Data_Unset_Until8BitAlignment;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Frame_Header")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Frame_Header;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_MetaDataBlock")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_MetaDataBlock;
				}
				else if(HardcodedText->GetText() == "Get_FLAC_Stream_Header")
				{
					Type._HardcodedGetter = Inspection::Get_FLAC_Stream_Header;
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
				else if(HardcodedText->GetText() == "Get_ID3_1_Genre")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_1_Genre;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_Tag")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_Tag;
				}
				else if(HardcodedText->GetText() == "Get_ID3_2_2_Frame")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_2_2_Frame;
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
				else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_35Bit_SynchSafe_40Bit_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_35Bit_SynchSafe_40Bit_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Track")
				{
					Type._HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Track;
				}
				else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Tracks")
				{
					Type._HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Tracks;
				}
				else if(HardcodedText->GetText() == "Get_ISO_639_2_1998_Code")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_639_2_1998_Code;
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
				else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength;
				}
				else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByNumberOfCodePoints")
				{
					Type._HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByNumberOfCodePoints;
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
				else if(HardcodedText->GetText() == "Get_Ogg_Stream")
				{
					Type._HardcodedGetter = Inspection::Get_Ogg_Stream;
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
				else if(HardcodedText->GetText() == "Get_SignedInteger_32Bit_LittleEndian")
				{
					Type._HardcodedGetter = Inspection::Get_SignedInteger_32Bit_LittleEndian;
				}
				else if(HardcodedText->GetText() == "Get_SignedInteger_32Bit_RiceEncoded")
				{
					Type._HardcodedGetter = Inspection::Get_SignedInteger_32Bit_RiceEncoded;
				}
				else if(HardcodedText->GetText() == "Get_SignedInteger_BigEndian")
				{
					Type._HardcodedGetter = Inspection::Get_SignedInteger_BigEndian;
				}
				else if(HardcodedText->GetText() == "Get_String_ASCII_ByTemplate")
				{
					Type._HardcodedGetter = Inspection::Get_String_ASCII_ByTemplate;
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
				else if(HardcodedText->GetText() == "Get_UnsignedInteger_5Bit")
				{
					Type._HardcodedGetter = Inspection::Get_UnsignedInteger_5Bit;
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
			else if((TypeChildElement->GetName() == "alternative") || (TypeChildElement->GetName() == "array") || (TypeChildElement->GetName() == "sequence") || (TypeChildElement->GetName() == "field") || (TypeChildElement->GetName() == "fields") || (TypeChildElement->GetName() == "forward"))
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
