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
#include <execution_context.h>
#include <result.h>
#include <value.h>

#include "../internal_output_operators.h"
#include "field_reference.h"

auto Inspection::TypeDefinition::FieldReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    NOT_IMPLEMENTED("Called GetAny() on a FieldReference expression.");
}

auto Inspection::TypeDefinition::FieldReference::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a FieldReference expression.");
}

auto Inspection::TypeDefinition::FieldReference::GetField(Inspection::ExecutionContext & ExecutionContext) const -> Inspection::Value const *
{
    auto Result = static_cast<Inspection::Value const *>(nullptr);
    auto const ExecutionStack = ExecutionContext.GetStack();
    auto ExecutionStackIterator = std::list<Inspection::ExecutionContext::Element const *>::const_iterator{};
    
    switch(m_Root)
    {
    case Inspection::TypeDefinition::FieldReference::Root::Current:
        {
            ASSERTION(ExecutionStack.size() > 0);
            ExecutionStackIterator = std::prev(std::end(ExecutionStack));
            
            break;
        }
    case Inspection::TypeDefinition::FieldReference::Root::Type:
        {
            ASSERTION(ExecutionStack.size() > 0);
            ExecutionStackIterator = std::begin(ExecutionStack);
            
            break;
        }
    }
    Result = (*ExecutionStackIterator)->GetResult().GetValue();
    
    auto PartIterator = std::begin(m_Parts);
    
    while(PartIterator != std::end(m_Parts))
    {
        // maybe, the field is already in the result
        if(Result->HasField(*PartIterator) == true)
        {
            Result = Result->GetField(*PartIterator);
            ++PartIterator;
        }
        // if not, the field might be in the current stack
        else
        {
            ++ExecutionStackIterator;
            Result = (*ExecutionStackIterator)->GetResult().GetValue();
            if(Result->HasField(*PartIterator) == true)
            {
                Result = Result->GetField(*PartIterator);
                ++PartIterator;
            }
        }
    }
    
    return Result;
}

auto Inspection::TypeDefinition::FieldReference::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::FieldReference>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::FieldReference>{new Inspection::TypeDefinition::FieldReference{}};
    
    INVALID_INPUT_IF(Element->HasAttribute("root") == false, "A \"field-reference\" needs a \"root\" attribute.");
    if(Element->GetAttribute("root") == "current")
    {
        Result->m_Root = Inspection::TypeDefinition::FieldReference::Root::Current;
    }
    else if(Element->GetAttribute("root") == "type")
    {
        Result->m_Root = Inspection::TypeDefinition::FieldReference::Root::Type;
    }
    else
    {
        INVALID_INPUT("\"field-reference\" attribute \"root\" has unknown value \"" + Element->GetAttribute("root") + "\".");
    }
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        INVALID_INPUT_IF(ChildElement->GetName() != "field", "Only \"field\" elements allowed in \"field-reference\".");
        ASSERTION(ChildElement->GetChildNodes().size() == 1);
        
        auto TextNode = dynamic_cast<XML::Text const *>(ChildElement->GetChildNode(0));
        
        ASSERTION(TextNode != nullptr);
        Result->m_Parts.push_back(TextNode->GetText());
    }
    
    return Result;
}
