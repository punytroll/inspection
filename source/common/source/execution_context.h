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

#include <any>
#include <list>
#include <unordered_map>

#include "type_definition/data_reference.h"

namespace Inspection
{
    class Length;
    class Reader;
    class Result;
    class TypeRepository;
    class Value;
    
    namespace TypeDefinition
    {
        class FieldReference;
        class ParameterReference;
        class Part;
    }
    
    class ExecutionContext
    {
    private:
        class Element
        {
        public:
            Element(Inspection::TypeDefinition::Part const & Part, Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters);
            
            std::unordered_map<std::string, std::any> const & m_Parameters;
            Inspection::TypeDefinition::Part const & m_Part;
            Inspection::Reader & m_Reader;
            Inspection::Result & m_Result;
        };
    public:
        ExecutionContext(Inspection::TypeRepository & TypeRepository);
        void Push(Inspection::TypeDefinition::Part const & Part, Inspection::Result & Result, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters);
        void Pop();
        Inspection::Result & GetTopLevelResult() const;
        Inspection::Value * GetValueFromDataReference(Inspection::TypeDefinition::DataReference const & DataReference);
        Inspection::Value * GetFieldFromFieldReference(Inspection::TypeDefinition::FieldReference const & FieldReference);
        const std::any & GetAnyReferenceFromParameterReference(Inspection::TypeDefinition::ParameterReference const & ParameterReference);
        std::unordered_map<std::string, std::any> GetAllParameters();
        std::uint32_t GetExecutionStackSize() const;
        Inspection::TypeRepository & GetTypeRepository();
    private:
        Inspection::Value * m_GetValueFromDataReferenceFromCurrent(std::vector<Inspection::TypeDefinition::DataReference::Part> const & Parts, Inspection::Value * Current);
        std::list<Inspection::ExecutionContext::Element> m_ExecutionStack;
        Inspection::TypeRepository & m_TypeRepository;
    };
}

#endif
