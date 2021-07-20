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
			
			Inspection::TypeDefinition::DataReference::Root Root;
			std::vector<Inspection::TypeDefinition::DataReference::Part> Parts;
		};
		
		class FieldReference
		{
		public:
			class Part
			{
			public:
				std::string FieldName;
			};
			
			enum class Root
			{
				Current,
				Type
			};
			
			Inspection::TypeDefinition::FieldReference::Root Root;
			std::vector<Inspection::TypeDefinition::FieldReference::Part> Parts;
		};
		
		class LengthReference
		{
		public:
			enum class Name
			{
				Consumed
			};
			
			enum class Root
			{
				Type
			};
			
			Inspection::TypeDefinition::LengthReference::Name Name;
			Inspection::TypeDefinition::LengthReference::Root Root;
		};
		
		class ParameterReference
		{
		public:
			std::string Name;
		};
		
		class Parameters
		{
		public:
			std::vector<Inspection::TypeDefinition::Parameter> Parameters;
		};
		
		class TypeReference
		{
		public:
			std::vector<std::string> Parts;
		};
		
		class Statement
		{
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
			
			Statement(void);
			Statement(const Inspection::TypeDefinition::Statement & Statement) = delete;
			Statement(Inspection::TypeDefinition::Statement && Statement) = default;
			virtual ~Statement(void);
			Inspection::TypeDefinition::Statement & operator=(Inspection::TypeDefinition::Statement && Statement) = delete;
			Inspection::TypeDefinition::Statement & operator=(const Inspection::TypeDefinition::Statement & Statement) = delete;
			
			Inspection::TypeDefinition::Statement::Type Type;
			// content depending on type
			std::unique_ptr<Inspection::TypeDefinition::Add> Add;
			std::unique_ptr<Inspection::TypeDefinition::Cast> Cast;
			std::unique_ptr<Inspection::TypeDefinition::Divide> Divide;
			std::unique_ptr<Inspection::TypeDefinition::Equals> Equals;
			std::unique_ptr<Inspection::TypeDefinition::Subtract> Subtract;
			std::unique_ptr<Inspection::TypeDefinition::Value> Value;
		};
		
		class Add
		{
		public:
			Add(void);
			Add(Inspection::TypeDefinition::Add && Add) = default;
			Add(const Inspection::TypeDefinition::Add & Add) = delete;
			virtual ~Add(void);
			Inspection::TypeDefinition::Add & operator=(Inspection::TypeDefinition::Add && Add) = delete;
			Inspection::TypeDefinition::Add & operator=(const Inspection::TypeDefinition::Add & Add) = delete;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Summand1;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Summand2;
		};
		
		class Cast
		{
		public:
			Cast(void);
			Cast(Inspection::TypeDefinition::Cast && Cast) = default;
			Cast(const Inspection::TypeDefinition::Cast & Cast) = delete;
			virtual ~Cast(void);
			Inspection::TypeDefinition::Cast & operator=(Inspection::TypeDefinition::Cast && Cast) = delete;
			Inspection::TypeDefinition::Cast & operator=(const Inspection::TypeDefinition::Cast & Cast) = delete;
			Inspection::TypeDefinition::DataType DataType;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Statement;
		};
		
		class Divide
		{
		public:
			Divide(void);
			Divide(Inspection::TypeDefinition::Divide && Divide) = default;
			Divide(const Inspection::TypeDefinition::Divide & Divide) = delete;
			virtual ~Divide(void);
			Inspection::TypeDefinition::Divide & operator=(Inspection::TypeDefinition::Divide && Divide) = delete;
			Inspection::TypeDefinition::Divide & operator=(const Inspection::TypeDefinition::Divide & Divide) = delete;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Dividend;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Divisor;
		};
		
		class Equals
		{
		public:
			Equals(void);
			Equals(Inspection::TypeDefinition::Equals && Equals) = default;
			Equals(const Inspection::TypeDefinition::Equals & Equals) = delete;
			virtual ~Equals(void);
			Inspection::TypeDefinition::Equals & operator=(Inspection::TypeDefinition::Equals && Equals) = delete;
			Inspection::TypeDefinition::Equals & operator=(const Inspection::TypeDefinition::Equals & Equals) = delete;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Statement1;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Statement2;
		};
		
		class Length
		{
		public:
			Length(void);
			Length(Inspection::TypeDefinition::Length && Length) = default;
			Length(const Inspection::TypeDefinition::Length & Length) = delete;
			Inspection::TypeDefinition::Length & operator=(Inspection::TypeDefinition::Length && Length) = delete;
			Inspection::TypeDefinition::Length & operator=(const Inspection::TypeDefinition::Length & Length) = delete;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Bytes;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Bits;
		};
		
		class Subtract
		{
		public:
			Subtract(void);
			Subtract(Inspection::TypeDefinition::Subtract && Subtract) = default;
			Subtract(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
			virtual ~Subtract(void);
			Inspection::TypeDefinition::Subtract & operator=(Inspection::TypeDefinition::Subtract && Subtract) = delete;
			Inspection::TypeDefinition::Subtract & operator=(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Minuend;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Subtrahend;
		};
		
		class Value
		{
		public:
			Inspection::TypeDefinition::DataType DataType;
			std::variant<bool, Inspection::TypeDefinition::DataReference, Inspection::GUID, Inspection::TypeDefinition::Length, Inspection::TypeDefinition::LengthReference, Inspection::TypeDefinition::ParameterReference, Inspection::TypeDefinition::Parameters, float, std::string, Inspection::TypeDefinition::TypeReference, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> Data;
		};
		
		class Parameter
		{
		public:
			Parameter(void);
			Parameter(Inspection::TypeDefinition::Parameter && Parameter) = default;
			Parameter(const Inspection::TypeDefinition::Parameter & Parameter) = delete;
			Inspection::TypeDefinition::Subtract & operator=(Inspection::TypeDefinition::Subtract && Subtract) = delete;
			Inspection::TypeDefinition::Subtract & operator=(const Inspection::TypeDefinition::Subtract & Subtract) = delete;
			std::string Name;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Statement;
		};
		
		class Tag
		{
		public:
			Tag(void);
			Tag(Inspection::TypeDefinition::Tag && Tag) = default;
			Tag(const Inspection::TypeDefinition::Tag & Tag) = delete;
			Inspection::TypeDefinition::Tag & operator=(Inspection::TypeDefinition::Tag && Tag) = delete;
			Inspection::TypeDefinition::Tag & operator=(const Inspection::TypeDefinition::Tag & Tag) = delete;
			std::string Name;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Statement;
		};
		
		class Enumeration
		{
		public:
			class Element
			{
			public:
				std::string BaseValue;
				std::vector<Inspection::TypeDefinition::Tag> Tags;
				bool Valid;
			};
			Inspection::TypeDefinition::DataType BaseDataType;
			std::vector<Inspection::TypeDefinition::Enumeration::Element> Elements;
			std::optional<Inspection::TypeDefinition::Enumeration::Element> FallbackElement;
		};
		
		class ApplyEnumeration
		{
		public:
			Inspection::TypeDefinition::Enumeration Enumeration;
		};
		
		class Interpretation
		{
		public:
			enum class Type
			{
				ApplyEnumeration
			};
			
			std::optional<Inspection::TypeDefinition::ApplyEnumeration> ApplyEnumeration;
			Inspection::TypeDefinition::Interpretation::Type Type;
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
			std::optional<Inspection::TypeDefinition::FieldReference> IterateForEachField;
			std::unique_ptr<Inspection::TypeDefinition::Statement> IterateNumberOfElements;
			std::optional<std::string> ElementName;
			std::optional<Inspection::TypeDefinition::Parameters> ElementParameters;
			Inspection::TypeDefinition::TypeReference ElementType;
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
			std::optional<Inspection::TypeDefinition::Parameters> Parameters;
			std::optional<std::string> FieldName;
			std::optional<Inspection::TypeDefinition::TypeReference> TypeReference;
			std::optional<Inspection::TypeDefinition::Interpretation> Interpretation;
			std::unique_ptr<Inspection::TypeDefinition::Statement> Length;
			std::optional<std::vector<Inspection::TypeDefinition::Part>> Parts;
			std::vector<Inspection::TypeDefinition::Tag> Tags;
			Inspection::TypeDefinition::Part::Type Type;
			std::vector<std::unique_ptr<Inspection::TypeDefinition::Statement>> Verifications;
		};
		
		Inspection::TypeDefinition::DataType GetDataTypeFromString(const std::string & String);
	}
}

#endif
