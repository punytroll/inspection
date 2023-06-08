/**
 * Copyright (C) 2023  Hagen M�bius
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

#include <assertion.h>

#include "../internal_output_operators.h"
#include "add.h"

auto Inspection::TypeDefinition::Add::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Summand1 != nullptr);
    ASSERTION(m_Summand2 != nullptr);
    
    auto Summand1Any = m_Summand1->GetAny(ExecutionContext);
    auto Summand2Any = m_Summand2->GetAny(ExecutionContext);
    
    ASSERTION(Summand1Any.has_value() == true);
    ASSERTION(Summand2Any.has_value() == true);
    INVALID_INPUT_IF(Summand1Any.type() != Summand2Any.type(), "Summand one and two must have the same type: " + Inspection::to_string(Summand1Any.type()) + " != " + Inspection::to_string(Summand2Any.type()));
    if(Summand1Any.type() == typeid(std::uint8_t))
    {
        return static_cast<std::uint8_t>(std::any_cast<std::uint8_t>(Summand1Any) + std::any_cast<std::uint8_t>(Summand2Any));
    }
    else
    {
        UNEXPECTED_CASE("Summand types: " + Inspection::to_string(Summand1Any.type()));
    }
    
    return nullptr;
}

auto Inspection::TypeDefinition::Add::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on an Add expression.");
}

auto Inspection::TypeDefinition::Add::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Add>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Add>{new Inspection::TypeDefinition::Add{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        if(Result->m_Summand1 == nullptr)
        {
            Result->m_Summand1 = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
        else
        {
            INVALID_INPUT_IF(Result->m_Summand2 != nullptr, "More than two operands for Add expression.");
            Result->m_Summand2 = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
    }
    INVALID_INPUT_IF(Result->m_Summand1 == nullptr, "Missing operands for Add expression.");
    INVALID_INPUT_IF(Result->m_Summand2 == nullptr, "Missing second operand for Add expression.");
    
    return Result;
}
