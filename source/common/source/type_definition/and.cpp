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
#include "and.h"
#include "data_type.h"

auto Inspection::TypeDefinition::And::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Operands.size() > 0);
    
    auto Result = true;
    
    for(auto const & Operand : m_Operands)
    {
        ASSERTION(Operand != nullptr);
        
        auto OperandAny = Operand->GetAny(ExecutionContext);
        
        ASSERTION(OperandAny.has_value() == true);
        INVALID_INPUT_IF(OperandAny.type() != typeid(bool), "All operands in an \"and\" have to evaluate to a boolean value (not \"" + Inspection::to_string(OperandAny.type()) + "\").");
        Result = Result && std::any_cast<bool>(OperandAny);
        if(Result == false)
        {
            break;
        }
    }
    
    return Result;
}

auto Inspection::TypeDefinition::And::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return Inspection::TypeDefinition::DataType::Boolean;
}

auto Inspection::TypeDefinition::And::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::And>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::And>{new Inspection::TypeDefinition::And{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        Result->m_Operands.push_back(Inspection::TypeDefinition::Expression::Load(ChildElement));
    }
    INVALID_INPUT_IF(Result->m_Operands.size() == 0, "Missing operands for And expression.");
    
    return Result;
}
