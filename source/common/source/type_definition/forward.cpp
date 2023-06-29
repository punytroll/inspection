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

#include <reader.h>
#include <result.h>

#include "../execution_context.h"
#include "forward.h"
#include "part_type.h"
#include "type.h"

Inspection::TypeDefinition::Forward::Forward() :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Forward}
{
}

auto Inspection::TypeDefinition::Forward::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    if(HasTypeReference() == true)
    {
        auto const & ForwardType = GetTypeFromTypeReference(ExecutionContext);
        auto ForwardResult = ForwardType.Get(ExecutionContext, Reader, ExecutionContext.GetAllParameters());
        
        Continue = ForwardResult->GetSuccess();
        Result->GetValue()->Extend(ForwardResult->ExtractValue());
        // interpretation
        if(Continue == true)
        {
            Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
        }
    }
    else
    {
        // if there is no type reference, we expect exactly one array in the parts
        // this allows forwarding the array's element to this part -> see WavPack/Blocks
        ASSERTION(GetParts().size() == 1);
        
        auto const & Part = GetParts().front();
        
        ASSERTION(Part->GetPartType() == Inspection::TypeDefinition::PartType::Array);
        
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(Part->HasLength() == true)
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<Inspection::Length const &>(Part->GetLengthAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader);
        }
        
        auto PartParameters = Part->GetParameters(ExecutionContext);
        auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader->GetConsumedLength());
        // interpretation
        if(Continue == true)
        {
            Continue = ApplyInterpretations(ExecutionContext, Result->GetValue());
        }
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Forward::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Forward>
{
    return std::unique_ptr<Inspection::TypeDefinition::Forward>{new Inspection::TypeDefinition::Forward{}};
}
