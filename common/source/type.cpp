#include <any>
#include <optional>

#include "assertion.h"
#include "execution_context.h"
#include "getters.h"
#include "result.h"
#include "type.h"
#include "type_definition.h"
#include "type_repository.h"
#include "xml_helper.h"
#include "xml_puny_dom.h"

using namespace std::string_literals;

Inspection::TypeDefinition::Type::Type(const std::vector<std::string> & PathParts, Inspection::TypeRepository & TypeRepository) :
	_PathParts{PathParts},
	_TypeRepository{TypeRepository}
{
}

Inspection::TypeDefinition::Type::~Type(void)
{
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
			auto ExecutionContext = Inspection::ExecutionContext{*this, _TypeRepository};
			auto TypePart = Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Type};
			
			ExecutionContext.Push(TypePart, *Result, Reader, Parameters);
			
			auto PartReader = std::unique_ptr<Inspection::Reader>{};
			
			if(_Part->Length != nullptr)
			{
				PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(_Part->Length->GetAny(ExecutionContext)));
			}
			else
			{
				PartReader = std::make_unique<Inspection::Reader>(Reader);
			}
			if(PartReader != nullptr)
			{
				auto PartParameters = _Part->GetParameters(ExecutionContext);
				
				switch(_Part->GetPartType())
				{
				case Inspection::TypeDefinition::PartType::Alternative:
					{
						auto SequenceResult = _GetAlternative(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = SequenceResult->GetSuccess();
						Result->SetValue(SequenceResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::PartType::Array:
					{
						auto Array = dynamic_cast<const Inspection::TypeDefinition::Array *>(_Part.get());
						
						ASSERTION(Array != nullptr);
						
						auto ArrayResult = _GetArray(ExecutionContext, *Array, *PartReader, PartParameters);
						
						Continue = ArrayResult->GetSuccess();
						Result->GetValue()->AppendField(_Part->FieldName.value(), ArrayResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::PartType::Sequence:
					{
						auto SequenceResult = _GetSequence(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = SequenceResult->GetSuccess();
						Result->SetValue(SequenceResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::PartType::Field:
					{
						auto FieldResult = _GetField(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = FieldResult->GetSuccess();
						Result->GetValue()->AppendField(_Part->FieldName.value(), FieldResult->ExtractValue());
						
						break;
					}
				case Inspection::TypeDefinition::PartType::Fields:
					{
						auto FieldsResult = _GetFields(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = FieldsResult->GetSuccess();
						Result->GetValue()->AppendFields(FieldsResult->GetValue()->ExtractFields());
						
						break;
					}
				case Inspection::TypeDefinition::PartType::Forward:
					{
						auto ForwardResult = _GetForward(ExecutionContext, *_Part, *PartReader, PartParameters);
						
						Continue = ForwardResult->GetSuccess();
						Result->SetValue(ForwardResult->ExtractValue());
						
						break;
					}
				default:
					{
						ASSERTION(false);
						
						break;
					}
				}
				Reader.AdvancePosition(PartReader->GetConsumedLength());
			}
			ExecutionContext.Pop();
			ASSERTION(ExecutionContext.GetExecutionStackSize() == 0);
		}
		else
		{
			ASSERTION(false);
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
	ASSERTION(Alternative.GetPartType() == Inspection::TypeDefinition::PartType::Alternative);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto FoundAlternative = false;
	
	ExecutionContext.Push(Alternative, *Result, Reader, Parameters);
	for(auto AlternativePartIterator = std::begin(Alternative.Parts); ((FoundAlternative == false) && (AlternativePartIterator != std::end(Alternative.Parts))); ++AlternativePartIterator)
	{
		auto & AlternativePart = *AlternativePartIterator;
		auto AlternativePartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(AlternativePart->Length != nullptr)
		{
			AlternativePartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(AlternativePart->Length->GetAny(ExecutionContext)));
		}
		else
		{
			AlternativePartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		if(AlternativePartReader != nullptr)
		{
			auto AlternativePartParameters = AlternativePart->GetParameters(ExecutionContext);
			
			switch(AlternativePart->GetPartType())
			{
			case Inspection::TypeDefinition::PartType::Array:
				{
					auto Array = dynamic_cast<const Inspection::TypeDefinition::Array *>(AlternativePart.get());
					
					ASSERTION(Array != nullptr);
					
					auto ArrayResult = _GetArray(ExecutionContext, *Array, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = ArrayResult->GetSuccess();
					if(FoundAlternative == true)
					{
						ASSERTION(AlternativePart->FieldName.has_value() == true);
						Result->GetValue()->AppendField(AlternativePart->FieldName.value(), ArrayResult->ExtractValue());
					}
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Field:
				{
					auto FieldResult = _GetField(ExecutionContext, *AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = FieldResult->GetSuccess();
					if(FoundAlternative == true)
					{
						ASSERTION(AlternativePart->FieldName.has_value() == true);
						Result->GetValue()->AppendField(AlternativePart->FieldName.value(), FieldResult->ExtractValue());
					}
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Fields:
				{
					auto FieldsResult = _GetFields(ExecutionContext, *AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = FieldsResult->GetSuccess();
					if(FoundAlternative == true)
					{
						Result->GetValue()->AppendFields(FieldsResult->GetValue()->ExtractFields());
					}
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Forward:
				{
					auto ForwardResult = _GetForward(ExecutionContext, *AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = ForwardResult->GetSuccess();
					if(FoundAlternative == true)
					{
						Result->SetValue(ForwardResult->ExtractValue());
					}
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Sequence:
				{
					auto SequenceResult = _GetSequence(ExecutionContext, *AlternativePart, *AlternativePartReader, AlternativePartParameters);
					
					FoundAlternative = SequenceResult->GetSuccess();
					if(FoundAlternative == true)
					{
						Result->SetValue(SequenceResult->ExtractValue());
					}
					
					break;
				}
			default:
				{
					ASSERTION(false);
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

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetArray(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Array & Array, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	ASSERTION(Array.GetPartType() == Inspection::TypeDefinition::PartType::Array);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Array, *Result, Reader, Parameters);
	Result->GetValue()->AddTag("array");
	switch(Array.IterateType)
	{
	case Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength:
		{
			auto ElementParameters = Array.GetElementParameters(ExecutionContext);
			
			ASSERTION(Array.ElementType != nullptr);
			
			auto ElementType = Array.ElementType->GetType(ExecutionContext);
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
					if(Array.ElementName)
					{
						Result->GetValue()->AppendField(Array.ElementName.value(), ElementResult->ExtractValue());
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
			auto ElementParameters = Array.GetElementParameters(ExecutionContext);
			
			ASSERTION(Array.IterateForEachField != nullptr);
			
			auto IterateField = ExecutionContext.GetFieldFromFieldReference(*(Array.IterateForEachField));
			
			ASSERTION(IterateField != nullptr);
			
			auto ElementProperties = std::vector<std::pair<Inspection::Length, Inspection::Length>>{};
			
			for(auto & Field : IterateField->GetFields())
			{
				ElementProperties.emplace_back(std::any_cast<const Inspection::Length &>(Field->GetTag("position")->GetData()), std::any_cast<const Inspection::Length &>(Field->GetTag("length")->GetData()));
			}
			std::sort(std::begin(ElementProperties), std::end(ElementProperties));
			ASSERTION(Array.ElementType != nullptr);
			
			auto ElementType = Array.ElementType->GetType(ExecutionContext);
			auto NumberOfAppendedElements = static_cast<std::uint64_t>(0);
			
			for(auto ElementPropertiesIndex = static_cast<std::uint64_t>(0); (Continue == true) && (ElementPropertiesIndex < ElementProperties.size()); ++ElementPropertiesIndex)
			{
				auto & Properties = ElementProperties[ElementPropertiesIndex];
				
				ASSERTION(Reader.GetReadPositionInInput() == Properties.first);
				
				auto ElementReader = Inspection::Reader{Reader, Properties.second};
				auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
				
				Continue = ElementResult->GetSuccess();
				Result->GetValue()->AppendField(Array.ElementName.value(), ElementResult->ExtractValue());
				Reader.AdvancePosition(ElementReader.GetConsumedLength());
				++NumberOfAppendedElements;
			}
			Result->GetValue()->AddTag("number of elements", NumberOfAppendedElements);
			
			break;
		}
	case Inspection::TypeDefinition::Array::IterateType::NumberOfElements:
		{
			auto ElementParameters = Array.GetElementParameters(ExecutionContext);
			
			ASSERTION(Array.ElementType != nullptr);
			
			auto ElementType = Array.ElementType->GetType(ExecutionContext);
			auto NumberOfElementsAny = Array.IterateNumberOfElements->GetAny(ExecutionContext);
			
			ASSERTION(NumberOfElementsAny.type() == typeid(std::uint64_t));
			
			auto NumberOfElements = std::any_cast<std::uint64_t>(NumberOfElementsAny);
			auto ElementIndexInArray = static_cast<std::uint64_t>(0);
			
			while(true)
			{
				if(ElementIndexInArray < NumberOfElements)
				{
					auto ElementReader = Inspection::Reader{Reader};
					
					ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
					
					auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
					
					Continue = ElementResult->GetSuccess();
					ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
					if(Array.ElementName)
					{
						Result->GetValue()->AppendField(Array.ElementName.value(), ElementResult->ExtractValue());
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
			auto ElementParameters = Array.GetElementParameters(ExecutionContext);
			
			ASSERTION(Array.ElementType != nullptr);
			
			auto ElementType = Array.ElementType->GetType(ExecutionContext);
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
					if(Array.ElementName)
					{
						Result->GetValue()->AppendField(Array.ElementName.value(), ElementResult->ExtractValue());
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
			ASSERTION(false);
		}
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetField(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Field, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	ASSERTION(Field.GetPartType() == Inspection::TypeDefinition::PartType::Field);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Field, *Result, Reader, Parameters);
	if(Field.TypeReference)
	{
		auto FieldType = Field.TypeReference->GetType(ExecutionContext);
		
		ASSERTION(FieldType != nullptr);
		
		auto FieldResult = FieldType->Get(Reader, ExecutionContext.GetAllParameters());
		
		Continue = FieldResult->GetSuccess();
		Result->SetValue(FieldResult->ExtractValue());
	}
	else
	{
		ASSERTION(Field.Parts.size() == 1);
		
		auto & FieldPart = Field.Parts.front();
		auto FieldPartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(FieldPart->Length != nullptr)
		{
			FieldPartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(FieldPart->Length->GetAny(ExecutionContext)));
		}
		else
		{
			FieldPartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		if(FieldPartReader != nullptr)
		{
			auto FieldPartParameters = FieldPart->GetParameters(ExecutionContext);
			
			switch(FieldPart->GetPartType())
			{
			case Inspection::TypeDefinition::PartType::Alternative:
				{
					auto AlternativeResult = _GetAlternative(ExecutionContext, *FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = AlternativeResult->GetSuccess();
					Result->SetValue(AlternativeResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Field:
				{
					auto FieldResult = _GetField(ExecutionContext, *FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = FieldResult->GetSuccess();
					ASSERTION(FieldPart->FieldName.has_value() == true);
					Result->GetValue()->AppendField(FieldPart->FieldName.value(), FieldResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Fields:
				{
					auto FieldsResult = _GetFields(ExecutionContext, *FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = FieldsResult->GetSuccess();
					Result->SetValue(FieldsResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Forward:
				{
					auto ForwardResult = _GetForward(ExecutionContext, *FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = ForwardResult->GetSuccess();
					Result->SetValue(ForwardResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Sequence:
				{
					auto SequenceResult = _GetSequence(ExecutionContext, *FieldPart, *FieldPartReader, FieldPartParameters);
					
					Continue = SequenceResult->GetSuccess();
					Result->SetValue(SequenceResult->ExtractValue());
					
					break;
				}
			default:
				{
					ASSERTION(false);
				}
			}
			Reader.AdvancePosition(FieldPartReader->GetConsumedLength());
		}
	}
	// interpretation
	if(Continue == true)
	{
		Continue = Field.ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetFields(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Fields, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	ASSERTION(Fields.GetPartType() == Inspection::TypeDefinition::PartType::Fields);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Fields, *Result, Reader, Parameters);
	ASSERTION(Fields.TypeReference != nullptr);
	
	auto FieldsType = Fields.TypeReference->GetType(ExecutionContext);
	
	ASSERTION(FieldsType != nullptr);
	
	auto FieldsResult = FieldsType->Get(Reader, ExecutionContext.GetAllParameters());
	
	Continue = FieldsResult->GetSuccess();
	Result->SetValue(FieldsResult->ExtractValue());
	// interpretation
	if(Continue == true)
	{
		Continue = Fields.ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetForward(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Forward, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	ASSERTION(Forward.GetPartType() == Inspection::TypeDefinition::PartType::Forward);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Forward, *Result, Reader, Parameters);
	ASSERTION(Forward.TypeReference != nullptr);
	
	auto ForwardType = Forward.TypeReference->GetType(ExecutionContext);
	
	ASSERTION(ForwardType != nullptr);
	
	auto ForwardResult = ForwardType->Get(Reader, ExecutionContext.GetAllParameters());
	
	Continue = ForwardResult->GetSuccess();
	Result->SetValue(ForwardResult->ExtractValue());
	// interpretation
	if(Continue == true)
	{
		Continue = Forward.ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

std::unique_ptr<Inspection::Result> Inspection::TypeDefinition::Type::_GetSequence(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Sequence, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const
{
	ASSERTION(Sequence.GetPartType() == Inspection::TypeDefinition::PartType::Sequence);
	
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(Sequence, *Result, Reader, Parameters);
	for(auto SequencePartIterator = std::begin(Sequence.Parts); ((Continue == true) && (SequencePartIterator != std::end(Sequence.Parts))); ++SequencePartIterator)
	{
		auto & SequencePart = *SequencePartIterator;
		auto SequencePartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(SequencePart->Length != nullptr)
		{
			SequencePartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(SequencePart->Length->GetAny(ExecutionContext)));
		}
		else
		{
			SequencePartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		if(SequencePartReader != nullptr)
		{
			auto SequencePartParameters = SequencePart->GetParameters(ExecutionContext);
			
			switch(SequencePart->GetPartType())
			{
			case Inspection::TypeDefinition::PartType::Alternative:
				{
					auto AlternativeResult = _GetAlternative(ExecutionContext, *SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = AlternativeResult->GetSuccess();
					Result->GetValue()->AppendFields(AlternativeResult->GetValue()->ExtractFields());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Array:
				{
					auto Array = dynamic_cast<const Inspection::TypeDefinition::Array *>(SequencePart.get());
					
					ASSERTION(Array != nullptr);
					
					auto ArrayResult = _GetArray(ExecutionContext, *Array, *SequencePartReader, SequencePartParameters);
					
					Continue = ArrayResult->GetSuccess();
					ASSERTION(SequencePart->FieldName.has_value() == true);
					Result->GetValue()->AppendField(SequencePart->FieldName.value(), ArrayResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Field:
				{
					auto FieldResult = _GetField(ExecutionContext, *SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = FieldResult->GetSuccess();
					ASSERTION(SequencePart->FieldName.has_value() == true);
					Result->GetValue()->AppendField(SequencePart->FieldName.value(), FieldResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Fields:
				{
					auto FieldsResult = _GetFields(ExecutionContext, *SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = FieldsResult->GetSuccess();
					Result->GetValue()->AppendFields(FieldsResult->GetValue()->ExtractFields());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Forward:
				{
					auto ForwardResult = _GetForward(ExecutionContext, *SequencePart, *SequencePartReader, SequencePartParameters);
					
					Continue = ForwardResult->GetSuccess();
					Result->SetValue(ForwardResult->ExtractValue());
					
					break;
				}
			default:
				{
					ASSERTION(false);
				}
			}
			Reader.AdvancePosition(SequencePartReader->GetConsumedLength());
		}
	}
	// interpretation
	if(Continue == true)
	{
		Continue = Sequence.ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

void Inspection::TypeDefinition::Type::Load(std::istream & InputStream)
{
	auto Document = XML::Document{InputStream};
	auto DocumentElement = Document.GetDocumentElement();
	
	ASSERTION(DocumentElement != nullptr);
	ASSERTION(DocumentElement->GetName() == "type");
	_LoadType(*this, DocumentElement);
}

void Inspection::TypeDefinition::Type::_LoadType(Inspection::TypeDefinition::Type & Type, const XML::Element * TypeElement)
{
	for(auto TypeChildNode : TypeElement->GetChilds())
	{
		if(TypeChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto TypeChildElement = dynamic_cast<const XML::Element *>(TypeChildNode);
			
			ASSERTION(TypeChildElement != nullptr);
			if(TypeChildElement->GetName() == "hardcoded")
			{
				ASSERTION(TypeChildElement->GetChilds().size() == 1);
				
				auto HardcodedText = dynamic_cast<const XML::Text *>(TypeChildElement->GetChild(0));
				
				ASSERTION(HardcodedText != nullptr);
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
					ASSERTION(false);
				}
			}
			else if((TypeChildElement->GetName() == "alternative") || (TypeChildElement->GetName() == "array") || (TypeChildElement->GetName() == "sequence") || (TypeChildElement->GetName() == "field") || (TypeChildElement->GetName() == "fields") || (TypeChildElement->GetName() == "forward"))
			{
				ASSERTION(Type._Part == nullptr);
				Type._Part = Inspection::TypeDefinition::Part::Load(TypeChildElement);
			}
			else
			{
				ASSERTION(false);
			}
		}
	}
}
