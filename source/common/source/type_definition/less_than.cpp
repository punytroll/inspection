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
#include "data_type.h"
#include "less_than.h"

auto Inspection::TypeDefinition::LessThan::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_LeftHandSide != nullptr);
    ASSERTION(m_RightHandSide != nullptr);
    
    auto LeftHandSideAny = m_LeftHandSide->GetAny(ExecutionContext);
    auto RightHandSideAny = m_RightHandSide->GetAny(ExecutionContext);
    
    ASSERTION(LeftHandSideAny.has_value() == true);
    ASSERTION(RightHandSideAny.has_value() == true);
    INVALID_INPUT_IF(LeftHandSideAny.type() != RightHandSideAny.type(), "Left hand side and right hand side must have the same type for less-than operator: " + Inspection::to_string(LeftHandSideAny.type()) + " != " + Inspection::to_string(RightHandSideAny.type()));
    if(LeftHandSideAny.type() == typeid(std::uint8_t))
    {
        return std::any_cast<std::uint8_t const &>(LeftHandSideAny) < std::any_cast<std::uint8_t const &>(RightHandSideAny);
    }
    else if(LeftHandSideAny.type() == typeid(std::uint16_t))
    {
        return std::any_cast<std::uint16_t const &>(LeftHandSideAny) < std::any_cast<std::uint16_t const &>(RightHandSideAny);
    }
    else if(LeftHandSideAny.type() == typeid(std::uint32_t))
    {
        return std::any_cast<std::uint32_t const &>(LeftHandSideAny) < std::any_cast<std::uint32_t const &>(RightHandSideAny);
    }
    else
    {
        UNEXPECTED_CASE("Less-than operand types: " + Inspection::to_string(LeftHandSideAny.type()));
    }
    
    return false;
}

auto Inspection::TypeDefinition::LessThan::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return TypeDefinition::DataType::Boolean;
}

auto Inspection::TypeDefinition::LessThan::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::LessThan>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::LessThan>{new Inspection::TypeDefinition::LessThan{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(Result->m_LeftHandSide == nullptr)
        {
            Result->m_LeftHandSide = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
        else
        {
            INVALID_INPUT_IF(Result->m_RightHandSide != nullptr, "More than two operands for less-than expression.");
            Result->m_RightHandSide = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
    }
    INVALID_INPUT_IF(Result->m_LeftHandSide == nullptr, "Missing operands for less-than expression.");
    INVALID_INPUT_IF(Result->m_RightHandSide == nullptr, "Missing right hand side for less-than expression.");
    
    return Result;
}
