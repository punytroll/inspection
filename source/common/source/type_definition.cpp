/**
 * Copyright (C) 2019-2022  Hagen Möbius
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

#include <bitset>

#include <string_cast/string_cast.h>

#include <xml_puny_dom/xml_puny_dom.h>

#include "assertion.h"
#include "execution_context.h"
#include "internal_output_operators.h"
#include "length.h"
#include "result.h"
#include "type.h"
#include "type_definition/add_tag.h"
#include "type_definition/alternative.h"
#include "type_definition/apply_enumeration.h"
#include "type_definition/array.h"
#include "type_definition/bits_interpretation.h"
#include "type_definition/data_type.h"
#include "type_definition/enumeration.h"
#include "type_definition/expression.h"
#include "type_definition/field_reference.h"
#include "type_definition/function_call.h"
#include "type_definition/helper.h"
#include "type_definition/parameters.h"
#include "type_definition/part_type.h"
#include "type_definition/tag.h"
#include "type_definition/type_reference.h"
#include "type_definition/verification.h"
#include "type_definition.h"
#include "type_repository.h"
#include "value.h"
#include "xml_helper.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Field                                                                                         //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Field::Field(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Field}
{
}

auto Inspection::TypeDefinition::Field::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    if(TypeReference)
    {
        ASSERTION(TypeReference != nullptr);
        
        const auto * FieldType = TypeReference->GetType(ExecutionContext);
        
        ASSERTION(FieldType != nullptr);
        
        auto FieldResult = FieldType->Get(ExecutionContext, Reader, ExecutionContext.GetAllParameters());
        
        Continue = FieldResult->GetSuccess();
        Result->GetValue()->Extend(FieldResult->ExtractValue());
    }
    else
    {
        ASSERTION(Parts.size() == 1);
        
        const auto & Part = Parts.front();
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(Part->Length != nullptr)
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part->Length->GetAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader);
        }
        
        auto PartParameters = Part->GetParameters(ExecutionContext);
        auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader->GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Field::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Field>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Field>{new Inspection::TypeDefinition::Field{}};
    
    ASSERTION(Element->HasAttribute("name") == true);
    Result->FieldName = Element->GetAttribute("name");
    
    return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward                                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Forward::Forward(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Forward}
{
}

auto Inspection::TypeDefinition::Forward::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    if(TypeReference != nullptr)
    {
        auto ForwardType = TypeReference->GetType(ExecutionContext);
        
        ASSERTION(ForwardType != nullptr);
        
        auto ForwardResult = ForwardType->Get(ExecutionContext, Reader, ExecutionContext.GetAllParameters());
        
        Continue = ForwardResult->GetSuccess();
        Result->GetValue()->Extend(ForwardResult->ExtractValue());
        // interpretation
        if(Continue == true)
        {
            Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
        }
    }
    else
    {
        // if there is no type reference, we expect exactly one array in the parts
        ASSERTION(Parts.size() == 1);
        ASSERTION(Parts[0]->GetPartType() == Inspection::TypeDefinition::PartType::Array);
        
        auto & Part = Parts[0];
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(Part->Length != nullptr)
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part->Length->GetAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader);
        }
        
        auto PartParameters = Part->GetParameters(ExecutionContext);
        auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader->GetConsumedLength());
        // interpretation
        if(Continue == true)
        {
            Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
        }
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Forward::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Forward>
{
    return std::unique_ptr<Inspection::TypeDefinition::Forward>{new Inspection::TypeDefinition::Forward{}};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Part                                                                                          //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Part::Part(Inspection::TypeDefinition::PartType PartType) :
    m_PartType{PartType}
{
}

auto Inspection::TypeDefinition::Part::ApplyInterpretations(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
{
    auto Result = true;
    
    for(const auto & Interpretation : Interpretations)
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

auto Inspection::TypeDefinition::Part::GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>
{
    if(Parameters != nullptr)
    {
        return Parameters->GetParameters(ExecutionContext);
    }
    else
    {
        return std::unordered_map<std::string, std::any>{};
    }
}

auto Inspection::TypeDefinition::Part::GetPartType(void) const -> Inspection::TypeDefinition::PartType
{
    return m_PartType;
}

auto Inspection::TypeDefinition::Part::Load(const XML::Element * Element) -> std::unique_ptr<Inspection::TypeDefinition::Part>
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
            Result->GetValue()->Extend(PartResult->ExtractValue());
            
            break;
        }
    case Inspection::TypeDefinition::PartType::Array:
    case Inspection::TypeDefinition::PartType::Field:
        {
            ASSERTION(Part.FieldName.has_value() == true);
            Result->GetValue()->AppendField(Part.FieldName.value(), PartResult->ExtractValue());
            
            break;
        }
    case Inspection::TypeDefinition::PartType::Type:
        {
            IMPOSSIBLE_CODE_REACHED("a \"type\" should not be possible inside of a part");
        }
    }
}

auto Inspection::TypeDefinition::Part::_LoadProperties(const XML::Element * Element) -> void
{
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        _LoadProperty(ChildElement);
    }
}

auto Inspection::TypeDefinition::Part::_LoadProperty(const XML::Element * Element) -> void
{
    ASSERTION(Element != nullptr);
    if(Element->GetName() == "type-reference")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        TypeReference = Inspection::TypeDefinition::TypeReference::Load(Element);
    }
    else if(Element->GetName() == "apply-enumeration")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        Interpretations.push_back(Inspection::TypeDefinition::ApplyEnumeration::Load(Element));
    }
    else if(Element->GetName() == "length")
    {
        Length = Inspection::TypeDefinition::Expression::Load(Element);
    }
    else if(Element->GetName() == "parameters")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        Parameters = Inspection::TypeDefinition::Parameters::Load(Element);
    }
    else if(Element->GetName() == "verification")
    {
        for(auto ChildElement : Element->GetChildElements())
        {
            ASSERTION(ChildElement != nullptr);
            Interpretations.push_back(Inspection::TypeDefinition::Verification::Load(dynamic_cast<const XML::Element *>(ChildElement)));
        }
    }
    else if(Element->GetName() == "tag")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward) || (m_PartType == Inspection::TypeDefinition::PartType::Sequence));
        Interpretations.push_back(Inspection::TypeDefinition::AddTag::Load(Element));
    }
    else if(Element->GetName() == "array")
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Sequence) || (m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Alternative) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        Parts.emplace_back(Inspection::TypeDefinition::Part::Load(Element));
    }
    else if((Element->GetName() == "alternative") || (Element->GetName() == "field") || (Element->GetName() == "forward") || (Element->GetName() == "select") || (Element->GetName() == "sequence"))
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Sequence) || (m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Alternative));
        Parts.emplace_back(Inspection::TypeDefinition::Part::Load(Element));
    }
    else if((Element->GetName() == "bit") || (Element->GetName() == "bits"))
    {
        ASSERTION((m_PartType == Inspection::TypeDefinition::PartType::Field) || (m_PartType == Inspection::TypeDefinition::PartType::Forward));
        Interpretations.push_back(Inspection::TypeDefinition::BitsInterpretation::Load(Element));
    }
    else
    {
        UNEXPECTED_CASE("Element->GetName() == " + Element->GetName());
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Select                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Select::Select(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Select}
{
}

auto Inspection::TypeDefinition::Select::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    
    auto FoundCase = false;
    
    for(auto CaseIterator = std::begin(m_Cases); CaseIterator != std::end(m_Cases); ++CaseIterator)
    {
        auto const & Case = *CaseIterator;
        auto WhenAny = Case->GetWhen().GetAny(ExecutionContext);
        
        ASSERTION(WhenAny.type() == typeid(bool));
        if(std::any_cast<bool>(WhenAny) == true)
        {
            FoundCase = true;
            if(Case->HasPart() == true)
            {
                auto const & Part = Case->GetPart();
                auto PartReader = std::unique_ptr<Inspection::Reader>{};
                
                if(Part.Length != nullptr)
                {
                    PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part.Length->GetAny(ExecutionContext)));
                }
                else
                {
                    PartReader = std::make_unique<Inspection::Reader>(Reader);
                }
                
                auto PartParameters = Part.GetParameters(ExecutionContext);
                auto PartResult = Part.Get(ExecutionContext, *PartReader, PartParameters);
                
                Continue = PartResult->GetSuccess();
                m_AddPartResult(Result.get(), Part, PartResult.get());
                Reader.AdvancePosition(PartReader->GetConsumedLength());
            }
            
            break;
        }
    }
    if(FoundCase == false)
    {
        ASSERTION(Continue == true);
        INVALID_INPUT_IF(m_Else == nullptr, "At least one \"case\" must evaluate to true or an \"else\" must be given.");
        if(m_Else->HasPart() == true)
        {
            auto const & Part = m_Else->GetPart();
            auto PartReader = std::unique_ptr<Inspection::Reader>{};
            
            if(Part.Length != nullptr)
            {
                PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part.Length->GetAny(ExecutionContext)));
            }
            else
            {
                PartReader = std::make_unique<Inspection::Reader>(Reader);
            }
            
            auto PartParameters = Part.GetParameters(ExecutionContext);
            auto PartResult = Part.Get(ExecutionContext, *PartReader, PartParameters);
            
            Continue = PartResult->GetSuccess();
            m_AddPartResult(Result.get(), Part, PartResult.get());
            Reader.AdvancePosition(PartReader->GetConsumedLength());
        }
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Select::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Select>
{
    return std::unique_ptr<Inspection::TypeDefinition::Select>{new Inspection::TypeDefinition::Select{}};
}

auto Inspection::TypeDefinition::Select::_LoadProperty(XML::Element const * Element) -> void
{
    if(Element->GetName() == "case")
    {
        m_Cases.push_back(Inspection::TypeDefinition::Select::Case::Load(Element));
    }
    else if(Element->GetName() == "else")
    {
        INVALID_INPUT_IF(m_Else != nullptr, "Only one \"else\" element allowed on \"select\" parts.");
        m_Else = Inspection::TypeDefinition::Select::Case::Load(Element);
    }
    else
    {
        Inspection::TypeDefinition::Part::_LoadProperty(Element);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Select::Case                                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::TypeDefinition::Select::Case::GetPart() const -> Inspection::TypeDefinition::Part const &
{
    ASSERTION(m_Part != nullptr);
    
    return *m_Part;
}

auto Inspection::TypeDefinition::Select::Case::GetWhen() const -> Inspection::TypeDefinition::Expression const &
{
    ASSERTION(m_When != nullptr);
    
    return *m_When;
}

auto Inspection::TypeDefinition::Select::Case::HasPart() const -> bool
{
    return m_Part != nullptr;
}

auto Inspection::TypeDefinition::Select::Case::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Select::Case>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Select::Case>{new Inspection::TypeDefinition::Select::Case{}};
    
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "when")
        {
            ASSERTION(Result->m_When== nullptr);
            Result->m_When = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
        }
        else
        {
            ASSERTION(Result->m_Part == nullptr);
            Result->m_Part = Inspection::TypeDefinition::Part::Load(ChildElement);
        }
    }
    INVALID_INPUT_IF(Element->GetName() == "case" && Result->m_When == nullptr, "Case parts need a <when> specification.");
    
    return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Sequence                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::Sequence::Sequence(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Sequence}
{
}

auto Inspection::TypeDefinition::Sequence::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    for(auto PartIterator = std::begin(Parts); ((Continue == true) && (PartIterator != std::end(Parts))); ++PartIterator)
    {
        auto const & Part = *PartIterator;
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(Part->Length != nullptr)
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<const Inspection::Length &>(Part->Length->GetAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader);
        }
        
        auto PartParameters = Part->GetParameters(ExecutionContext);
        auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
        
        Continue = PartResult->GetSuccess();
        m_AddPartResult(Result.get(), *Part, PartResult.get());
        Reader.AdvancePosition(PartReader->GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Sequence::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Sequence>
{
    return std::unique_ptr<Inspection::TypeDefinition::Sequence>{new Inspection::TypeDefinition::Sequence{}};
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// TypePart                                                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////////

Inspection::TypeDefinition::TypePart::TypePart(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Type}
{
}

auto Inspection::TypeDefinition::TypePart::Create() -> std::unique_ptr<Inspection::TypeDefinition::TypePart>
{
    return std::unique_ptr<Inspection::TypeDefinition::TypePart>{new Inspection::TypeDefinition::TypePart{}};
}

auto Inspection::TypeDefinition::TypePart::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    NOT_IMPLEMENTED("Called Get() on a Type part.");
}
