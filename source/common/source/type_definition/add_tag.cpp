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
#include <value.h>

#include "add_tag.h"
#include "expression.h"
#include "tag.h"

auto Inspection::TypeDefinition::AddTag::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
{
    ASSERTION(Target != nullptr);
    ASSERTION(m_Tag != nullptr);
    Target->AddTag(m_Tag->Get(ExecutionContext));
    
    return true;
}

auto Inspection::TypeDefinition::AddTag::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::AddTag>
{
    ASSERTION(Element != nullptr);
    ASSERTION(Element->GetName() == "tag");
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::AddTag>{new Inspection::TypeDefinition::AddTag{}};
    
    Result->m_Tag = Inspection::TypeDefinition::Tag::Load(Element);
    
    return Result;
}
