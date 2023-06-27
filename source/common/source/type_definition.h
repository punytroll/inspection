/**
 * Copyright (C) 2019  Hagen Möbius
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

#ifndef COMMON_TYPE_DEFINITION_H
#define COMMON_TYPE_DEFINITION_H

#include <any>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "type_definition/interpretation.h"

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    class Parameters;
    class Reader;
    class Result;
    class Type;
    class Value;
    
    namespace TypeDefinition
    {
        class Enumeration;
        enum class DataType;
        class Parameters;
        enum class PartType;
        class TypeReference;
        
        class Part
        {
        public:
            static auto Load(const XML::Element * Element) -> std::unique_ptr<Inspection::TypeDefinition::Part>;
        public:
            Part(Inspection::TypeDefinition::PartType Type);
            virtual ~Part(void) = default;
            auto ApplyInterpretations(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool;
            virtual auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> = 0;
            auto GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>;
            auto GetPartType(void) const -> Inspection::TypeDefinition::PartType;
        public:
            std::optional<std::string> FieldName;
            std::unique_ptr<Inspection::TypeDefinition::TypeReference> TypeReference;
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> Interpretations;
            std::unique_ptr<Inspection::TypeDefinition::Expression> Length;
            std::unique_ptr<Inspection::TypeDefinition::Parameters> Parameters;
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Part>> Parts;
        protected:
            auto _LoadProperties(XML::Element const * Element) -> void;
            virtual auto _LoadProperty(XML::Element const * Element) -> void;
            auto m_AddPartResult(Inspection::Result * Result, Inspection::TypeDefinition::Part const & Part, Inspection::Result * PartResult) const -> void;
        private:
            Part(Inspection::TypeDefinition::Part const & Part) = delete;
            Part(Inspection::TypeDefinition::Part && Part) = delete;
            Inspection::TypeDefinition::Part & operator=(Inspection::TypeDefinition::Part const & Part) = delete;
            Inspection::TypeDefinition::Part & operator=(Inspection::TypeDefinition::Part && Part) = delete;
        private:
            Inspection::TypeDefinition::PartType m_PartType;
        };
        
        class Field : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Field>;
        public:
            ~Field(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            Field(void);
            Field(Inspection::TypeDefinition::Field const & Field) = delete;
            Field(Inspection::TypeDefinition::Field && Field) = delete;
            auto operator=(Inspection::TypeDefinition::Field const & Field) -> Inspection::TypeDefinition::Field & = delete;
            auto operator=(Inspection::TypeDefinition::Field && Field) -> Inspection::TypeDefinition::Field & = delete;
        };
        
        class Forward : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Forward>;
        public:
            ~Forward(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            Forward(void);
            Forward(Inspection::TypeDefinition::Forward const & Forward) = delete;
            Forward(Inspection::TypeDefinition::Forward && Forward) = delete;
            auto operator=(Inspection::TypeDefinition::Forward const & Forward) -> Inspection::TypeDefinition::Forward & = delete;
            auto operator=(Inspection::TypeDefinition::Forward && Forward) -> Inspection::TypeDefinition::Forward & = delete;
        };
        
        class Select : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Select>;
        public:
            ~Select(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            class Case
            {
            public:
                static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Select::Case>;
            public:
                auto GetPart() const -> Inspection::TypeDefinition::Part const &;
                auto GetWhen() const -> Inspection::TypeDefinition::Expression const &;
                auto HasPart() const -> bool;
            private:
                Case(void) = default;
                Case(Inspection::TypeDefinition::Select::Case const & Case) = delete;
                Case(Inspection::TypeDefinition::Select::Case && Case) = delete;
                auto operator=(Inspection::TypeDefinition::Select::Case const & Case) -> Inspection::TypeDefinition::Select::Case & = delete;
                auto operator=(Inspection::TypeDefinition::Select::Case && Case) -> Inspection::TypeDefinition::Select::Case & = delete;
            private:
                std::unique_ptr<Inspection::TypeDefinition::Part> m_Part;
                std::unique_ptr<Inspection::TypeDefinition::Expression> m_When;
            };
        protected:
            auto _LoadProperty(XML::Element const * Element) -> void override;
        private:
            Select(void);
            Select(Inspection::TypeDefinition::Select const & Select) = delete;
            Select(Inspection::TypeDefinition::Select && Select) = delete;
            auto operator=(Inspection::TypeDefinition::Select const & Select) -> Inspection::TypeDefinition::Select & = delete;
            auto operator=(Inspection::TypeDefinition::Select && Select) -> Inspection::TypeDefinition::Select & = delete;
        private:
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Select::Case>> m_Cases;
            std::unique_ptr<Inspection::TypeDefinition::Select::Case> m_Else;
        };
        
        class Sequence : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Sequence>;
        public:
            ~Sequence(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            Sequence(void);
            Sequence(Inspection::TypeDefinition::Sequence const & Sequence) = delete;
            Sequence(Inspection::TypeDefinition::Sequence && Sequence) = delete;
            auto operator=(Inspection::TypeDefinition::Sequence const & Sequence) -> Inspection::TypeDefinition::Sequence & = delete;
            auto operator=(Inspection::TypeDefinition::Sequence && Sequence) -> Inspection::TypeDefinition::Sequence & = delete;
        };
        
        class TypePart : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Create() -> std::unique_ptr<Inspection::TypeDefinition::TypePart>;
        public:
            ~TypePart(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            TypePart(void);
            TypePart(Inspection::TypeDefinition::TypePart const & TypePart) = delete;
            TypePart(Inspection::TypeDefinition::TypePart && TypePart) = delete;
            auto operator=(Inspection::TypeDefinition::TypePart const & TypePart) -> Inspection::TypeDefinition::TypePart & = delete;
            auto operator=(Inspection::TypeDefinition::TypePart && TypePart) -> Inspection::TypeDefinition::TypePart & = delete;
        };
    }
}

#endif
