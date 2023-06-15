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
#include "data_type.h"
#include "parameters.h"

auto Inspection::TypeDefinition::Parameters::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    return GetParameters(ExecutionContext);
}

auto Inspection::TypeDefinition::Parameters::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return Inspection::TypeDefinition::DataType::Parameters;
}

auto Inspection::TypeDefinition::Parameters::GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>
{
    auto Result = std::unordered_map<std::string, std::any>{};
    
    for(auto const & Parameter : m_Parameters)
    {
        Result.emplace(Parameter.Name, Parameter.Expression->GetAny(ExecutionContext));
    }
    
    return Result;
}

auto Inspection::TypeDefinition::Parameters::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Parameters>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Parameters>{new Inspection::TypeDefinition::Parameters{}};
    
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        INVALID_INPUT_IF(ChildElement->GetName() != "parameter", "A \"parameters\" may only contain \"parameter\" childs.");
        INVALID_INPUT_IF(ChildElement->HasAttribute("name") == false, "A \"parameter\" needs a \"name\" attribute.");
        Result->m_Parameters.emplace_back(ChildElement->GetAttribute("name"), Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement));
    }
    
    return Result;
}
