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

#include <format>

#include <xml_puny_dom/xml_puny_dom.h>

#include <common/assertion.h>
#include <common/execution_context.h>
#include <common/value.h>

#include "../internal_output_operators.h"
#include "apply_enumeration.h"
#include "data_type.h"
#include "enumeration.h"
#include "expression.h"
#include "tag.h"

static auto m_ApplyTags(Inspection::ExecutionContext & ExecutionContext, std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> const & Tags, Inspection::Value * Target) -> void
{
    ASSERTION(Target != nullptr);
    for(auto const & Tag : Tags)
    {
        ASSERTION(Tag != nullptr);
        Target->AddTag(Tag->Get(ExecutionContext));
    }
}

template<typename DataType>
static auto m_ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, Inspection::TypeDefinition::Enumeration const & Enumeration, Inspection::Value * Target) -> bool
{
    auto Result = false;
    auto BaseValueString = std::format("{}", std::any_cast<DataType const &>(Target->GetData()));
    auto ElementIterator = std::find_if(Enumeration.GetElements().begin(), Enumeration.GetElements().end(), [&BaseValueString](auto & Element)
                                                                                                            {
                                                                                                                return Element.BaseValue == BaseValueString;
                                                                                                            });
    
    if(ElementIterator != Enumeration.GetElements().end())
    {
        ::m_ApplyTags(ExecutionContext, ElementIterator->Tags, Target);
        Result = ElementIterator->Valid;
    }
    else
    {
        if(Enumeration.GetFallbackElement().has_value() == true)
        {
            ::m_ApplyTags(ExecutionContext, Enumeration.GetFallbackElement()->Tags, Target);
            if(Enumeration.GetFallbackElement()->Valid == false)
            {
                Target->AddTag("error", "Could find no enumeration element for the base value \"" + BaseValueString + "\".");
            }
            Result = Enumeration.GetFallbackElement()->Valid;
        }
        else
        {
            Target->AddTag("error", "Could find neither an enumeration element nor an enumeration fallback element for the base value \"" + BaseValueString + "\".");
            Result = false;
        }
    }
    
    return Result;
}

Inspection::TypeDefinition::ApplyEnumeration::ApplyEnumeration()
{
}

auto Inspection::TypeDefinition::ApplyEnumeration::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
{
    ASSERTION(m_Enumeration != nullptr);
    
    auto Result = true;
    
    switch(m_Enumeration->GetBaseDataType())
    {
    case Inspection::TypeDefinition::DataType::String:
        {
            ::m_ApplyEnumeration<std::string>(ExecutionContext, *m_Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
        {
            ::m_ApplyEnumeration<std::uint8_t>(ExecutionContext, *m_Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
        {
            ::m_ApplyEnumeration<std::uint16_t>(ExecutionContext, *m_Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
        {
            ::m_ApplyEnumeration<std::uint32_t>(ExecutionContext, *m_Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::Boolean:
        {
            ::m_ApplyEnumeration<bool>(ExecutionContext, *m_Enumeration, Target);
            
            break;
        }
    default:
        {
            UNEXPECTED_CASE("Enumeration->BaseDataType == " + Inspection::to_string(m_Enumeration->GetBaseDataType()));
        }
    }
    
    return Result;
}

auto Inspection::TypeDefinition::ApplyEnumeration::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration>{new Inspection::TypeDefinition::ApplyEnumeration{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "enumeration")
        {
            INVALID_INPUT_IF(Result->m_Enumeration != nullptr, "Only one \"enumeration\" allowed inside of an \"apply-enumeration\".");
            Result->m_Enumeration = Inspection::TypeDefinition::Enumeration::Load(ChildElement);
        }
        else
        {
            INVALID_INPUT("\"apply-enumeration\" elements may only contain an \"enumeration\" element, not \"" + ChildElement->GetName() + "\".");
        }
    }
    
    return Result;
}
