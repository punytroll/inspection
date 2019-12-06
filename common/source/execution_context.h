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
			Element(const Inspection::TypeDefinition::Part & Part, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters);
			Element(const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters);
			
			const std::unordered_map< std::string, std::experimental::any > & _Parameters;
			const Inspection::TypeDefinition::Part * _Part;
			const std::unique_ptr< Inspection::Result > & _Result;
		private:
			Element(const Inspection::TypeDefinition::Part * Part, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		};
	public:
		void Push(const Inspection::TypeDefinition::Part & Part, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		void Push(const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		void Pop(void);
		const std::unique_ptr< Inspection::Result > & GetTopLevelResult(void) const;
		std::shared_ptr< Inspection::Value > GetValueFromDataReference(const Inspection::TypeDefinition::DataReference & DataReference);
		std::shared_ptr< Inspection::Value > GetFieldFromFieldReference(const Inspection::TypeDefinition::FieldReference & FieldReference);
		const std::experimental::any & GetAnyReferenceFromParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference);
		std::unordered_map< std::string, std::experimental::any > GetAllParameters(void);
		std::uint32_t GetExecutionStackSize(void) const;
	private:
		std::list< Inspection::ExecutionContext::Element > _ExecutionStack;
	};
}

#endif
