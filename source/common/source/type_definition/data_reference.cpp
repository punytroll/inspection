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
#include <value.h>

#include "../execution_context.h"
#include "data_reference.h"

auto Inspection::TypeDefinition::DataReference::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    auto Value = ExecutionContext.GetValueFromDataReference(*this);
    
    ASSERTION(Value != nullptr);
    
    return Value->GetData();
}

auto Inspection::TypeDefinition::DataReference::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a DataReference expression.");
}

auto Inspection::TypeDefinition::DataReference::GetParts() const -> std::vector<Inspection::TypeDefinition::DataReference::Part> const &
{
    return m_Parts;
}

auto Inspection::TypeDefinition::DataReference::GetRoot() const -> Inspection::TypeDefinition::DataReference::Root
{
    return m_Root;
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
