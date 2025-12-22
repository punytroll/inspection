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

#include <bitset>

#include <xml_puny_dom/xml_puny_dom.h>

#include <common/assertion.h>
#include <common/from_string.h>
#include <common/length.h>
#include <common/value.h>

#include "../internal_output_operators.h"
#include "add_tag.h"
#include "apply_enumeration.h"
#include "bits_interpretation.h"
#include "data_type.h"
#include "data_verification.h"
#include "enumeration.h"
#include "expression.h"
#include "helper.h"
#include "tag.h"

using namespace std::string_literals;

static auto ApplyBitsInterpretation(Inspection::ExecutionContext & ExecutionContext, auto const & Bitset, Inspection::TypeDefinition::BitsInterpretation const & BitsInterpretation, Inspection::Value * Target) -> bool
{
    auto Continue = true;
    auto BitsInterpretationValue = Target->AppendField(BitsInterpretation.GetName());
    auto DataVerification = BitsInterpretation.GetDataVerification();
    
    if(DataVerification == Inspection::TypeDefinition::DataVerification::Set)
    {
        BitsInterpretationValue->AddTag("set bit"s + ((BitsInterpretation.GetLength() > 1) ? ("s") : ("")));
    }
    else if(DataVerification == Inspection::TypeDefinition::DataVerification::Unset)
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
            
            if(DataVerification == Inspection::TypeDefinition::DataVerification::Set)
            {
                if(BitValue == false)
                {
                    BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be set.");
                }
            }
            else if(DataVerification == Inspection::TypeDefinition::DataVerification::Unset)
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
                
                if(DataVerification == Inspection::TypeDefinition::DataVerification::Set)
                {
                    if(BitValue == false)
                    {
                        BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be set.");
                    }
                }
                else if(DataVerification == Inspection::TypeDefinition::DataVerification::Unset)
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
                
                if(DataVerification == Inspection::TypeDefinition::DataVerification::Set)
                {
                    if(BitValue == false)
                    {
                        BitsInterpretationValue->AddTag("error", "Bit '" + std::to_string(BitIndex) + "' must be set.");
                    }
                }
                else if(DataVerification == Inspection::TypeDefinition::DataVerification::Unset)
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

auto Inspection::TypeDefinition::BitsInterpretation::GetDataVerification() const -> Inspection::TypeDefinition::DataVerification
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
        
        auto Index = Inspection::FromString<std::uint64_t>(Element->GetAttribute("index"));
        
        INVALID_INPUT_IF(Index.has_value() == false, "The index attribute of a bit interpretation must be an integer.");
        Result->m_BeginIndex = Index.value();
        Result->m_Length = 1;
    }
    else if(Element->GetName() == "bits")
    {
        INVALID_INPUT_IF(Element->HasAttribute("index") == true, "A bits interpretation must not have an index attribute.");
        INVALID_INPUT_IF(Element->HasAttribute("begin-index") == false, "A bits interpretation must have a begin-index attribute.");
        INVALID_INPUT_IF(Element->HasAttribute("length") == false, "A bits interpretation must have a length attribute.");
        
        auto BeginIndex = Inspection::FromString<std::uint64_t>(Element->GetAttribute("begin-index"));
        
        INVALID_INPUT_IF(BeginIndex.has_value() == false, "The begin-index attribute of a bit interpretation must be an integer.");
        Result->m_BeginIndex = BeginIndex.value();
        
        auto Length = Inspection::FromString<std::uint64_t>(Element->GetAttribute("length"));
        
        INVALID_INPUT_IF(Length.has_value() == false, "The length attribute of a bit interpretation must be an integer.");
        Result->m_Length = Length.value();
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
        Result->m_DataVerification = Inspection::TypeDefinition::DataVerification::SetOrUnset;
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
