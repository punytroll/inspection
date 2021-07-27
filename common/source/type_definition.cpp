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
#include "length.h"
#include "string_cast.h"
#include "type.h"
#include "type_definition.h"
#include "type_repository.h"
#include "value.h"
#include "xml_helper.h"
#include "xml_puny_dom.h"

template<typename Type>
Type CastTo(const std::any & Any)
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
		ASSERTION(false);
	}
}

Inspection::TypeDefinition::Add::Add(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::ExpressionType::Add}
{
}

std::any Inspection::TypeDefinition::Add::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(_Summand1 != nullptr);
	
	auto Summand1Any = _Summand1->GetAny(ExecutionContext);
	
	ASSERTION(_Summand2 != nullptr);
	
	auto Summand2Any = _Summand2->GetAny(ExecutionContext);
	
	if(Summand1Any.type() == Summand2Any.type())
	{
		if(Summand1Any.type() == typeid(std::uint8_t))
		{
			return static_cast<std::uint8_t>(std::any_cast<std::uint8_t>(Summand1Any) + std::any_cast<std::uint8_t>(Summand2Any));
		}
		else
		{
			ASSERTION(false);
		}
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Add::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on an Add expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Add> Inspection::TypeDefinition::Add::Load(const XML::Element * Element)
{
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
				Result->_Summand1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->_Summand2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
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
				ASSERTION(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Cast::Cast(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::ExpressionType::Cast},
	_DataType{Inspection::TypeDefinition::DataType::Unknown}
{
}

std::any Inspection::TypeDefinition::Cast::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(_Expression != nullptr);
	switch(_DataType)
	{
	case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
		{
			return CastTo<float>(_Expression->GetAny(ExecutionContext));
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
		{
			return CastTo<std::uint8_t>(_Expression->GetAny(ExecutionContext));
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
		{
			return CastTo<std::uint64_t>(_Expression->GetAny(ExecutionContext));
		}
	default:
		{
			ASSERTION(false);
		}
	}
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Cast::GetDataType(void) const
{
	return _DataType;
}

const Inspection::TypeDefinition::Expression & Inspection::TypeDefinition::Cast::GetExpression(void) const
{
	ASSERTION(_Expression != nullptr);
	
	return *_Expression;
}

std::unique_ptr<Inspection::TypeDefinition::Cast> Inspection::TypeDefinition::Cast::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Cast>{new Inspection::TypeDefinition::Cast{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			Result->_DataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetName());
			Result->_Expression = Inspection::TypeDefinition::Expression::Load(ChildElement);
			
			break;
		}
	}
	ASSERTION(Result->_DataType != Inspection::TypeDefinition::DataType::Unknown);
	ASSERTION(Result->_Expression != nullptr);
	
	return Result;
}

Inspection::TypeDefinition::DataReference::DataReference(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::DataReference}
{
}

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
	return _Parts;
}

Inspection::TypeDefinition::DataReference::Root Inspection::TypeDefinition::DataReference::GetRoot(void) const
{
	return _Root;
}

std::unique_ptr<Inspection::TypeDefinition::DataReference> Inspection::TypeDefinition::DataReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::DataReference>{new Inspection::TypeDefinition::DataReference{}};
	
	ASSERTION(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "current")
	{
		Result->_Root = Inspection::TypeDefinition::DataReference::Root::Current;
	}
	else if(Element->GetAttribute("root") == "type")
	{
		Result->_Root = Inspection::TypeDefinition::DataReference::Root::Type;
	}
	else
	{
		ASSERTION(false);
	}
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(ChildElement->GetName() == "field")
			{
				ASSERTION(ChildElement->GetChilds().size() == 1);
				
				auto & DataReferencePart = Result->_Parts.emplace_back(Inspection::TypeDefinition::DataReference::Part::Type::Field);
				auto SubText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
				
				ASSERTION(SubText != nullptr);
				DataReferencePart.DetailName = SubText->GetText();
			}
			else if(ChildElement->GetName() == "tag")
			{
				ASSERTION(ChildElement->GetChilds().size() == 1);
				
				auto & DataReferencePart = Result->_Parts.emplace_back(Inspection::TypeDefinition::DataReference::Part::Type::Tag);
				auto TagText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
				
				ASSERTION(TagText != nullptr);
				DataReferencePart.DetailName = TagText->GetText();
			}
			else
			{
				ASSERTION(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::DataReference::Part::Part(Inspection::TypeDefinition::DataReference::Part::Type Type) :
	_Type{Type}
{
}

Inspection::TypeDefinition::DataReference::Part::Type Inspection::TypeDefinition::DataReference::Part::GetType(void) const
{
	return _Type;
}

Inspection::TypeDefinition::Divide::Divide(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::ExpressionType::Divide}
{
}

std::any Inspection::TypeDefinition::Divide::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	auto DividendAny = _Dividend->GetAny(ExecutionContext);
	auto DivisorAny = _Divisor->GetAny(ExecutionContext);
	
	if(DividendAny.type() == DivisorAny.type())
	{
		if(DividendAny.type() == typeid(float))
		{
			return std::any_cast<float>(DividendAny) / std::any_cast<float>(DivisorAny);
		}
		else
		{
			ASSERTION(false);
		}
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Divide::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a Divide expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Divide> Inspection::TypeDefinition::Divide::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Divide>{new Inspection::TypeDefinition::Divide{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(First == true)
			{
				Result->_Dividend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->_Divisor = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

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
							ASSERTION(false);
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
							ASSERTION(false);
						}
					}
				}
			}
			else
			{
				ASSERTION(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Equals::Equals(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::ExpressionType::Equals}
{
}

std::any Inspection::TypeDefinition::Equals::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	auto Any1 = _Expression1->GetAny(ExecutionContext);
	auto Any2 = _Expression2->GetAny(ExecutionContext);

	if((Any1.has_value() == true) && (Any2.has_value() == true))
	{
		if(Any1.type() == Any2.type())
		{
			if(Any1.type() == typeid(std::uint8_t))
			{
				return std::any_cast<const std::uint8_t &>(Any1) == std::any_cast<const std::uint8_t &>(Any2);
			}
			else if(Any1.type() == typeid(std::uint16_t))
			{
				return std::any_cast<const std::uint16_t &>(Any1) == std::any_cast<const std::uint16_t &>(Any2);
			}
			else if(Any1.type() == typeid(std::uint32_t))
			{
				return std::any_cast<const std::uint32_t &>(Any1) == std::any_cast<const std::uint32_t &>(Any2);
			}
			else if(Any1.type() == typeid(Inspection::GUID))
			{
				return std::any_cast<const Inspection::GUID &>(Any1) == std::any_cast<const Inspection::GUID &>(Any2);
			}
			else if(Any1.type() == typeid(std::string))
			{
				return std::any_cast<const std::string &>(Any1) == std::any_cast<const std::string &>(Any2);
			}
			else
			{
				NOT_IMPLEMENTED("Comparing two values of an unkown type.");
			}
		}
	}

	return false;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Equals::GetDataType(void) const
{
	return TypeDefinition::DataType::Boolean;
}

std::unique_ptr<Inspection::TypeDefinition::Equals> Inspection::TypeDefinition::Equals::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Equals>{new Inspection::TypeDefinition::Equals{}};
	auto First = true;
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			if(First == true)
			{
				Result->_Expression1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->_Expression2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Expression::Expression(Inspection::TypeDefinition::ExpressionType ExpressionType) :
	_ExpressionType{ExpressionType}
{
}

Inspection::TypeDefinition::ExpressionType Inspection::TypeDefinition::Expression::GetExpressionType(void) const
{
	return _ExpressionType;
}

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

Inspection::TypeDefinition::FieldReference::FieldReference(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::FieldReference}
{
}

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
	return _Root;
}

std::unique_ptr<Inspection::TypeDefinition::FieldReference> Inspection::TypeDefinition::FieldReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::FieldReference>{new Inspection::TypeDefinition::FieldReference{}};
	
	ASSERTION(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "current")
	{
		Result->_Root = Inspection::TypeDefinition::FieldReference::Root::Current;
	}
	else if(Element->GetAttribute("root") == "type")
	{
		Result->_Root = Inspection::TypeDefinition::FieldReference::Root::Type;
	}
	else
	{
		ASSERTION(false);
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

std::unique_ptr<Inspection::TypeDefinition::Interpretation> Inspection::TypeDefinition::Interpretation::Load(const XML::Element * Element)
{
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
				ASSERTION(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Length::Length(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::Length}
{
}

std::any Inspection::TypeDefinition::Length::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	auto BytesAny = _Bytes->GetAny(ExecutionContext);
	
	ASSERTION(BytesAny.type() == typeid(std::uint64_t));
	
	auto BitsAny = _Bits->GetAny(ExecutionContext);
	
	ASSERTION(BitsAny.type() == typeid(std::uint64_t));
	
	return Inspection::Length{std::any_cast<std::uint64_t>(BytesAny), std::any_cast<std::uint64_t>(BitsAny)};
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Length::GetDataType(void) const
{
	return Inspection::TypeDefinition::DataType::Length;
}

std::unique_ptr<Inspection::TypeDefinition::Length> Inspection::TypeDefinition::Length::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Length>{new Inspection::TypeDefinition::Length{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			ASSERTION(ChildElement != nullptr);
			if(ChildElement->GetName() == "bytes")
			{
				Result->_Bytes = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
			}
			else if(ChildElement->GetName() == "bits")
			{
				Result->_Bits = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
			}
			else
			{
				ASSERTION(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::LengthReference::LengthReference(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::LengthReference}
{
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
	return _Root;
}

Inspection::TypeDefinition::LengthReference::Name Inspection::TypeDefinition::LengthReference::GetName(void) const
{
	return _Name;
}

std::unique_ptr<Inspection::TypeDefinition::LengthReference> Inspection::TypeDefinition::LengthReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::LengthReference>{new Inspection::TypeDefinition::LengthReference{}};
	
	ASSERTION(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "type")
	{
		Result->_Root = Inspection::TypeDefinition::LengthReference::Root::Type;
	}
	else
	{
		ASSERTION(false);
	}
	ASSERTION(Element->HasAttribute("name") == true);
	if(Element->GetAttribute("name") == "consumed")
	{
		Result->_Name = Inspection::TypeDefinition::LengthReference::Name::Consumed;
	}
	else
	{
		ASSERTION(false);
	}
	
	return Result;
}

Inspection::TypeDefinition::ParameterReference::ParameterReference(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::ParameterReference}
{
}

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

Inspection::TypeDefinition::Parameters::Parameters(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::Parameters}
{
}

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
	
	for(auto & Parameter : _Parameters)
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
			Result->_Parameters.push_back(Inspection::TypeDefinition::Parameters::Parameter::Load(ChildElement));
		}
	}
	
	return Result;
}

std::any Inspection::TypeDefinition::Parameters::Parameter::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(_Expression != nullptr);
	
	return _Expression->GetAny(ExecutionContext);
}

const std::string & Inspection::TypeDefinition::Parameters::Parameter::GetName(void) const
{
	return _Name;
}

std::unique_ptr<Inspection::TypeDefinition::Parameters::Parameter> Inspection::TypeDefinition::Parameters::Parameter::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Parameters::Parameter>{new Inspection::TypeDefinition::Parameters::Parameter{}};
	
	ASSERTION(Element->HasAttribute("name") == true);
	Result->_Name = Element->GetAttribute("name");
	Result->_Expression = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
	
	return Result;
}

Inspection::TypeDefinition::Subtract::Subtract(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::ExpressionType::Subtract}
{
}

std::any Inspection::TypeDefinition::Subtract::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(_Minuend != nullptr);
	
	auto MinuendAny = _Minuend->GetAny(ExecutionContext);
	
	ASSERTION(_Subtrahend != nullptr);
	
	auto SubtrahendAny = _Subtrahend->GetAny(ExecutionContext);
	
	if(MinuendAny.type() == SubtrahendAny.type())
	{
		if(MinuendAny.type() == typeid(Inspection::Length))
		{
			return std::any_cast<const Inspection::Length &>(MinuendAny) - std::any_cast<const Inspection::Length &>(SubtrahendAny);
		}
		else
		{
			ASSERTION(false);
		}
	}
	
	return nullptr;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Subtract::GetDataType(void) const
{
	NOT_IMPLEMENTED("Called GetDataType() on a Subtract expression.");
}

std::unique_ptr<Inspection::TypeDefinition::Subtract> Inspection::TypeDefinition::Subtract::Load(const XML::Element * Element)
{
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
				Result->_Minuend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->_Subtrahend = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

std::any Inspection::TypeDefinition::Tag::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	ASSERTION(_Expression != nullptr);
	
	return _Expression->GetAny(ExecutionContext);
}

const std::string & Inspection::TypeDefinition::Tag::GetName(void) const
{
	return _Name;
}

bool Inspection::TypeDefinition::Tag::HasExpression(void) const
{
	return _Expression != nullptr;
}

std::unique_ptr<Inspection::TypeDefinition::Tag> Inspection::TypeDefinition::Tag::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Tag>{new Inspection::TypeDefinition::Tag{}};
	
	ASSERTION(Element->HasAttribute("name") == true);
	Result->_Name = Element->GetAttribute("name");
	if(XML::HasChildElements(Element) == true)
	{
		Result->_Expression = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
	}
	
	return Result;
}

Inspection::TypeDefinition::TypeReference::TypeReference(void) :
	Inspection::TypeDefinition::Expression{Inspection::TypeDefinition::ExpressionType::TypeReference}
{
}

std::any Inspection::TypeDefinition::TypeReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	return Inspection::g_TypeRepository.GetType(_Parts);
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::TypeReference::GetDataType(void) const
{
	return Inspection::TypeDefinition::DataType::Type;
}

const Inspection::TypeDefinition::Type * Inspection::TypeDefinition::TypeReference::GetType(void) const
{
	return Inspection::g_TypeRepository.GetType(_Parts);
}

std::unique_ptr<Inspection::TypeDefinition::TypeReference> Inspection::TypeDefinition::TypeReference::Load(const XML::Element * TypeReferenceElement)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::TypeReference>{new Inspection::TypeDefinition::TypeReference{}};
	
	for(auto TypeReferenceChildNode : TypeReferenceElement->GetChilds())
	{
		if(TypeReferenceChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto TypeReferenceChildElement = dynamic_cast<const XML::Element *>(TypeReferenceChildNode);
			
			ASSERTION(TypeReferenceChildElement != nullptr);
			ASSERTION(TypeReferenceChildElement->GetName() == "part");
			ASSERTION(TypeReferenceChildElement->GetChilds().size() == 1);
			
			auto TypeReferencePartText = dynamic_cast<const XML::Text *>(TypeReferenceChildElement->GetChild(0));
			
			ASSERTION(TypeReferencePartText != nullptr);
			Result->_Parts.push_back(TypeReferencePartText->GetText());
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Value::Value(void) :
	Inspection::TypeDefinition::Value{Inspection::TypeDefinition::DataType::Unknown}
{
}

Inspection::TypeDefinition::Value::Value(Inspection::TypeDefinition::DataType DataType) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::ExpressionType::Value},
	_DataType{DataType}
{
}

std::any Inspection::TypeDefinition::Value::GetAny(Inspection::ExecutionContext & ExecutionContext) const
{
	switch(_DataType)
	{
	case Inspection::TypeDefinition::DataType::Boolean:
		{
			ASSERTION(std::holds_alternative<bool>(_Data) == true);
			
			return std::get<bool>(_Data);
		}
	case Inspection::TypeDefinition::DataType::GUID:
		{
			ASSERTION(std::holds_alternative<Inspection::GUID>(_Data) == true);
			
			return std::get<Inspection::GUID>(_Data);
		}
	case Inspection::TypeDefinition::DataType::Nothing:
		{
			return nullptr;
		}
	case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
		{
			ASSERTION(std::holds_alternative<float>(_Data) == true);
			
			return std::get<float>(_Data);
		}
	case Inspection::TypeDefinition::DataType::String:
		{
			ASSERTION(std::holds_alternative<std::string>(_Data) == true);
			
			return std::get<std::string>(_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
		{
			ASSERTION(std::holds_alternative<std::uint8_t>(_Data) == true);
			
			return std::get<std::uint8_t>(_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
		{
			ASSERTION(std::holds_alternative<std::uint16_t>(_Data) == true);
			
			return std::get<std::uint16_t>(_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
		{
			ASSERTION(std::holds_alternative<std::uint32_t>(_Data) == true);
			
			return std::get<std::uint32_t>(_Data);
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
		{
			ASSERTION(std::holds_alternative<std::uint64_t>(_Data) == true);
			
			return std::get<std::uint64_t>(_Data);
		}
	default:
		{
			ASSERTION(false);
		}
	}
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Value::GetDataType(void) const
{
	return _DataType;
}

std::unique_ptr<Inspection::TypeDefinition::Value> Inspection::TypeDefinition::Value::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{}};
	
	ASSERTION(Element != nullptr);
	if(Element->GetName() == "nothing")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::Nothing;
		ASSERTION(Element->GetChilds().size() == 0);
	}
	else if(Element->GetName() == "boolean")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::Boolean;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = from_string_cast<bool>(TextNode->GetText());
	}
	else if(Element->GetName() == "guid")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::GUID;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data.emplace<Inspection::GUID>(TextNode->GetText());
	}
	else if(Element->GetName() == "single-precision-real")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::SinglePrecisionReal;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = from_string_cast<float>(TextNode->GetText());
	}
	else if(Element->GetName() == "string")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::String;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = std::string{TextNode->GetText()};
	}
	else if(Element->GetName() == "unsigned-integer-8bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = from_string_cast<std::uint8_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-16bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = from_string_cast<std::uint16_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-32bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = from_string_cast<std::uint32_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-64bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
		ASSERTION((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		ASSERTION(TextNode != nullptr);
		Result->_Data = from_string_cast<std::uint64_t>(TextNode->GetText());
	}
	else
	{
		ASSERTION(false);
	}
	ASSERTION(Result->_DataType != Inspection::TypeDefinition::DataType::Unknown);
	
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
		ASSERTION(false);
	}
}
