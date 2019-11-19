#include <experimental/optional>
#include <fstream>

#include "getters.h"
#include "not_implemented_exception.h"
#include "result.h"
#include "type.h"
#include "type_repository.h"
#include "xml_puny_dom.h"

using namespace std::string_literals;

namespace Inspection
{
	enum class DataType
	{
		Unknown,
		Boolean,
		DataReference,
		TypeReference,
		Nothing,
		ParameterReference,
		Parameters,
		SinglePrecisionReal,
		String,
		UnsignedInteger8Bit,
		UnsignedInteger16Bit,
		UnsignedInteger32Bit,
		UnsignedInteger64Bit
	};
	
	Inspection::DataType GetDataTypeFromString(const std::string & String)
	{
		if(String == "boolean")
		{
			return Inspection::DataType::Boolean;
		}
		else if(String == "data-reference")
		{
			return Inspection::DataType::DataReference;
		}
		else if(String == "nothing")
		{
			return Inspection::DataType::Nothing;
		}
		else if(String == "parameter-reference")
		{
			return Inspection::DataType::ParameterReference;
		}
		else if(String == "parameters")
		{
			return Inspection::DataType::Parameters;
		}
		else if(String == "single-precision-real")
		{
			return Inspection::DataType::SinglePrecisionReal;
		}
		else if(String == "string")
		{
			return Inspection::DataType::String;
		}
		else if(String == "type-reference")
		{
			return Inspection::DataType::TypeReference;
		}
		else if(String == "unsigned integer 8bit")
		{
			return Inspection::DataType::UnsignedInteger8Bit;
		}
		else if(String == "unsigned integer 16bit")
		{
			return Inspection::DataType::UnsignedInteger16Bit;
		}
		else if(String == "unsigned integer 32bit")
		{
			return Inspection::DataType::UnsignedInteger32Bit;
		}
		else if(String == "unsigned integer 64bit")
		{
			return Inspection::DataType::UnsignedInteger64Bit;
		}
		else
		{
			throw std::domain_error("Unknown type \"" + String + "\".");
		}
	}
	
	class EvaluationResult
	{
	public:
		std::experimental::optional< bool > AbortEvaluation;
		std::experimental::optional< bool > DataIsValid;
		std::experimental::optional< bool > EngineError;
		std::experimental::optional< bool > StructureIsValid;
	};
	
	class DataReference
	{
	public:
		class PartDescriptor
		{
		public:
			enum class Type
			{
				Field,
				Tag
			};
			
			Inspection::DataReference::PartDescriptor::Type Type;
			std::string DetailName;
		};
		
		std::experimental::optional< Inspection::DataType > CastToType;
		std::vector< Inspection::DataReference::PartDescriptor > PartDescriptors;
	};
	
	class TypeReference
	{
	public:
		std::vector< std::string > Parts;
	};
	
	class ParameterReference
	{
	public:
		std::experimental::optional< Inspection::DataType > CastToType;
		std::string Name;
	};
	
	class ValueDescriptor
	{
	public:
		Inspection::DataType Type;
		std::experimental::optional< bool > Boolean;
		std::experimental::optional< Inspection::DataReference > DataReference;
		std::experimental::optional< Inspection::TypeReference > TypeReference;
		std::experimental::optional< Inspection::ParameterReference > ParameterReference;
		std::experimental::optional< std::vector< Inspection::TypeDefinition::Parameter > > Parameters;
		std::experimental::optional< float > SinglePrecisionReal;
		std::experimental::optional< std::string > String;
		std::experimental::optional< std::uint8_t > UnsignedInteger8Bit;
		std::experimental::optional< std::uint16_t > UnsignedInteger16Bit;
		std::experimental::optional< std::uint32_t > UnsignedInteger32Bit;
		std::experimental::optional< std::uint64_t > UnsignedInteger64Bit;
	};
	
	class Enumeration
	{
	public:
		class Element
		{
		public:
			std::string BaseValue;
			std::vector< Inspection::TypeDefinition::Tag > Tags;
			bool Valid;
		};
		std::string BaseType;
		std::vector< Inspection::Enumeration::Element > Elements;
		std::experimental::optional< Inspection::Enumeration::Element > FallbackElement;
	};
	
	class ApplyEnumeration
	{
	public:
		Inspection::Enumeration Enumeration;
	};

	class Interpretation
	{
	public:
		enum class Type
		{
			ApplyEnumeration
		};
		
		std::experimental::optional< Inspection::ApplyEnumeration > ApplyEnumeration;
		Inspection::Interpretation::Type Type;
	};
	
	namespace TypeDefinition
	{
		class Statement
		{
		public:
			enum class Type
			{
				Equals,
				Value
			};
			
			Inspection::TypeDefinition::Statement::Type Type;
			Inspection::TypeDefinition::Equals * Equals;
			Inspection::ValueDescriptor * Value;
		};
		
		class Equals
		{
		public:
			Inspection::TypeDefinition::Statement Statement1;
			Inspection::TypeDefinition::Statement Statement2;
		};
		
		class Length
		{
		public:
			Inspection::TypeDefinition::Statement Bytes;
			Inspection::TypeDefinition::Statement Bits;
		};
		
		class Parameter
		{
		public:
			std::string Name;
			Inspection::ValueDescriptor ValueDescriptor;
		};
		
		class Tag
		{
		public:
			std::string Name;
			Inspection::TypeDefinition::Statement Statement;
		};
	}
	
	class PartDescriptor
	{
	public:
		enum class Type
		{
			Field,
			Fields,
			Forward
		};
		
		std::vector< Inspection::TypeDefinition::Parameter > Parameters;
		std::experimental::optional< std::string > FieldName;
		std::experimental::optional< Inspection::TypeReference > TypeReference;
		std::experimental::optional< Inspection::Interpretation > Interpretation;
		std::experimental::optional< Inspection::TypeDefinition::Length > Length;
		std::vector< Inspection::TypeDefinition::Tag > Tags;
		Inspection::PartDescriptor::Type Type;
		std::vector< Inspection::TypeDefinition::Statement > Verifications;
	};
	
	const std::experimental::any & GetAnyReferenceByDataReference(const Inspection::DataReference & DataReference, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		std::shared_ptr< Inspection::Value > Value{CurrentValue};
		
		for(auto & PartDescriptor : DataReference.PartDescriptors)
		{
			switch(PartDescriptor.Type)
			{
			case Inspection::DataReference::PartDescriptor::Type::Field:
				{
					Value = Value->GetField(PartDescriptor.DetailName);
					
					break;
				}
			case Inspection::DataReference::PartDescriptor::Type::Tag:
				{
					Value = Value->GetTag(PartDescriptor.DetailName);
					
					break;
				}
			}
		}
		assert(Value != nullptr);
		
		return Value->GetData();
	}
	
	std::experimental::any GetAnyFromValueDescriptor(const Inspection::ValueDescriptor & ValueDescriptor, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		std::experimental::any Result;
		
		if(ValueDescriptor.Type == Inspection::DataType::Boolean)
		{
			assert(ValueDescriptor.Boolean);
			Result = ValueDescriptor.Boolean.value();
		}
		else if(ValueDescriptor.Type == Inspection::DataType::DataReference)
		{
			assert(ValueDescriptor.DataReference);
			Result = GetAnyReferenceByDataReference(ValueDescriptor.DataReference.value(), CurrentValue, Parameters);
		}
		else if(ValueDescriptor.Type == Inspection::DataType::Nothing)
		{
			Result = nullptr;
		}
		else if(ValueDescriptor.Type == Inspection::DataType::SinglePrecisionReal)
		{
			assert(ValueDescriptor.SinglePrecisionReal);
			Result = ValueDescriptor.SinglePrecisionReal.value();
		}
		else if(ValueDescriptor.Type == Inspection::DataType::String)
		{
			assert(ValueDescriptor.String);
			Result = ValueDescriptor.String.value();
		}
		else if(ValueDescriptor.Type == Inspection::DataType::UnsignedInteger8Bit)
		{
			assert(ValueDescriptor.UnsignedInteger8Bit);
			Result = ValueDescriptor.UnsignedInteger8Bit.value();
		}
		else if(ValueDescriptor.Type == Inspection::DataType::UnsignedInteger16Bit)
		{
			assert(ValueDescriptor.UnsignedInteger16Bit);
			Result = ValueDescriptor.UnsignedInteger16Bit.value();
		}
		else if(ValueDescriptor.Type == Inspection::DataType::UnsignedInteger32Bit)
		{
			assert(ValueDescriptor.UnsignedInteger32Bit);
			Result = ValueDescriptor.UnsignedInteger32Bit.value();
		}
		else if(ValueDescriptor.Type == Inspection::DataType::UnsignedInteger64Bit)
		{
			assert(ValueDescriptor.UnsignedInteger64Bit);
			Result = ValueDescriptor.UnsignedInteger64Bit.value();
		}
		else
		{
			assert(false);
		}
		
		return Result;
	}
	
	template< typename Type >
	Type GetDataFromValueDescriptor(const Inspection::ValueDescriptor & ValueDescriptor, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		Type Result{};
		
		if(ValueDescriptor.Type == Inspection::DataType::DataReference)
		{
			assert(ValueDescriptor.DataReference);
			
			auto & Any{GetAnyReferenceByDataReference(ValueDescriptor.DataReference.value(), CurrentValue, Parameters)};
			
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
		else if(ValueDescriptor.Type == Inspection::DataType::String)
		{
			assert(ValueDescriptor.String);
			Result = from_string_cast< Type >(ValueDescriptor.String.value());
		}
		else if(ValueDescriptor.Type == Inspection::DataType::UnsignedInteger64Bit)
		{
			assert(ValueDescriptor.UnsignedInteger64Bit);
			Result = ValueDescriptor.UnsignedInteger64Bit.value();
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
	bool ApplyEnumeration(const Inspection::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		bool Result{false};
		auto BaseValueString{to_string_cast(std::experimental::any_cast< const DataType & >(Target->GetData()))};
		auto ElementIterator{std::find_if(Enumeration.Elements.begin(), Enumeration.Elements.end(), [BaseValueString](auto Element){ return Element.BaseValue == BaseValueString; })};
		
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
	
	void FillNewParameters(std::unordered_map< std::string, std::experimental::any > & NewParameters, const std::vector< Inspection::TypeDefinition::Parameter > & ParameterDefinitions, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
	{
		for(auto & ParameterDefinition : ParameterDefinitions)
		{
			if(ParameterDefinition.ValueDescriptor.Type == Inspection::DataType::String)
			{
				NewParameters.emplace(ParameterDefinition.Name, GetDataFromValueDescriptor< std::string >(ParameterDefinition.ValueDescriptor, CurrentValue, Parameters));
			}
			else if(ParameterDefinition.ValueDescriptor.Type == Inspection::DataType::DataReference)
			{
				assert(ParameterDefinition.ValueDescriptor.DataReference);
				
				auto & ValueAny{GetAnyReferenceByDataReference(ParameterDefinition.ValueDescriptor.DataReference.value(), CurrentValue, Parameters)};
				
				if(!ParameterDefinition.ValueDescriptor.DataReference->CastToType)
				{
					NewParameters.emplace(ParameterDefinition.Name, ValueAny);
				}
				else if(ParameterDefinition.ValueDescriptor.DataReference->CastToType.value() == Inspection::DataType::UnsignedInteger64Bit)
				{
					if(ValueAny.type() == typeid(std::uint16_t))
					{
						NewParameters.emplace(ParameterDefinition.Name, static_cast< std::uint64_t >(std::experimental::any_cast< std::uint16_t >(ValueAny)));
					}
					else if(ValueAny.type() == typeid(std::uint32_t))
					{
						NewParameters.emplace(ParameterDefinition.Name, static_cast< std::uint64_t >(std::experimental::any_cast< std::uint32_t >(ValueAny)));
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
			else if(ParameterDefinition.ValueDescriptor.Type == Inspection::DataType::TypeReference)
			{
				assert(ParameterDefinition.ValueDescriptor.TypeReference);
				NewParameters.emplace(ParameterDefinition.Name, ParameterDefinition.ValueDescriptor.TypeReference->Parts);
			}
			else if(ParameterDefinition.ValueDescriptor.Type == Inspection::DataType::Parameters)
			{
				assert(ParameterDefinition.ValueDescriptor.Parameters);
				
				std::unordered_map< std::string, std::experimental::any > InnerParameters;
				
				FillNewParameters(InnerParameters, ParameterDefinition.ValueDescriptor.Parameters.value(), CurrentValue, Parameters);
				NewParameters.emplace(ParameterDefinition.Name, InnerParameters);
			}
			else if(ParameterDefinition.ValueDescriptor.Type == Inspection::DataType::ParameterReference)
			{
				assert(ParameterDefinition.ValueDescriptor.ParameterReference);
				
				auto & Any{Parameters.at(ParameterDefinition.ValueDescriptor.ParameterReference->Name)};
				
				if(!ParameterDefinition.ValueDescriptor.ParameterReference->CastToType)
				{
					NewParameters.emplace(ParameterDefinition.Name, Any);
				}
				else
				{
					if(ParameterDefinition.ValueDescriptor.ParameterReference->CastToType.value() == Inspection::DataType::UnsignedInteger64Bit)
					{
						if(Any.type() == typeid(std::uint8_t))
						{
							NewParameters.emplace(ParameterDefinition.Name, static_cast< std::uint64_t >(std::experimental::any_cast< std::uint8_t >(Any)));
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
			else
			{
				assert(false);
			}
		}
	}
	
	namespace Algorithms
	{
		template< typename Type >
		Type GetDataFromStatement(const Inspection::TypeDefinition::Statement & Statement, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			if(Statement.Type == Inspection::TypeDefinition::Statement::Type::Value)
			{
				assert(Statement.Value != nullptr);
				return GetDataFromValueDescriptor< Type >(*(Statement.Value), CurrentValue, Parameters);
			}
			else
			{
				assert(false);
			}
		}
		
		bool Equals(const Inspection::ValueDescriptor & Value1, const Inspection::ValueDescriptor & Value2, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			auto Result{false};
			auto Any1{GetAnyFromValueDescriptor(Value1, CurrentValue, Parameters)};
			auto Any2{GetAnyFromValueDescriptor(Value2, CurrentValue, Parameters)};
			
			if((Any1.empty() == false) && (Any2.empty() == false))
			{
				if(Any1.type() == Any2.type())
				{
					if(Any1.type() == typeid(std::uint8_t))
					{
						Result = std::experimental::any_cast< std::uint8_t >(Any1) == std::experimental::any_cast< std::uint8_t >(Any2);
					}
					else if(Any1.type() == typeid(std::uint32_t))
					{
						Result = std::experimental::any_cast< std::uint32_t >(Any1) == std::experimental::any_cast< std::uint32_t >(Any2);
					}
					else
					{
						assert(false);
					}
				}
			}
			
			return Result;
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
	}
}

Inspection::Type::Type(Inspection::TypeRepository * TypeRepository) :
	_TypeRepository{TypeRepository}
{
}

Inspection::Type::~Type(void)
{
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
		else
		{
			for(auto & PartDescriptor : _PartDescriptors)
			{
				Inspection::Reader * PartReader{nullptr};
				
				if(PartDescriptor.Length)
				{
					auto Bytes{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(PartDescriptor.Length->Bytes, Result->GetValue(), Parameters)};
					auto Bits{Inspection::Algorithms::GetDataFromStatement< std::uint64_t >(PartDescriptor.Length->Bits, Result->GetValue(), Parameters)};
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
					std::unordered_map< std::string, std::experimental::any > NewParameters;
					
					FillNewParameters(NewParameters, PartDescriptor.Parameters, Result->GetValue(), Parameters);
					
					auto PartResult{g_TypeRepository.Get(PartDescriptor.TypeReference->Parts, *PartReader, NewParameters)};
					
					Continue = PartResult->GetSuccess();
					switch(PartDescriptor.Type)
					{
					case Inspection::PartDescriptor::Type::Field:
						{
							assert(PartDescriptor.FieldName);
							Result->GetValue()->AppendField(PartDescriptor.FieldName.value(), PartResult->GetValue());
							
							break;
						}
					case Inspection::PartDescriptor::Type::Fields:
						{
							Result->GetValue()->AppendFields(PartResult->GetValue()->GetFields());
							
							break;
						}
					case Inspection::PartDescriptor::Type::Forward:
						{
							Result->SetValue(PartResult->GetValue());
							
							break;
						}
					}
					Reader.AdvancePosition(PartReader->GetConsumedLength());
					// tags
					if(Continue == true)
					{
						if(PartDescriptor.Tags.empty() == false)
						{
							assert(PartDescriptor.Type == Inspection::PartDescriptor::Type::Field);
							for(auto & Tag : PartDescriptor.Tags)
							{
								assert((Tag.Statement.Type == Inspection::TypeDefinition::Statement::Type::Value) && (Tag.Statement.Value != nullptr));
								PartResult->GetValue()->AddTag(Tag.Name, GetAnyFromValueDescriptor(*(Tag.Statement.Value), PartResult->GetValue(), Parameters));
							}
						}
					}
					// interpretation
					if(Continue == true)
					{
						if(PartDescriptor.Interpretation)
						{
							auto EvaluationResult{_ApplyInterpretation(PartDescriptor.Interpretation.value(), PartResult->GetValue(), PartResult->GetValue(), Parameters)};
							
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
						for(auto & Statement : PartDescriptor.Verifications)
						{
							switch(Statement.Type)
							{
							case Inspection::TypeDefinition::Statement::Type::Equals:
								{
									assert(Statement.Equals);
									Continue = Inspection::Algorithms::Equals(Statement.Equals->Statement1, Statement.Equals->Statement2, PartResult->GetValue(), Parameters);
									if(Continue == false)
									{
										Result->GetValue()->AddTag("error", "Failed to verify a value."s);
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
					delete PartReader;
				}
				if(Continue == false)
				{
					break;
				}
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

Inspection::EvaluationResult Inspection::Type::_ApplyEnumeration(const Inspection::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	Inspection::EvaluationResult Result;
	
	if(Enumeration.BaseType == "string")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::string >(Enumeration, Target, CurrentValue, Parameters);
	}
	else if(Enumeration.BaseType == "unsigned integer 8bit")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint8_t >(Enumeration, Target, CurrentValue, Parameters);
	}
	else if(Enumeration.BaseType == "unsigned integer 16bit")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint16_t >(Enumeration, Target, CurrentValue, Parameters);
	}
	else if(Enumeration.BaseType == "unsigned integer 32bit")
	{
		Result.DataIsValid = Inspection::ApplyEnumeration< std::uint32_t >(Enumeration, Target, CurrentValue, Parameters);
	}
	else
	{
		Target->AddTag("error", "Could not handle the enumeration base type \"" + Enumeration.BaseType + "\".");
		Result.AbortEvaluation = true;
	}
	
	return Result;
}
	
Inspection::EvaluationResult Inspection::Type::_ApplyInterpretation(const Inspection::Interpretation & Interpretation, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	Inspection::EvaluationResult Result;
	
	switch(Interpretation.Type)
	{
	case Inspection::Interpretation::Type::ApplyEnumeration:
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
				auto GetterChildElement{dynamic_cast< const XML::Element * >(TypeChildNode)};
				
				if(GetterChildElement->GetName() == "hardcoded")
				{
					assert(GetterChildElement->GetChilds().size() == 1);
					
					auto HardcodedGetterText{dynamic_cast< const XML::Text * >(GetterChildElement->GetChild(0))};
					
					assert(HardcodedGetterText != nullptr);
					if(HardcodedGetterText->GetText() == "Get_APE_Flags")
					{
						_HardcodedGetter = Inspection::Get_APE_Flags;
					}
					else if(HardcodedGetterText->GetText() == "Get_APE_Item")
					{
						_HardcodedGetter = Inspection::Get_APE_Item;
					}
					else if(HardcodedGetterText->GetText() == "Get_Apple_AppleDouble_File")
					{
						_HardcodedGetter = Inspection::Get_Apple_AppleDouble_File;
					}
					else if(HardcodedGetterText->GetText() == "Get_Array_EndedByFailureOrLength_ResetPositionOnFailure")
					{
						_HardcodedGetter = Inspection::Get_Array_EndedByFailureOrLength_ResetPositionOnFailure;
					}
					else if(HardcodedGetterText->GetText() == "Get_Array_EndedByLength")
					{
						_HardcodedGetter = Inspection::Get_Array_EndedByLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_Array_EndedByNumberOfElements")
					{
						_HardcodedGetter = Inspection::Get_Array_EndedByNumberOfElements;
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
						_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASCII_String_Printable_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByTermination;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASF_DataObject")
					{
						_HardcodedGetter = Get_ASF_DataObject;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASF_ExtendedContentDescription_ContentDescriptor_Data")
					{
						_HardcodedGetter = Get_ASF_ExtendedContentDescription_ContentDescriptor_Data;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASF_HeaderObject")
					{
						_HardcodedGetter = Get_ASF_HeaderObject;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASF_Metadata_DescriptionRecord_Data")
					{
						_HardcodedGetter = Get_ASF_Metadata_DescriptionRecord_Data;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASF_MetadataLibrary_DescriptionRecord_Data")
					{
						_HardcodedGetter = Get_ASF_MetadataLibrary_DescriptionRecord_Data;
					}
					else if(HardcodedGetterText->GetText() == "Get_ASF_Object")
					{
						_HardcodedGetter = Get_ASF_Object;
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
					else if(HardcodedGetterText->GetText() == "Get_FLAC_Frame")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Frame;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_MetaDataBlock")
					{
						_HardcodedGetter = Inspection::Get_FLAC_MetaDataBlock;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_Stream_Header")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Stream_Header;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_StreamInfoBlock_BitsPerSample")
					{
						_HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_BitsPerSample;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_StreamInfoBlock_NumberOfChannels")
					{
						_HardcodedGetter = Inspection::Get_FLAC_StreamInfoBlock_NumberOfChannels;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_Subframe_CalculateBitsPerSample")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Subframe_CalculateBitsPerSample;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_Subframe_Residual")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual;
					}
					else if(HardcodedGetterText->GetText() == "Get_FLAC_Subframe_Residual_Rice_Partition")
					{
						_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual_Rice_Partition;
					}
					else if(HardcodedGetterText->GetText() == "Get_GUID_LittleEndian")
					{
						_HardcodedGetter = Inspection::Get_GUID_LittleEndian;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_Frame")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_Frame;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_Language")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_Language;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_Frame")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_Frame;
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
						_HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_Frame")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_Frame;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_Language")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_Language;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination;
					}
					else if(HardcodedGetterText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength;
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
					else if(HardcodedGetterText->GetText() == "Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian;
					}
					else if(HardcodedGetterText->GetText() == "Get_IEC_60908_1999_TableOfContents_Track")
					{
						_HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Track;
					}
					else if(HardcodedGetterText->GetText() == "Get_IEC_60908_1999_TableOfContents_Tracks")
					{
						_HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Tracks;
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
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints;
					}
					else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength;
					}
					else if(HardcodedGetterText->GetText() == "Get_ISO_IEC_IEEE_60559_2011_binary32")
					{
						_HardcodedGetter = Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32;
					}
					else if(HardcodedGetterText->GetText() == "Get_MPEG_1_Frame")
					{
						_HardcodedGetter = Inspection::Get_MPEG_1_Frame;
					}
					else if(HardcodedGetterText->GetText() == "Get_Ogg_Page")
					{
						_HardcodedGetter = Inspection::Get_Ogg_Page;
					}
					else if(HardcodedGetterText->GetText() == "Get_RIFF_Chunk")
					{
						_HardcodedGetter = Inspection::Get_RIFF_Chunk;
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
						_HardcodedGetter = Inspection::Get_SignedInteger_32Bit_RiceEncoded;
					}
					else if(HardcodedGetterText->GetText() == "Get_SignedInteger_BigEndian")
					{
						_HardcodedGetter = Inspection::Get_SignedInteger_BigEndian;
					}
					else if(HardcodedGetterText->GetText() == "Get_String_ASCII_Alphabetic_ByTemplate")
					{
						_HardcodedGetter = Inspection::Get_String_ASCII_Alphabetic_ByTemplate;
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
						_HardcodedGetter = Inspection::Get_UnsignedInteger_BigEndian;
					}
					else
					{
						throw std::domain_error{"Invalid reference to hardcoded getter \"" + HardcodedGetterText->GetText() + "\"."};
					}
				}
				else if((GetterChildElement->GetName() == "field") || (GetterChildElement->GetName() == "fields") || (GetterChildElement->GetName() == "forward"))
				{
					_PartDescriptors.emplace_back();
					
					auto & PartDescriptor{_PartDescriptors.back()};
					
					if(GetterChildElement->GetName() == "field")
					{
						PartDescriptor.Type = Inspection::PartDescriptor::Type::Field;
						PartDescriptor.FieldName = GetterChildElement->GetAttribute("name");
					}
					else if(GetterChildElement->GetName() == "fields")
					{
						PartDescriptor.Type = Inspection::PartDescriptor::Type::Fields;
					}
					else if(GetterChildElement->GetName() == "forward")
					{
						PartDescriptor.Type = Inspection::PartDescriptor::Type::Forward;
					}
					for(auto PartChildNode : GetterChildElement->GetChilds())
					{
						if(PartChildNode->GetNodeType() == XML::NodeType::Element)
						{
							auto PartChildElement{dynamic_cast< const XML::Element * >(PartChildNode)};
							
							if(PartChildElement->GetName() == "type-reference")
							{
								PartDescriptor.TypeReference.emplace();
								_LoadTypeReference(PartDescriptor.TypeReference.value(), PartChildElement);
							}
							else if(PartChildElement->GetName() == "interpretation")
							{
								PartDescriptor.Interpretation.emplace();
								_LoadInterpretation(PartDescriptor.Interpretation.value(), PartChildElement);
							}
							else if(PartChildElement->GetName() == "length")
							{
								PartDescriptor.Length.emplace();
								_LoadLength(PartDescriptor.Length.value(), PartChildElement);
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
											PartDescriptor.Parameters.emplace_back();
											
											auto & Parameter{PartDescriptor.Parameters.back()};
											
											assert(PartParametersChildElement->HasAttribute("name") == true);
											Parameter.Name = PartParametersChildElement->GetAttribute("name");
											_LoadValueDescriptorFromWithin(Parameter.ValueDescriptor, PartParametersChildElement);
										}
										else
										{
											throw std::domain_error{"/getter/field/parameters/" + PartParametersChildElement->GetName() + " not allowed."};
										}
									}
								}
							}
							else if(PartChildElement->GetName() == "verification")
							{
								PartDescriptor.Verifications.emplace_back();
								
								auto & Verification{PartDescriptor.Verifications.back()};
								
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
								assert(PartDescriptor.Type == Inspection::PartDescriptor::Type::Field);
								PartDescriptor.Tags.emplace_back();
								
								auto & Tag{PartDescriptor.Tags.back()};
								
								_LoadTag(Tag, PartChildElement);
							}
							else
							{
								throw std::domain_error{"/getter/field/" + PartChildElement->GetName() + " not allowed."};
							}
						}
					}
				}
				else
				{
					throw std::domain_error{"/getter/" + GetterChildElement->GetName() + " not allowed."};
				}
			}
		}
	}
	catch(std::domain_error & Exception)
	{
		std::throw_with_nested(std::runtime_error("Type path: " + TypePath));
	}
}

void Inspection::Type::_LoadInterpretation(Inspection::Interpretation & Interpretation, const XML::Element * InterpretationElement)
{
	assert(InterpretationElement->GetName() == "interpretation");
	for(auto InterpretationChildNode : InterpretationElement->GetChilds())
	{
		if(InterpretationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto InterpretationChildElement{dynamic_cast< const XML::Element * >(InterpretationChildNode)};
			
			if(InterpretationChildElement->GetName() == "apply-enumeration")
			{
				Interpretation.Type = Inspection::Interpretation::Type::ApplyEnumeration;
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

void Inspection::Type::_LoadEnumeration(Inspection::Enumeration & Enumeration, const XML::Element * EnumerationElement)
{
	assert(EnumerationElement != nullptr);
	assert(EnumerationElement->GetName() == "enumeration");
	Enumeration.BaseType = EnumerationElement->GetAttribute("base-type");
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

void Inspection::Type::_LoadStatement(Inspection::TypeDefinition::Statement & Statement, const XML::Element * StatementElement)
{
	// statement element may be nullptr, if it represents the "nothing" value
	if((StatementElement != nullptr) && (StatementElement->GetName() == "equals"))
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Equals;
		Statement.Equals = new Inspection::TypeDefinition::Equals{};
		_LoadEquals(*(Statement.Equals), StatementElement);
	}
	else
	{
		Statement.Type = Inspection::TypeDefinition::Statement::Type::Value;
		Statement.Value = new Inspection::ValueDescriptor{};
		_LoadValueDescriptor(*(Statement.Value), StatementElement);
	}
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
	Tag.Name = TagElement->GetAttribute("name");
	_LoadStatementFromWithin(Tag.Statement, TagElement);
}

void Inspection::Type::_LoadTypeReference(Inspection::TypeReference & TypeReference, const XML::Element * TypeReferenceElement)
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

void Inspection::Type::_LoadValueDescriptorFromWithin(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ParentElement)
{
	if(ParentElement->GetChilds().size() == 0)
	{
		_LoadValueDescriptor(ValueDescriptor, nullptr);
	}
	else
	{
		for(auto ChildNode : ParentElement->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				_LoadValueDescriptor(ValueDescriptor, dynamic_cast< XML::Element * >(ChildNode));
			}
		}
	}
	if(ValueDescriptor.Type == Inspection::DataType::Unknown)
	{
		throw std::domain_error{"No valid value description."};
	}
}

void Inspection::Type::_LoadValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ValueElement)
{
	if(ValueElement == nullptr)
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::Nothing;
	}
	else if(ValueElement->GetName() == "data-reference")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::DataReference;
		ValueDescriptor.DataReference.emplace();
		if(ValueElement->HasAttribute("cast-to-type") == true)
		{
			ValueDescriptor.DataReference->CastToType = GetDataTypeFromString(ValueElement->GetAttribute("cast-to-type"));
		}
		for(auto DataReferenceChildNode : ValueElement->GetChilds())
		{
			if(DataReferenceChildNode->GetNodeType() == XML::NodeType::Element)
			{
				auto & DataReferencePartDescriptors{ValueDescriptor.DataReference->PartDescriptors};
				auto DataReferencePartDescriptor{DataReferencePartDescriptors.emplace(DataReferencePartDescriptors.end())};
				auto DataReferenceChildElement{dynamic_cast< const XML::Element * >(DataReferenceChildNode)};
				
				if(DataReferenceChildElement->GetName() == "field")
				{
					assert(DataReferenceChildElement->GetChilds().size() == 1);
					DataReferencePartDescriptor->Type = Inspection::DataReference::PartDescriptor::Type::Field;
					
					auto SubText{dynamic_cast< const XML::Text * >(DataReferenceChildElement->GetChild(0))};
					
					assert(SubText != nullptr);
					DataReferencePartDescriptor->DetailName = SubText->GetText();
				}
				else if(DataReferenceChildElement->GetName() == "tag")
				{
					assert(DataReferenceChildElement->GetChilds().size() == 1);
					DataReferencePartDescriptor->Type = Inspection::DataReference::PartDescriptor::Type::Tag;
					
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
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::ParameterReference;
		ValueDescriptor.ParameterReference.emplace();
		if(ValueElement->HasAttribute("cast-to-type") == true)
		{
			ValueDescriptor.ParameterReference->CastToType = GetDataTypeFromString(ValueElement->GetAttribute("cast-to-type"));
		}
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.ParameterReference->Name = TextNode->GetText();
	}
	else if(ValueElement->GetName() == "boolean")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::Boolean;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.Boolean = from_string_cast< bool >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "string")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::String;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.String = TextNode->GetText();
	}
	else if(ValueElement->GetName() == "type-reference")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::TypeReference;
		ValueDescriptor.TypeReference.emplace();
		_LoadTypeReference(ValueDescriptor.TypeReference.value(), ValueElement);
	}
	else if(ValueElement->GetName() == "unsigned-integer-8bit")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::UnsignedInteger8Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.UnsignedInteger8Bit = from_string_cast< std::uint8_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-16bit")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::UnsignedInteger16Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.UnsignedInteger8Bit = from_string_cast< std::uint16_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-32bit")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::UnsignedInteger32Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.UnsignedInteger32Bit = from_string_cast< std::uint32_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "unsigned-integer-64bit")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::UnsignedInteger64Bit;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.UnsignedInteger64Bit = from_string_cast< std::uint64_t >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "single-precision-real")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::SinglePrecisionReal;
		assert((ValueElement->GetChilds().size() == 1) && (ValueElement->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode{dynamic_cast< const XML::Text * >(ValueElement->GetChild(0))};
		
		assert(TextNode != nullptr);
		ValueDescriptor.SinglePrecisionReal = from_string_cast< float >(TextNode->GetText());
	}
	else if(ValueElement->GetName() == "parameters")
	{
		assert(ValueDescriptor.Type == Inspection::DataType::Unknown);
		ValueDescriptor.Type = Inspection::DataType::Parameters;
		ValueDescriptor.Parameters.emplace();
		assert(ValueElement->HasAttribute("cast-to-type") == false);
		for(auto ParametersChildNode : ValueElement->GetChilds())
		{
			if(ParametersChildNode->GetNodeType() == XML::NodeType::Element)
			{
				auto ParametersChildElement{dynamic_cast< const XML::Element * >(ParametersChildNode)};
				
				if(ParametersChildElement->GetName() == "parameter")
				{
					ValueDescriptor.Parameters.value().emplace_back();
					
					auto & Parameter{ValueDescriptor.Parameters.value().back()};
					
					assert(ParametersChildElement->HasAttribute("name") == true);
					Parameter.Name = ParametersChildElement->GetAttribute("name");
					_LoadValueDescriptorFromWithin(Parameter.ValueDescriptor, ParametersChildElement);
				}
				else
				{
					throw std::domain_error{ParametersChildElement->GetName() + " not allowed."};
				}
			}
		}
	}
	else
	{
		throw std::domain_error{ValueElement->GetName() + " not allowed."};
	}
}
