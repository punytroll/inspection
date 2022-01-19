/**
 * Copyright (C) 2019  Hagen Möbius
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "assertion.h"
#include "execution_context.h"
#include "internal_output_operators.h"
#include "length.h"
#include "result.h"
#include "string_cast.h"
#include "type.h"
#include "type_definition.h"
#include "type_repository.h"
#include "value.h"
#include "xml_helper.h"
#include "xml_puny_dom.h"

using namespace std::string_literals;

static void ApplyTag(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Tag & Tag, Inspection::Value * Target)
{
	if(Tag.HasExpression() == true)
	{
		Target->AddTag(Tag.GetName(), Tag.GetAny(ExecutionContext));
	}
	else
	{
		Target->AddTag(Tag.GetName());
	}
}

static void ApplyTags(Inspection::ExecutionContext & ExecutionContext, const std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> & Tags, Inspection::Value * Target)
{
	for(auto & Tag : Tags)
	{
		ASSERTION(Tag != nullptr);
		::ApplyTag(ExecutionContext, *Tag, Target);
	}
}

template<typename DataType>
static bool ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, Inspection::Value * Target)
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
		if(Enumeration.FallbackElement.has_value() == true)
		{
			ApplyTags(ExecutionContext, Enumeration.FallbackElement->Tags, Target);
			Target->AddTag("error", "Could find no enumeration element for the base value \"" + BaseValueString + "\".");
			Result = Enumeration.FallbackElement->Valid;
		}
		else
		{
			Target->AddTag("error", "Could find neither an enumeration element nor an enumeration fallback element for the base value \"" + BaseValueString + "\".");
			Result = false;
		}
	}
	
	return Result;
}

template<typename Type>
static Type CastTo(const std::any & Any)
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
		UNEXPECTED_CASE("Any.type() == " + Inspection::to_string(Any.type()));
	}
}

static std::unordered_map<std::string, std::any> GetParameters(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Parameters * Parameters)
{
	if(Parameters != nullptr)
	{
		return Parameters->GetParameters(ExecutionContext);
	}
	else
	{
		return std::unordered_map<std::string, std::any>{};
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Add                                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Add::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Summand1 != nullptr);
	ASSERTION(m_Summand2 != nullptr);
	
	auto Summand1Any = m_Summand1->GetAny(ExecutionContext);
	auto Summand2Any = m_Summand2->GetAny(ExecutionContext);
	
	ASSERTION(Summand1Any.has_value() == true);
	ASSERTION(Summand2Any.has_value() == true);
	ASSERTION(Summand1Any.type() == Summand2Any.type());
	if(Summand1Any.type() == typeid(std::uint8_t))
	{
		return static_cast<std::uint8_t>(std::any_cast<std::uint8_t>(Summand1Any) + std::any_cast<std::uint8_t>(Summand2Any));
	}
	else
	{
		UNEXPECTED_CASE("Summand1Any.type() == " + Inspection::to_string(Summand1Any.type()));
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Add::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on an Add expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Add> Inspection::TypeDefinition::Add::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Add>{new Inspection::TypeDefinition::Add{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			if(First == true)
			{
				Result->m_Summand1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				INVALID_INPUT_IF(Result->m_Summand2 != nullptr, "More than two operands for Add expression.");
				Result->m_Summand2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	INVALID_INPUT_IF(Result->m_Summand1 == nullptr, "Missing operands for Add expression.");
	INVALID_INPUT_IF(Result->m_Summand2 == nullptr, "Missing second operand for Add expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// AddTag                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Inspection::TypeDefinition::AddTag::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const
{
	ASSERTION(m_Tag != nullptr);
	::ApplyTag(ExecutionContext, *m_Tag, Target);
	
	return true;
}

std::unique_ptr<Inspection::TypeDefinition::AddTag> Inspection::TypeDefinition::AddTag::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	ASSERTION(Element->GetName() == "tag");
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::AddTag>{new Inspection::TypeDefinition::AddTag{}};
	
	Result->m_Tag = Inspection::TypeDefinition::Tag::Load(Element);
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Alternative                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Alternative::Alternative(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Alternative}
{
}

auto Inspection::TypeDefinition::Alternative::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	auto Result = std::make_unique<Inspection::Result>();
	auto FoundAlternative = false;
	
	ExecutionContext.Push(*this, *Result, Reader, Parameters);
	for(auto PartIterator = std::begin(Parts); ((FoundAlternative == false) && (PartIterator != std::end(Parts))); ++PartIterator)
	{
		const auto & Part = *PartIterator;
		auto PartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(Part->Length != nullptr)
		{
			PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part->Length->GetAny(ExecutionContext)));
		}
		else
		{
			PartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		
		auto PartParameters = Part->GetParameters(ExecutionContext);
		auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
		
		FoundAlternative = PartResult->GetSuccess();
		if(FoundAlternative == true)
		{
			switch(Part->GetPartType())
			{
			case Inspection::TypeDefinition::PartType::Alternative:
				{
					Result->GetValue()->Extend(PartResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Array:
				{
					ASSERTION(Part->FieldName.has_value() == true);
					Result->GetValue()->AppendField(Part->FieldName.value(), PartResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Field:
				{
					ASSERTION(Part->FieldName.has_value() == true);
					Result->GetValue()->AppendField(Part->FieldName.value(), PartResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Fields:
				{
					Result->GetValue()->Extend(PartResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Forward:
				{
					Result->GetValue()->Extend(PartResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Sequence:
				{
					Result->GetValue()->Extend(PartResult->ExtractValue());
					
					break;
				}
			case Inspection::TypeDefinition::PartType::Type:
				{
					IMPOSSIBLE_CODE_REACHED("a Type should not be possible inside an Alternative");
				}
			}
			Reader.AdvancePosition(PartReader->GetConsumedLength());
		}
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(FoundAlternative);
	
	return Result;
}

auto Inspection::TypeDefinition::Alternative::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Alternative>
{
	return std::unique_ptr<Inspection::TypeDefinition::Alternative>{new Inspection::TypeDefinition::Alternative{}};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ApplyEnumeration                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Inspection::TypeDefinition::ApplyEnumeration::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const
{
	ASSERTION(Enumeration != nullptr);
	
	auto Result = true;
	
	if(Enumeration->BaseDataType == Inspection::TypeDefinition::DataType::String)
	{
		::ApplyEnumeration<std::string>(ExecutionContext, *Enumeration, Target);
	}
	else if(Enumeration->BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger8Bit)
	{
		::ApplyEnumeration<std::uint8_t>(ExecutionContext, *Enumeration, Target);
	}
	else if(Enumeration->BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger16Bit)
	{
		::ApplyEnumeration<std::uint16_t>(ExecutionContext, *Enumeration, Target);
	}
	else if(Enumeration->BaseDataType == Inspection::TypeDefinition::DataType::UnsignedInteger32Bit)
	{
		::ApplyEnumeration<std::uint32_t>(ExecutionContext, *Enumeration, Target);
	}
	else
	{
		UNEXPECTED_CASE("Enumeration->BaseDataType == " + Inspection::to_string(Enumeration->BaseDataType));
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration> Inspection::TypeDefinition::ApplyEnumeration::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration>{new Inspection::TypeDefinition::ApplyEnumeration{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(ChildElement->GetName() == "enumeration")
			{
				Result->Enumeration = Inspection::TypeDefinition::Enumeration::Load(ChildElement);
			}
			else
			{
				UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
			}
		}
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Array                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Array::Array(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Array}
{
}

auto Inspection::TypeDefinition::Array::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(*this, *Result, Reader, Parameters);
	Result->GetValue()->AddTag("array");
	switch(IterateType)
	{
	case Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength:
		{
			auto ElementParameters = GetElementParameters(ExecutionContext);
			
			ASSERTION(m_ElementType != nullptr);
			
			auto ElementType = m_ElementType->GetType(ExecutionContext);
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
					if(ElementName.has_value() == true)
					{
						Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
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
			auto ElementParameters = GetElementParameters(ExecutionContext);
			
			ASSERTION(IterateForEachField != nullptr);
			
			auto IterateField = ExecutionContext.GetFieldFromFieldReference(*IterateForEachField);
			
			ASSERTION(IterateField != nullptr);
			
			auto ElementProperties = std::vector<std::pair<Inspection::Length, Inspection::Length>>{};
			
			for(auto & Field : IterateField->GetFields())
			{
				ElementProperties.emplace_back(std::any_cast<const Inspection::Length &>(Field->GetTag("position")->GetData()), std::any_cast<const Inspection::Length &>(Field->GetTag("length")->GetData()));
			}
			std::sort(std::begin(ElementProperties), std::end(ElementProperties));
			ASSERTION(m_ElementType != nullptr);
			
			auto ElementType = m_ElementType->GetType(ExecutionContext);
			auto NumberOfAppendedElements = static_cast<std::uint64_t>(0);
			
			for(auto ElementPropertiesIndex = static_cast<std::uint64_t>(0); (Continue == true) && (ElementPropertiesIndex < ElementProperties.size()); ++ElementPropertiesIndex)
			{
				auto & Properties = ElementProperties[ElementPropertiesIndex];
				
				ASSERTION(Reader.GetReadPositionInInput() == Properties.first);
				
				auto ElementReader = Inspection::Reader{Reader, Properties.second};
				auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
				
				Continue = ElementResult->GetSuccess();
				if(ElementName.has_value() == true)
				{
					Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
				}
				else
				{
					Result->GetValue()->AppendField(ElementResult->ExtractValue());
				}
				Reader.AdvancePosition(ElementReader.GetConsumedLength());
				++NumberOfAppendedElements;
			}
			Result->GetValue()->AddTag("number of elements", NumberOfAppendedElements);
			
			break;
		}
	case Inspection::TypeDefinition::Array::IterateType::NumberOfElements:
		{
			auto ElementParameters = GetElementParameters(ExecutionContext);
			
			ASSERTION(m_ElementType != nullptr);
			
			auto ElementType = m_ElementType->GetType(ExecutionContext);
			auto NumberOfElementsAny = IterateNumberOfElements->GetAny(ExecutionContext);
			
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
					if(ElementName.has_value() == true)
					{
						Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
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
			auto ElementParameters = GetElementParameters(ExecutionContext);
			
			ASSERTION(m_ElementType != nullptr);
			
			auto ElementType = m_ElementType->GetType(ExecutionContext);
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
					if(ElementName.has_value() == true)
					{
						Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
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
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

auto Inspection::TypeDefinition::Array::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Array>
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Array>{new Inspection::TypeDefinition::Array{}};
	
	ASSERTION(Element->HasAttribute("name") == true);
	Result->FieldName = Element->GetAttribute("name");
	
	return Result;
}

auto Inspection::TypeDefinition::Array::_LoadProperty(XML::Element const * Element) -> void
{
	if(Element->GetName() == "iterate")
	{
		ASSERTION(GetPartType() == Inspection::TypeDefinition::PartType::Array);
		ASSERTION(Element->HasAttribute("type") == true);
		if(Element->GetAttribute("type") == "at-least-one-until-failure-or-length")
		{
			IterateType = Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength;
			ASSERTION(XML::HasChildNodes(Element) == false);
		}
		else if(Element->GetAttribute("type") == "for-each-field")
		{
			IterateType = Inspection::TypeDefinition::Array::IterateType::ForEachField;
			
			auto FieldReferenceElement = static_cast<const XML::Element *>(nullptr);
			
			for(auto PartIterateChildNode : Element->GetChilds())
			{
				if(PartIterateChildNode->GetNodeType() == XML::NodeType::Element)
				{
					ASSERTION(FieldReferenceElement == nullptr);
					FieldReferenceElement = dynamic_cast<const XML::Element *>(PartIterateChildNode);
				}
			}
			ASSERTION(FieldReferenceElement != nullptr);
			IterateForEachField = Inspection::TypeDefinition::FieldReference::Load(FieldReferenceElement);
		}
		else if(Element->GetAttribute("type") == "number-of-elements")
		{
			IterateType = Inspection::TypeDefinition::Array::IterateType::NumberOfElements;
			IterateNumberOfElements = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
		}
		else if(Element->GetAttribute("type") == "until-failure-or-length")
		{
			IterateType = Inspection::TypeDefinition::Array::IterateType::UntilFailureOrLength;
			ASSERTION(XML::HasChildNodes(Element) == false);
		}
		else
		{
			UNEXPECTED_CASE("Element->GetAttribute(\"type\") == " + Element->GetAttribute("type"));
		}
	}
	else if(Element->GetName() == "element-name")
	{
		ASSERTION(Element->GetChilds().size() == 1);
		
		auto ElementNameText = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(ElementNameText != nullptr);
		ElementName = ElementNameText->GetText();
	}
	else if(Element->GetName() == "element-type")
	{
		m_ElementType = Inspection::TypeDefinition::TypeReference::Load(Element);
	}
	else if(Element->GetName() == "element-parameters")
	{
		ElementParameters = Inspection::TypeDefinition::Parameters::Load(Element);
	}
	else
	{
		Inspection::TypeDefinition::Part::_LoadProperty(Element);
	}
}

std::unordered_map<std::string, std::any> Inspection::TypeDefinition::Array::GetElementParameters(Inspection::ExecutionContext & ExecutionContext) const
{
	return ::GetParameters(ExecutionContext, ElementParameters.get());
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Cast                                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Cast::Cast(Inspection::TypeDefinition::DataType DataType) :
	m_DataType{DataType}
{
}

std::any Inspection::TypeDefinition::Cast::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Expression != nullptr);
	switch(m_DataType)
	{
	case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
		{
			return ::CastTo<float>(m_Expression->GetAny(ExecutionContext));
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
		{
			return ::CastTo<std::uint8_t>(m_Expression->GetAny(ExecutionContext));
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
		{
			return ::CastTo<std::uint64_t>(m_Expression->GetAny(ExecutionContext));
		}
	default:
		{
			UNEXPECTED_CASE("_DataType == " + Inspection::to_string(m_DataType));
		}
	}
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Cast::GetDataType(void) const
{
	return m_DataType;
}

const Inspection::TypeDefinition::Expression & Inspection::TypeDefinition::Cast::GetExpression(void) const
{
	ASSERTION(m_Expression != nullptr);
	
	return *m_Expression;
}

std::unique_ptr<Inspection::TypeDefinition::Cast> Inspection::TypeDefinition::Cast::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Cast>{};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			Result = std::unique_ptr<Inspection::TypeDefinition::Cast>{new Inspection::TypeDefinition::Cast{Inspection::TypeDefinition::GetDataTypeFromString(Element->GetName())}};
			Result->m_Expression = Inspection::TypeDefinition::Expression::Load(ChildElement);
			
			break;
		}
	}
	INVALID_INPUT_IF((Result == nullptr) || (Result->m_Expression == nullptr), "Missing operand for Cast expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// DataReference                                                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::DataReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	auto Value = ExecutionContext.GetValueFromDataReference(*this);
	
	ASSERTION(Value != nullptr);
	
	return Value->GetData();
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::DataReference::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a DataReference expression.");
}

const std::vector<Inspection::TypeDefinition::DataReference::Part> & Inspection::TypeDefinition::DataReference::GetParts(void) const
{
	return m_Parts;
}

Inspection::TypeDefinition::DataReference::Root Inspection::TypeDefinition::DataReference::GetRoot(void) const
{
	return m_Root;
}

std::unique_ptr<Inspection::TypeDefinition::DataReference> Inspection::TypeDefinition::DataReference::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::DataReference>{new Inspection::TypeDefinition::DataReference{}};
	
	ASSERTION(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "current")
	{
		Result->m_Root = Inspection::TypeDefinition::DataReference::Root::Current;
	}
	else if(Element->GetAttribute("root") == "type")
	{
		Result->m_Root = Inspection::TypeDefinition::DataReference::Root::Type;
	}
	else
	{
		UNEXPECTED_CASE("Element->GetAttribute(\"root\") == " + Element->GetAttribute("root"));
	}
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(ChildElement->GetName() == "field")
			{
				ASSERTION(ChildElement->GetChilds().size() == 1);
				
				auto & DataReferencePart = Result->m_Parts.emplace_back(Inspection::TypeDefinition::DataReference::Part::Type::Field);
				auto SubText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
				
				ASSERTION(SubText != nullptr);
				DataReferencePart.DetailName = SubText->GetText();
			}
			else if(ChildElement->GetName() == "tag")
			{
				ASSERTION(ChildElement->GetChilds().size() == 1);
				
				auto & DataReferencePart = Result->m_Parts.emplace_back(Inspection::TypeDefinition::DataReference::Part::Type::Tag);
				auto TagText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
				
				ASSERTION(TagText != nullptr);
				DataReferencePart.DetailName = TagText->GetText();
			}
			else
			{
				UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::DataReference::Part::Part(Inspection::TypeDefinition::DataReference::Part::Type Type) :
	m_Type{Type}
{
}

Inspection::TypeDefinition::DataReference::Part::Type Inspection::TypeDefinition::DataReference::Part::GetType(void) const
{
	return m_Type;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Divide                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Divide::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Dividend != nullptr);
	ASSERTION(m_Divisor != nullptr);
	
	auto DividendAny = m_Dividend->GetAny(ExecutionContext);
	auto DivisorAny = m_Divisor->GetAny(ExecutionContext);
	
	ASSERTION(DividendAny.has_value() == true);
	ASSERTION(DivisorAny.has_value() == true);
	ASSERTION(DividendAny.type() == DivisorAny.type());
	if(DividendAny.type() == typeid(float))
	{
		return std::any_cast<float>(DividendAny) / std::any_cast<float>(DivisorAny);
	}
	else
	{
		UNEXPECTED_CASE("DividendAny.type() == " + Inspection::to_string(DividendAny.type()));
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Divide::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a Divide expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Divide> Inspection::TypeDefinition::Divide::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Divide>{new Inspection::TypeDefinition::Divide{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(First == true)
			{
				Result->m_Dividend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				INVALID_INPUT_IF(Result->m_Divisor != nullptr, "More than two operands for Divide expression.");
				Result->m_Divisor = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	INVALID_INPUT_IF(Result->m_Dividend == nullptr, "Missing operands for Divide expression.");
	INVALID_INPUT_IF(Result->m_Divisor == nullptr, "Missing second operand for Devide expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Enumeration                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Inspection::TypeDefinition::Enumeration> Inspection::TypeDefinition::Enumeration::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Enumeration>{new Inspection::TypeDefinition::Enumeration{}};
	
	ASSERTION(Element != nullptr);
	ASSERTION(Element->GetName() == "enumeration");
	Result->BaseDataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetAttribute("base-data-type"));
	for(auto EnumerationChildNode : Element->GetChilds())
	{
		if(EnumerationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto EnumerationChildElement = dynamic_cast<const XML::Element *>(EnumerationChildNode);
			
			if(EnumerationChildElement->GetName() == "element")
			{
				auto & Element = Result->Elements.emplace_back();
				
				ASSERTION(EnumerationChildElement->HasAttribute("base-value") == true);
				Element.BaseValue = EnumerationChildElement->GetAttribute("base-value");
				ASSERTION(EnumerationChildElement->HasAttribute("valid") == true);
				Element.Valid = from_string_cast<bool>(EnumerationChildElement->GetAttribute("valid"));
				for(auto EnumerationElementChildNode : EnumerationChildElement->GetChilds())
				{
					if(EnumerationElementChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto EnumerationElementChildElement = dynamic_cast<const XML::Element *>(EnumerationElementChildNode);
						
						if(EnumerationElementChildElement->GetName() == "tag")
						{
							Element.Tags.push_back(Inspection::TypeDefinition::Tag::Load(EnumerationElementChildElement));
						}
						else
						{
							UNEXPECTED_CASE("EnumerationElementChildElement->GetName() == " + EnumerationElementChildElement->GetName());
						}
					}
				}
			}
			else if(EnumerationChildElement->GetName() == "fallback-element")
			{
				ASSERTION(Result->FallbackElement.has_value() == false);
				Result->FallbackElement.emplace();
				Result->FallbackElement->Valid = from_string_cast<bool>(EnumerationChildElement->GetAttribute("valid"));
				for(auto EnumerationFallbackElementChildNode : EnumerationChildElement->GetChilds())
				{
					if(EnumerationFallbackElementChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto EnumerationFallbackElementChildElement = dynamic_cast<const XML::Element *>(EnumerationFallbackElementChildNode);
						
						if(EnumerationFallbackElementChildElement->GetName() == "tag")
						{
							Result->FallbackElement->Tags.push_back(Inspection::TypeDefinition::Tag::Load(EnumerationFallbackElementChildElement));
						}
						else
						{
							UNEXPECTED_CASE("EnumerationFallbackElementChildElement->GetName() == " + EnumerationFallbackElementChildElement->GetName());
						}
					}
				}
			}
			else
			{
				UNEXPECTED_CASE("EnumerationChildElement->GetName() == " + EnumerationChildElement->GetName());
			}
		}
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Equals                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Equals::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Expression1 != nullptr);
	ASSERTION(m_Expression2 != nullptr);
	
	auto Expression1Any = m_Expression1->GetAny(ExecutionContext);
	auto Expression2Any = m_Expression2->GetAny(ExecutionContext);
	
	ASSERTION(Expression1Any.has_value() == true);
	ASSERTION(Expression2Any.has_value() == true);
	ASSERTION(Expression1Any.type() == Expression2Any.type());
	if(Expression1Any.type() == typeid(std::uint8_t))
	{
		return std::any_cast<const std::uint8_t &>(Expression1Any) == std::any_cast<const std::uint8_t &>(Expression2Any);
	}
	else if(Expression1Any.type() == typeid(std::uint16_t))
	{
		return std::any_cast<const std::uint16_t &>(Expression1Any) == std::any_cast<const std::uint16_t &>(Expression2Any);
	}
	else if(Expression1Any.type() == typeid(std::uint32_t))
	{
		return std::any_cast<const std::uint32_t &>(Expression1Any) == std::any_cast<const std::uint32_t &>(Expression2Any);
	}
	else if(Expression1Any.type() == typeid(Inspection::GUID))
	{
		return std::any_cast<const Inspection::GUID &>(Expression1Any) == std::any_cast<const Inspection::GUID &>(Expression2Any);
	}
	else if(Expression1Any.type() == typeid(std::string))
	{
		return std::any_cast<const std::string &>(Expression1Any) == std::any_cast<const std::string &>(Expression2Any);
	}
	else
	{
		UNEXPECTED_CASE("Expression1Any.type() == " + Inspection::to_string(Expression1Any.type()));
	}
	
	return false;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Equals::GetDataType(void) const
{
	return TypeDefinition::DataType::Boolean;
}

std::unique_ptr<Inspection::TypeDefinition::Equals> Inspection::TypeDefinition::Equals::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Equals>{new Inspection::TypeDefinition::Equals{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(First == true)
			{
				Result->m_Expression1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				INVALID_INPUT_IF(Result->m_Expression2 != nullptr, "More than two operands for Equals expression.");
				Result->m_Expression2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	INVALID_INPUT_IF(Result->m_Expression1 == nullptr, "Missing operands for Equals expression.");
	INVALID_INPUT_IF(Result->m_Expression2 == nullptr, "Missing second operand for Equals expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Expression                                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Inspection::TypeDefinition::Expression> Inspection::TypeDefinition::Expression::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Expression>{};
	
	if(Element->GetName() == "add")
	{
		Result = Inspection::TypeDefinition::Add::Load(Element);
	}
	else if(Element->GetName() == "data-reference")
	{
		Result = Inspection::TypeDefinition::DataReference::Load(Element);
	}
	else if(Element->GetName() == "divide")
	{
		Result = Inspection::TypeDefinition::Divide::Load(Element);
	}
	else if(Element->GetName() == "equals")
	{
		Result = Inspection::TypeDefinition::Equals::Load(Element);
	}
	else if(Element->GetName() == "field-reference")
	{
		Result = Inspection::TypeDefinition::FieldReference::Load(Element);
	}
	else if(Element->GetName() == "length")
	{
		if(XML::HasOneChildElement(Element) == true)
		{
			// <length>-Element contains an expression (i.e. <subtract>)
			Result = Inspection::TypeDefinition::Expression::Load(XML::GetFirstChildElement(Element));
		}
		else
		{
			// <length>-Element IS a length with <bytes> and <bits>
			Result = Inspection::TypeDefinition::Length::Load(Element);
		}
	}
	else if(Element->GetName() == "length-reference")
	{
		Result = Inspection::TypeDefinition::LengthReference::Load(Element);
	}
	else if(Element->GetName() == "less-than")
	{
		Result = Inspection::TypeDefinition::LessThan::Load(Element);
	}
	else if(Element->GetName() == "modulus")
	{
		Result = Inspection::TypeDefinition::Modulus::Load(Element);
	}
	else if(Element->GetName() == "parameter-reference")
	{
		Result = Inspection::TypeDefinition::ParameterReference::Load(Element);
	}
	else if(Element->GetName() == "parameters")
	{
		Result = Inspection::TypeDefinition::Parameters::Load(Element);
	}
	else if(Element->GetName() == "subtract")
	{
		Result = Inspection::TypeDefinition::Subtract::Load(Element);
	}
	else if(Element->GetName() == "type-reference")
	{
		Result = Inspection::TypeDefinition::TypeReference::Load(Element);
	}
	else if((Element->GetName() == "unsigned-integer-8bit") && (XML::HasOneChildElement(Element) == true))
	{
		Result = Inspection::TypeDefinition::Cast::Load(Element);
	}
	else if((Element->GetName() == "unsigned-integer-64bit") && (XML::HasOneChildElement(Element) == true))
	{
		Result = Inspection::TypeDefinition::Cast::Load(Element);
	}
	else if((Element->GetName() == "single-precision-real") && (XML::HasOneChildElement(Element) == true))
	{
		Result = Inspection::TypeDefinition::Cast::Load(Element);
	}
	else
	{
		Result = Inspection::TypeDefinition::Value::Load(Element);
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Expression> Inspection::TypeDefinition::Expression::LoadFromWithin(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto ExpressionElement = static_cast<const XML::Element *>(nullptr);
	
	if(Element->GetChilds().size() > 0)
	{
		for(auto ChildNode : Element->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				ExpressionElement = dynamic_cast<const XML::Element *>(ChildNode);
				
				break;
			}
		}
		if(ExpressionElement == nullptr)
		{
			ASSERTION(false);
		}
	}
	
	return Inspection::TypeDefinition::Expression::Load(ExpressionElement);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Field                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Field::Field(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Field}
{
}

auto Inspection::TypeDefinition::Field::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(*this, *Result, Reader, Parameters);
	if(TypeReference)
	{
		ASSERTION(TypeReference != nullptr);
		
		const auto * FieldType = TypeReference->GetType(ExecutionContext);
		
		ASSERTION(FieldType != nullptr);
		
		auto FieldResult = FieldType->Get(Reader, ExecutionContext.GetAllParameters());
		
		Continue = FieldResult->GetSuccess();
		Result->GetValue()->Extend(FieldResult->ExtractValue());
	}
	else
	{
		ASSERTION(Parts.size() == 1);
		
		const auto & Part = Parts.front();
		auto PartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(Part->Length != nullptr)
		{
			PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part->Length->GetAny(ExecutionContext)));
		}
		else
		{
			PartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		
		auto PartParameters = Part->GetParameters(ExecutionContext);
		auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->Extend(PartResult->ExtractValue());
		Reader.AdvancePosition(PartReader->GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

auto Inspection::TypeDefinition::Field::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Field>
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Field>{new Inspection::TypeDefinition::Field{}};
	
	ASSERTION(Element->HasAttribute("name") == true);
	Result->FieldName = Element->GetAttribute("name");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// FieldReference                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::FieldReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	NOT_IMPLEMENTED("Called GetAny() on a FieldReference expression.");
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::FieldReference::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a FieldReference expression.");
}

Inspection::TypeDefinition::FieldReference::Root Inspection::TypeDefinition::FieldReference::GetRoot(void) const
{
	return m_Root;
}

std::unique_ptr<Inspection::TypeDefinition::FieldReference> Inspection::TypeDefinition::FieldReference::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::FieldReference>{new Inspection::TypeDefinition::FieldReference{}};
	
	ASSERTION(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "current")
	{
		Result->m_Root = Inspection::TypeDefinition::FieldReference::Root::Current;
	}
	else if(Element->GetAttribute("root") == "type")
	{
		Result->m_Root = Inspection::TypeDefinition::FieldReference::Root::Type;
	}
	else
	{
		UNEXPECTED_CASE("Element->GetAttribute(\"root\") == " + Element->GetAttribute("root"));
	}
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			ASSERTION(ChildElement->GetName() == "field");
			ASSERTION(ChildElement->GetChilds().size() == 1);
			
			auto FieldReferenceFieldText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
			
			ASSERTION(FieldReferenceFieldText != nullptr);
			Result->Parts.push_back(FieldReferenceFieldText->GetText());
		}
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Fields                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Fields::Fields(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Fields}
{
}

auto Inspection::TypeDefinition::Fields::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(*this, *Result, Reader, Parameters);
	ASSERTION(TypeReference != nullptr);
	
	auto FieldsType = TypeReference->GetType(ExecutionContext);
	
	ASSERTION(FieldsType != nullptr);
	
	auto FieldsResult = FieldsType->Get(Reader, ExecutionContext.GetAllParameters());
	
	Continue = FieldsResult->GetSuccess();
	Result->GetValue()->Extend(FieldsResult->ExtractValue());
	// interpretation
	if(Continue == true)
	{
		Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

auto Inspection::TypeDefinition::Fields::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Fields>
{
	return std::unique_ptr<Inspection::TypeDefinition::Fields>{new Inspection::TypeDefinition::Fields{}};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Forward::Forward(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Forward}
{
}

auto Inspection::TypeDefinition::Forward::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(*this, *Result, Reader, Parameters);
	ASSERTION(TypeReference != nullptr);
	
	auto ForwardType = TypeReference->GetType(ExecutionContext);
	
	ASSERTION(ForwardType != nullptr);
	
	auto ForwardResult = ForwardType->Get(Reader, ExecutionContext.GetAllParameters());
	
	Continue = ForwardResult->GetSuccess();
	Result->GetValue()->Extend(ForwardResult->ExtractValue());
	// interpretation
	if(Continue == true)
	{
		Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

auto Inspection::TypeDefinition::Forward::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Forward>
{
	return std::unique_ptr<Inspection::TypeDefinition::Forward>{new Inspection::TypeDefinition::Forward{}};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Interpretation                                                                                //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Inspection::TypeDefinition::Interpretation> Inspection::TypeDefinition::Interpretation::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Interpretation>{};
	
	ASSERTION(Element->GetName() == "interpretation");
	for(auto InterpretationChildNode : Element->GetChilds())
	{
		if(InterpretationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto InterpretationChildElement = dynamic_cast<const XML::Element *>(InterpretationChildNode);
			
			if(InterpretationChildElement->GetName() == "apply-enumeration")
			{
				Result = Inspection::TypeDefinition::ApplyEnumeration::Load(InterpretationChildElement);
			}
			else
			{
				UNEXPECTED_CASE("InterpretationChildElement->GetName() == " + InterpretationChildElement->GetName());
			}
		}
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Length                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Length::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Bytes != nullptr);
	ASSERTION(m_Bits != nullptr);
	
	auto BytesAny = m_Bytes->GetAny(ExecutionContext);
	
	ASSERTION(BytesAny.has_value() == true);
	ASSERTION(BytesAny.type() == typeid(std::uint64_t));
	
	auto BitsAny = m_Bits->GetAny(ExecutionContext);
	
	ASSERTION(BitsAny.has_value() == true);
	ASSERTION(BitsAny.type() == typeid(std::uint64_t));
	
	return Inspection::Length{std::any_cast<std::uint64_t>(BytesAny), std::any_cast<std::uint64_t>(BitsAny)};
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Length::GetDataType(void) const
{
	return Inspection::TypeDefinition::DataType::Length;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// LengthReference                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Inspection::TypeDefinition::Length> Inspection::TypeDefinition::Length::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Length>{new Inspection::TypeDefinition::Length{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			if(ChildElement->GetName() == "bytes")
			{
				Result->m_Bytes = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
			}
			else if(ChildElement->GetName() == "bits")
			{
				Result->m_Bits = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
			}
			else
			{
				UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
			}
		}
	}
	
	return Result;
}

std::any Inspection::TypeDefinition::LengthReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	return ExecutionContext.CalculateLengthFromReference(*this);
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::LengthReference::GetDataType(void) const
{
	return Inspection::TypeDefinition::DataType::Length;
}

Inspection::TypeDefinition::LengthReference::Root Inspection::TypeDefinition::LengthReference::GetRoot(void) const
{
	return m_Root;
}

Inspection::TypeDefinition::LengthReference::Name Inspection::TypeDefinition::LengthReference::GetName(void) const
{
	return m_Name;
}

std::unique_ptr<Inspection::TypeDefinition::LengthReference> Inspection::TypeDefinition::LengthReference::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::LengthReference>{new Inspection::TypeDefinition::LengthReference{}};
	
	ASSERTION(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "type")
	{
		Result->m_Root = Inspection::TypeDefinition::LengthReference::Root::Type;
	}
	else
	{
		UNEXPECTED_CASE("Element->GetAttribute(\"root\") == " + Element->GetAttribute("root"));
	}
	ASSERTION(Element->HasAttribute("name") == true);
	if(Element->GetAttribute("name") == "consumed")
	{
		Result->m_Name = Inspection::TypeDefinition::LengthReference::Name::Consumed;
	}
	else
	{
		UNEXPECTED_CASE("Element->GetAttribute(\"name\") == " + Element->GetAttribute("name"));
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// LessThan                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::LessThan::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Expression1 != nullptr);
	ASSERTION(m_Expression2 != nullptr);
	
	auto Any1 = m_Expression1->GetAny(ExecutionContext);
	auto Any2 = m_Expression2->GetAny(ExecutionContext);
	
	ASSERTION(Any1.has_value() == true);
	ASSERTION(Any2.has_value() == true);
	ASSERTION(Any1.type() == Any2.type());
	if(Any1.type() == typeid(std::uint8_t))
	{
		return std::any_cast<const std::uint8_t &>(Any1) < std::any_cast<const std::uint8_t &>(Any2);
	}
	else if(Any1.type() == typeid(std::uint16_t))
	{
		return std::any_cast<const std::uint16_t &>(Any1) < std::any_cast<const std::uint16_t &>(Any2);
	}
	else if(Any1.type() == typeid(std::uint32_t))
	{
		return std::any_cast<const std::uint32_t &>(Any1) < std::any_cast<const std::uint32_t &>(Any2);
	}
	else
	{
		UNEXPECTED_CASE("Any1.type() == " + Inspection::to_string(Any1.type()));
	}
	
	return false;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::LessThan::GetDataType(void) const
{
	return TypeDefinition::DataType::Boolean;
}

std::unique_ptr<Inspection::TypeDefinition::LessThan> Inspection::TypeDefinition::LessThan::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::LessThan>{new Inspection::TypeDefinition::LessThan{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(First == true)
			{
				Result->m_Expression1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				INVALID_INPUT_IF(Result->m_Expression2 != nullptr, "More than two operands for LessThan expression.");
				Result->m_Expression2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	INVALID_INPUT_IF(Result->m_Expression1 == nullptr, "Missing operands for LessThan expression.");
	INVALID_INPUT_IF(Result->m_Expression2 == nullptr, "Missing second operand for LessThan expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Modulus                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Modulus::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Dividend != nullptr);
	ASSERTION(m_Divisor != nullptr);
	
	auto DividendAny = m_Dividend->GetAny(ExecutionContext);
	auto DivisorAny = m_Divisor->GetAny(ExecutionContext);
	
	ASSERTION(DividendAny.has_value() == true);
	ASSERTION(DivisorAny.has_value() == true);
	ASSERTION(DividendAny.type() == DivisorAny.type());
	if(DividendAny.type() == typeid(std::uint16_t))
	{
		return static_cast<std::uint16_t>(std::any_cast<std::uint16_t>(DividendAny) % std::any_cast<std::uint16_t>(DivisorAny));
	}
	else
	{
		UNEXPECTED_CASE("DividendAny.type() == " + Inspection::to_string(DividendAny.type()));
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Modulus::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a Modulus expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Modulus> Inspection::TypeDefinition::Modulus::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Modulus>{new Inspection::TypeDefinition::Modulus{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(First == true)
			{
				Result->m_Dividend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				INVALID_INPUT_IF(Result->m_Divisor != nullptr, "More than two operands for Modulus expression.");
				Result->m_Divisor = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	INVALID_INPUT_IF(Result->m_Dividend == nullptr, "Missing operands for Modulus expression.");
	INVALID_INPUT_IF(Result->m_Divisor == nullptr, "Missing second operand for Modulus expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ParameterReference                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::ParameterReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	return ExecutionContext.GetAnyReferenceFromParameterReference(*this);
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::ParameterReference::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a ParameterReference expression.");
}

std::unique_ptr<Inspection::TypeDefinition::ParameterReference> Inspection::TypeDefinition::ParameterReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::ParameterReference>{new Inspection::TypeDefinition::ParameterReference{}};
	
	ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
	
	auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
	
	ASSERTION(TextNode != nullptr);
	Result->Name = TextNode->GetText();
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Parameters                                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Parameters::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	return GetParameters(ExecutionContext);
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Parameters::GetDataType(void) const
{
	return Inspection::TypeDefinition::DataType::Parameters;
}

std::unordered_map<std::string, std::any> Inspection::TypeDefinition::Parameters::GetParameters(Inspection::ExecutionContext & ExecutionContext) const
{
	auto Result = std::unordered_map<std::string, std::any>{};
	
	for(auto & Parameter : m_Parameters)
	{
		Result.emplace(Parameter->GetName(), Parameter->GetAny(ExecutionContext));
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Parameters> Inspection::TypeDefinition::Parameters::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Parameters>{new Inspection::TypeDefinition::Parameters{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			ASSERTION(ChildElement->GetName() == "parameter");
			Result->m_Parameters.push_back(Inspection::TypeDefinition::Parameters::Parameter::Load(ChildElement));
		}
	}
	
	return Result;
}

std::any Inspection::TypeDefinition::Parameters::Parameter::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Expression != nullptr);
	
	return m_Expression->GetAny(ExecutionContext);
}

const std::string & Inspection::TypeDefinition::Parameters::Parameter::GetName(void) const
{
	return m_Name;
}

std::unique_ptr<Inspection::TypeDefinition::Parameters::Parameter> Inspection::TypeDefinition::Parameters::Parameter::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Parameters::Parameter>{new Inspection::TypeDefinition::Parameters::Parameter{}};
	
	ASSERTION(Element->HasAttribute("name") == true);
	Result->m_Name = Element->GetAttribute("name");
	Result->m_Expression = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Part                                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Part::Part(Inspection::TypeDefinition::PartType PartType) :
	m_PartType{PartType}
{
}

auto Inspection::TypeDefinition::Part::ApplyInterpretations(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
{
	auto Result = true;
	
	for(const auto & Interpretation : Interpretations)
	{
		ASSERTION(Interpretation != nullptr);
		Result &= Interpretation->Apply(ExecutionContext, Target);
		if(Result == false)
		{
			break;
		}
	}
	
	return Result;
}

auto Inspection::TypeDefinition::Part::GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>
{
	return ::GetParameters(ExecutionContext, Parameters.get());
}

auto Inspection::TypeDefinition::Part::GetPartType(void) const -> Inspection::TypeDefinition::PartType
{
	return m_PartType;
}

auto Inspection::TypeDefinition::Part::Load(const XML::Element * Element) -> std::unique_ptr<Inspection::TypeDefinition::Part>
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Part>{};
	
	if(Element->GetName() == "alternative")
	{
		Result = Inspection::TypeDefinition::Alternative::Load(Element);
	}
	else if(Element->GetName() == "array")
	{
		Result = Inspection::TypeDefinition::Array::Load(Element);
	}
	else if(Element->GetName() == "sequence")
	{
		Result = Inspection::TypeDefinition::Sequence::Load(Element);
	}
	else if(Element->GetName() == "field")
	{
		Result = Inspection::TypeDefinition::Field::Load(Element);
	}
	else if(Element->GetName() == "fields")
	{
		Result = Inspection::TypeDefinition::Fields::Load(Element);
	}
	else if(Element->GetName() == "forward")
	{
		Result = Inspection::TypeDefinition::Forward::Load(Element);
	}
	else
	{
		UNEXPECTED_CASE("Element->GetName() == " + Element->GetName());
	}
	Result->_LoadProperties(Element);
	
	return Result;
}

auto Inspection::TypeDefinition::Part::_LoadProperties(const XML::Element * Element) -> void
{
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			_LoadProperty(ChildElement);
		}
	}
}

auto Inspection::TypeDefinition::Part::_LoadProperty(const XML::Element * Element) -> void
{
	ASSERTION(Element != nullptr);
	if(Element->GetName() == "type-reference")
	{
		ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Fields) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
		TypeReference = Inspection::TypeDefinition::TypeReference::Load(Element);
	}
	else if(Element->GetName() == "interpretation")
	{
		Interpretations.push_back(Inspection::TypeDefinition::Interpretation::Load(Element));
	}
	else if(Element->GetName() == "length")
	{
		Length = Inspection::TypeDefinition::Expression::Load(Element);
	}
	else if(Element->GetName() == "parameters")
	{
		ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Fields) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
		Parameters = Inspection::TypeDefinition::Parameters::Load(Element);
	}
	else if(Element->GetName() == "verification")
	{
		for(auto GetterPartVerificationChildNode : Element->GetChilds())
		{
			if(GetterPartVerificationChildNode->GetNodeType() == XML::NodeType::Element)
			{
				Interpretations.push_back(Inspection::TypeDefinition::Verification::Load(dynamic_cast<const XML::Element *>(GetterPartVerificationChildNode)));
			}
		}
	}
	else if(Element->GetName() == "tag")
	{
		ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward) || (m_PartType == Inspection::TypeDefinition::PartType::Sequence));
		Interpretations.push_back(Inspection::TypeDefinition::AddTag::Load(Element));
	}
	else if((Element->GetName() == "alternative") || (Element->GetName() == "sequence") || (Element->GetName() == "field") || (Element->GetName() == "fields") || (Element->GetName() == "forward") || (Element->GetName() == "array"))
	{
		ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Sequence) || (m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Alternative));
		Parts.emplace_back(Inspection::TypeDefinition::Part::Load(Element));
	}
	else
	{
		UNEXPECTED_CASE("Element->GetName() == " + Element->GetName());
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Sequence                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Sequence::Sequence(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Sequence}
{
}

auto Inspection::TypeDefinition::Sequence::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	auto Result = std::make_unique<Inspection::Result>();
	auto Continue = true;
	
	ExecutionContext.Push(*this, *Result, Reader, Parameters);
	for(auto PartIterator = std::begin(Parts); ((Continue == true) && (PartIterator != std::end(Parts))); ++PartIterator)
	{
		const auto & Part = *PartIterator;
		auto PartReader = std::unique_ptr<Inspection::Reader>{};
		
		if(Part->Length != nullptr)
		{
			PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part->Length->GetAny(ExecutionContext)));
		}
		else
		{
			PartReader = std::make_unique<Inspection::Reader>(Reader);
		}
		
		auto PartParameters = Part->GetParameters(ExecutionContext);
		auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
		
		Continue = PartResult->GetSuccess();
		switch(Part->GetPartType())
		{
		case Inspection::TypeDefinition::PartType::Alternative:
			{
				Result->GetValue()->Extend(PartResult->ExtractValue());
				
				break;
			}
		case Inspection::TypeDefinition::PartType::Array:
			{
				ASSERTION(Part->FieldName.has_value() == true);
				Result->GetValue()->AppendField(Part->FieldName.value(), PartResult->ExtractValue());
				
				break;
			}
		case Inspection::TypeDefinition::PartType::Field:
			{
				ASSERTION(Part->FieldName.has_value() == true);
				Result->GetValue()->AppendField(Part->FieldName.value(), PartResult->ExtractValue());
				
				break;
			}
		case Inspection::TypeDefinition::PartType::Fields:
			{
				Result->GetValue()->Extend(PartResult->ExtractValue());
				
				break;
			}
		case Inspection::TypeDefinition::PartType::Forward:
			{
				Result->GetValue()->Extend(PartResult->ExtractValue());
				
				break;
			}
		case Inspection::TypeDefinition::PartType::Sequence:
			{
				Result->GetValue()->Extend(PartResult->ExtractValue());
				
				break;
			}
		case Inspection::TypeDefinition::PartType::Type:
			{
				IMPOSSIBLE_CODE_REACHED("a Type should not be possible inside a Sequence");
			}
		}
		Reader.AdvancePosition(PartReader->GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
	}
	ExecutionContext.Pop();
	// finalization
	Result->SetSuccess(Continue);
	
	return Result;
}

auto Inspection::TypeDefinition::Sequence::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Sequence>
{
	return std::unique_ptr<Inspection::TypeDefinition::Sequence>{new Inspection::TypeDefinition::Sequence{}};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Subtract                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Subtract::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Minuend != nullptr);
	ASSERTION(m_Subtrahend != nullptr);
	
	auto MinuendAny = m_Minuend->GetAny(ExecutionContext);
	auto SubtrahendAny = m_Subtrahend->GetAny(ExecutionContext);
	
	ASSERTION(MinuendAny.has_value() == true);
	ASSERTION(SubtrahendAny.has_value() == true);
	ASSERTION(MinuendAny.type() == SubtrahendAny.type());
	if(MinuendAny.type() == typeid(Inspection::Length))
	{
		return std::any_cast<const Inspection::Length &>(MinuendAny) - std::any_cast<const Inspection::Length &>(SubtrahendAny);
	}
	else
	{
		UNEXPECTED_CASE("MinuendAny.type() == " + Inspection::to_string(MinuendAny.type()));
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Subtract::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a Subtract expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Subtract> Inspection::TypeDefinition::Subtract::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Subtract>{new Inspection::TypeDefinition::Subtract{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			if(First == true)
			{
				Result->m_Minuend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				INVALID_INPUT_IF(Result->m_Subtrahend != nullptr, "More than two operands for Subtract expression.");
				Result->m_Subtrahend = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	INVALID_INPUT_IF(Result->m_Minuend == nullptr, "Missing operands for Subtract expression.");
	INVALID_INPUT_IF(Result->m_Subtrahend == nullptr, "Missing second operand for Subtract expression.");
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Tag                                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::Tag::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(m_Expression != nullptr);
	
	return m_Expression->GetAny(ExecutionContext);
}

const std::string & Inspection::TypeDefinition::Tag::GetName(void) const
{
	return m_Name;
}

bool Inspection::TypeDefinition::Tag::HasExpression(void) const
{
	return m_Expression != nullptr;
}

std::unique_ptr<Inspection::TypeDefinition::Tag> Inspection::TypeDefinition::Tag::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Tag>{new Inspection::TypeDefinition::Tag{}};
	
	ASSERTION(Element->HasAttribute("name") == true);
	Result->m_Name = Element->GetAttribute("name");
	if(XML::HasChildElements(Element) == true)
	{
		Result->m_Expression = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// TypePart                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::TypePart::TypePart(void) :
	Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Type}
{
}

auto Inspection::TypeDefinition::TypePart::Create() -> std::unique_ptr<Inspection::TypeDefinition::TypePart>
{
	return std::unique_ptr<Inspection::TypeDefinition::TypePart>{new Inspection::TypeDefinition::TypePart{}};
}

auto Inspection::TypeDefinition::TypePart::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
	NOT_IMPLEMENTED("Called Get() on a Type part.");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// TypeReference                                                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::any Inspection::TypeDefinition::TypeReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	return ExecutionContext.GetTypeRepository().GetType(m_Parts);
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::TypeReference::GetDataType(void) const
{
	return Inspection::TypeDefinition::DataType::Type;
}

const Inspection::TypeDefinition::Type * Inspection::TypeDefinition::TypeReference::GetType(Inspection::ExecutionContext & ExecutionContext) const
{
	return ExecutionContext.GetTypeRepository().GetType(m_Parts);
}

std::unique_ptr<Inspection::TypeDefinition::TypeReference> Inspection::TypeDefinition::TypeReference::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::TypeReference>{new Inspection::TypeDefinition::TypeReference{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			ASSERTION(ChildElement->GetName() == "part");
			ASSERTION(ChildElement->GetChilds().size() == 1);
			
			auto PartText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
			
			ASSERTION(PartText != nullptr);
			Result->m_Parts.push_back(PartText->GetText());
		}
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Value                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Value::Value(Inspection::TypeDefinition::DataType DataType) :
	m_DataType{DataType}
{
}

std::any Inspection::TypeDefinition::Value::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	switch(m_DataType)
	{
	case Inspection::TypeDefinition::DataType::Boolean:
		{
			ASSERTION(std::holds_alternative<bool>(m_Data) == true);
			
			return std::get<bool>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::GUID:
		{
			ASSERTION(std::holds_alternative<Inspection::GUID>(m_Data) == true);
			
			return std::get<Inspection::GUID>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::Nothing:
		{
			return nullptr;
		}
	case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
		{
			ASSERTION(std::holds_alternative<float>(m_Data) == true);
			
			return std::get<float>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::String:
		{
			ASSERTION(std::holds_alternative<std::string>(m_Data) == true);
			
			return std::get<std::string>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
		{
			ASSERTION(std::holds_alternative<std::uint8_t>(m_Data) == true);
			
			return std::get<std::uint8_t>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
		{
			ASSERTION(std::holds_alternative<std::uint16_t>(m_Data) == true);
			
			return std::get<std::uint16_t>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
		{
			ASSERTION(std::holds_alternative<std::uint32_t>(m_Data) == true);
			
			return std::get<std::uint32_t>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
		{
			ASSERTION(std::holds_alternative<std::uint64_t>(m_Data) == true);
			
			return std::get<std::uint64_t>(m_Data);
		}
	case Inspection::TypeDefinition::DataType::Length:
		{
			IMPOSSIBLE_CODE_REACHED("Length should always be an Expression, not a Value");
		}
	case Inspection::TypeDefinition::DataType::Parameters:
		{
			IMPOSSIBLE_CODE_REACHED("Parameters should always be an Expression, not a Value");
		}
	case Inspection::TypeDefinition::DataType::Type:
		{
			IMPOSSIBLE_CODE_REACHED("Type cannot be defined by Value (or Expression)");
		}
	}
	IMPOSSIBLE_CODE_REACHED("switch handling of Inspection::TypeDefinition::DataType incomplete");
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Value::GetDataType(void) const
{
	return m_DataType;
}

std::unique_ptr<Inspection::TypeDefinition::Value> Inspection::TypeDefinition::Value::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Value>{};
	
	ASSERTION(Element != nullptr);
	if(Element->GetName() == "nothing")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::Nothing}};
		ASSERTION(Element->GetChilds().size() == 0);
	}
	else if(Element->GetName() == "boolean")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::Boolean}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = from_string_cast<bool>(TextNode->GetText());
	}
	else if(Element->GetName() == "guid")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::GUID}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data.emplace<Inspection::GUID>(TextNode->GetText());
	}
	else if(Element->GetName() == "single-precision-real")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::SinglePrecisionReal}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = from_string_cast<float>(TextNode->GetText());
	}
	else if(Element->GetName() == "string")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::String}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = std::string{TextNode->GetText()};
	}
	else if(Element->GetName() == "unsigned-integer-8bit")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::UnsignedInteger8Bit}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = from_string_cast<std::uint8_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-16bit")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::UnsignedInteger16Bit}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = from_string_cast<std::uint16_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-32bit")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::UnsignedInteger32Bit}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = from_string_cast<std::uint32_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-64bit")
	{
		Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::UnsignedInteger64Bit}};
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->m_Data = from_string_cast<std::uint64_t>(TextNode->GetText());
	}
	else
	{
		UNEXPECTED_CASE("Element->GetName() == " + Element->GetName());
	}
	
	return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Verification                                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Inspection::TypeDefinition::Verification::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const
{
	ASSERTION(m_Expression != nullptr);
	
	auto VerificationAny = m_Expression->GetAny(ExecutionContext);
	
	ASSERTION(VerificationAny.type() == typeid(bool));
	
	auto Result = std::any_cast<bool>(VerificationAny);
	
	if(Result == false)
	{
		Target->AddTag("error", "The value failed to verify."s);
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Verification> Inspection::TypeDefinition::Verification::Load(const XML::Element * Element)
{
	ASSERTION(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Verification>{new Inspection::TypeDefinition::Verification{}};
	
	Result->m_Expression = Inspection::TypeDefinition::Expression::Load(Element);
	
	return Result;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::GetDataTypeFromString(const std::string & String)
{
	if(String == "boolean")
	{
		return Inspection::TypeDefinition::DataType::Boolean;
	}
	else if(String == "length")
	{
		return Inspection::TypeDefinition::DataType::Length;
	}
	else if(String == "nothing")
	{
		return Inspection::TypeDefinition::DataType::Nothing;
	}
	else if(String == "parameters")
	{
		return Inspection::TypeDefinition::DataType::Parameters;
	}
	else if(String == "single-precision-real")
	{
		return Inspection::TypeDefinition::DataType::SinglePrecisionReal;
	}
	else if(String == "string")
	{
		return Inspection::TypeDefinition::DataType::String;
	}
	else if(String == "type")
	{
		return Inspection::TypeDefinition::DataType::Type;
	}
	else if((String == "unsigned integer 8bit") || (String == "unsigned-integer-8bit"))
	{
		return Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
	}
	else if((String == "unsigned integer 16bit") || (String == "unsigned-integer-16bit"))
	{
		return Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
	}
	else if((String == "unsigned integer 32bit") || (String == "unsigned-integer-32bit"))
	{
		return Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
	}
	else if((String == "unsigned integer 64bit") || (String == "unsigned-integer-64bit"))
	{
		return Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
	}
	else
	{
		UNEXPECTED_CASE("String == " + String);
	}
}
