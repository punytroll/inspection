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

#include <common/assertion.h>

#include "enumeration.h"
#include "expression.h"
#include "helper.h"
#include "tag.h"
#include "value.h"

auto Inspection::TypeDefinition::Enumeration::GetBaseDataType() const -> Inspection::TypeDefinition::DataType
{
    return m_BaseDataType;
}

auto Inspection::TypeDefinition::Enumeration::GetElements() const -> std::vector<Inspection::TypeDefinition::Enumeration::Element> const &
{
    return m_Elements;
}

auto Inspection::TypeDefinition::Enumeration::GetFallbackElement() const -> std::optional<Inspection::TypeDefinition::Enumeration::Element> const &
{
    return m_FallbackElement;
}

auto Inspection::TypeDefinition::Enumeration::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Enumeration>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Enumeration>{new Inspection::TypeDefinition::Enumeration{}};
    
    ASSERTION(Element != nullptr);
    ASSERTION(Element->GetName() == "enumeration");
    INVALID_INPUT_IF(Element->HasAttribute("base-data-type") == false, "An enumeration needs to have a \"base-data-type\" attribute.");
    Result->m_BaseDataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetAttribute("base-data-type"));
    for(auto EnumerationChildElement : Element->GetChildElements())
    {
        ASSERTION(EnumerationChildElement != nullptr);
        if(EnumerationChildElement->GetName() == "element")
        {
            auto & Element = Result->m_Elements.emplace_back();
            
            ASSERTION(EnumerationChildElement->HasAttribute("valid") == true);
            Element.Valid = Inspection::TypeDefinition::GetBooleanFromString(EnumerationChildElement->GetAttribute("valid"));
            for(auto EnumerationElementChildElement : EnumerationChildElement->GetChildElements())
            {
                ASSERTION(EnumerationElementChildElement != nullptr);
                if(EnumerationElementChildElement->GetName() == "tag")
                {
                    Element.Tags.push_back(Inspection::TypeDefinition::Tag::Load(EnumerationElementChildElement));
                }
                else if(EnumerationElementChildElement->GetName() == "base-value")
                {
                    Element.BaseValue = Inspection::TypeDefinition::Value::LoadFromWithin(EnumerationElementChildElement);
                }
                else
                {
                    UNEXPECTED_CASE("EnumerationElementChildElement->GetName() == " + EnumerationElementChildElement->GetName());
                }
            }
            INVALID_INPUT_IF(Element.BaseValue == nullptr, "Missing base value for Enumeration/Element.");
            INVALID_INPUT_IF(Element.BaseValue->GetDataType() != Result->m_BaseDataType, "Enumeration elements must have the same data type as the parent enumeration's base data type.");
        }
        else if(EnumerationChildElement->GetName() == "fallback-element")
        {
            ASSERTION(Result->m_FallbackElement.has_value() == false);
            Result->m_FallbackElement.emplace();
            Result->m_FallbackElement->Valid = Inspection::TypeDefinition::GetBooleanFromString(EnumerationChildElement->GetAttribute("valid"));
            for(auto EnumerationFallbackElementChildElement : EnumerationChildElement->GetChildElements())
            {
                ASSERTION(EnumerationFallbackElementChildElement != nullptr);
                if(EnumerationFallbackElementChildElement->GetName() == "tag")
                {
                    Result->m_FallbackElement->Tags.push_back(Inspection::TypeDefinition::Tag::Load(EnumerationFallbackElementChildElement));
                }
                else
                {
                    UNEXPECTED_CASE("EnumerationFallbackElementChildElement->GetName() == " + EnumerationFallbackElementChildElement->GetName());
                }
            }
        }
        else
        {
            UNEXPECTED_CASE("EnumerationChildElement->GetName() == " + EnumerationChildElement->GetName());
        }
    }
    
    return Result;
}
