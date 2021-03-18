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

#ifndef COMMON_EXECUTION_CONTEXT_H
#define COMMON_EXECUTION_CONTEXT_H

#include <any>
#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>

#include "result.h"
#include "type_definition.h"

namespace Inspection
{
	class Result;
	
	class ExecutionContext
	{
	private:
		class Element
		{
		public:
			Element(const Inspection::TypeDefinition::Part & Part, Inspection::Result & Result, Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
			
			const Inspection::TypeDefinition::Part & GetPart(void);
			const std::unordered_map< std::string, std::any > & GetParameters(void);
			Inspection::Reader & GetReader(void);
			Inspection::Result & GetResult(void);
			
			const std::unordered_map< std::string, std::any > & _Parameters;
			const Inspection::TypeDefinition::Part & _Part;
			Inspection::Reader & _Reader;
			Inspection::Result & _Result;
		};
	public:
		void Push(const Inspection::TypeDefinition::Part & Part, Inspection::Result & Result, Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters);
		void Pop(void);
		Inspection::Result & GetTopLevelResult(void) const;
		Inspection::Length CalculateLengthFromReference(const Inspection::TypeDefinition::LengthReference & LengthReference);
		std::shared_ptr< Inspection::Value > GetValueFromDataReference(const Inspection::TypeDefinition::DataReference & DataReference);
		std::shared_ptr< Inspection::Value > GetFieldFromFieldReference(const Inspection::TypeDefinition::FieldReference & FieldReference);
		const std::any & GetAnyReferenceFromParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference);
		std::unordered_map< std::string, std::any > GetAllParameters(void);
		std::uint32_t GetExecutionStackSize(void) const;
	private:
		std::list< Inspection::ExecutionContext::Element > _ExecutionStack;
	};
}

#endif
