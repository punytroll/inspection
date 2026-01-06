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

#include <common/execution_context.h>
#include <common/length.h>
#include <common/result.h>

#include "alternative.h"
#include "part_type.h"

Inspection::TypeDefinition::Alternative::Alternative(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Alternative}
{
}

auto Inspection::TypeDefinition::Alternative::Get(Inspection::ExecutionContext & ExecutionContext) const -> void
{
    auto FoundAlternative = false;
    
    for(auto const & Part  : GetParts())
    {
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
        auto PartResult = std::make_unique<Inspection::Result>();
        
        ExecutionContext.Push(*PartResult, *PartReader, PartParameters);
        Part->Get(ExecutionContext);
        FoundAlternative = PartResult->GetSuccess();
        if(FoundAlternative == true)
        {
            auto PartName = Part->GetName(ExecutionContext);
            
            ExecutionContext.Pop();
            m_AddPartResult(ExecutionContext, PartName, PartResult.get());
            ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
            
            break;
        }
        else
        {
            ExecutionContext.Pop();
        }
    }
    // finalization
    ExecutionContext.GetCurrentResult().SetSuccess(FoundAlternative);
}

auto Inspection::TypeDefinition::Alternative::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Alternative>
{
    return std::unique_ptr<Inspection::TypeDefinition::Alternative>{new Inspection::TypeDefinition::Alternative{}};
} 
