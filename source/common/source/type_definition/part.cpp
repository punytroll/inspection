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
#include <result.h>

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
#include "verification.h"

Inspection::TypeDefinition::Part::Part(Inspection::TypeDefinition::PartType PartType) :
    m_PartType{PartType}
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

auto Inspection::TypeDefinition::Part::GetFieldName() const -> std::string const &
{
    ASSERTION(m_FieldName.has_value() == true);
    
    return m_FieldName.value();
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

auto Inspection::TypeDefinition::Part::GetTypeFromTypeReference(Inspection::ExecutionContext & ExecutionContext) const -> Inspection::TypeDefinition::Type const &
{
    ASSERTION(m_TypeReference != nullptr);
    
    return m_TypeReference->GetType(ExecutionContext);
}

auto Inspection::TypeDefinition::Part::HasFieldName() const -> bool
{
    return m_FieldName.has_value();
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
    Result->_LoadProperties(Element);
    
    return Result;
}

auto Inspection::TypeDefinition::Part::m_AddPartResult(Inspection::Result * Result, Inspection::TypeDefinition::Part const & Part, Inspection::Result * PartResult) const -> void
{
    switch(Part.GetPartType())
    {
    case Inspection::TypeDefinition::PartType::Alternative:
    case Inspection::TypeDefinition::PartType::Forward:
    case Inspection::TypeDefinition::PartType::Select:
    case Inspection::TypeDefinition::PartType::Sequence:
        {
            ASSERTION(Part.HasFieldName() == false);
            Result->GetValue()->Extend(PartResult->ExtractValue());
            
            break;
        }
    case Inspection::TypeDefinition::PartType::Array:
    case Inspection::TypeDefinition::PartType::Field:
        {
            ASSERTION(Part.HasFieldName() == true);
            Result->GetValue()->AppendField(Part.GetFieldName(), PartResult->ExtractValue());
            
            break;
        }
    case Inspection::TypeDefinition::PartType::Type:
        {
            IMPOSSIBLE_CODE_REACHED("a \"type\" should not be possible inside of a part");
        }
    }
}

auto Inspection::TypeDefinition::Part::_LoadProperties(XML::Element const * Element) -> void
{
    ASSERTION(Element != nullptr);
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        _LoadProperty(ChildElement);
    }
}

auto Inspection::TypeDefinition::Part::_LoadProperty(XML::Element const * Element) -> void
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
