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
#include "type_definition/alternative.h"
#include "type_definition/array.h"
#include "type_definition/data_type.h"
#include "type_definition/enumeration.h"
#include "type_definition/expression.h"
#include "type_definition/field_reference.h"
#include "type_definition/function_call.h"
#include "type_definition/helper.h"
#include "type_definition/parameters.h"
#include "type_definition/tag.h"
#include "type_definition/type_reference.h"
#include "type_definition.h"
#include "type_repository.h"
#include "value.h"
#include "xml_helper.h"

using namespace std::string_literals;

static auto ApplyTags(Inspection::ExecutionContext & ExecutionContext, std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> const & Tags, Inspection::Value * Target) -> void
{
    ASSERTION(Target != nullptr);
    for(auto & Tag : Tags)
    {
        ASSERTION(Tag != nullptr);
        Target->AddTag(Tag->Get(ExecutionContext));
    }
}

template<typename DataType>
static auto ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, Inspection::TypeDefinition::Enumeration const & Enumeration, Inspection::Value * Target) -> bool
{
    auto Result = false;
    auto BaseValueString = to_string_cast(std::any_cast<DataType const &>(Target->GetData()));
    auto ElementIterator = std::find_if(Enumeration.GetElements().begin(), Enumeration.GetElements().end(), [&BaseValueString](auto & Element)
                                                                                                            {
                                                                                                                return Element.BaseValue == BaseValueString;
                                                                                                            });
    
    if(ElementIterator != Enumeration.GetElements().end())
    {
        ApplyTags(ExecutionContext, ElementIterator->Tags, Target);
        Result = ElementIterator->Valid;
    }
    else
    {
        if(Enumeration.GetFallbackElement().has_value() == true)
        {
            ApplyTags(ExecutionContext, Enumeration.GetFallbackElement()->Tags, Target);
            Target->AddTag("error", "Could find no enumeration element for the base value \"" + BaseValueString + "\".");
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


///////////////////////////////////////////////////////////////////////////////////////////////////
// AddTag                                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Inspection::TypeDefinition::AddTag::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const
{
    ASSERTION(Target != nullptr);
    ASSERTION(m_Tag != nullptr);
    Target->AddTag(m_Tag->Get(ExecutionContext));
    
    return true;
}

std::unique_ptr<Inspection::TypeDefinition::AddTag> Inspection::TypeDefinition::AddTag::Load(const XML::Element * Element)
{
    ASSERTION(Element != nullptr);
    ASSERTION(Element->GetName() == "tag");
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::AddTag>{new Inspection::TypeDefinition::AddTag{}};
    
    Result->m_Tag = Inspection::TypeDefinition::Tag::Load(Element);
    
    return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ApplyEnumeration                                                                              //
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Inspection::TypeDefinition::ApplyEnumeration::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const
{
    ASSERTION(Enumeration != nullptr);
    
    auto Result = true;
    
    switch(Enumeration->GetBaseDataType())
    {
    case Inspection::TypeDefinition::DataType::String:
        {
            ::ApplyEnumeration<std::string>(ExecutionContext, *Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
        {
            ::ApplyEnumeration<std::uint8_t>(ExecutionContext, *Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
        {
            ::ApplyEnumeration<std::uint16_t>(ExecutionContext, *Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
        {
            ::ApplyEnumeration<std::uint32_t>(ExecutionContext, *Enumeration, Target);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::Boolean:
        {
            ::ApplyEnumeration<bool>(ExecutionContext, *Enumeration, Target);
            
            break;
        }
    default:
        {
            UNEXPECTED_CASE("Enumeration->BaseDataType == " + Inspection::to_string(Enumeration->GetBaseDataType()));
        }
    }
    
    return Result;
}

std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration> Inspection::TypeDefinition::ApplyEnumeration::Load(const XML::Element * Element)
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration>{new Inspection::TypeDefinition::ApplyEnumeration{}};
    
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "enumeration")
        {
            Result->Enumeration = Inspection::TypeDefinition::Enumeration::Load(ChildElement);
        }
        else
        {
            UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
        }
    }
    
    return Result;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// BitsInterpretation                                                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

static auto ApplyBitsInterpretation(Inspection::ExecutionContext & ExecutionContext, auto const & Bitset, Inspection::TypeDefinition::BitsInterpretation const & BitsInterpretation, Inspection::Value * Target) -> bool
{
    auto Continue = true;
    auto BitsInterpretationValue = Target->AppendField(BitsInterpretation.GetName());
    auto DataVerification = BitsInterpretation.GetDataVerification();
    
    if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Set)
    {
        BitsInterpretationValue->AddTag("set bit"s + ((BitsInterpretation.GetLength() > 1) ? ("s") : ("")));
    }
    else if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Unset)
    {
        BitsInterpretationValue->AddTag("unset bit"s + ((BitsInterpretation.GetLength() > 1) ? ("s") : ("")));
    }
    else
    {
        BitsInterpretationValue->AddTag("bit"s + ((BitsInterpretation.GetLength() > 1) ? ("s") : ("")));
    }
    BitsInterpretationValue->AddTag("begin index", BitsInterpretation.GetBeginIndex());
    Inspection::TypeDefinition::AppendBitLengthTag(BitsInterpretationValue, Inspection::Length{0, BitsInterpretation.GetLength()});
    
    
    switch(BitsInterpretation.GetAsDataType())
    {
    case Inspection::TypeDefinition::DataType::Boolean:
        {
            INVALID_INPUT_IF(BitsInterpretation.GetLength() != 1, "A boolean interpretation of bits requires the length to be one.");
            BitsInterpretationValue->AddTag("boolean");
            
            auto BitIndex = BitsInterpretation.GetBeginIndex();
            auto BitValue = Bitset[BitIndex];
            
            if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Set)
            {
                if(BitValue == false)
                {
                    BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be set.");
                }
            }
            else if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Unset)
            {
                if(BitValue == true)
                {
                    BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be unset.");
                }
            }
            BitsInterpretationValue->SetData(BitValue);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
        {
            INVALID_INPUT_IF(BitsInterpretation.GetLength() == 0, "The length must not be zero.");
            
            auto Value = static_cast<std::uint8_t>(0);
            
            for(auto Index = 0u; Index < BitsInterpretation.GetLength(); ++Index)
            {
                if(Index > 0)
                {
                    Value <<= 1;
                }
                
                auto BitIndex = BitsInterpretation.GetBeginIndex() + BitsInterpretation.GetLength() - Index - 1;
                auto BitValue = Bitset[BitIndex];
                
                if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Set)
                {
                    if(BitValue == false)
                    {
                        BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be set.");
                    }
                }
                else if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Unset)
                {
                    if(BitValue == true)
                    {
                        BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be unset.");
                    }
                }
                Value |= ((BitValue == true) ? (static_cast<std::uint8_t>(1)) : (static_cast<std::uint8_t>(0)));
            }
            BitsInterpretationValue->AddTag("integer");
            BitsInterpretationValue->AddTag("unsigned");
            BitsInterpretationValue->SetData(Value);
            
            break;
        }
    case Inspection::TypeDefinition::DataType::Nothing:
        {
            INVALID_INPUT_IF(BitsInterpretation.GetLength() == 0, "The length must not be zero.");
            for(auto Index = 0u; Index < BitsInterpretation.GetLength(); ++Index)
            {
                auto BitIndex = BitsInterpretation.GetBeginIndex() + Index;
                auto BitValue = Bitset[BitIndex];
                
                if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Set)
                {
                    if(BitValue == false)
                    {
                        BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be set.");
                    }
                }
                else if(DataVerification == Inspection::TypeDefinition::BitsInterpretation::DataVerification::Unset)
                {
                    if(BitValue == true)
                    {
                        BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be unset.");
                    }
                }
            }
            BitsInterpretationValue->AddTag("integer");
            BitsInterpretationValue->AddTag("unsigned");
            
            break;
        }
    default:
        {
            UNEXPECTED_CASE("AsDataType == " + Inspection::to_string(BitsInterpretation.GetAsDataType()));
        }
    }
    for(auto const & Interpretation : BitsInterpretation.GetInterpretations())
    {
        ASSERTION(Interpretation != nullptr);
        Continue = Interpretation->Apply(ExecutionContext, BitsInterpretationValue);
        if(Continue == false)
        {
            break;
        }
    }
    
    return Continue;
}

auto Inspection::TypeDefinition::BitsInterpretation::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
{
    auto Result = true;
    auto const & Data = Target->GetData();
    
    if(Data.type() == typeid(std::bitset<8>))
    {
        Result = ::ApplyBitsInterpretation(ExecutionContext, std::any_cast<std::bitset<8> const &>(Data), *this, Target);
    }
    else if(Data.type() == typeid(std::bitset<16>))
    {
        Result = ::ApplyBitsInterpretation(ExecutionContext, std::any_cast<std::bitset<16> const &>(Data), *this, Target);
    }
    else if(Data.type() == typeid(std::bitset<32>))
    {
        Result = ::ApplyBitsInterpretation(ExecutionContext, std::any_cast<std::bitset<32> const &>(Data), *this, Target);
    }
    else
    {
        UNEXPECTED_CASE("Data.type() == " + Inspection::to_string(Data.type()));
    }
    
    return Result;
}

auto Inspection::TypeDefinition::BitsInterpretation::GetAsDataType(void) const -> Inspection::TypeDefinition::DataType
{
    return m_AsDataType;
}

auto Inspection::TypeDefinition::BitsInterpretation::GetBeginIndex(void) const -> std::uint64_t
{
    return m_BeginIndex;
}

auto Inspection::TypeDefinition::BitsInterpretation::GetDataVerification() const -> Inspection::TypeDefinition::BitsInterpretation::DataVerification
{
    return m_DataVerification;
}

auto Inspection::TypeDefinition::BitsInterpretation::GetInterpretations(void) const -> std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> const &
{
    return m_Interpretations;
}

auto Inspection::TypeDefinition::BitsInterpretation::GetLength(void) const -> std::uint64_t
{
    return m_Length;
}

auto Inspection::TypeDefinition::BitsInterpretation::GetName(void) const -> std::string const &
{
    return m_Name;
}

auto Inspection::TypeDefinition::BitsInterpretation::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::BitsInterpretation>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::BitsInterpretation>{new Inspection::TypeDefinition::BitsInterpretation{}};
    
    if(Element->GetName() == "bit")
    {
        INVALID_INPUT_IF(Element->HasAttribute("begin-index") == true, "A bit interpretation must not have a begin-index attribute.");
        INVALID_INPUT_IF(Element->HasAttribute("length") == true, "A bit interpretation must not have a length attribute.");
        INVALID_INPUT_IF(Element->HasAttribute("index") == false, "A bit interpretation must have an index attribute.");
        Result->m_BeginIndex = from_string_cast<std::uint64_t>(Element->GetAttribute("index"));
        Result->m_Length = 1;
    }
    else if(Element->GetName() == "bits")
    {
        INVALID_INPUT_IF(Element->HasAttribute("index") == true, "A bits interpretation must not have an index attribute.");
        INVALID_INPUT_IF(Element->HasAttribute("begin-index") == false, "A bits interpretation must have a begin-index attribute.");
        INVALID_INPUT_IF(Element->HasAttribute("length") == false, "A bits interpretation must have a length attribute.");
        Result->m_BeginIndex = from_string_cast<std::uint64_t>(Element->GetAttribute("begin-index"));
        Result->m_Length = from_string_cast<std::uint64_t>(Element->GetAttribute("length"));
    }
    ASSERTION(Element->HasAttribute("name") == true);
    Result->m_Name = Element->GetAttribute("name");
    ASSERTION(Element->HasAttribute("as-data-type") == true);
    Result->m_AsDataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetAttribute("as-data-type"));
    if(Element->HasAttribute("data-verification") == true)
    {
        Result->m_DataVerification = Inspection::TypeDefinition::GetDataVerificationFromString(Element->GetAttribute("data-verification"));
    }
    else
    {
        Result->m_DataVerification = Inspection::TypeDefinition::BitsInterpretation::DataVerification::SetOrUnset;
    }
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "apply-enumeration")
        {
            Result->m_Interpretations.push_back(Inspection::TypeDefinition::ApplyEnumeration::Load(ChildElement));
        }
        else if(ChildElement->GetName() == "tag")
        {
            Result->m_Interpretations.push_back(Inspection::TypeDefinition::AddTag::Load(ChildElement));
        }
        else
        {
            UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
        }
    }
    
    return Result;
}


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


///////////////////////////////////////////////////////////////////////////////////////////////////
// Verification                                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

bool Inspection::TypeDefinition::Verification::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const
{
    ASSERTION(m_Expression != nullptr);
    
    auto VerificationAny = m_Expression->GetAny(ExecutionContext);
    
    ASSERTION(VerificationAny.type() == typeid(bool));
    
    auto Result = std::any_cast<bool>(VerificationAny);
    
    if(Result == false)
    {
        Target->AddTag("error", "The value failed to verify."s);
    }
    
    return Result;
}

std::unique_ptr<Inspection::TypeDefinition::Verification> Inspection::TypeDefinition::Verification::Load(const XML::Element * Element)
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Verification>{new Inspection::TypeDefinition::Verification{}};
    
    Result->m_Expression = Inspection::TypeDefinition::Expression::Load(Element);
    
    return Result;
}

auto Inspection::TypeDefinition::GetDataVerificationFromString(std::string_view String) -> Inspection::TypeDefinition::BitsInterpretation::DataVerification
{
    if(String == "set")
    {
        return Inspection::TypeDefinition::BitsInterpretation::DataVerification::Set;
    }
    else if(String == "set or unset")
    {
        return Inspection::TypeDefinition::BitsInterpretation::DataVerification::SetOrUnset;
    }
    else if(String == "unset")
    {
        return Inspection::TypeDefinition::BitsInterpretation::DataVerification::Unset;
    }
    else
    {
        UNEXPECTED_CASE("Data verification string == \"" + std::string{String} + '"');
    }
}
