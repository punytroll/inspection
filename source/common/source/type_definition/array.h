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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ARRAY_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ARRAY_H

#include <memory>
#include <string>

#include "../type_definition.h"

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    class Value;
    
    namespace TypeDefinition
    {
        class Array : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Array>;
        public:
            enum class IterateType
            {
                AtLeastOneUntilFailureOrLength,
                ForEachField,
                NumberOfElements,
                UntilFailureOrLength,
                UntilLength
            };
        public:
            ~Array() override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
            auto GetElementParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>;
        public:
            Inspection::TypeDefinition::Array::IterateType IterateType;
            std::unique_ptr<Inspection::TypeDefinition::FieldReference> IterateForEachField;
            std::unique_ptr<Inspection::TypeDefinition::Expression> IterateNumberOfElements;
            std::optional<std::string> ElementName;
            std::unique_ptr<Inspection::TypeDefinition::Parameters> ElementParameters;
        protected:
            auto _LoadProperty(XML::Element const * Element) -> void override;
        private:
            Array(void);
            Array(Inspection::TypeDefinition::Array const & Array) = delete;
            Array(Inspection::TypeDefinition::Array && Array) = delete;
            auto operator=(Inspection::TypeDefinition::Array const & Array) -> Inspection::TypeDefinition::Array & = delete;
            auto operator=(Inspection::TypeDefinition::Array && Array) -> Inspection::TypeDefinition::Array & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_ElementType;
        };
    }
}

#endif
