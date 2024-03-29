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
#include "modulus.h"

auto Inspection::TypeDefinition::Modulus::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Dividend != nullptr);
    ASSERTION(m_Divisor != nullptr);
    
    auto DividendAny = m_Dividend->GetAny(ExecutionContext);
    auto DivisorAny = m_Divisor->GetAny(ExecutionContext);
    
    ASSERTION(DividendAny.has_value() == true);
    ASSERTION(DivisorAny.has_value() == true);
    INVALID_INPUT_IF(DividendAny.type() != DivisorAny.type(), "Dividend and divisor must have the same type: " + Inspection::to_string(DividendAny.type()) + " != " + Inspection::to_string(DivisorAny.type()));
    if(DividendAny.type() == typeid(std::uint16_t))
    {
        return static_cast<std::uint16_t>(std::any_cast<std::uint16_t const &>(DividendAny) % std::any_cast<std::uint16_t const &>(DivisorAny));
    }
    else
    {
        UNEXPECTED_CASE("Dividend and divisor types: " + Inspection::to_string(DividendAny.type()));
    }
    
    return nullptr;
}

auto Inspection::TypeDefinition::Modulus::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a Modulus expression.");
}

auto Inspection::TypeDefinition::Modulus::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Modulus>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Modulus>{new Inspection::TypeDefinition::Modulus{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(Result->m_Dividend == nullptr)
        {
            Result->m_Dividend = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
        else
        {
            INVALID_INPUT_IF(Result->m_Divisor != nullptr, "More than two operands for Modulus expression.");
            Result->m_Divisor = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
    }
    INVALID_INPUT_IF(Result->m_Dividend == nullptr, "Missing operands for Modulus expression.");
    INVALID_INPUT_IF(Result->m_Divisor == nullptr, "Missing second operand for Modulus expression.");
    
    return Result;
}
