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

#include "type_definition/expression.h"
#include "type_definition/parameters.h"
#include "type_definition/type_reference.h"

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    class Reader;
    class Result;
    class Type;
    class Value;
    
    namespace TypeDefinition
    {
        class Type;
        
        enum class PartType
        {
            Alternative,
            Array,
            Field,
            Forward,
            Select,
            Sequence,
            Type
        };
        
        class Tag;
        
        class Enumeration
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Enumeration> Load(const XML::Element * Element);
        public:
            class Element
            {
            public:
                std::string BaseValue;
                std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> Tags;
                bool Valid;
            };
        public:
            Inspection::TypeDefinition::DataType BaseDataType;
            std::vector<Inspection::TypeDefinition::Enumeration::Element> Elements;
            std::optional<Inspection::TypeDefinition::Enumeration::Element> FallbackElement;
        private:
            Enumeration(void) = default;
            Enumeration(const Inspection::TypeDefinition::Enumeration & Enumeration) = delete;
            Enumeration(Inspection::TypeDefinition::Enumeration && Enumeration) = delete;
            Inspection::TypeDefinition::Enumeration & operator=(const Inspection::TypeDefinition::Enumeration & Enumeration) = delete;
            Inspection::TypeDefinition::Enumeration & operator=(Inspection::TypeDefinition::Enumeration && Enumeration) = delete;
        };
        
        class Interpretation
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Interpretation> Load(const XML::Element * Element);
        public:
            virtual ~Interpretation(void) = default;
            virtual bool Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const = 0;
        protected:
            Interpretation(void) = default;
        private:
            Interpretation(const Inspection::TypeDefinition::Interpretation & Interpretation) = delete;
            Interpretation(Inspection::TypeDefinition::Interpretation && Interpretation) = delete;
            Inspection::TypeDefinition::Interpretation & operator=(const Inspection::TypeDefinition::Interpretation & Interpretation) = delete;
            Inspection::TypeDefinition::Interpretation & operator=(Inspection::TypeDefinition::Interpretation && Interpretation) = delete;
        };
        
        class AddTag : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::AddTag> Load(const XML::Element * Element);
        public:
            virtual ~AddTag(void) = default;
            bool Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const override;
        private:
            AddTag(void) = default;
            AddTag(const Inspection::TypeDefinition::AddTag & AddTag) = delete;
            AddTag(Inspection::TypeDefinition::AddTag && AddTag) = delete;
            Inspection::TypeDefinition::AddTag & operator=(const Inspection::TypeDefinition::AddTag & AddTag) = delete;
            Inspection::TypeDefinition::AddTag & operator=(Inspection::TypeDefinition::AddTag && AddTag) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Tag> m_Tag;
        };
        
        class ApplyEnumeration : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration> Load(const XML::Element * Element);
        public:
            virtual ~ApplyEnumeration(void) = default;
            bool Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const override;
        public:
            std::unique_ptr<Inspection::TypeDefinition::Enumeration> Enumeration;
        private:
            ApplyEnumeration(void) = default;
            ApplyEnumeration(const Inspection::TypeDefinition::ApplyEnumeration & ApplyEnumeration) = delete;
            ApplyEnumeration(Inspection::TypeDefinition::ApplyEnumeration && ApplyEnumeration) = delete;
            Inspection::TypeDefinition::ApplyEnumeration & operator=(const Inspection::TypeDefinition::ApplyEnumeration & ApplyEnumeration) = delete;
            Inspection::TypeDefinition::ApplyEnumeration & operator=(Inspection::TypeDefinition::ApplyEnumeration && ApplyEnumeration) = delete;
        };
        
        class BitsInterpretation : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::BitsInterpretation>;
        public:
            enum class DataVerification
            {
                Set,
                SetOrUnset,
                Unset
            };
        public:
            auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool override;
            auto GetAsDataType() const -> Inspection::TypeDefinition::DataType;
            auto GetBeginIndex(void) const -> std::uint64_t;
            auto GetDataVerification() const -> Inspection::TypeDefinition::BitsInterpretation::DataVerification;
            auto GetInterpretations(void) const -> std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> const &;
            auto GetLength(void) const -> std::uint64_t;
            auto GetName(void) const -> std::string const &;
        private:
            BitsInterpretation(void) = default;
            BitsInterpretation(Inspection::TypeDefinition::BitsInterpretation const & BitsInterpretation) = delete;
            BitsInterpretation(Inspection::TypeDefinition::BitsInterpretation && BitsInterpretation) = delete;
            auto operator=(Inspection::TypeDefinition::BitsInterpretation const & BitsInterpretation) -> Inspection::TypeDefinition::BitsInterpretation & = delete;
            auto operator=(Inspection::TypeDefinition::BitsInterpretation && BitsInterpretation) -> Inspection::TypeDefinition::BitsInterpretation & = delete;
        private:
            Inspection::TypeDefinition::DataType m_AsDataType;
            std::uint64_t m_BeginIndex;
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> m_Interpretations;
            std::uint64_t m_Length;
            Inspection::TypeDefinition::BitsInterpretation::DataVerification m_DataVerification;
            std::string m_Name;
        };
        
        class Verification : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Verification> Load(const XML::Element * Element);
        public:
            virtual ~Verification(void) = default;
            bool Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const override;
        private:
            Verification(void) = default;
            Verification(const Inspection::TypeDefinition::Verification & Verification) = delete;
            Verification(Inspection::TypeDefinition::Verification && Verification) = delete;
            Inspection::TypeDefinition::Verification & operator=(const Inspection::TypeDefinition::Verification & Verification) = delete;
            Inspection::TypeDefinition::Verification & operator=(Inspection::TypeDefinition::Verification && Verification) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression;
        };
        
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
        
        auto GetDataTypeFromString(std::string const & String) -> Inspection::TypeDefinition::DataType;
        auto GetDataVerificationFromString(std::string_view String) -> Inspection::TypeDefinition::BitsInterpretation::DataVerification;
    }
}

#endif
