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
#include <variant>
#include <vector>

#include "guid.h"

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
        
        enum class DataType
        {
            Boolean,
            GUID,
            Nothing,
            Length,
            Parameters,
            SinglePrecisionReal,
            String,
            Type,
            UnsignedInteger8Bit,
            UnsignedInteger16Bit,
            UnsignedInteger32Bit,
            UnsignedInteger64Bit
        };
        
        enum class PartType
        {
            Alternative,
            Array,
            Field,
            Fields,
            Forward,
            Select,
            Sequence,
            Type
        };
        
        class Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Expression> Load(const XML::Element * Element);
            static std::unique_ptr<Inspection::TypeDefinition::Expression> LoadFromWithin(const XML::Element * Element);
        public:
            virtual ~Expression(void) = default;
            virtual std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const = 0;
            virtual Inspection::TypeDefinition::DataType GetDataType(void) const = 0;
        protected:
            Expression(void) = default;
        private:
            Expression(const Inspection::TypeDefinition::Expression & Expression) = delete;
            Expression(Inspection::TypeDefinition::Expression && Expression) = delete;
            Inspection::TypeDefinition::Expression & operator=(const Inspection::TypeDefinition::Expression & Expression) = delete;
            Inspection::TypeDefinition::Expression & operator=(Inspection::TypeDefinition::Expression && Expression) = delete;
        };
        
        class Add : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Add> Load(const XML::Element * Element);
        public:
            virtual ~Add(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Add(void) = default;
            Add(const Inspection::TypeDefinition::Add & Add) = delete;
            Add(Inspection::TypeDefinition::Add && Add) = delete;
            Inspection::TypeDefinition::Add & operator=(const Inspection::TypeDefinition::Add & Add) = delete;
            Inspection::TypeDefinition::Add & operator=(Inspection::TypeDefinition::Add && Add) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Summand1;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Summand2;
        };
        
        class And : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::And> Load(const XML::Element * Element);
        public:
            virtual ~And(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            And(void) = default;
            And(const Inspection::TypeDefinition::And & And) = delete;
            And(Inspection::TypeDefinition::And && And) = delete;
            Inspection::TypeDefinition::And & operator=(const Inspection::TypeDefinition::And & And) = delete;
            Inspection::TypeDefinition::And & operator=(Inspection::TypeDefinition::And && And) = delete;
        private:
            std::list<std::unique_ptr<Inspection::TypeDefinition::Expression>> m_Operands;
        };
        
        class Cast : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Cast> Load(const XML::Element * Element);
        public:
            virtual ~Cast(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
            const Inspection::TypeDefinition::Expression & GetExpression(void) const;
        private:
            Cast(Inspection::TypeDefinition::DataType DataType);
            Cast(const Inspection::TypeDefinition::Cast & Cast) = delete;
            Cast(Inspection::TypeDefinition::Cast && Cast) = delete;
            Inspection::TypeDefinition::Cast & operator=(const Inspection::TypeDefinition::Cast & Cast) = delete;
            Inspection::TypeDefinition::Cast & operator=(Inspection::TypeDefinition::Cast && Cast) = delete;
        private:
            Inspection::TypeDefinition::DataType m_DataType;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression;
        };
        
        class DataReference : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::DataReference> Load(const XML::Element * Element);
        public:
            class Part
            {
            public:
                enum class Type
                {
                    Field,
                    Tag
                };
            public:
                Part(Inspection::TypeDefinition::DataReference::Part::Type Type);
            public:
                Inspection::TypeDefinition::DataReference::Part::Type GetType(void) const;
            public:
                std::string DetailName;
            private:
                Inspection::TypeDefinition::DataReference::Part::Type m_Type;
            };
            
            enum class Root
            {
                Current,
                Type
            };
        public:
            virtual ~DataReference(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
            const std::vector<Inspection::TypeDefinition::DataReference::Part> & GetParts(void) const;
        public:
            Inspection::TypeDefinition::DataReference::Root GetRoot(void) const;
        private:
            DataReference(void) = default;
            DataReference(const Inspection::TypeDefinition::DataReference & DataReference) = delete;
            DataReference(Inspection::TypeDefinition::DataReference && DataReference) = delete;
            Inspection::TypeDefinition::DataReference & operator=(const Inspection::TypeDefinition::DataReference & DataReference) = delete;
            Inspection::TypeDefinition::DataReference & operator=(Inspection::TypeDefinition::DataReference && DataReference) = delete;
        private:
            std::vector<Inspection::TypeDefinition::DataReference::Part> m_Parts;
            Inspection::TypeDefinition::DataReference::Root m_Root;
        };
        
        class Divide : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Divide> Load(const XML::Element * Element);
        public:
            virtual ~Divide(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Divide(void) = default;
            Divide(const Inspection::TypeDefinition::Divide & Divide) = delete;
            Divide(Inspection::TypeDefinition::Divide && Divide) = delete;
            Inspection::TypeDefinition::Divide & operator=(const Inspection::TypeDefinition::Divide & Divide) = delete;
            Inspection::TypeDefinition::Divide & operator=(Inspection::TypeDefinition::Divide && Divide) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Dividend;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Divisor;
        };
        
        class Equals : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Equals> Load(const XML::Element * Element);
        public:
            virtual ~Equals(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Equals(void) = default;
            Equals(const Inspection::TypeDefinition::Equals & Equals) = delete;
            Equals(Inspection::TypeDefinition::Equals && Equals) = delete;
            Inspection::TypeDefinition::Equals & operator=(const Inspection::TypeDefinition::Equals & Equals) = delete;
            Inspection::TypeDefinition::Equals & operator=(Inspection::TypeDefinition::Equals && Equals) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression1;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression2;
        };
        
        class FieldReference : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::FieldReference> Load(const XML::Element * Element);
        public:
            enum class Root
            {
                Current,
                Type
            };
        public:
            virtual ~FieldReference(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        public:
            Inspection::TypeDefinition::FieldReference::Root GetRoot(void) const;
        public:
            std::vector<std::string> Parts;
        private:
            FieldReference(void) = default;
            FieldReference(const Inspection::TypeDefinition::FieldReference & FieldReference) = delete;
            FieldReference(Inspection::TypeDefinition::FieldReference && FieldReference) = delete;
            Inspection::TypeDefinition::FieldReference & operator=(const Inspection::TypeDefinition::FieldReference & FieldReference) = delete;
            Inspection::TypeDefinition::FieldReference & operator=(Inspection::TypeDefinition::FieldReference && FieldReference) = delete;
        private:
            Inspection::TypeDefinition::FieldReference::Root m_Root;
        };
        
        class Length : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Length> Load(const XML::Element * Element);
        public:
            virtual ~Length(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Length(void) = default;
            Length(const Inspection::TypeDefinition::Length & Length) = delete;
            Length(Inspection::TypeDefinition::Length && Length) = delete;
            Inspection::TypeDefinition::Length & operator=(const Inspection::TypeDefinition::Length & Length) = delete;
            Inspection::TypeDefinition::Length & operator=(Inspection::TypeDefinition::Length && Length) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Bytes;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Bits;
        };
        
        class LessThan : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::LessThan> Load(const XML::Element * Element);
        public:
            virtual ~LessThan(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            LessThan(void) = default;
            LessThan(const Inspection::TypeDefinition::LessThan & LessThan) = delete;
            LessThan(Inspection::TypeDefinition::LessThan && LessThan) = delete;
            Inspection::TypeDefinition::LessThan & operator=(const Inspection::TypeDefinition::LessThan & LessThan) = delete;
            Inspection::TypeDefinition::LessThan & operator=(Inspection::TypeDefinition::LessThan && LessThan) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression1;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression2;
        };
        
        class Modulus : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Modulus> Load(const XML::Element * Element);
        public:
            virtual ~Modulus(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Modulus(void) = default;
            Modulus(const Inspection::TypeDefinition::Modulus & Modulus) = delete;
            Modulus(Inspection::TypeDefinition::Modulus && Modulus) = delete;
            Inspection::TypeDefinition::Modulus & operator=(const Inspection::TypeDefinition::Modulus & Modulus) = delete;
            Inspection::TypeDefinition::Modulus & operator=(Inspection::TypeDefinition::Modulus && Modulus) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Dividend;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Divisor;
        };
        
        class Multiply : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Multiply> Load(const XML::Element * Element);
        public:
            virtual ~Multiply(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Multiply(void) = default;
            Multiply(const Inspection::TypeDefinition::Multiply & Multiply) = delete;
            Multiply(Inspection::TypeDefinition::Multiply && Multiply) = delete;
            Inspection::TypeDefinition::Multiply & operator=(const Inspection::TypeDefinition::Multiply & Multiply) = delete;
            Inspection::TypeDefinition::Multiply & operator=(Inspection::TypeDefinition::Multiply && Multiply) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Multiplier;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Multiplicand;
        };
        
        class ParameterReference : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::ParameterReference> Load(const XML::Element * Element);
        public:
            virtual ~ParameterReference(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        public:
            std::string Name;
        private:
            ParameterReference(void) = default;
            ParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference) = delete;
            ParameterReference(Inspection::TypeDefinition::ParameterReference && ParameterReference) = delete;
            Inspection::TypeDefinition::ParameterReference & operator=(const Inspection::TypeDefinition::ParameterReference & ParameterReference) = delete;
            Inspection::TypeDefinition::ParameterReference & operator=(Inspection::TypeDefinition::ParameterReference && ParameterReference) = delete;
        };
        
        class Parameters : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Parameters> Load(const XML::Element * Element);
        public:
            class Parameter
            {
            public:
                static std::unique_ptr<Inspection::TypeDefinition::Parameters::Parameter> Load(const XML::Element * Element);
            public:
                std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const;
                const std::string & GetName(void) const;
            private:
                Parameter(void) = default;
                Parameter(const Inspection::TypeDefinition::Parameters::Parameter & Parameter) = delete;
                Parameter(Inspection::TypeDefinition::Parameters::Parameter && Parameter) = delete;
                Inspection::TypeDefinition::Parameters::Parameter & operator=(const Inspection::TypeDefinition::Parameters::Parameter & Parameter) = delete;
                Inspection::TypeDefinition::Parameters::Parameter & operator=(Inspection::TypeDefinition::Parameters::Parameter && Parameter) = delete;
            private:
                std::string m_Name;
                std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression;
            };
        public:
            virtual ~Parameters(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
            std::unordered_map<std::string, std::any> GetParameters(Inspection::ExecutionContext & ExecutionContext) const;
        private:
            Parameters(void) = default;
            Parameters(const Inspection::TypeDefinition::Parameters & Parameters) = delete;
            Parameters(Inspection::TypeDefinition::Parameters && Parameters) = delete;
            Inspection::TypeDefinition::Parameters & operator=(const Inspection::TypeDefinition::Parameters & Parameters) = delete;
            Inspection::TypeDefinition::Parameters & operator=(Inspection::TypeDefinition::Parameters && Parameters) = delete;
        private:
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Parameters::Parameter>> m_Parameters;
        };
        
        class Subtract : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Subtract> Load(const XML::Element * Element);
        public:
            virtual ~Subtract(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Subtract(void) = default;
            Subtract(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
            Subtract(Inspection::TypeDefinition::Subtract && Subtract) = delete;
            Inspection::TypeDefinition::Subtract & operator=(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
            Inspection::TypeDefinition::Subtract & operator=(Inspection::TypeDefinition::Subtract && Subtract) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Minuend;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Subtrahend;
        };
        
        class TypeReference : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::TypeReference> Load(const XML::Element * Element);
        public:
            virtual ~TypeReference(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
            const Inspection::TypeDefinition::Type * GetType(Inspection::ExecutionContext & ExecutionContext) const;
        private:
            TypeReference(void) = default;
            TypeReference(const Inspection::TypeDefinition::TypeReference & TypeReference) = delete;
            TypeReference(Inspection::TypeDefinition::TypeReference && TypeReference) = delete;
            Inspection::TypeDefinition::TypeReference & operator=(const Inspection::TypeDefinition::TypeReference & TypeReference) = delete;
            Inspection::TypeDefinition::TypeReference & operator=(Inspection::TypeDefinition::TypeReference && TypeReference) = delete;
        private:
            std::vector<std::string> m_Parts;
        };
        
        class TypeValue : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::TypeValue>;
        public:
            virtual ~TypeValue() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            TypeValue() = default;
            TypeValue(Inspection::TypeDefinition::TypeValue const & TypeValue) = delete;
            TypeValue(Inspection::TypeDefinition::TypeValue && TypeValue) = delete;
            auto operator=(Inspection::TypeDefinition::TypeValue const & TypeValue) -> Inspection::TypeDefinition::TypeValue & = delete;
            auto operator=(Inspection::TypeDefinition::TypeValue && TypeValue) -> Inspection::TypeDefinition::TypeValue & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Type> m_Type;
        };
        
        class Value : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Value> Load(const XML::Element * Element);
        public:
            virtual ~Value(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        protected:
            Value(Inspection::TypeDefinition::DataType DataType);
        private:
            Value(const Inspection::TypeDefinition::Value & Value) = delete;
            Value(Inspection::TypeDefinition::Value && Value) = delete;
            Inspection::TypeDefinition::Value & operator=(const Inspection::TypeDefinition::Value & Value) = delete;
            Inspection::TypeDefinition::Value & operator=(Inspection::TypeDefinition::Value && Value) = delete;
        private:
            std::variant<bool, Inspection::GUID, float, std::string, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> m_Data;
            Inspection::TypeDefinition::DataType m_DataType;
        };
        
        class Tag
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Tag> Load(const XML::Element * Element);
        public:
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const;
            const std::string & GetName(void) const;
            bool HasExpression(void) const;
        private:
            Tag(void) = default;
            Tag(const Inspection::TypeDefinition::Tag & Tag) = delete;
            Tag(Inspection::TypeDefinition::Tag && Tag) = delete;
            Inspection::TypeDefinition::Tag & operator=(const Inspection::TypeDefinition::Tag & Tag) = delete;
            Inspection::TypeDefinition::Tag & operator=(Inspection::TypeDefinition::Tag && Tag) = delete;
        private:
            std::string m_Name;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression;
        };
        
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
        
        class BitInterpretation : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::BitInterpretation>;
        public:
            auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool override;
            auto GetIndex(void) const -> std::uint64_t;
            auto GetInterpretations(void) const -> std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> const &;
            auto GetName(void) const -> std::string const &;
        private:
            BitInterpretation(void) = default;
            BitInterpretation(Inspection::TypeDefinition::BitInterpretation const & BitInterpretation) = delete;
            BitInterpretation(Inspection::TypeDefinition::BitInterpretation && BitInterpretation) = delete;
            auto operator=(Inspection::TypeDefinition::BitInterpretation const & BitInterpretation) -> Inspection::TypeDefinition::BitInterpretation & = delete;
            auto operator=(Inspection::TypeDefinition::BitInterpretation && BitInterpretation) -> Inspection::TypeDefinition::BitInterpretation & = delete;
        private:
            std::uint64_t m_Index;
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> m_Interpretations;
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
        private:
            Part(Inspection::TypeDefinition::Part const & Part) = delete;
            Part(Inspection::TypeDefinition::Part && Part) = delete;
            Inspection::TypeDefinition::Part & operator=(Inspection::TypeDefinition::Part const & Part) = delete;
            Inspection::TypeDefinition::Part & operator=(Inspection::TypeDefinition::Part && Part) = delete;
        private:
            Inspection::TypeDefinition::PartType m_PartType;
        };
        
        class Alternative : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Alternative>;
        public:
            ~Alternative(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            Alternative(void);
            Alternative(Inspection::TypeDefinition::Alternative const & Alternative) = delete;
            Alternative(Inspection::TypeDefinition::Alternative && Alternative) = delete;
            auto operator=(Inspection::TypeDefinition::Alternative const & Alternative) -> Inspection::TypeDefinition::Alternative & = delete;
            auto operator=(Inspection::TypeDefinition::Alternative && Alternative) -> Inspection::TypeDefinition::Alternative & = delete;
        };
        
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
            ~Array(void) override = default;
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
        
        class Fields : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Fields>;
        public:
            ~Fields(void) override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            Fields(void);
            Fields(Inspection::TypeDefinition::Fields const & Fields) = delete;
            Fields(Inspection::TypeDefinition::Fields && Fields) = delete;
            auto operator=(Inspection::TypeDefinition::Fields const & Fields) -> Inspection::TypeDefinition::Fields & = delete;
            auto operator=(Inspection::TypeDefinition::Fields && Fields) -> Inspection::TypeDefinition::Fields & = delete;
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
        
        Inspection::TypeDefinition::DataType GetDataTypeFromString(const std::string & String);
    }
}

#endif
