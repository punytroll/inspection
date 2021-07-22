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

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "guid.h"

namespace XML
{
	class Element;
}

namespace Inspection
{
	class Type;
	class Value;
	
	namespace TypeDefinition
	{
		class Add;
		class Cast;
		class Divide;
		class Equals;
		class Parameter;
		class Subtract;
		class Value;
		
		enum class DataType
		{
			Unknown,
			Boolean,
			DataReference,
			GUID,
			Nothing,
			Length,
			LengthReference,
			ParameterReference,
			Parameters,
			SinglePrecisionReal,
			String,
			TypeReference,
			UnsignedInteger8Bit,
			UnsignedInteger16Bit,
			UnsignedInteger32Bit,
			UnsignedInteger64Bit
		};
		
		class DataReference
		{
		public:
			class Part
			{
			public:
				enum class Type
				{
					Field,
					Tag
				};
				
				Inspection::TypeDefinition::DataReference::Part::Type Type;
				std::string DetailName;
			};
			
			enum class Root
			{
				Current,
				Type
			};
		public:
			static std::unique_ptr<Inspection::TypeDefinition::DataReference> Load(const XML::Element * Element);
		public:
			Inspection::TypeDefinition::DataReference::Root Root;
			std::vector<Inspection::TypeDefinition::DataReference::Part> Parts;
		private:
			DataReference(void) = default;
			DataReference(const Inspection::TypeDefinition::DataReference & DataReference) = delete;
			DataReference(Inspection::TypeDefinition::DataReference && DataReference) = delete;
			Inspection::TypeDefinition::DataReference & operator=(const Inspection::TypeDefinition::DataReference & DataReference) = delete;
			Inspection::TypeDefinition::DataReference & operator=(Inspection::TypeDefinition::DataReference && DataReference) = delete;
		};
		
		class FieldReference
		{
		public:
			enum class Root
			{
				Current,
				Type
			};
		public:
			static std::unique_ptr<Inspection::TypeDefinition::FieldReference> Load(const XML::Element * Element);
		public:
			Inspection::TypeDefinition::FieldReference::Root Root;
			std::vector<std::string> Parts;
		private:
			FieldReference(void) = default;
			FieldReference(const Inspection::TypeDefinition::FieldReference & FieldReference) = delete;
			FieldReference(Inspection::TypeDefinition::FieldReference && FieldReference) = delete;
			Inspection::TypeDefinition::FieldReference & operator=(const Inspection::TypeDefinition::FieldReference & FieldReference) = delete;
			Inspection::TypeDefinition::FieldReference & operator=(Inspection::TypeDefinition::FieldReference && FieldReference) = delete;
		};
		
		class LengthReference
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::LengthReference> Load(const XML::Element * Element);
		public:
			enum class Name
			{
				Consumed
			};
			
			enum class Root
			{
				Type
			};
		public:
			Inspection::TypeDefinition::LengthReference::Name Name;
			Inspection::TypeDefinition::LengthReference::Root Root;
		private:
			LengthReference(void) = default;
			LengthReference(const Inspection::TypeDefinition::LengthReference & LengthReference) = delete;
			LengthReference(Inspection::TypeDefinition::LengthReference && LengthReference) = delete;
			Inspection::TypeDefinition::LengthReference & operator=(const Inspection::TypeDefinition::LengthReference & LengthReference) = delete;
			Inspection::TypeDefinition::LengthReference & operator=(Inspection::TypeDefinition::LengthReference && LengthReference) = delete;
		};
		
		class ParameterReference
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::ParameterReference> Load(const XML::Element * Element);
		public:
			std::string Name;
		private:
			ParameterReference(void) = default;
			ParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference) = delete;
			ParameterReference(Inspection::TypeDefinition::ParameterReference && ParameterReference) = delete;
			Inspection::TypeDefinition::ParameterReference & operator=(const Inspection::TypeDefinition::ParameterReference & ParameterReference) = delete;
			Inspection::TypeDefinition::ParameterReference & operator=(Inspection::TypeDefinition::ParameterReference && ParameterReference) = delete;
		};
		
		class Parameters
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Parameters> Load(const XML::Element * Element);
		public:
			const std::vector<std::unique_ptr<Inspection::TypeDefinition::Parameter>> & GetParameters(void) const;
		private:
			std::vector<std::unique_ptr<Inspection::TypeDefinition::Parameter>> _Parameters;
		private:
			Parameters(void) = default;
			Parameters(const Inspection::TypeDefinition::Parameters & Parameters) = delete;
			Parameters(Inspection::TypeDefinition::Parameters && Parameters) = delete;
			Inspection::TypeDefinition::Parameters & operator=(const Inspection::TypeDefinition::Parameters & Parameters) = delete;
			Inspection::TypeDefinition::Parameters & operator=(Inspection::TypeDefinition::Parameters && Parameters) = delete;
		};
		
		class TypeReference
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::TypeReference> Load(const XML::Element * TypeReferenceElement);
		public:
			std::vector<std::string> Parts;
		private:
			TypeReference(void) = default;
			TypeReference(const Inspection::TypeDefinition::TypeReference & TypeReference) = delete;
			TypeReference(Inspection::TypeDefinition::TypeReference && TypeReference) = delete;
			Inspection::TypeDefinition::TypeReference & operator=(const Inspection::TypeDefinition::TypeReference & TypeReference) = delete;
			Inspection::TypeDefinition::TypeReference & operator=(Inspection::TypeDefinition::TypeReference && TypeReference) = delete;
		};
		
		class Expression
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Expression> Load(const XML::Element * Element);
			static std::unique_ptr<Inspection::TypeDefinition::Expression> LoadFromWithin(const XML::Element * Element);
		public:
			enum class Type
			{
				Unknown,
				Add,
				Cast,
				Divide,
				Equals,
				Subtract,
				Value
			};
		public:
			virtual ~Expression(void) = default;
		public:
			Inspection::TypeDefinition::Expression::Type Type;
		protected:
			Expression(void);
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
		public:
			std::unique_ptr<Inspection::TypeDefinition::Expression> Summand1;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Summand2;
		private:
			Add(void) = default;
			Add(const Inspection::TypeDefinition::Add & Add) = delete;
			Add(Inspection::TypeDefinition::Add && Add) = delete;
			Inspection::TypeDefinition::Add & operator=(const Inspection::TypeDefinition::Add & Add) = delete;
			Inspection::TypeDefinition::Add & operator=(Inspection::TypeDefinition::Add && Add) = delete;
		};
		
		class Cast : public Inspection::TypeDefinition::Expression
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Cast> Load(const XML::Element * Element);
		public:
			virtual ~Cast(void) = default;
		public:
			Inspection::TypeDefinition::DataType DataType;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Expression;
		private:
			Cast(void);
			Cast(const Inspection::TypeDefinition::Cast & Cast) = delete;
			Cast(Inspection::TypeDefinition::Cast && Cast) = delete;
			Inspection::TypeDefinition::Cast & operator=(const Inspection::TypeDefinition::Cast & Cast) = delete;
			Inspection::TypeDefinition::Cast & operator=(Inspection::TypeDefinition::Cast && Cast) = delete;
		};
		
		class Divide : public Inspection::TypeDefinition::Expression
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Divide> Load(const XML::Element * Element);
		public:
			virtual ~Divide(void) = default;
		public:
			std::unique_ptr<Inspection::TypeDefinition::Expression> Dividend;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Divisor;
		private:
			Divide(void) = default;
			Divide(const Inspection::TypeDefinition::Divide & Divide) = delete;
			Divide(Inspection::TypeDefinition::Divide && Divide) = delete;
			Inspection::TypeDefinition::Divide & operator=(const Inspection::TypeDefinition::Divide & Divide) = delete;
			Inspection::TypeDefinition::Divide & operator=(Inspection::TypeDefinition::Divide && Divide) = delete;
		};
		
		class Equals : public Inspection::TypeDefinition::Expression
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Equals> Load(const XML::Element * Element);
		public:
			virtual ~Equals(void) = default;
		public:
			std::unique_ptr<Inspection::TypeDefinition::Expression> Expression1;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Expression2;
		private:
			Equals(void) = default;
			Equals(const Inspection::TypeDefinition::Equals & Equals) = delete;
			Equals(Inspection::TypeDefinition::Equals && Equals) = delete;
			Inspection::TypeDefinition::Equals & operator=(const Inspection::TypeDefinition::Equals & Equals) = delete;
			Inspection::TypeDefinition::Equals & operator=(Inspection::TypeDefinition::Equals && Equals) = delete;
		};
		
		class Subtract : public Inspection::TypeDefinition::Expression
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Subtract> Load(const XML::Element * Element);
		public:
			virtual ~Subtract(void) = default;
		public:
			std::unique_ptr<Inspection::TypeDefinition::Expression> Minuend;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Subtrahend;
		private:
			Subtract(void) = default;
			Subtract(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
			Subtract(Inspection::TypeDefinition::Subtract && Subtract) = delete;
			Inspection::TypeDefinition::Subtract & operator=(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
			Inspection::TypeDefinition::Subtract & operator=(Inspection::TypeDefinition::Subtract && Subtract) = delete;
		};
		
		class Length;
		
		class Value : public Inspection::TypeDefinition::Expression
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Value> Load(const XML::Element * Element);
		public:
			virtual ~Value(void) = default;
		public:
			Inspection::TypeDefinition::DataType DataType;
			std::variant<bool, std::unique_ptr<Inspection::TypeDefinition::DataReference>, Inspection::GUID, std::unique_ptr<Inspection::TypeDefinition::Length>, std::unique_ptr<Inspection::TypeDefinition::LengthReference>, std::unique_ptr<Inspection::TypeDefinition::ParameterReference>, std::unique_ptr<Inspection::TypeDefinition::Parameters>, float, std::string, std::unique_ptr<Inspection::TypeDefinition::TypeReference>, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> Data;
		protected:
			Value(void);
		private:
			Value(const Inspection::TypeDefinition::Value & Value) = delete;
			Value(Inspection::TypeDefinition::Value && Value) = delete;
			Inspection::TypeDefinition::Value & operator=(const Inspection::TypeDefinition::Value & Value) = delete;
			Inspection::TypeDefinition::Value & operator=(Inspection::TypeDefinition::Value && Value) = delete;
		};
		
		class Length
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Length> Load(const XML::Element * Element);
		public:
			std::unique_ptr<Inspection::TypeDefinition::Expression> Bytes;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Bits;
		private:
			Length(void) = default;
			Length(const Inspection::TypeDefinition::Length & Length) = delete;
			Length(Inspection::TypeDefinition::Length && Length) = delete;
			Inspection::TypeDefinition::Length & operator=(const Inspection::TypeDefinition::Length & Length) = delete;
			Inspection::TypeDefinition::Length & operator=(Inspection::TypeDefinition::Length && Length) = delete;
		};
		
		class Parameter
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Parameter> Load(const XML::Element * Element);
		public:
			std::string Name;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Expression;
		private:
			Parameter(void) = default;
			Parameter(const Inspection::TypeDefinition::Parameter & Parameter) = delete;
			Parameter(Inspection::TypeDefinition::Parameter && Parameter) = delete;
			Inspection::TypeDefinition::Parameter & operator=(const Inspection::TypeDefinition::Parameter & Parameter) = delete;
			Inspection::TypeDefinition::Parameter & operator=(Inspection::TypeDefinition::Parameter && Parameter) = delete;
		};
		
		class Tag
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Tag> Load(const XML::Element * Element);
		public:
			std::string Name;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Expression;
		private:
			Tag(void) = default;
			Tag(const Inspection::TypeDefinition::Tag & Tag) = delete;
			Tag(Inspection::TypeDefinition::Tag && Tag) = delete;
			Inspection::TypeDefinition::Tag & operator=(const Inspection::TypeDefinition::Tag & Tag) = delete;
			Inspection::TypeDefinition::Tag & operator=(Inspection::TypeDefinition::Tag && Tag) = delete;
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
		
		class ApplyEnumeration
		{
		public:
			std::unique_ptr<Inspection::TypeDefinition::Enumeration> Enumeration;
		};
		
		class Interpretation
		{
		public:
			static std::unique_ptr<Inspection::TypeDefinition::Interpretation> Load(const XML::Element * Element);
		public:
			enum class Type
			{
				ApplyEnumeration
			};
		public:
			std::optional<Inspection::TypeDefinition::ApplyEnumeration> ApplyEnumeration;
			Inspection::TypeDefinition::Interpretation::Type Type;
		private:
			Interpretation(void) = default;
			Interpretation(const Inspection::TypeDefinition::Interpretation & Interpretation) = delete;
			Interpretation(Inspection::TypeDefinition::Interpretation && Interpretation) = delete;
			Inspection::TypeDefinition::Interpretation & operator=(const Inspection::TypeDefinition::Interpretation & Interpretation) = delete;
			Inspection::TypeDefinition::Interpretation & operator=(Inspection::TypeDefinition::Interpretation && Interpretation) = delete;
		};
		
		class Array
		{
		public:
			enum class IterateType
			{
				AtLeastOneUntilFailureOrLength,
				ForEachField,
				NumberOfElements,
				UntilFailureOrLength
			};
			
			Inspection::TypeDefinition::Array::IterateType IterateType;
			std::unique_ptr<Inspection::TypeDefinition::FieldReference> IterateForEachField;
			std::unique_ptr<Inspection::TypeDefinition::Expression> IterateNumberOfElements;
			std::optional<std::string> ElementName;
			std::unique_ptr<Inspection::TypeDefinition::Parameters> ElementParameters;
			std::unique_ptr<Inspection::TypeDefinition::TypeReference> ElementType;
		};
		
		class Part
		{
		public:
			enum class Type
			{
				Alternative,
				Array,
				Field,
				Fields,
				Forward,
				Sequence,
				Type
			};
			
			Part(void)
			{
			}
				
			Part(Inspection::TypeDefinition::Part && Part) = default;
			
			Part(const Inspection::TypeDefinition::Part & Part) = delete;
			
			std::optional<Inspection::TypeDefinition::Array> Array;
			std::unique_ptr<Inspection::TypeDefinition::Parameters> Parameters;
			std::optional<std::string> FieldName;
			std::unique_ptr<Inspection::TypeDefinition::TypeReference> TypeReference;
			std::unique_ptr<Inspection::TypeDefinition::Interpretation> Interpretation;
			std::unique_ptr<Inspection::TypeDefinition::Expression> Length;
			std::optional<std::vector<Inspection::TypeDefinition::Part>> Parts;
			std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> Tags;
			Inspection::TypeDefinition::Part::Type Type;
			std::vector<std::unique_ptr<Inspection::TypeDefinition::Expression>> Verifications;
		};
		
		Inspection::TypeDefinition::DataType GetDataTypeFromString(const std::string & String);
	}
}

#endif
