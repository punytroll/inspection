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
#include <common/execution_context.h>

#include "parameter_reference.h"

auto Inspection::TypeDefinition::ParameterReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    return ExecutionContext.GetParameterAny(m_Name);
}

auto Inspection::TypeDefinition::ParameterReference::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a ParameterReference expression.");
}

auto Inspection::TypeDefinition::ParameterReference::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::ParameterReference>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::ParameterReference>{new Inspection::TypeDefinition::ParameterReference{}};
    
    ASSERTION(Element->GetChildNodes().size() == 1);
    
    auto TextNode = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
    
    ASSERTION(TextNode != nullptr);
    Result->m_Name = TextNode->GetText();
    
    return Result;
}
