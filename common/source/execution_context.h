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
			Element(const Inspection::TypeDefinition::Part & Part, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters) :
				Inspection::ExecutionContext::Element(&Part, Result, Parameters)
			{
			}
			
			Element(const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters) :
				Inspection::ExecutionContext::Element(nullptr, Result, Parameters)
			{
			}
			
			const std::unordered_map< std::string, std::experimental::any > _Parameters;
			const Inspection::TypeDefinition::Part * _Part;
			const std::unique_ptr< Inspection::Result > & _Result;
		private:
			Element(const Inspection::TypeDefinition::Part * Part, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters) :
				_Parameters{Parameters},
				_Part{Part},
				_Result{Result}
			{
			}
		};
	public:
		void Push(const Inspection::TypeDefinition::Part & Part, const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			_ExecutionStack.emplace_back(Part, Result, Parameters);
		}
		
		void Push(const std::unique_ptr< Inspection::Result > & Result, const std::unordered_map< std::string, std::experimental::any > & Parameters)
		{
			_ExecutionStack.emplace_back(Result, Parameters);
		}
		
		void Pop(void)
		{
			_ExecutionStack.pop_back();
		}
		
		const std::unique_ptr< Inspection::Result > & GetTopLevelResult(void) const
		{
			return _ExecutionStack.front()._Result;
		}
		
		std::shared_ptr< Inspection::Value > GetValueFromDataReference(const Inspection::TypeDefinition::DataReference & DataReference)
		{
			std::shared_ptr< Inspection::Value > Result;
			std::list< Inspection::ExecutionContext::Element >::iterator ExecutionStackIterator;
			
			switch(DataReference.Root)
			{
			case Inspection::TypeDefinition::DataReference::Root::Current:
				{
					assert(_ExecutionStack.size() > 0);
					ExecutionStackIterator = std::prev(std::end(_ExecutionStack));
					
					break;
				}
			case Inspection::TypeDefinition::DataReference::Root::Type:
				{
					assert(_ExecutionStack.size() > 0);
					ExecutionStackIterator = std::begin(_ExecutionStack);
					
					break;
				}
			default:
				{
					assert(false);
				}
			}
			Result = ExecutionStackIterator->_Result->GetValue();
			
			auto PartIterator{std::begin(DataReference.PartDescriptors)};
			
			while(PartIterator != std::end(DataReference.PartDescriptors))
			{
				switch(PartIterator->Type)
				{
				case Inspection::TypeDefinition::DataReference::PartDescriptor::Type::Field:
					{
						// we are looking for a field
						// maybe, the field is already in the result
						if(Result->HasField(PartIterator->DetailName) == true)
						{
							Result = Result->GetField(PartIterator->DetailName);
							++PartIterator;
						}
						// if not, the field might be in the current stack
						else
						{
							++ExecutionStackIterator;
							Result = ExecutionStackIterator->_Result->GetValue();
							assert(ExecutionStackIterator->_Part != nullptr);
							switch(ExecutionStackIterator->_Part->Type)
							{
							case Inspection::TypeDefinition::Part::Type::Sequence:
								{
									if(Result->HasField(PartIterator->DetailName) == true)
									{
										Result = Result->GetField(PartIterator->DetailName);
										++PartIterator;
									}
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Field:
								{
									assert(false);
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Fields:
								{
									// fields are flattened onto the parent, behaving just like a sequence
									if(Result->HasField(PartIterator->DetailName) == true)
									{
										Result = Result->GetField(PartIterator->DetailName);
										++PartIterator;
									}
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Forward:
								{
									assert(false);
									
									break;
								}
							}
						}
						
						break;
					}
				case Inspection::TypeDefinition::DataReference::PartDescriptor::Type::Tag:
					{
						// we are looking for a tag
						Result = Result->GetTag(PartIterator->DetailName);
						++PartIterator;
						
						break;
					}
				}
			}
			
			return Result;
		}
		
		const std::experimental::any & GetAnyReferenceFromParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference)
		{
			auto ExecutionStackIterator{std::rbegin(_ExecutionStack)};
			
			while(ExecutionStackIterator != std::rend(_ExecutionStack))
			{
				auto ParameterIterator{ExecutionStackIterator->_Parameters.find(ParameterReference.Name)};
				
				if(ParameterIterator != ExecutionStackIterator->_Parameters.end())
				{
					return ParameterIterator->second;
				}
				else
				{
					++ExecutionStackIterator;
				}
			}
			throw std::runtime_error{"Could not find named parameter \"" + ParameterReference.Name + "\"."};
		}
		
		std::unordered_map< std::string, std::experimental::any > GetAllParameters(void)
		{
			std::unordered_map< std::string, std::experimental::any > Result;
			
			for(auto ExecutionStackElement : _ExecutionStack)
			{
				Result.insert(std::begin(ExecutionStackElement._Parameters), std::end(ExecutionStackElement._Parameters));
			}
			
			return Result;
		}
		
		std::list< Inspection::ExecutionContext::Element > _ExecutionStack;
	};
}

#endif
