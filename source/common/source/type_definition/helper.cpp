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

#include <length.h>
#include <value.h>

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