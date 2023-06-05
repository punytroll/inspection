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

#include <length.h>
#include <result.h>

#include "../execution_context.h"
#include "alternative.h"

Inspection::TypeDefinition::Alternative::Alternative(void) :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Alternative}
{
}

auto Inspection::TypeDefinition::Alternative::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto FoundAlternative = false;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    for(auto PartIterator = std::begin(Parts); ((FoundAlternative == false) && (PartIterator != std::end(Parts))); ++PartIterator)
    {
        auto const & Part = *PartIterator;
        auto PartReader = std::unique_ptr<Inspection::Reader>{};
        
        if(Part->Length != nullptr)
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<Inspection::Length const &>(Part->Length->GetAny(ExecutionContext)));
        }
        else
        {
            PartReader = std::make_unique<Inspection::Reader>(Reader);
        }
        
        auto PartParameters = Part->GetParameters(ExecutionContext);
        auto PartResult = Part->Get(ExecutionContext, *PartReader, PartParameters);
        
        FoundAlternative = PartResult->GetSuccess();
        if(FoundAlternative == true)
        {
            switch(Part->GetPartType())
            {
            case Inspection::TypeDefinition::PartType::Alternative:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Array:
                {
                    ASSERTION(Part->FieldName.has_value() == true);
                    Result->GetValue()->AppendField(Part->FieldName.value(), PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Field:
                {
                    ASSERTION(Part->FieldName.has_value() == true);
                    Result->GetValue()->AppendField(Part->FieldName.value(), PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Fields:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Forward:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Select:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Sequence:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Type:
                {
                    IMPOSSIBLE_CODE_REACHED("a Type should not be possible inside an Alternative");
                }
            }
            Reader.AdvancePosition(PartReader->GetConsumedLength());
        }
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(FoundAlternative);
    
    return Result;
}

auto Inspection::TypeDefinition::Alternative::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Alternative>
{
    return std::unique_ptr<Inspection::TypeDefinition::Alternative>{new Inspection::TypeDefinition::Alternative{}};
} 
