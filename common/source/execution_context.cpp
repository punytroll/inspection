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

#include <any>
#include <cassert>
#include <list>
#include <unordered_map>

#include "execution_context.h"
#include "length.h"
#include "result.h"
#include "type.h"
#include "value.h"

Inspection::ExecutionContext::Element::Element(const Inspection::TypeDefinition::Part & Part, Inspection::Result & Result, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) :
	_Parameters{Parameters},
	_Part{Part},
	_Reader{Reader},
	_Result{Result}
{
}

Inspection::ExecutionContext::ExecutionContext(const Inspection::TypeDefinition::Type & Type) :
	_Type{Type}
{
}

void Inspection::ExecutionContext::Push(const Inspection::TypeDefinition::Part & Part, Inspection::Result & Result, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
	_ExecutionStack.emplace_back(Part, Result, Reader, Parameters);
}

void Inspection::ExecutionContext::Pop(void)
{
	_ExecutionStack.pop_back();
}

Inspection::Result & Inspection::ExecutionContext::GetTopLevelResult(void) const
{
	return _ExecutionStack.front()._Result;
}

Inspection::Length Inspection::ExecutionContext::CalculateLengthFromReference(const Inspection::TypeDefinition::LengthReference & LengthReference)
{
	assert(LengthReference.GetRoot() == Inspection::TypeDefinition::LengthReference::Root::Type);
	assert(LengthReference.GetName() == Inspection::TypeDefinition::LengthReference::Name::Consumed);
	assert(_ExecutionStack.size() >= 2);
	
	// Although the first element in the stack is the "type", its length is the last to be modified (right at the end).
	// We expect at least the second element to be present at use its length. (might be a sequence)
	return std::next(std::begin(_ExecutionStack))->_Reader.GetConsumedLength();
}

Inspection::Value * Inspection::ExecutionContext::GetValueFromDataReference(const Inspection::TypeDefinition::DataReference & DataReference)
{
	auto Result = static_cast<Inspection::Value *>(nullptr);
	
	assert(_ExecutionStack.size() > 0);
	switch(DataReference.GetRoot())
	{
	case Inspection::TypeDefinition::DataReference::Root::Current:
		{
			Result = _GetValueFromDataReferenceFromCurrent(DataReference.Parts, _ExecutionStack.back()._Result.GetValue());
			
			break;
		}
	case Inspection::TypeDefinition::DataReference::Root::Type:
		{
			auto ExecutionStackIterator = std::begin(_ExecutionStack);
			
			Result = ExecutionStackIterator->_Result.GetValue();
			
			auto PartIterator = std::begin(DataReference.Parts);
			
			while(PartIterator != std::end(DataReference.Parts))
			{
				switch(PartIterator->GetType())
				{
				case Inspection::TypeDefinition::DataReference::Part::Type::Field:
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
							Result = ExecutionStackIterator->_Result.GetValue();
							switch(ExecutionStackIterator->_Part.Type)
							{
							case Inspection::TypeDefinition::Part::Type::Alternative:
								{
									assert(false);
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Array:
								{
									assert(false);
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Field:
								{
									if(Result->HasField(PartIterator->DetailName) == true)
									{
										Result = Result->GetField(PartIterator->DetailName);
										++PartIterator;
									}
									
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
									if(Result->HasField(PartIterator->DetailName) == true)
									{
										Result = Result->GetField(PartIterator->DetailName);
										++PartIterator;
									}
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Sequence:
								{
									if(Result->HasField(PartIterator->DetailName) == true)
									{
										Result = Result->GetField(PartIterator->DetailName);
										++PartIterator;
									}
									
									break;
								}
							case Inspection::TypeDefinition::Part::Type::Type:
								{
									// skipped intentionally
									assert(false);
									
									break;
								}
							default:
								{
									assert(false);
								}
							}
						}
						
						break;
					}
				case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
					{
						// we are looking for a tag
						Result = Result->GetTag(PartIterator->DetailName);
						++PartIterator;
						
						break;
					}
				}
			}
			
			break;
		}
	default:
		{
			assert(false);
		}
	}
	
	return Result;
}

Inspection::Value * Inspection::ExecutionContext::GetFieldFromFieldReference(const Inspection::TypeDefinition::FieldReference & FieldReference)
{
	auto Result = static_cast<Inspection::Value *>(nullptr);
	auto ExecutionStackIterator = std::list<Inspection::ExecutionContext::Element>::iterator{};
	
	switch(FieldReference.GetRoot())
	{
	case Inspection::TypeDefinition::FieldReference::Root::Current:
		{
			assert(_ExecutionStack.size() > 0);
			ExecutionStackIterator = std::prev(std::end(_ExecutionStack));
			
			break;
		}
	case Inspection::TypeDefinition::FieldReference::Root::Type:
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
	Result = ExecutionStackIterator->_Result.GetValue();
	
	auto PartIterator = std::begin(FieldReference.Parts);
	
	while(PartIterator != std::end(FieldReference.Parts))
	{
		// maybe, the field is already in the result
		if(Result->HasField(*PartIterator) == true)
		{
			Result = Result->GetField(*PartIterator);
			++PartIterator;
		}
		// if not, the field might be in the current stack
		else
		{
			++ExecutionStackIterator;
			Result = ExecutionStackIterator->_Result.GetValue();
			switch(ExecutionStackIterator->_Part.Type)
			{
			case Inspection::TypeDefinition::Part::Type::Alternative:
				{
					assert(false);
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Array:
				{
					assert(false);
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Field:
				{
					if(Result->HasField(*PartIterator) == true)
					{
						Result = Result->GetField(*PartIterator);
						++PartIterator;
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Fields:
				{
					// fields are flattened onto the parent, behaving just like a sequence
					if(Result->HasField(*PartIterator) == true)
					{
						Result = Result->GetField(*PartIterator);
						++PartIterator;
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Forward:
				{
					assert(false);
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Sequence:
				{
					if(Result->HasField(*PartIterator) == true)
					{
						Result = Result->GetField(*PartIterator);
						++PartIterator;
					}
					
					break;
				}
			case Inspection::TypeDefinition::Part::Type::Type:
				{
					// skipped intentionally
					
					break;
				}
			default:
				{
					assert(false);
				}
			}
		}
	}
	
	return Result;
}

const std::any & Inspection::ExecutionContext::GetAnyReferenceFromParameterReference(const Inspection::TypeDefinition::ParameterReference & ParameterReference)
{
	auto ExecutionStackIterator = std::rbegin(_ExecutionStack);
	
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

std::unordered_map<std::string, std::any> Inspection::ExecutionContext::GetAllParameters(void)
{
	auto Result = std::unordered_map<std::string, std::any>{};
	
	for(auto ExecutionStackElement : _ExecutionStack)
	{
		Result.insert(std::begin(ExecutionStackElement._Parameters), std::end(ExecutionStackElement._Parameters));
	}
	
	return Result;
}

std::uint32_t Inspection::ExecutionContext::GetExecutionStackSize(void) const
{
	return _ExecutionStack.size();
}

Inspection::Value * Inspection::ExecutionContext::_GetValueFromDataReferenceFromCurrent(const std::vector<Inspection::TypeDefinition::DataReference::Part> & Parts, Inspection::Value * Current)
{
	auto Result = Current;
	auto EndIterator = std::end(Parts);
	
	for(auto PartIterator = std::begin(Parts); (Result != nullptr) && (PartIterator != EndIterator); ++PartIterator)
	{
		switch(PartIterator->GetType())
		{
		case Inspection::TypeDefinition::DataReference::Part::Type::Field:
			{
				// we are looking for a field
				Result = Result->GetField(PartIterator->DetailName);
				
				break;
			}
		case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
			{
				// we are looking for a tag
				Result = Result->GetTag(PartIterator->DetailName);
				
				break;
			}
		}
	}
	
	return Result;
}
