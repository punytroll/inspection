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
#include <list>
#include <unordered_map>

#include "type_definition.h"

namespace Inspection
{
	class Length;
	class Reader;
	class Result;
	class TypeRepository;
	class Value;
	
	namespace TypeDefinition
	{
		class Type;
	}
	
	class ExecutionContext
	{
	private:
		class Element
		{
		public:
			Element(const Inspection::TypeDefinition::Part & Part, Inspection::Result & Result, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters);
			
			const std::unordered_map<std::string, std::any> & _Parameters;
			const Inspection::TypeDefinition::Part & _Part;
			Inspection::Reader & _Reader;
			Inspection::Result & _Result;
		};
	public:
		ExecutionContext(const Inspection::TypeDefinition::Type & Type, Inspection::TypeRepository & TypeRepository);
		void Push(const Inspection::TypeDefinition::Part & Part, Inspection::Result & Result, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters);
		void Pop(void);
		Inspection::Result & GetTopLevelResult(void) const;
		Inspection::Length CalculateLengthFromReference(const Inspection::TypeDefinition::LengthReference & LengthReference);
		Inspection::Value * GetValueFromDataReference(const Inspection::TypeDefinition::DataReference & DataReference);
		Inspection::Value * GetFieldFromFieldReference(const Inspection::TypeDefinition::FieldReference & FieldReference);
		const std::any & GetAnyReferenceFromParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference);
		std::unordered_map<std::string, std::any> GetAllParameters(void);
		std::uint32_t GetExecutionStackSize(void) const;
		Inspection::TypeRepository & GetTypeRepository(void);
	private:
		Inspection::Value * _GetValueFromDataReferenceFromCurrent(const std::vector<Inspection::TypeDefinition::DataReference::Part> & Parts, Inspection::Value * Current);
		std::list<Inspection::ExecutionContext::Element> _ExecutionStack;
		const Inspection::TypeDefinition::Type & _Type;
		Inspection::TypeRepository & _TypeRepository;
	};
}

#endif
