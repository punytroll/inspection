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

Inspection::ExecutionContext::Element::Element(Inspection::TypeDefinition::Part const & Part, Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) :
    m_Parameters{Parameters},
    m_Part{Part},
    m_Reader{Reader},
    m_Result{Result}
{
}

Inspection::ExecutionContext::ExecutionContext(Inspection::TypeRepository & TypeRepository) :
    m_TypeRepository{TypeRepository}
{
}

void Inspection::ExecutionContext::Push(Inspection::TypeDefinition::Part const & Part, Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters)
{
    m_ExecutionStack.emplace_back(Part, Result, Reader, Parameters);
}

void Inspection::ExecutionContext::Pop()
{
    m_ExecutionStack.pop_back();
}

Inspection::Result & Inspection::ExecutionContext::GetTopLevelResult() const
{
    return m_ExecutionStack.front().m_Result;
}

Inspection::Value * Inspection::ExecutionContext::GetValueFromDataReference(Inspection::TypeDefinition::DataReference const & DataReference)
{
    auto Result = static_cast<Inspection::Value *>(nullptr);
    
    ASSERTION(m_ExecutionStack.size() > 0);
    switch(DataReference.GetRoot())
    {
    case Inspection::TypeDefinition::DataReference::Root::Current:
        {
            Result = m_GetValueFromDataReferenceFromCurrent(DataReference.GetParts(), m_ExecutionStack.back().m_Result.GetValue());
            
            break;
        }
    case Inspection::TypeDefinition::DataReference::Root::Type:
        {
            auto ExecutionStackIterator = std::begin(m_ExecutionStack);
            
            Result = ExecutionStackIterator->m_Result.GetValue();
            
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
                            Result = ExecutionStackIterator->m_Result.GetValue();
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

Inspection::Value * Inspection::ExecutionContext::GetFieldFromFieldReference(Inspection::TypeDefinition::FieldReference const & FieldReference)
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
    Result = ExecutionStackIterator->m_Result.GetValue();
    
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
            Result = ExecutionStackIterator->m_Result.GetValue();
            if(Result->HasField(*PartIterator) == true)
            {
                Result = Result->GetField(*PartIterator);
                ++PartIterator;
            }
        }
    }
    
    return Result;
}

const std::any & Inspection::ExecutionContext::GetAnyReferenceFromParameterReference(Inspection::TypeDefinition::ParameterReference const & ParameterReference)
{
    auto ExecutionStackIterator = std::rbegin(m_ExecutionStack);
    
    while(ExecutionStackIterator != std::rend(m_ExecutionStack))
    {
        auto ParameterIterator = ExecutionStackIterator->m_Parameters.find(ParameterReference.GetName());
        
        if(ParameterIterator != ExecutionStackIterator->m_Parameters.end())
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

std::unordered_map<std::string, std::any> Inspection::ExecutionContext::GetAllParameters()
{
    auto Result = std::unordered_map<std::string, std::any>{};
    
    for(auto ExecutionStackElement : m_ExecutionStack)
    {
        Result.insert(std::begin(ExecutionStackElement.m_Parameters), std::end(ExecutionStackElement.m_Parameters));
    }
    
    return Result;
}

std::uint32_t Inspection::ExecutionContext::GetExecutionStackSize() const
{
    return m_ExecutionStack.size();
}

Inspection::Value * Inspection::ExecutionContext::m_GetValueFromDataReferenceFromCurrent(std::vector<Inspection::TypeDefinition::DataReference::Part> const & Parts, Inspection::Value * Current)
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
