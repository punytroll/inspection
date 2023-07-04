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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__PART_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__PART_H

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    class Reader;
    class Result;
    class Value;
    class Type;
    
    namespace TypeDefinition
    {
        class Expression;
        class Interpretation;
        class Parameters;
        enum class PartType;
        class TypeReference;
        
        class Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Part>;
        public:
            Part(Inspection::TypeDefinition::PartType Type);
            virtual ~Part();
            auto ApplyInterpretations(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool;
            virtual auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> = 0;
            auto GetFieldName() const -> std::string const &;
            auto GetLengthAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any;
            auto GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>;
            auto GetParts() const -> std::vector<std::unique_ptr<Inspection::TypeDefinition::Part>> const &;
            auto GetPartType() const -> Inspection::TypeDefinition::PartType;
            auto GetTypeFromTypeReference(Inspection::ExecutionContext & ExecutionContext) const -> Inspection::Type const &;
            auto HasFieldName() const -> bool;
            auto HasLength() const -> bool;
            auto HasTypeReference() const -> bool;
        protected:
            auto _LoadProperties(XML::Element const * Element) -> void;
            virtual auto _LoadProperty(XML::Element const * Element) -> void;
            auto m_AddPartResult(Inspection::Result * Result, Inspection::TypeDefinition::Part const & Part, Inspection::Result * PartResult) const -> void;
            std::optional<std::string> m_FieldName;
        private:
            Part(Inspection::TypeDefinition::Part const & Part) = delete;
            Part(Inspection::TypeDefinition::Part && Part) = delete;
            auto operator=(Inspection::TypeDefinition::Part const & Part) -> Inspection::TypeDefinition::Part & = delete;
            auto operator=(Inspection::TypeDefinition::Part && Part) -> Inspection::TypeDefinition::Part & = delete;
        private:
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> m_Interpretations;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Length;
            std::unique_ptr<Inspection::TypeDefinition::Parameters> m_Parameters;
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Part>> m_Parts;
            Inspection::TypeDefinition::PartType m_PartType;
            std::unique_ptr<Inspection::TypeDefinition::TypeReference> m_TypeReference;
        };
    }
}

#endif
