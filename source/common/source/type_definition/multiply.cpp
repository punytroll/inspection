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

#include <assertion.h>

#include "../internal_output_operators.h"
#include "multiply.h"

auto Inspection::TypeDefinition::Multiply::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Multiplier != nullptr);
    ASSERTION(m_Multiplicand != nullptr);
    
    auto MultiplierAny = m_Multiplier->GetAny(ExecutionContext);
    auto MultiplicandAny = m_Multiplicand->GetAny(ExecutionContext);
    
    ASSERTION(MultiplierAny.has_value() == true);
    ASSERTION(MultiplicandAny.has_value() == true);
    INVALID_INPUT_IF(MultiplierAny.type() != MultiplicandAny.type(), "Multiplier and multiplicand have to have the same type: " + Inspection::to_string(MultiplierAny.type()) + " != " + Inspection::to_string(MultiplicandAny.type()));
    if(MultiplierAny.type() == typeid(std::uint64_t))
    {
        return static_cast<std::uint64_t>(std::any_cast<std::uint64_t>(MultiplierAny) * std::any_cast<std::uint64_t>(MultiplicandAny));
    }
    else
    {
        UNEXPECTED_CASE("multiplier and multiplicand types == " + Inspection::to_string(MultiplierAny.type()));
    }
    
    return nullptr;
}

auto Inspection::TypeDefinition::Multiply::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a Multiply expression.");
}

auto Inspection::TypeDefinition::Multiply::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Multiply>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Multiply>{new Inspection::TypeDefinition::Multiply{}};
    auto First = true;
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(First == true)
        {
            Result->m_Multiplier = Inspection::TypeDefinition::Expression::Load(ChildElement);
            First = false;
        }
        else
        {
            INVALID_INPUT_IF(Result->m_Multiplicand != nullptr, "More than two operands for Multiply expression.");
            Result->m_Multiplicand = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
    }
    INVALID_INPUT_IF(Result->m_Multiplier == nullptr, "Missing operands for Multiply expression.");
    INVALID_INPUT_IF(Result->m_Multiplicand == nullptr, "Missing second operand for Multiply expression.");
    
    return Result;
}
