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

#ifndef COMMON_EXECUTION_CONTEXT_H
#define COMMON_EXECUTION_CONTEXT_H

#include <cstdint>
#include <any>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace Inspection
{
    class Length;
    class Reader;
    class Result;
    class TypeRepository;
    class Value;
    
    class ExecutionContext
    {
    public:
        class Element
        {
        public:
            Element(Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters);
            auto GetParameters() const -> std::unordered_map<std::string, std::any> const &;
            auto GetReader() -> Inspection::Reader &;
            auto GetReader() const -> Inspection::Reader const &;
            auto GetResult() -> Inspection::Result &;
            auto GetResult() const -> Inspection::Result const &;
        private:
            std::unordered_map<std::string, std::any> const & m_Parameters;
            Inspection::Reader & m_Reader;
            Inspection::Result & m_Result;
        };
    public:
        ExecutionContext(Inspection::TypeRepository & TypeRepository);
        auto Push(Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> void;
        auto Pop() -> void;
        auto GetCurrentParameters() const -> std::unordered_map<std::string, std::any> const &;
        auto GetCurrentReader() -> Inspection::Reader &;
        auto GetCurrentResult() -> Inspection::Result &;
        auto GetParameterAny(std::string const & ParameterName) -> std::any const &;
        auto GetAllParameters() -> std::unordered_map<std::string, std::any>;
        auto GetStack() const -> std::list<Inspection::ExecutionContext::Element const *>;
        auto GetStackSize() const -> std::uint32_t;
        auto GetTypeRepository() -> Inspection::TypeRepository &;
    private:
        std::list<Inspection::ExecutionContext::Element> m_Stack;
        Inspection::TypeRepository & m_TypeRepository;
    };
}

#endif
