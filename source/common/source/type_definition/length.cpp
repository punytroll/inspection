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

#include <any>

#include <xml_puny_dom/xml_puny_dom.h>

#include <assertion.h>
#include <length.h>

#include "../internal_output_operators.h"
#include "data_type.h"
#include "length.h"

auto Inspection::TypeDefinition::Length::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_Bytes != nullptr);
    ASSERTION(m_Bits != nullptr);
    
    auto BytesAny = m_Bytes->GetAny(ExecutionContext);
    
    ASSERTION(BytesAny.has_value() == true);
    INVALID_INPUT_IF(BytesAny.type() != typeid(std::uint64_t), "The \"bytes\" specification of a \"length\" must be an unsigned integer 64bit.");
    
    auto BitsAny = m_Bits->GetAny(ExecutionContext);
    
    ASSERTION(BitsAny.has_value() == true);
    INVALID_INPUT_IF(BitsAny.type() != typeid(std::uint64_t), "The \"bits\" specification of a \"length\" must be an unsigned integer 64bit.");
    
    return Inspection::Length{std::any_cast<std::uint64_t const &>(BytesAny), std::any_cast<std::uint64_t const &>(BitsAny)};
}

auto Inspection::TypeDefinition::Length::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return Inspection::TypeDefinition::DataType::Length;
}

auto Inspection::TypeDefinition::Length::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Length>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Length>{new Inspection::TypeDefinition::Length{}};
    
    for(auto const & ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "bytes")
        {
            INVALID_INPUT_IF(Result->m_Bytes != nullptr, "More than one bytes expression.");
            Result->m_Bytes = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
        }
        else if(ChildElement->GetName() == "bits")
        {
            INVALID_INPUT_IF(Result->m_Bits != nullptr, "More than one bits expression.");
            Result->m_Bits = Inspection::TypeDefinition::Expression::LoadFromWithin(ChildElement);
        }
        else
        {
            UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
        }
    }
    INVALID_INPUT_IF(Result->m_Bytes == nullptr, "A \"length\" needs a \"bytes\" specification.");
    INVALID_INPUT_IF(Result->m_Bits == nullptr, "A \"length\" needs a \"bits\" specification.");
    
    return Result;
}
