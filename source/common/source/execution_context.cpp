/**
 * inspection
 * Copyright (C) 2019-2022  Hagen Möbius
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
#include <list>
#include <unordered_map>

#include <type_repository.h>

#include "assertion.h"
#include "execution_context.h"
#include "length.h"
#include "result.h"
#include "type.h"
#include "type_definition/field_reference.h"
#include "type_definition/parameter_reference.h"
#include "value.h"

Inspection::ExecutionContext::Element::Element(Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) :
    m_Parameters{Parameters},
    m_Reader{Reader},
    m_Result{Result}
{
}

auto Inspection::ExecutionContext::Element::GetParameters() -> std::unordered_map<std::string, std::any> const &
{
    return m_Parameters;
}

auto Inspection::ExecutionContext::Element::GetReader() -> Inspection::Reader &
{
    return m_Reader;
}

auto Inspection::ExecutionContext::Element::GetResult() -> Inspection::Result &
{
    return m_Result;
}

Inspection::ExecutionContext::ExecutionContext(Inspection::TypeRepository & TypeRepository) :
    m_TypeRepository{TypeRepository}
{
}

auto Inspection::ExecutionContext::Push(Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> void
{
    m_ExecutionStack.emplace_back(Result, Reader, Parameters);
}

auto Inspection::ExecutionContext::Pop() -> void
{
    m_ExecutionStack.pop_back();
}

auto Inspection::ExecutionContext::GetValueFromDataReference(Inspection::TypeDefinition::DataReference const & DataReference) -> Inspection::Value *
{
    auto Result = static_cast<Inspection::Value *>(nullptr);
    
    ASSERTION(m_ExecutionStack.size() > 0);
    switch(DataReference.GetRoot())
    {
    case Inspection::TypeDefinition::DataReference::Root::Current:
        {
            Result = m_GetValueFromDataReferenceFromCurrent(DataReference.GetParts(), m_ExecutionStack.back().GetResult().GetValue());
            
            break;
        }
    case Inspection::TypeDefinition::DataReference::Root::Type:
        {
            auto ExecutionStackIterator = std::begin(m_ExecutionStack);
            
            Result = ExecutionStackIterator->GetResult().GetValue();
            
            auto const & Parts = DataReference.GetParts();
            auto PartIterator = std::begin(Parts);
            auto EndIterator = std::end(Parts);
            
            while(PartIterator != EndIterator)
            {
                switch(PartIterator->GetType())
                {
                case Inspection::TypeDefinition::DataReference::Part::Type::Field:
                    {
                        // we are looking for a field
                        // maybe, the field is already in the result
                        if(Result->HasField(PartIterator->GetName()) == true)
                        {
                            Result = Result->GetField(PartIterator->GetName());
                            ++PartIterator;
                        }
                        // if not, the field might be in the current stack
                        else
                        {
                            ++ExecutionStackIterator;
                            ASSERTION_MESSAGE(ExecutionStackIterator != std::end(m_ExecutionStack), "Could not find the field \"" + PartIterator->GetName() + "\" on the execution stack.");
                            Result = ExecutionStackIterator->GetResult().GetValue();
                            if(Result->HasField(PartIterator->GetName()) == true)
                            {
                                Result = Result->GetField(PartIterator->GetName());
                                ++PartIterator;
                            }
                        }
                        
                        break;
                    }
                case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
                    {
                        // we are looking for a tag
                        Result = Result->GetTag(PartIterator->GetName());
                        ++PartIterator;
                        
                        break;
                    }
                }
            }
            
            break;
        }
    }
    
    return Result;
}

auto Inspection::ExecutionContext::GetFieldFromFieldReference(Inspection::TypeDefinition::FieldReference const & FieldReference) -> Inspection::Value *
{
    auto Result = static_cast<Inspection::Value *>(nullptr);
    auto ExecutionStackIterator = std::list<Inspection::ExecutionContext::Element>::iterator{};
    
    switch(FieldReference.GetRoot())
    {
    case Inspection::TypeDefinition::FieldReference::Root::Current:
        {
            ASSERTION(m_ExecutionStack.size() > 0);
            ExecutionStackIterator = std::prev(std::end(m_ExecutionStack));
            
            break;
        }
    case Inspection::TypeDefinition::FieldReference::Root::Type:
        {
            ASSERTION(m_ExecutionStack.size() > 0);
            ExecutionStackIterator = std::begin(m_ExecutionStack);
            
            break;
        }
    }
    Result = ExecutionStackIterator->GetResult().GetValue();
    
    auto PartIterator = std::begin(FieldReference.GetParts());
    
    while(PartIterator != std::end(FieldReference.GetParts()))
    {
        // maybe, the field is already in the result
        if(Result->HasField(*PartIterator) == true)
        {
            Result = Result->GetField(*PartIterator);
            ++PartIterator;
        }
        // if not, the field might be in the current stack
        else
        {
            ++ExecutionStackIterator;
            Result = ExecutionStackIterator->GetResult().GetValue();
            if(Result->HasField(*PartIterator) == true)
            {
                Result = Result->GetField(*PartIterator);
                ++PartIterator;
            }
        }
    }
    
    return Result;
}

auto Inspection::ExecutionContext::GetAnyReferenceFromParameterReference(Inspection::TypeDefinition::ParameterReference const & ParameterReference) -> std::any const &
{
    auto ExecutionStackIterator = std::rbegin(m_ExecutionStack);
    
    while(ExecutionStackIterator != std::rend(m_ExecutionStack))
    {
        auto ParameterIterator = ExecutionStackIterator->GetParameters().find(ParameterReference.GetName());
        
        if(ParameterIterator != ExecutionStackIterator->GetParameters().end())
        {
            return ParameterIterator->second;
        }
        else
        {
            ++ExecutionStackIterator;
        }
    }
    throw std::runtime_error{"Could not find named parameter \"" + ParameterReference.GetName() + "\"."};
}

auto Inspection::ExecutionContext::GetAllParameters() -> std::unordered_map<std::string, std::any>
{
    auto Result = std::unordered_map<std::string, std::any>{};
    
    for(auto & ExecutionStackElement : m_ExecutionStack)
    {
        Result.insert(std::begin(ExecutionStackElement.GetParameters()), std::end(ExecutionStackElement.GetParameters()));
    }
    
    return Result;
}

auto Inspection::ExecutionContext::GetExecutionStackSize() const -> std::uint32_t
{
    return m_ExecutionStack.size();
}

auto Inspection::ExecutionContext::m_GetValueFromDataReferenceFromCurrent(std::vector<Inspection::TypeDefinition::DataReference::Part> const & Parts, Inspection::Value * Current) -> Inspection::Value *
{
    auto Result = Current;
    auto EndIterator = std::end(Parts);
    
    for(auto PartIterator = std::begin(Parts); (Result != nullptr) && (PartIterator != EndIterator); ++PartIterator)
    {
        switch(PartIterator->GetType())
        {
        case Inspection::TypeDefinition::DataReference::Part::Type::Field:
            {
                // we are looking for a field
                Result = Result->GetField(PartIterator->GetName());
                
                break;
            }
        case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
            {
                // we are looking for a tag
                Result = Result->GetTag(PartIterator->GetName());
                
                break;
            }
        }
    }
    
    return Result;
}

auto Inspection::ExecutionContext::GetTypeRepository() -> Inspection::TypeRepository &
{
    return m_TypeRepository;
}
