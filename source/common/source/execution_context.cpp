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
#include "value.h"

Inspection::ExecutionContext::Element::Element(Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) :
    m_Parameters{Parameters},
    m_Reader{Reader},
    m_Result{Result}
{
}

auto Inspection::ExecutionContext::Element::GetParameters() const -> std::unordered_map<std::string, std::any> const &
{
    return m_Parameters;
}

auto Inspection::ExecutionContext::Element::GetReader() -> Inspection::Reader &
{
    return m_Reader;
}

auto Inspection::ExecutionContext::Element::GetReader() const -> Inspection::Reader const &
{
    return m_Reader;
}

auto Inspection::ExecutionContext::Element::GetResult() -> Inspection::Result &
{
    return m_Result;
}

auto Inspection::ExecutionContext::Element::GetResult() const -> Inspection::Result const &
{
    return m_Result;
}

Inspection::ExecutionContext::ExecutionContext(Inspection::TypeRepository & TypeRepository) :
    m_TypeRepository{TypeRepository}
{
}

auto Inspection::ExecutionContext::Push(Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> void
{
    m_Stack.emplace_back(Result, Reader, Parameters);
}

auto Inspection::ExecutionContext::Pop() -> void
{
    m_Stack.pop_back();
}

auto Inspection::ExecutionContext::GetValueFromDataReference(Inspection::TypeDefinition::DataReference const & DataReference) -> Inspection::Value *
{
    auto Result = static_cast<Inspection::Value *>(nullptr);
    
    ASSERTION(m_Stack.size() > 0);
    switch(DataReference.GetRoot())
    {
    case Inspection::TypeDefinition::DataReference::Root::Current:
        {
            Result = m_GetValueFromDataReferenceFromCurrent(DataReference.GetParts(), m_Stack.back().GetResult().GetValue());
            
            break;
        }
    case Inspection::TypeDefinition::DataReference::Root::Type:
        {
            auto ExecutionStackIterator = std::begin(m_Stack);
            
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
                            ASSERTION_MESSAGE(ExecutionStackIterator != std::end(m_Stack), "Could not find the field \"" + PartIterator->GetName() + "\" on the execution stack.");
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

auto Inspection::ExecutionContext::GetParameterAny(std::string const & ParameterName) -> std::any const &
{
    auto ExecutionStackIterator = std::rbegin(m_Stack);
    
    while(ExecutionStackIterator != std::rend(m_Stack))
    {
        auto ParameterIterator = ExecutionStackIterator->GetParameters().find(ParameterName);
        
        if(ParameterIterator != ExecutionStackIterator->GetParameters().end())
        {
            return ParameterIterator->second;
        }
        else
        {
            ++ExecutionStackIterator;
        }
    }
    throw std::runtime_error{"Could not find named parameter \"" + ParameterName + "\"."};
}

auto Inspection::ExecutionContext::GetAllParameters() -> std::unordered_map<std::string, std::any>
{
    auto Result = std::unordered_map<std::string, std::any>{};
    
    for(auto & ExecutionStackElement : m_Stack)
    {
        Result.insert(std::begin(ExecutionStackElement.GetParameters()), std::end(ExecutionStackElement.GetParameters()));
    }
    
    return Result;
}

auto Inspection::ExecutionContext::GetStack() const -> std::list<Inspection::ExecutionContext::Element const *>
{
    auto Result = std::list<Inspection::ExecutionContext::Element const *>{};
    
    for(auto const & StackElement : m_Stack)
    {
        Result.push_back(&StackElement);
    }
    
    return Result;
}

auto Inspection::ExecutionContext::GetStackSize() const -> std::uint32_t
{
    return m_Stack.size();
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
