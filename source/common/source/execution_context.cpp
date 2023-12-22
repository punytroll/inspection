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

#include <assertion.h>
#include <execution_context.h>
#include <getter_functor_adapter.h>
#include <i_getter_adapter.h>
#include <length.h>
#include <result.h>
#include <type.h>
#include <type_repository.h>
#include <value.h>

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

auto Inspection::ExecutionContext::Call(std::function<void (Inspection::ExecutionContext &)> GetterFunctor, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>
{
    auto ExecutionContext = Inspection::ExecutionContext{Inspection::g_TypeRepository};
    
    return ExecutionContext.Call(Inspection::GetterFunctorAdapter{GetterFunctor}, Reader, Parameters);
}

auto Inspection::ExecutionContext::Call(Inspection::IGetterAdapter const & GetterAdapter, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    
    m_Stack.emplace_back(*Result, Reader, Parameters);
    GetterAdapter(*this);
    m_Stack.pop_back();
    
    return Result;
}

auto Inspection::ExecutionContext::GetCurrentParameters() const -> std::unordered_map<std::string, std::any> const &
{
    ASSERTION(m_Stack.size() > 0);
    
    return m_Stack.back().GetParameters();
}

auto Inspection::ExecutionContext::GetCurrentReader() -> Inspection::Reader &
{
    ASSERTION(m_Stack.size() > 0);
    
    return m_Stack.back().GetReader();
}

auto Inspection::ExecutionContext::GetCurrentResult() -> Inspection::Result &
{
    ASSERTION(m_Stack.size() > 0);
    
    return m_Stack.back().GetResult();
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

auto Inspection::ExecutionContext::GetTypeRepository() -> Inspection::TypeRepository &
{
    return m_TypeRepository;
}
