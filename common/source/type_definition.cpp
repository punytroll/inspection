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

#include <cassert>

#include "type_definition.h"

Inspection::TypeDefinition::Add::Add(void)
{
}

Inspection::TypeDefinition::Add::~Add(void)
{
}

Inspection::TypeDefinition::Cast::Cast(void) :
	DataType{Inspection::TypeDefinition::DataType::Unknown}
{
}

Inspection::TypeDefinition::Cast::~Cast(void)
{
}

Inspection::TypeDefinition::Divide::Divide(void)
{
}

Inspection::TypeDefinition::Divide::~Divide(void)
{
}

Inspection::TypeDefinition::Equals::Equals(void)
{
}

Inspection::TypeDefinition::Equals::~Equals(void)
{
}

Inspection::TypeDefinition::Subtract::Subtract(void)
{
}

Inspection::TypeDefinition::Subtract::~Subtract(void)
{
}

Inspection::TypeDefinition::Statement::Statement(void) :
	Type{Inspection::TypeDefinition::Statement::Type::Unknown}
{
}

Inspection::TypeDefinition::Statement::~Statement(void)
{
}

Inspection::TypeDefinition::DataType Inspection::TypeDefinition::GetDataTypeFromString(const std::string & String)
{
	if(String == "boolean")
	{
		return Inspection::TypeDefinition::DataType::Boolean;
	}
	else if(String == "data-reference")
	{
		return Inspection::TypeDefinition::DataType::DataReference;
	}
	else if(String == "length")
	{
		return Inspection::TypeDefinition::DataType::Length;
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
		assert(false);
	}
}
