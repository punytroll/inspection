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
#include <common/execution_context.h>
#include <common/result.h>

#include "add_tag.h"
#include "alternative.h"
#include "apply_enumeration.h"
#include "array.h"
#include "bits_interpretation.h"
#include "enumeration.h"
#include "expression.h"
#include "field.h"
#include "field_reference.h"
#include "forward.h"
#include "interpretation.h"
#include "parameters.h"
#include "part.h"
#include "part_type.h"
#include "select.h"
#include "sequence.h"
#include "tag.h"
#include "type_reference.h"
#include "value.h"
#include "verification.h"

Inspection::TypeDefinition::Part::Part(Inspection::TypeDefinition::PartType PartType) :
    m_PartType{PartType}
{
}

Inspection::TypeDefinition::Part::~Part()
{
}

auto Inspection::TypeDefinition::Part::ApplyInterpretations(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
{
    auto Result = true;
    
    for(auto const & Interpretation : m_Interpretations)
    {
        ASSERTION(Interpretation != nullptr);
        Result &= Interpretation->Apply(ExecutionContext, Target);
        if(Result == false)
        {
            break;
        }
    }
    
    return Result;
}

auto Inspection::TypeDefinition::Part::GetName(Inspection::ExecutionContext & ExecutionContext) const -> std::optional<std::string>
{
    return std::nullopt;
}

auto Inspection::TypeDefinition::Part::GetLengthAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Length != nullptr);
    
    return m_Length->GetAny(ExecutionContext);
}

auto Inspection::TypeDefinition::Part::GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>
{
    if(m_Parameters != nullptr)
    {
        return m_Parameters->GetParameters(ExecutionContext);
    }
    else
    {
        return std::unordered_map<std::string, std::any>{};
    }
}

auto Inspection::TypeDefinition::Part::GetParts() const -> std::vector<std::unique_ptr<Inspection::TypeDefinition::Part>> const &
{
    return m_Parts;
}

auto Inspection::TypeDefinition::Part::GetPartType() const -> Inspection::TypeDefinition::PartType
{
    return m_PartType;
}

auto Inspection::TypeDefinition::Part::GetTypeFromTypeReference(Inspection::ExecutionContext & ExecutionContext) const -> Inspection::Type const &
{
    ASSERTION(m_TypeReference != nullptr);
    
    return m_TypeReference->GetType(ExecutionContext);
}

auto Inspection::TypeDefinition::Part::HasLength() const -> bool
{
    return m_Length != nullptr;
}

auto Inspection::TypeDefinition::Part::HasTypeReference() const -> bool
{
    return m_TypeReference != nullptr;
}

auto Inspection::TypeDefinition::Part::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Part>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Part>{};
    
    if(Element->GetName() == "alternative")
    {
        Result = Inspection::TypeDefinition::Alternative::Load(Element);
    }
    else if(Element->GetName() == "array")
    {
        Result = Inspection::TypeDefinition::Array::Load(Element);
    }
    else if(Element->GetName() == "field")
    {
        Result = Inspection::TypeDefinition::Field::Load(Element);
    }
    else if(Element->GetName() == "forward")
    {
        Result = Inspection::TypeDefinition::Forward::Load(Element);
    }
    else if(Element->GetName() == "select")
    {
        Result = Inspection::TypeDefinition::Select::Load(Element);
    }
    else if(Element->GetName() == "sequence")
    {
        Result = Inspection::TypeDefinition::Sequence::Load(Element);
    }
    else
    {
        UNEXPECTED_CASE("Element->GetName() == " + Element->GetName());
    }
    Result->m_LoadProperties(Element);
    
    return Result;
}

auto Inspection::TypeDefinition::Part::m_AddPartResult(Inspection::ExecutionContext & ExecutionContext, std::optional<std::string> const & PartName, Inspection::Result * PartResult) const -> void
{
    if(PartName.has_value() == true)
    {
        ExecutionContext.GetCurrentResult().GetValue()->AppendField(PartName.value(), PartResult->ExtractValue());
    }
    else
    {
        ExecutionContext.GetCurrentResult().GetValue()->Extend(PartResult->ExtractValue());
    }
}

auto Inspection::TypeDefinition::Part::m_LoadProperties(XML::Element const * Element) -> void
{
    ASSERTION(Element != nullptr);
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        m_LoadProperty(ChildElement);
    }
}

auto Inspection::TypeDefinition::Part::m_LoadProperty(XML::Element const * Element) -> void
{
    ASSERTION(Element != nullptr);
    if(Element->GetName() == "type-reference")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        m_TypeReference = Inspection::TypeDefinition::TypeReference::Load(Element);
    }
    else if(Element->GetName() == "apply-enumeration")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        m_Interpretations.push_back(Inspection::TypeDefinition::ApplyEnumeration::Load(Element));
    }
    else if(Element->GetName() == "length")
    {
        m_Length = Inspection::TypeDefinition::Expression::Load(Element);
    }
    else if(Element->GetName() == "parameters")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        m_Parameters = Inspection::TypeDefinition::Parameters::Load(Element);
    }
    else if(Element->GetName() == "verification")
    {
        for(auto const & ChildElement : Element->GetChildElements())
        {
            ASSERTION(ChildElement != nullptr);
            m_Interpretations.push_back(Inspection::TypeDefinition::Verification::Load(ChildElement));
        }
    }
    else if(Element->GetName() == "tag")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward) || (m_PartType == Inspection::TypeDefinition::PartType::Sequence));
        m_Interpretations.push_back(Inspection::TypeDefinition::AddTag::Load(Element));
    }
    else if(Element->GetName() == "array")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Sequence) || (m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Alternative) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        m_Parts.emplace_back(Inspection::TypeDefinition::Part::Load(Element));
    }
    else if((Element->GetName() == "alternative") || (Element->GetName() == "field") || (Element->GetName() == "forward") || (Element->GetName() == "select") || (Element->GetName() == "sequence"))
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Sequence) || (m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Alternative));
        m_Parts.emplace_back(Inspection::TypeDefinition::Part::Load(Element));
    }
    else if((Element->GetName() == "bit") || (Element->GetName() == "bits"))
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        m_Interpretations.push_back(Inspection::TypeDefinition::BitsInterpretation::Load(Element));
    }
    else
    {
        UNEXPECTED_CASE("Element->GetName() == " + Element->GetName());
    }
}
