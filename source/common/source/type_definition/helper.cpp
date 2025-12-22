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

#include <common/length.h>
#include <common/value.h>

#include "data_type.h"
#include "data_verification.h"
#include "helper.h"

using namespace std::string_literals;

auto Inspection::TypeDefinition::AppendLengthTag(Inspection::Value * Value, Inspection::Length const & Length, std::string const & LengthName) -> Inspection::Value *
{
    auto Result = Value->AddTag(LengthName, Length);
    
    Result->AddTag("unit", "bytes and bits"s);
    
    return Result;
}

auto Inspection::TypeDefinition::AppendBitLengthTag(Inspection::Value * Value, Inspection::Length const & Length, std::string const & LengthName) -> Inspection::Value *
{
    auto Result = Value->AddTag(LengthName, Length.GetTotalBits());
    
    Result->AddTag("unit", "bits"s);
    
    return Result;
}

auto Inspection::TypeDefinition::AppendLengthField(Inspection::Value * Value, std::string const & FieldName, Inspection::Length const & Length) -> Inspection::Value *
{
    auto Result = Value->AppendField(FieldName);
    
    AppendLengthTag(Result, Length);
    
    return Result;
}

auto Inspection::TypeDefinition::AppendOtherData(Inspection::Value * Value, Inspection::Length const & Length) -> Inspection::Value *
{
    return AppendLengthField(Value, "OtherData", Length);
}

auto Inspection::TypeDefinition::GetBooleanFromString(std::string_view String) -> bool
{
    if(String == "true")
    {
        return true;
    }
    else
    {
        return false;
    }
}

auto Inspection::TypeDefinition::GetDataTypeFromString(std::string_view String) -> Inspection::TypeDefinition::DataType
{
    if(String == "boolean")
    {
        return Inspection::TypeDefinition::DataType::Boolean;
    }
    else if(String == "length")
    {
        return Inspection::TypeDefinition::DataType::Length;
    }
    else if(String == "nothing")
    {
        return Inspection::TypeDefinition::DataType::Nothing;
    }
    else if(String == "parameters")
    {
        return Inspection::TypeDefinition::DataType::Parameters;
    }
    else if(String == "single-precision-real")
    {
        return Inspection::TypeDefinition::DataType::SinglePrecisionReal;
    }
    else if(String == "string")
    {
        return Inspection::TypeDefinition::DataType::String;
    }
    else if(String == "type")
    {
        return Inspection::TypeDefinition::DataType::Type;
    }
    else if((String == "unsigned integer 8bit") || (String == "unsigned-integer-8bit"))
    {
        return Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
    }
    else if((String == "unsigned integer 16bit") || (String == "unsigned-integer-16bit"))
    {
        return Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
    }
    else if((String == "unsigned integer 32bit") || (String == "unsigned-integer-32bit"))
    {
        return Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
    }
    else if((String == "unsigned integer 64bit") || (String == "unsigned-integer-64bit"))
    {
        return Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
    }
    else
    {
        UNEXPECTED_CASE("Data type string == \"" + std::string{String} + '"');
    }
}

auto Inspection::TypeDefinition::GetDataVerificationFromString(std::string_view String) -> Inspection::TypeDefinition::DataVerification
{
    if(String == "set")
    {
        return Inspection::TypeDefinition::DataVerification::Set;
    }
    else if(String == "set or unset")
    {
        return Inspection::TypeDefinition::DataVerification::SetOrUnset;
    }
    else if(String == "unset")
    {
        return Inspection::TypeDefinition::DataVerification::Unset;
    }
    else
    {
        UNEXPECTED_CASE("Data verification string == \"" + std::string{String} + '"');
    }
}
