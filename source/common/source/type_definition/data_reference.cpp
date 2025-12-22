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
#include <common/result.h>
#include <common/value.h>

#include "data_reference.h"

auto Inspection::TypeDefinition::DataReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    auto const ExecutionStack = ExecutionContext.GetStack();
    
    ASSERTION(ExecutionStack.size() > 0);
    
    auto Value = static_cast<Inspection::Value const *>(nullptr);
    
    switch(m_Root)
    {
    case Inspection::TypeDefinition::DataReference::Root::Current:
        {
            Value = ExecutionStack.back()->GetResult().GetValue();
            
            auto EndIterator = std::end(m_Parts);
            
            for(auto PartIterator = std::begin(m_Parts); (Value != nullptr) && (PartIterator != EndIterator); ++PartIterator)
            {
                switch(PartIterator->GetType())
                {
                case Inspection::TypeDefinition::DataReference::Part::Type::Field:
                    {
                        // we are looking for a field
                        Value = Value->GetField(PartIterator->GetName());
                        
                        break;
                    }
                case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
                    {
                        // we are looking for a tag
                        Value = Value->GetTag(PartIterator->GetName());
                        
                        break;
                    }
                }
            }
            
            break;
        }
    case Inspection::TypeDefinition::DataReference::Root::Type:
        {
            auto ExecutionStackIterator = std::begin(ExecutionStack);
            
            Value = (*ExecutionStackIterator)->GetResult().GetValue();
            
            auto PartIterator = std::begin(m_Parts);
            auto EndIterator = std::end(m_Parts);
            
            while(PartIterator != EndIterator)
            {
                switch(PartIterator->GetType())
                {
                case Inspection::TypeDefinition::DataReference::Part::Type::Field:
                    {
                        // we are looking for a field
                        // maybe, the field is already in the result
                        if(Value->HasField(PartIterator->GetName()) == true)
                        {
                            Value = Value->GetField(PartIterator->GetName());
                            ++PartIterator;
                        }
                        // if not, the field might be in the current stack
                        else
                        {
                            ++ExecutionStackIterator;
                            ASSERTION_MESSAGE(ExecutionStackIterator != std::end(ExecutionStack), "Could not find the field \"" + PartIterator->GetName() + "\" on the execution stack.");
                            Value = (*ExecutionStackIterator)->GetResult().GetValue();
                            if(Value->HasField(PartIterator->GetName()) == true)
                            {
                                Value = Value->GetField(PartIterator->GetName());
                                ++PartIterator;
                            }
                        }
                        
                        break;
                    }
                case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
                    {
                        // we are looking for a tag
                        Value = Value->GetTag(PartIterator->GetName());
                        ++PartIterator;
                        
                        break;
                    }
                }
            }
            
            break;
        }
    }
    ASSERTION(Value != nullptr);
    
    return Value->GetData();
}

auto Inspection::TypeDefinition::DataReference::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a DataReference expression.");
}

auto Inspection::TypeDefinition::DataReference::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::DataReference>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::DataReference>{new Inspection::TypeDefinition::DataReference{}};
    
    INVALID_INPUT_IF(Element->HasAttribute("root") == false, "A \"data-reference\" needs a \"root\" attribute.");
    if(Element->GetAttribute("root") == "current")
    {
        Result->m_Root = Inspection::TypeDefinition::DataReference::Root::Current;
    }
    else if(Element->GetAttribute("root") == "type")
    {
        Result->m_Root = Inspection::TypeDefinition::DataReference::Root::Type;
    }
    else
    {
        INVALID_INPUT("The \"root\" attribute of a \"data-reference\" must be either \"current\" or \"type\", not \"" + Element->GetAttribute("root") + "\".");
    }
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "field")
        {
            ASSERTION(ChildElement->GetChildNodes().size() == 1);
            
            auto TextNode = dynamic_cast<XML::Text const *>(ChildElement->GetChildNode(0));
            
            ASSERTION(TextNode != nullptr);
            Result->m_Parts.emplace_back(Inspection::TypeDefinition::DataReference::Part::Type::Field, TextNode->GetText());
        }
        else if(ChildElement->GetName() == "tag")
        {
            ASSERTION(ChildElement->GetChildNodes().size() == 1);
            
            auto TextNode = dynamic_cast<XML::Text const *>(ChildElement->GetChildNode(0));
            
            ASSERTION(TextNode != nullptr);
            Result->m_Parts.emplace_back(Inspection::TypeDefinition::DataReference::Part::Type::Tag, TextNode->GetText());
        }
        else
        {
            INVALID_INPUT("A child of a \"data-reference\" can not be \"" + ChildElement->GetName() + "\".");
        }
    }
    
    return Result;
}

Inspection::TypeDefinition::DataReference::Part::Part(Inspection::TypeDefinition::DataReference::Part::Type Type, std::string_view Name) :
    m_Name{Name},
    m_Type{Type}
{
}

auto Inspection::TypeDefinition::DataReference::Part::GetName() const -> std::string const &
{
    return m_Name;
}

auto Inspection::TypeDefinition::DataReference::Part::GetType() const -> Inspection::TypeDefinition::DataReference::Part::Type
{
    return m_Type;
}
