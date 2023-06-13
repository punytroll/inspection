/**
 * Copyright (C) 2023  Hagen Möbius
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

#include <any>

#include <string_cast/string_cast.h>

#include <xml_puny_dom/xml_puny_dom.h>

#include <assertion.h>

#include "../internal_output_operators.h"
#include "data_type.h"
#include "value.h"

auto Inspection::TypeDefinition::Value::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
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

auto Inspection::TypeDefinition::Value::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return m_DataType;
}

auto Inspection::TypeDefinition::Value::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Value>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Value>{new Inspection::TypeDefinition::Value{}};
    
    ASSERTION(Element != nullptr);
    if(Element->GetName() == "nothing")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::Nothing;
        ASSERTION(Element->GetChildNodes().size() == 0);
    }
    else if(Element->GetName() == "boolean")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::Boolean;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data = from_string_cast<bool>(TextNode->GetText());
    }
    else if(Element->GetName() == "guid")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::GUID;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data.emplace<Inspection::GUID>(TextNode->GetText());
    }
    else if(Element->GetName() == "single-precision-real")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::SinglePrecisionReal;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data = from_string_cast<float>(TextNode->GetText());
    }
    else if(Element->GetName() == "string")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::String;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data = std::string{TextNode->GetText()};
    }
    else if(Element->GetName() == "unsigned-integer-8bit")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data = from_string_cast<std::uint8_t>(TextNode->GetText());
    }
    else if(Element->GetName() == "unsigned-integer-16bit")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data = from_string_cast<std::uint16_t>(TextNode->GetText());
    }
    else if(Element->GetName() == "unsigned-integer-32bit")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        ASSERTION(TextNode != nullptr);
        Result->m_Data = from_string_cast<std::uint32_t>(TextNode->GetText());
    }
    else if(Element->GetName() == "unsigned-integer-64bit")
    {
        Result->m_DataType = Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Data = from_string_cast<std::uint64_t>(TextNode->GetText());
    }
    else
    {
        INVALID_INPUT("Element name: " + Element->GetName());
    }
    
    return Result;
}
