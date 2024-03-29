/**
 * Copyright (C) 2023  Hagen M�bius
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
#include <length.h>
#include <result.h>

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
        auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
        
        FoundAlternative = PartResult->GetSuccess();
        if(FoundAlternative == true)
        {
            m_AddPartResult(ExecutionContext.GetCurrentResult(), *Part, PartResult.get());
            ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
            
            break;
        }
    }
    // finalization
    ExecutionContext.GetCurrentResult().SetSuccess(FoundAlternative);
}

auto Inspection::TypeDefinition::Alternative::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    
    ExecutionContext.Push(*Result, Reader, Parameters);
    Get(ExecutionContext);
    ExecutionContext.Pop();
    
    return Result;
}

auto Inspection::TypeDefinition::Alternative::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Alternative>
{
    return std::unique_ptr<Inspection::TypeDefinition::Alternative>{new Inspection::TypeDefinition::Alternative{}};
} 
