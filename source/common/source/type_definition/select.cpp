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

#include <assertion.h>
#include <execution_context.h>
#include <reader.h>
#include <result.h>

#include "../getter_part_adapter.h"
#include "expression.h"
#include "part_type.h"
#include "select.h"

Inspection::TypeDefinition::Select::Select() :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Select}
{
}

auto Inspection::TypeDefinition::Select::Get(Inspection::ExecutionContext & ExecutionContext) const -> void
{
    auto Continue = true;
    auto FoundCase = false;
    
    for(auto CaseIterator = std::begin(m_Cases); CaseIterator != std::end(m_Cases); ++CaseIterator)
    {
        auto const & Case = *CaseIterator;
        auto WhenAny = Case.When->GetAny(ExecutionContext);
        
        ASSERTION(WhenAny.type() == typeid(bool));
        if(std::any_cast<bool>(WhenAny) == true)
        {
            FoundCase = true;
            if(Case.Part != nullptr)
            {
                auto PartReader = std::unique_ptr<Inspection::Reader>{};
                
                if(Case.Part->HasLength() == true)
                {
                    PartReader = std::make_unique<Inspection::Reader>(ExecutionContext.GetCurrentReader(), std::any_cast<Inspection::Length const &>(Case.Part->GetLengthAny(ExecutionContext)));
                }
                else
                {
                    PartReader = std::make_unique<Inspection::Reader>(ExecutionContext.GetCurrentReader());
                }
                
                auto PartParameters = Case.Part->GetParameters(ExecutionContext);
                auto PartResult = ExecutionContext.Call(Inspection::GetterPartAdapter{*(Case.Part)}, *PartReader, PartParameters);
                
                Continue = PartResult->GetSuccess();
                m_AddPartResult(ExecutionContext.GetCurrentResult(), *(Case.Part), PartResult.get());
                ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
            }
            
            break;
        }
    }
    if(FoundCase == false)
    {
        ASSERTION(Continue == true);
        INVALID_INPUT_IF(m_ElsePart == nullptr, "At least one \"case\" must evaluate to true if no \"else\" has been given.");
        
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(m_ElsePart->HasLength() == true)
        {
            PartReader = std::make_unique<Inspection::Reader>(ExecutionContext.GetCurrentReader(), std::any_cast<Inspection::Length const &>(m_ElsePart->GetLengthAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(ExecutionContext.GetCurrentReader());
        }
        
        auto PartParameters = m_ElsePart->GetParameters(ExecutionContext);
        auto PartResult = ExecutionContext.Call(Inspection::GetterPartAdapter{*m_ElsePart}, *PartReader, PartParameters);
        
        Continue = PartResult->GetSuccess();
        m_AddPartResult(ExecutionContext.GetCurrentResult(), *m_ElsePart, PartResult.get());
        ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
    }
    // finalization
    ExecutionContext.GetCurrentResult().SetSuccess(Continue);
}

auto Inspection::TypeDefinition::Select::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Select>
{
    return std::unique_ptr<Inspection::TypeDefinition::Select>{new Inspection::TypeDefinition::Select{}};
}

auto Inspection::TypeDefinition::Select::_LoadProperty(XML::Element const * Element) -> void
{
    if(Element->GetName() == "case")
    {
        auto Case = Inspection::TypeDefinition::Select::Case{};
        
        for(auto const & ChildElement : Element->GetChildElements())
        {
            ASSERTION(ChildElement != nullptr);
            if(ChildElement->GetName() == "when")
            {
                INVALID_INPUT_IF(Case.When != nullptr, "Only one \"when\" element allowed per \"case\".");
                Case.When = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
            }
            else
            {
                INVALID_INPUT_IF(Case.Part != nullptr, "Only one part may be given in a \"case\".");
                Case.Part = Inspection::TypeDefinition::Part::Load(ChildElement);
            }
        }
        m_Cases.emplace_back(std::move(Case));
    }
    else if(Element->GetName() == "else")
    {
        INVALID_INPUT_IF(m_ElsePart != nullptr, "Only one \"else\" element allowed on \"select\" parts.");
        
        auto ChildElementsRange = Element->GetChildElements();
        auto ChildElements = std::vector<XML::Element const *>{ChildElementsRange.begin(), ChildElementsRange.end()};
        
        INVALID_INPUT_IF(ChildElements.size() > 1, "Too many parts in an \"else\" element.");
        if(ChildElements.size() > 0)
        {
            m_ElsePart = Inspection::TypeDefinition::Part::Load(ChildElements.front());
        }
    }
    else
    {
        Inspection::TypeDefinition::Part::_LoadProperty(Element);
    }
}
