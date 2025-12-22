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
#include <common/reader.h>
#include <common/result.h>
#include <common/value.h>

#include "field.h"
#include "part_type.h"
#include "type.h"

Inspection::TypeDefinition::Field::Field() :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Field}
{
}

auto Inspection::TypeDefinition::Field::Get(Inspection::ExecutionContext & ExecutionContext) const -> void
{
    auto Continue = true;
    
    if(HasTypeReference() == true)
    {
        auto const & FieldType = GetTypeFromTypeReference(ExecutionContext);
        auto FieldResult = FieldType.Get(ExecutionContext, ExecutionContext.GetCurrentReader(), ExecutionContext.GetAllParameters());
        
        Continue = FieldResult->GetSuccess();
        ExecutionContext.GetCurrentResult().GetValue()->Extend(FieldResult->ExtractValue());
    }
    else
    {
        ASSERTION(GetParts().size() == 1);
        
        auto const & Part = GetParts().front();
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
        
        Continue = PartResult->GetSuccess();
        ExecutionContext.GetCurrentResult().GetValue()->Extend(PartResult->ExtractValue());
        ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Continue = ApplyInterpretations(ExecutionContext, ExecutionContext.GetCurrentResult().GetValue());
    }
    ExecutionContext.GetCurrentResult().SetSuccess(Continue);
}

auto Inspection::TypeDefinition::Field::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    
    ExecutionContext.Push(*Result, Reader, Parameters);
    Get(ExecutionContext);
    ExecutionContext.Pop();
    
    return Result;
}

auto Inspection::TypeDefinition::Field::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Field>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Field>{new Inspection::TypeDefinition::Field{}};
    
    if(Element->HasAttribute("name") == true)
    {
        Result->m_FieldName = Element->GetAttribute("name");
    }
    
    return Result;
}
