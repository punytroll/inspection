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

#include <execution_context.h>
#include <reader.h>
#include <result.h>

#include "../getter_part_adapter.h"
#include "part_type.h"
#include "sequence.h"

Inspection::TypeDefinition::Sequence::Sequence() :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Sequence}
{
}

auto Inspection::TypeDefinition::Sequence::Get(Inspection::ExecutionContext & ExecutionContext) const -> void
{
    auto Continue = true;
    
    for(auto PartIterator = std::begin(GetParts()); ((Continue == true) && (PartIterator != std::end(GetParts()))); ++PartIterator)
    {
        auto const & Part = *PartIterator;
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(Part->HasLength() == true)
        {
            PartReader = std::make_unique<Inspection::Reader>(ExecutionContext.GetCurrentReader(), std::any_cast<Inspection::Length const &>(Part->GetLengthAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(ExecutionContext.GetCurrentReader());
        }
        
        auto PartParameters = Part->GetParameters(ExecutionContext);
        auto PartResult = ExecutionContext.Call(Inspection::GetterPartAdapter{*Part}, *PartReader, PartParameters);
        
        Continue = PartResult->GetSuccess();
        m_AddPartResult(ExecutionContext.GetCurrentResult(), *Part, PartResult.get());
        ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Continue = ApplyInterpretations(ExecutionContext, ExecutionContext.GetCurrentResult().GetValue());
    }
    // finalization
    ExecutionContext.GetCurrentResult().SetSuccess(Continue);
}

auto Inspection::TypeDefinition::Sequence::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Sequence>
{
    return std::unique_ptr<Inspection::TypeDefinition::Sequence>{new Inspection::TypeDefinition::Sequence{}};
}
