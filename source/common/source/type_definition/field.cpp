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
        auto PartResult = std::make_unique<Inspection::Result>();
        
        ExecutionContext.Push(*PartResult, *PartReader, PartParameters);
        Part->Get(ExecutionContext);
        Continue = PartResult->GetSuccess();
        
        auto PartName = Part->GetName(ExecutionContext);
        
        ExecutionContext.Pop();
        m_AddPartResult(ExecutionContext, PartName, PartResult.get());
        ExecutionContext.GetCurrentReader().AdvancePosition(PartReader->GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Continue = ApplyInterpretations(ExecutionContext, ExecutionContext.GetCurrentResult().GetValue());
    }
    ExecutionContext.GetCurrentResult().SetSuccess(Continue);
}

auto Inspection::TypeDefinition::Field::GetName(Inspection::ExecutionContext & ExecutionContext) const -> std::optional<std::string>
{
    if(m_Name != nullptr)
    {
        auto NameAny = m_Name->GetAny(ExecutionContext);
        
        ASSERTION(NameAny.has_value() == true);
        if(NameAny.type() == typeid(std::string))
        {
            return std::any_cast<std::string const &>(NameAny);
        }
        else if(NameAny.type() == typeid(std::nullptr_t))
        {
            return std::string{};
        }
        else
        {
            INVALID_INPUT("The \"name\" of a \"field\" must either be a string or nothing.");
        }
    }
    else
    {
        return std::nullopt;
    }
}

auto Inspection::TypeDefinition::Field::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Field>
{
    return std::unique_ptr<Inspection::TypeDefinition::Field>{new Inspection::TypeDefinition::Field{}};
}

auto Inspection::TypeDefinition::Field::m_LoadProperty(XML::Element const * Element) -> void
{
    if(Element->GetName() == "name")
    {
        m_Name = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
    }
    else
    {
        Inspection::TypeDefinition::Part::m_LoadProperty(Element);
    }
}
