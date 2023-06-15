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
#include <type.h>

#include "../execution_context.h"
#include "data_type.h"
#include "type_value.h"

auto Inspection::TypeDefinition::TypeValue::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    m_Type->SetTypeRepository(ExecutionContext.GetTypeRepository());
    
    return const_cast<Inspection::TypeDefinition::Type const *>(m_Type.get());
}

auto Inspection::TypeDefinition::TypeValue::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    return Inspection::TypeDefinition::DataType::Type;
}

auto Inspection::TypeDefinition::TypeValue::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::TypeValue>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::TypeValue>{new Inspection::TypeDefinition::TypeValue{}};
    
    Result->m_Type = Inspection::TypeDefinition::Type::Load(Element, {"<anonymous>"}, nullptr);
    
    return Result;
}
