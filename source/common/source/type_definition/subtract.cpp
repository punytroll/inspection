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
#include <common/length.h>

#include "../internal_output_operators.h"
#include "subtract.h"

auto Inspection::TypeDefinition::Subtract::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Minuend != nullptr);
    ASSERTION(m_Subtrahend != nullptr);
    
    auto MinuendAny = m_Minuend->GetAny(ExecutionContext);
    auto SubtrahendAny = m_Subtrahend->GetAny(ExecutionContext);
    
    ASSERTION(MinuendAny.has_value() == true);
    ASSERTION(SubtrahendAny.has_value() == true);
    INVALID_INPUT_IF(MinuendAny.type() != SubtrahendAny.type(), "Minuend and subtrahend must have the same type: " + Inspection::to_string(MinuendAny.type()) + " != " + Inspection::to_string(SubtrahendAny.type()));
    if(MinuendAny.type() == typeid(Inspection::Length))
    {
        return std::any_cast<Inspection::Length const &>(MinuendAny) - std::any_cast<Inspection::Length const &>(SubtrahendAny);
    }
    else if(MinuendAny.type() == typeid(std::uint64_t))
    {
        return std::any_cast<std::uint64_t const &>(MinuendAny) - std::any_cast<std::uint64_t const &>(SubtrahendAny);
    }
    else
    {
        UNEXPECTED_CASE("Minuend and subtrahend types: " + Inspection::to_string(MinuendAny.type()));
    }
    
    return nullptr;
}

auto Inspection::TypeDefinition::Subtract::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a subtract expression.");
}

auto Inspection::TypeDefinition::Subtract::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Subtract>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Subtract>{new Inspection::TypeDefinition::Subtract{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(Result->m_Minuend == nullptr)
        {
            Result->m_Minuend = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
        else
        {
            INVALID_INPUT_IF(Result->m_Subtrahend != nullptr, "More than two operands for subtract expression.");
            Result->m_Subtrahend = Inspection::TypeDefinition::Expression::Load(ChildElement);
        }
    }
    INVALID_INPUT_IF(Result->m_Minuend == nullptr, "Missing operands for subtract expression.");
    INVALID_INPUT_IF(Result->m_Subtrahend == nullptr, "Missing subtrahend for subtract expression.");
    
    return Result;
}
