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

#include <xml_puny_dom/xml_puny_dom.h>

#include <common/value.h>

#include "expression.h"
#include "tag.h"

auto Inspection::TypeDefinition::Tag::Get(Inspection::ExecutionContext & ExecutionContext) const -> std::unique_ptr<Inspection::Value>
{
    auto Result = std::make_unique<Inspection::Value>();
    
    Result->SetName(m_Name);
    if(m_ValueExpression != nullptr)
    {
        Result->SetData(m_ValueExpression->GetAny(ExecutionContext));
    }
    for(auto const & Tag : m_Tags)
    {
        ASSERTION(Tag != nullptr);
        Result->AddTag(Tag->Get(ExecutionContext));
    }
    
    return Result;
}

auto Inspection::TypeDefinition::Tag::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Tag>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Tag>{new Inspection::TypeDefinition::Tag{}};
    
    INVALID_INPUT_IF(Element->HasAttribute("name") == false, "A \"tag\" requires a \"name\" attribute.");
    Result->m_Name = Element->GetAttribute("name");
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "value")
        {
            Result->m_ValueExpression = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
        }
        else if(ChildElement->GetName() == "tag")
        {
            Result->m_Tags.push_back(Inspection::TypeDefinition::Tag::Load(ChildElement));
        }
        else
        {
            UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
        }
    }
    
    return Result;
} 
