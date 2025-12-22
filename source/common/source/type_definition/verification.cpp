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
#include <common/value.h>

#include "expression.h"
#include "verification.h"

using namespace std::string_literals;

auto Inspection::TypeDefinition::Verification::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool
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

auto Inspection::TypeDefinition::Verification::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Verification>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Verification>{new Inspection::TypeDefinition::Verification{}};
    
    Result->m_Expression = Inspection::TypeDefinition::Expression::Load(Element);
    
    return Result;
}
