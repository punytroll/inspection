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

#include <cassert>

#include "string_cast.h"
#include "type_definition.h"
#include "xml_helper.h"
#include "xml_puny_dom.h"

Inspection::TypeDefinition::Add::Add(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::Expression::Type::Add}
{
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
			
			assert(ChildElement != nullptr);
			if(First == true)
			{
				Result->Summand1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->Summand2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Cast::Cast(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::Expression::Type::Cast},
	DataType{Inspection::TypeDefinition::DataType::Unknown}
{
}

std::unique_ptr<Inspection::TypeDefinition::Cast> Inspection::TypeDefinition::Cast::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Cast>{new Inspection::TypeDefinition::Cast{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			Result->DataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetName());
			Result->Expression = Inspection::TypeDefinition::Expression::Load(ChildElement);
			
			break;
		}
	}
	assert(Result->DataType != Inspection::TypeDefinition::DataType::Unknown);
	assert(Result->Expression != nullptr);
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::DataReference> Inspection::TypeDefinition::DataReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::DataReference>{new Inspection::TypeDefinition::DataReference{}};
	
	assert(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "current")
	{
		Result->Root = Inspection::TypeDefinition::DataReference::Root::Current;
	}
	else if(Element->GetAttribute("root") == "type")
	{
		Result->Root = Inspection::TypeDefinition::DataReference::Root::Type;
	}
	else
	{
		assert(false);
	}
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			auto & DataReferencePart = Result->Parts.emplace_back();
			
			if(ChildElement->GetName() == "field")
			{
				assert(ChildElement->GetChilds().size() == 1);
				DataReferencePart.Type = Inspection::TypeDefinition::DataReference::Part::Type::Field;
				
				auto SubText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
				
				assert(SubText != nullptr);
				DataReferencePart.DetailName = SubText->GetText();
			}
			else if(ChildElement->GetName() == "tag")
			{
				assert(ChildElement->GetChilds().size() == 1);
				DataReferencePart.Type = Inspection::TypeDefinition::DataReference::Part::Type::Tag;
				
				auto TagText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
				
				assert(TagText != nullptr);
				DataReferencePart.DetailName = TagText->GetText();
			}
			else
			{
				assert(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Divide::Divide(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::Expression::Type::Divide}
{
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
				Result->Dividend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->Divisor = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Enumeration> Inspection::TypeDefinition::Enumeration::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Enumeration>{new Inspection::TypeDefinition::Enumeration{}};
	
	assert(Element != nullptr);
	assert(Element->GetName() == "enumeration");
	Result->BaseDataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetAttribute("base-data-type"));
	for(auto EnumerationChildNode : Element->GetChilds())
	{
		if(EnumerationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto EnumerationChildElement = dynamic_cast<const XML::Element *>(EnumerationChildNode);
			
			if(EnumerationChildElement->GetName() == "element")
			{
				auto & Element = Result->Elements.emplace_back();
				
				assert(EnumerationChildElement->HasAttribute("base-value") == true);
				Element.BaseValue = EnumerationChildElement->GetAttribute("base-value");
				assert(EnumerationChildElement->HasAttribute("valid") == true);
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
							assert(false);
						}
					}
				}
			}
			else if(EnumerationChildElement->GetName() == "fallback-element")
			{
				assert(Result->FallbackElement.has_value() == false);
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
							assert(false);
						}
					}
				}
			}
			else
			{
				assert(false);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Equals::Equals(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::Expression::Type::Equals}
{
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
				Result->Expression1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->Expression2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Expression::Expression(Inspection::TypeDefinition::Expression::Type ExpressionType) :
	_ExpressionType{ExpressionType}
{
}

Inspection::TypeDefinition::Expression::Type Inspection::TypeDefinition::Expression::GetExpressionType(void) const
{
	return _ExpressionType;
}

std::unique_ptr<Inspection::TypeDefinition::Expression> Inspection::TypeDefinition::Expression::Load(const XML::Element * Element)
{
	assert(Element != nullptr);
	
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Expression>{};
	
	if(Element->GetName() == "add")
	{
		Result = Inspection::TypeDefinition::Add::Load(Element);
	}
	else if(Element->GetName() == "divide")
	{
		Result = Inspection::TypeDefinition::Divide::Load(Element);
	}
	else if(Element->GetName() == "equals")
	{
		Result = Inspection::TypeDefinition::Equals::Load(Element);
	}
	else if(Element->GetName() == "subtract")
	{
		Result = Inspection::TypeDefinition::Subtract::Load(Element);
	}
	else if((Element->GetName() == "length") && (XML::HasOneChildElement(Element) == true))
	{
		Result = Inspection::TypeDefinition::Cast::Load(Element);
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
			assert(false);
		}
	}
	
	return Inspection::TypeDefinition::Expression::Load(ExpressionElement);
}

std::unique_ptr<Inspection::TypeDefinition::FieldReference> Inspection::TypeDefinition::FieldReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::FieldReference>{new Inspection::TypeDefinition::FieldReference{}};
	
	assert(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "current")
	{
		Result->Root = Inspection::TypeDefinition::FieldReference::Root::Current;
	}
	else if(Element->GetAttribute("root") == "type")
	{
		Result->Root = Inspection::TypeDefinition::FieldReference::Root::Type;
	}
	else
	{
		assert(false);
	}
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			assert(ChildElement != nullptr);
			assert(ChildElement->GetName() == "field");
			assert(ChildElement->GetChilds().size() == 1);
			
			auto FieldReferenceFieldText = dynamic_cast<const XML::Text *>(ChildElement->GetChild(0));
			
			assert(FieldReferenceFieldText != nullptr);
			Result->Parts.push_back(FieldReferenceFieldText->GetText());
		}
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Interpretation> Inspection::TypeDefinition::Interpretation::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Interpretation>{new Inspection::TypeDefinition::Interpretation{}};
	
	assert(Element->GetName() == "interpretation");
	for(auto InterpretationChildNode : Element->GetChilds())
	{
		if(InterpretationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto InterpretationChildElement = dynamic_cast<const XML::Element *>(InterpretationChildNode);
			
			if(InterpretationChildElement->GetName() == "apply-enumeration")
			{
				Result->Type = Inspection::TypeDefinition::Interpretation::Type::ApplyEnumeration;
				Result->ApplyEnumeration.emplace();
				for(auto InterpretationApplyEnumerationChildNode : InterpretationChildElement->GetChilds())
				{
					if(InterpretationApplyEnumerationChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto InterpretationApplyEnumerationChildElement = dynamic_cast<const XML::Element *>(InterpretationApplyEnumerationChildNode);
						
						if(InterpretationApplyEnumerationChildElement->GetName() == "enumeration")
						{
							Result->ApplyEnumeration->Enumeration = Inspection::TypeDefinition::Enumeration::Load(InterpretationApplyEnumerationChildElement);
						}
						else
						{
							assert(false);
						}
					}
				}
			}
			else
			{
				assert(false);
			}
		}
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Length> Inspection::TypeDefinition::Length::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Length>{new Inspection::TypeDefinition::Length{}};
	
	for(auto ChildNode : Element->GetChilds())
	{
		if(ChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto ChildElement = dynamic_cast<const XML::Element *>(ChildNode);
			
			assert(ChildElement != nullptr);
			if(ChildElement->GetName() == "bytes")
			{
				Result->Bytes = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
			}
			else if(ChildElement->GetName() == "bits")
			{
				Result->Bits = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
			}
			else
			{
				assert(false);
			}
		}
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::LengthReference> Inspection::TypeDefinition::LengthReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::LengthReference>{new Inspection::TypeDefinition::LengthReference{}};
	
	assert(Element->HasAttribute("root") == true);
	if(Element->GetAttribute("root") == "type")
	{
		Result->Root = Inspection::TypeDefinition::LengthReference::Root::Type;
	}
	else
	{
		assert(false);
	}
	assert(Element->HasAttribute("name") == true);
	if(Element->GetAttribute("name") == "consumed")
	{
		Result->Name = Inspection::TypeDefinition::LengthReference::Name::Consumed;
	}
	else
	{
		assert(false);
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Parameter> Inspection::TypeDefinition::Parameter::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Parameter>{new Inspection::TypeDefinition::Parameter{}};
	
	assert(Element->HasAttribute("name") == true);
	Result->Name = Element->GetAttribute("name");
	Result->Expression = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::ParameterReference> Inspection::TypeDefinition::ParameterReference::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::ParameterReference>{new Inspection::TypeDefinition::ParameterReference{}};
	
	assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
	
	auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
	
	assert(TextNode != nullptr);
	Result->Name = TextNode->GetText();
	
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
			
			assert(ChildElement != nullptr);
			assert(ChildElement->GetName() == "parameter");
			Result->_Parameters.push_back(Inspection::TypeDefinition::Parameter::Load(ChildElement));
		}
	}
	
	return Result;
}

const std::vector<std::unique_ptr<Inspection::TypeDefinition::Parameter>> & Inspection::TypeDefinition::Parameters::GetParameters(void) const
{
	return _Parameters;
}

Inspection::TypeDefinition::Subtract::Subtract(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::Expression::Type::Subtract}
{
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
			
			assert(ChildElement != nullptr);
			if(First == true)
			{
				Result->Minuend = Inspection::TypeDefinition::Expression::Load(ChildElement);
				First = false;
			}
			else
			{
				Result->Subtrahend = Inspection::TypeDefinition::Expression::Load(ChildElement);
			}
		}
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Tag> Inspection::TypeDefinition::Tag::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Tag>{new Inspection::TypeDefinition::Tag{}};
	
	assert(Element->HasAttribute("name") == true);
	Result->Name = Element->GetAttribute("name");
	if(XML::HasChildElements(Element) == true)
	{
		Result->Expression = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
	}
	
	return Result;
}

std::unique_ptr<Inspection::TypeDefinition::TypeReference> Inspection::TypeDefinition::TypeReference::Load(const XML::Element * TypeReferenceElement)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::TypeReference>{new Inspection::TypeDefinition::TypeReference{}};
	
	for(auto TypeReferenceChildNode : TypeReferenceElement->GetChilds())
	{
		if(TypeReferenceChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto TypeReferenceChildElement = dynamic_cast<const XML::Element *>(TypeReferenceChildNode);
			
			assert(TypeReferenceChildElement != nullptr);
			assert(TypeReferenceChildElement->GetName() == "part");
			assert(TypeReferenceChildElement->GetChilds().size() == 1);
			
			auto TypeReferencePartText = dynamic_cast<const XML::Text *>(TypeReferenceChildElement->GetChild(0));
			
			assert(TypeReferencePartText != nullptr);
			Result->Parts.push_back(TypeReferencePartText->GetText());
		}
	}
	
	return Result;
}

Inspection::TypeDefinition::Value::Value(void) :
	Inspection::TypeDefinition::Expression::Expression{Inspection::TypeDefinition::Expression::Type::Value},
	_DataType{Inspection::TypeDefinition::DataType::Unknown}
{
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::Value::GetDataType(void) const
{
	return _DataType;
}

std::unique_ptr<Inspection::TypeDefinition::Value> Inspection::TypeDefinition::Value::Load(const XML::Element * Element)
{
	auto Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{}};
	
	assert(Element != nullptr);
	if(Element->GetName() == "nothing")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::Nothing;
		assert(Element->GetChilds().size() == 0);
	}
	else if(Element->GetName() == "boolean")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::Boolean;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = from_string_cast<bool>(TextNode->GetText());
	}
	else if(Element->GetName() == "data-reference")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::DataReference;
		Result->Data = Inspection::TypeDefinition::DataReference::Load(Element);
	}
	else if(Element->GetName() == "guid")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::GUID;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data.emplace<Inspection::GUID>(TextNode->GetText());
	}
	else if(Element->GetName() == "length")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::Length;
		Result->Data = Inspection::TypeDefinition::Length::Load(Element);
	}
	else if(Element->GetName() == "length-reference")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::LengthReference;
		Result->Data = Inspection::TypeDefinition::LengthReference::Load(Element);
	}
	else if(Element->GetName() == "parameter-reference")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::ParameterReference;
		Result->Data = Inspection::TypeDefinition::ParameterReference::Load(Element);
	}
	else if(Element->GetName() == "parameters")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::Parameters;
		Result->Data = Inspection::TypeDefinition::Parameters::Load(Element);
	}
	else if(Element->GetName() == "single-precision-real")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::SinglePrecisionReal;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = from_string_cast<float>(TextNode->GetText());
	}
	else if(Element->GetName() == "string")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::String;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = std::string{TextNode->GetText()};
	}
	else if(Element->GetName() == "type-reference")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::TypeReference;
		Result->Data = Inspection::TypeDefinition::TypeReference::Load(Element);
	}
	else if(Element->GetName() == "unsigned-integer-8bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = from_string_cast<std::uint8_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-16bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = from_string_cast<std::uint16_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-32bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = from_string_cast<std::uint32_t>(TextNode->GetText());
	}
	else if(Element->GetName() == "unsigned-integer-64bit")
	{
		Result->_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
		assert((Element->GetChilds().size() == 1) && (Element->GetChild(0)->GetNodeType() == XML::NodeType::Text));
		
		auto TextNode = dynamic_cast<const XML::Text *>(Element->GetChild(0));
		
		assert(TextNode != nullptr);
		Result->Data = from_string_cast<std::uint64_t>(TextNode->GetText());
	}
	else
	{
		assert(false);
	}
	assert(Result->_DataType != Inspection::TypeDefinition::DataType::Unknown);
	
	return Result;
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::GetDataTypeFromString(const std::string & String)
{
	if(String == "boolean")
	{
		return Inspection::TypeDefinition::DataType::Boolean;
	}
	else if(String == "data-reference")
	{
		return Inspection::TypeDefinition::DataType::DataReference;
	}
	else if(String == "length")
	{
		return Inspection::TypeDefinition::DataType::Length;
	}
	else if(String == "nothing")
	{
		return Inspection::TypeDefinition::DataType::Nothing;
	}
	else if(String == "parameter-reference")
	{
		return Inspection::TypeDefinition::DataType::ParameterReference;
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
	else if(String == "type-reference")
	{
		return Inspection::TypeDefinition::DataType::TypeReference;
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
		assert(false);
	}
}
