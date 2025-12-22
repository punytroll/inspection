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

#include <xml_puny_dom/xml_puny_dom.h>

#include <common/assertion.h>

#include "../internal_output_operators.h"
#include "cast.h"
#include "data_type.h"
#include "helper.h"

template<typename Type>
static auto CastTo(std::any const & Any) -> Type
{
    if(Any.type() == typeid(float))
    {
        return static_cast<Type>(std::any_cast<float>(Any));
    }
    else if(Any.type() == typeid(std::int32_t))
    {
        return static_cast<Type>(std::any_cast<std::int32_t>(Any));
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

Inspection::TypeDefinition::Cast::Cast(Inspection::TypeDefinition::DataType DataType) :
    m_DataType{DataType}
{
}

auto Inspection::TypeDefinition::Cast::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
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
            UNEXPECTED_CASE("m_DataType == " + Inspection::to_string(m_DataType));
        }
    }
}

auto Inspection::TypeDefinition::Cast::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return m_DataType;
}

auto Inspection::TypeDefinition::Cast::GetExpression() const -> Inspection::TypeDefinition::Expression const &
{
    ASSERTION(m_Expression != nullptr);
    
    return *m_Expression;
}

auto Inspection::TypeDefinition::Cast::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Cast>
{
    ASSERTION(Element != nullptr);
    
    auto ChildElementsRange = Element->GetChildElements();
    auto ChildElements = std::vector<XML::Element const *>{ChildElementsRange.begin(), ChildElementsRange.end()};
    
    INVALID_INPUT_IF(ChildElements.size() == 0, "Missing operand for Cast expression.");
    INVALID_INPUT_IF(ChildElements.size() > 1, "Too many operands for Cast expression.");
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Cast>{new Inspection::TypeDefinition::Cast{Inspection::TypeDefinition::GetDataTypeFromString(Element->GetName())}};
    
    Result->m_Expression = Inspection::TypeDefinition::Expression::Load(ChildElements.front());
    
    return Result;
}
