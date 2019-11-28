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

#include <vector>

namespace Inspection
{
	class Type;
	class Value;
	
	namespace TypeDefinition
	{
		class Cast;
		class Divide;
		class Equals;
		class Parameter;
		class Value;
		
		enum class DataType
		{
			Unknown,
			Boolean,
			DataReference,
			TypeReference,
			Nothing,
			Length,
			ParameterReference,
			Parameters,
			SinglePrecisionReal,
			String,
			UnsignedInteger8Bit,
			UnsignedInteger16Bit,
			UnsignedInteger32Bit,
			UnsignedInteger64Bit
		};
		
		class DataReference
		{
		public:
			class PartDescriptor
			{
			public:
				enum class Type
				{
					Field,
					Tag
				};
				
				Inspection::TypeDefinition::DataReference::PartDescriptor::Type Type;
				std::string DetailName;
			};
			
			enum class Root
			{
				Current,
				Type
			};
			
			Inspection::TypeDefinition::DataReference::Root Root;
			std::vector< Inspection::TypeDefinition::DataReference::PartDescriptor > PartDescriptors;
		};
		
		class ParameterReference
		{
		public:
			std::string Name;
		};
		
		class Parameters
		{
		public:
			std::vector< Inspection::TypeDefinition::Parameter > Parameters;
		};
		
		class TypeReference
		{
		public:
			std::vector< std::string > Parts;
		};
		
		class Statement
		{
		public:
			enum class Type
			{
				Unknown,
				Cast,
				Divide,
				Equals,
				Value
			};
			
			Statement(void) :
				Type{Inspection::TypeDefinition::Statement::Type::Unknown},
				Cast{nullptr},
				Divide{nullptr},
				Equals{nullptr},
				Value{nullptr}
			{
			}
			
			Statement(const Inspection::TypeDefinition::Statement & Statement) = delete;
			
			Statement(Inspection::TypeDefinition::Statement && Statement) :
				Type{Statement.Type},
				Cast{Statement.Cast},
				Divide{Statement.Divide},
				Equals{Statement.Equals},
				Value{Statement.Value}
			{
				Statement.Cast = nullptr;
				Statement.Divide = nullptr;
				Statement.Equals = nullptr;
				Statement.Value = nullptr;
			}
			
			~Statement(void);
			
			Inspection::TypeDefinition::Statement::Type Type;
			// content depending on type
			Inspection::TypeDefinition::Cast * Cast;
			Inspection::TypeDefinition::Divide * Divide;
			Inspection::TypeDefinition::Equals * Equals;
			Inspection::TypeDefinition::Value * Value;
		};
		
		class Cast
		{
		public:
			Cast(void) :
				DataType{Inspection::TypeDefinition::DataType::Unknown}
			{
			}
			
			Cast(Inspection::TypeDefinition::Cast && Cast) = default;
			
			Cast(const Inspection::TypeDefinition::Cast & Cast) = delete;
			
			Inspection::TypeDefinition::DataType DataType;
			Inspection::TypeDefinition::Statement Statement;
		};
		
		class Divide
		{
		public:
			Divide(void)
			{
			}
			
			Divide(Inspection::TypeDefinition::Divide && Divide) = default;
			
			Divide(const Inspection::TypeDefinition::Divide & Divide) = delete;
			
			Inspection::TypeDefinition::Statement Dividend;
			Inspection::TypeDefinition::Statement Divisor;
		};
		
		class Equals
		{
		public:
			Equals(void)
			{
			}
			
			Equals(Inspection::TypeDefinition::Equals && Equals) = default;
			
			Equals(const Inspection::TypeDefinition::Equals & Equals) = delete;
			
			Inspection::TypeDefinition::Statement Statement1;
			Inspection::TypeDefinition::Statement Statement2;
		};
		
		class Length
		{
		public:
			Length(void)
			{
			}
			
			Length(Inspection::TypeDefinition::Length && Length) = default;
			
			Length(const Inspection::TypeDefinition::Length & Length) = delete;
			
			Inspection::TypeDefinition::Statement Bytes;
			Inspection::TypeDefinition::Statement Bits;
		};
		
		class Value
		{
		public:
			Inspection::TypeDefinition::DataType DataType;
			std::experimental::optional< bool > Boolean;
			std::experimental::optional< Inspection::TypeDefinition::DataReference > DataReference;
			std::experimental::optional< Inspection::TypeDefinition::Length > Length;
			std::experimental::optional< Inspection::TypeDefinition::ParameterReference > ParameterReference;
			std::experimental::optional< Inspection::TypeDefinition::Parameters > Parameters;
			std::experimental::optional< float > SinglePrecisionReal;
			std::experimental::optional< std::string > String;
			std::experimental::optional< Inspection::TypeDefinition::TypeReference > TypeReference;
			std::experimental::optional< std::uint8_t > UnsignedInteger8Bit;
			std::experimental::optional< std::uint16_t > UnsignedInteger16Bit;
			std::experimental::optional< std::uint32_t > UnsignedInteger32Bit;
			std::experimental::optional< std::uint64_t > UnsignedInteger64Bit;
		};
		
		class Parameter
		{
		public:
			Parameter(void)
			{
			}
			
			Parameter(Inspection::TypeDefinition::Parameter && Parameter) = default;
			
			Parameter(const Inspection::TypeDefinition::Parameter & Parameter) = delete;
			
			std::string Name;
			Inspection::TypeDefinition::Statement Statement;
		};
		
		class Tag
		{
		public:
			Tag(void)
			{
			}
			
			Tag(Inspection::TypeDefinition::Tag && Tag) = default;
			
			Tag(const Inspection::TypeDefinition::Tag & Tag) = delete;
			
			std::string Name;
			Inspection::TypeDefinition::Statement Statement;
		};
			
		Statement::~Statement(void)
		{
			delete Cast;
			delete Equals;
			delete Value;
		}
		
		class Enumeration
		{
		public:
			class Element
			{
			public:
				std::string BaseValue;
				std::vector< Inspection::TypeDefinition::Tag > Tags;
				bool Valid;
			};
			Inspection::TypeDefinition::DataType BaseDataType;
			std::vector< Inspection::TypeDefinition::Enumeration::Element > Elements;
			std::experimental::optional< Inspection::TypeDefinition::Enumeration::Element > FallbackElement;
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
			
			std::experimental::optional< Inspection::TypeDefinition::ApplyEnumeration > ApplyEnumeration;
			Inspection::TypeDefinition::Interpretation::Type Type;
		};
		
		class Part
		{
		public:
			enum class Type
			{
				Field,
				Fields,
				Forward,
				Sequence
			};
			
			Part(void)
			{
			}
				
			Part(Inspection::TypeDefinition::Part && Part) = default;
			
			Part(const Inspection::TypeDefinition::Part & Part) = delete;
			
			std::experimental::optional< Inspection::TypeDefinition::Parameters > Parameters;
			std::experimental::optional< std::string > FieldName;
			std::experimental::optional< Inspection::TypeDefinition::TypeReference > TypeReference;
			std::experimental::optional< Inspection::TypeDefinition::Interpretation > Interpretation;
			std::experimental::optional< Inspection::TypeDefinition::Length > Length;
			std::experimental::optional< std::vector< Inspection::TypeDefinition::Part > > Parts;
			std::vector< Inspection::TypeDefinition::Tag > Tags;
			Inspection::TypeDefinition::Part::Type Type;
			std::vector< Inspection::TypeDefinition::Statement > Verifications;
		};
		
		Inspection::TypeDefinition::DataType GetDataTypeFromString(const std::string & String)
		{
			if(String == "boolean")
			{
				return Inspection::TypeDefinition::DataType::Boolean;
			}
			else if(String == "data-reference")
			{
				return Inspection::TypeDefinition::DataType::DataReference;
			}
			else if(String == "nothing")
			{
				return Inspection::TypeDefinition::DataType::Nothing;
			}
			else if(String == "parameter-reference")
			{
				return Inspection::TypeDefinition::DataType::ParameterReference;
			}
			else if(String == "parameters")
			{
				return Inspection::TypeDefinition::DataType::Parameters;
			}
			else if(String == "single-precision-real")
			{
				return Inspection::TypeDefinition::DataType::SinglePrecisionReal;
			}
			else if(String == "string")
			{
				return Inspection::TypeDefinition::DataType::String;
			}
			else if(String == "type-reference")
			{
				return Inspection::TypeDefinition::DataType::TypeReference;
			}
			else if((String == "unsigned integer 8bit") || (String == "unsigned-integer-8bit"))
			{
				return Inspection::TypeDefinition::DataType::UnsignedInteger8Bit;
			}
			else if((String == "unsigned integer 16bit") || (String == "unsigned-integer-16bit"))
			{
				return Inspection::TypeDefinition::DataType::UnsignedInteger16Bit;
			}
			else if((String == "unsigned integer 32bit") || (String == "unsigned-integer-32bit"))
			{
				return Inspection::TypeDefinition::DataType::UnsignedInteger32Bit;
			}
			else if((String == "unsigned integer 64bit") || (String == "unsigned-integer-64bit"))
			{
				return Inspection::TypeDefinition::DataType::UnsignedInteger64Bit;
			}
			else
			{
				return Inspection::TypeDefinition::DataType::Unknown;
			}
		}
	}
}

#endif
