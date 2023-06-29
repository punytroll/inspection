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
#include <type_repository.h>

#include "../execution_context.h"
#include "../internal_output_operators.h"
#include "data_type.h"
#include "type.h"
#include "type_reference.h"

auto Inspection::TypeDefinition::TypeReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    return ExecutionContext.GetTypeRepository().GetType(m_Parts);
}

auto Inspection::TypeDefinition::TypeReference::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return Inspection::TypeDefinition::DataType::Type;
}

auto Inspection::TypeDefinition::TypeReference::GetType(Inspection::ExecutionContext & ExecutionContext) const -> Inspection::TypeDefinition::Type const &
{
    return dynamic_cast<Inspection::TypeDefinition::Type const &>(*(ExecutionContext.GetTypeRepository().GetType(m_Parts)));
}

auto Inspection::TypeDefinition::TypeReference::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::TypeReference>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::TypeReference>{new Inspection::TypeDefinition::TypeReference{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        INVALID_INPUT_IF(ChildElement->GetName() != "part", "\"part\" was expected but \"" + ChildElement->GetName() + "\" was found.");
        ASSERTION(ChildElement->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(ChildElement->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Parts.push_back(TextNode->GetText());
    }
    
    return Result;
}
