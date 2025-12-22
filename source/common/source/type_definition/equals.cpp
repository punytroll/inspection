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
#include <common/guid.h>

#include "../internal_output_operators.h"
#include "data_type.h"
#include "equals.h"

auto Inspection::TypeDefinition::Equals::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Expression1 != nullptr);
    ASSERTION(m_Expression2 != nullptr);
    
    auto Expression1Any = m_Expression1->GetAny(ExecutionContext);
    auto Expression2Any = m_Expression2->GetAny(ExecutionContext);
    
    ASSERTION(Expression1Any.has_value() == true);
    ASSERTION(Expression2Any.has_value() == true);
    // if we compare two values of different types, they are definitively not equal, as we have no implicit conversion
    if(Expression1Any.type() == Expression2Any.type())
    {
        if(Expression1Any.type() == typeid(bool))
        {
            return std::any_cast<bool const &>(Expression1Any) == std::any_cast<bool const &>(Expression2Any);
        }
        else if(Expression1Any.type() == typeid(std::uint8_t))
        {
            return std::any_cast<std::uint8_t const &>(Expression1Any) == std::any_cast<std::uint8_t const &>(Expression2Any);
        }
        else if(Expression1Any.type() == typeid(std::uint16_t))
        {
            return std::any_cast<std::uint16_t const &>(Expression1Any) == std::any_cast<std::uint16_t const &>(Expression2Any);
        }
        else if(Expression1Any.type() == typeid(std::uint32_t))
        {
            return std::any_cast<std::uint32_t const &>(Expression1Any) == std::any_cast<std::uint32_t const &>(Expression2Any);
        }
        else if(Expression1Any.type() == typeid(Inspection::GUID))
        {
            return std::any_cast<Inspection::GUID const &>(Expression1Any) == std::any_cast<Inspection::GUID const &>(Expression2Any);
        }
        else if(Expression1Any.type() == typeid(std::string))
        {
            return std::any_cast<std::string const &>(Expression1Any) == std::any_cast<std::string const &>(Expression2Any);
        }
        else
        {
            UNEXPECTED_CASE("Expression1Any.type() == " + Inspection::to_string(Expression1Any.type()));
        }
    }
    
    return false;
}

auto Inspection::TypeDefinition::Equals::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return TypeDefinition::DataType::Boolean;
}

auto Inspection::TypeDefinition::Equals::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Equals>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Equals>{new Inspection::TypeDefinition::Equals{}};
    auto First = true;
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
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
    INVALID_INPUT_IF(Result->m_Expression1 == nullptr, "Missing operands for Equals expression.");
    INVALID_INPUT_IF(Result->m_Expression2 == nullptr, "Missing second operand for Equals expression.");
    
    return Result;
}
