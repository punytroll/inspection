#ifndef INSPECTION_COMMON_VALUE_H
#define INSPECTION_COMMON_VALUE_H

#include <algorithm>
#include <any>
#include <list>
#include <memory>
#include <stdexcept>

#include "assertion.h"

namespace Inspection
{
	class Value
	{
	public:
		Value(void)
		{
		}
		
		Inspection::Value * AppendField(std::unique_ptr<Inspection::Value> Field)
		{
			auto Result = Field.get();
			
			_Fields.push_back(std::move(Field));
			
			return Result;
		}
		
		Inspection::Value * AppendField(const std::string & Name, std::unique_ptr<Inspection::Value> Field)
		{
			Field->SetName(Name);
			
			return AppendField(std::move(Field));
		}
		
		Inspection::Value * AppendField(const std::string & Name)
		{
			auto Field = std::make_unique<Inspection::Value>();
			
			Field->SetName(Name);
			
			return AppendField(std::move(Field));
		}
		
		template<typename DataType>
		Inspection::Value * AppendField(const std::string & Name, const DataType & Data)
		{
			auto Result = AppendField(Name);
			
			Result->SetData(Data);
			
			return Result;
		}
		
		/**
		 * Extends this Value by moving the fields and tags from the other Value to here.
		 * If the name on the other value is set, the name on this value MUST be empty!
		 * If the data on the other value is set, the data on this value must be empty!
		 **/
		auto Extend(std::unique_ptr<Inspection::Value> Value) -> void
		{
			if(Value->_Data.has_value() == true)
			{
				ASSERTION(_Data.has_value() == false);
				_Data = std::move(Value->_Data);
			}
			if(Value->_Name.empty() == false)
			{
				ASSERTION(_Name.empty() == true);
				_Name = std::move(Value->_Name);
			}
			for(auto & Field : Value->_Fields)
			{
				_Fields.push_back(std::move(Field));
			}
			for(auto & Tag : Value->_Tags)
			{
				_Tags.push_back(std::move(Tag));
			}
		}
		
		Inspection::Value * AddTag(std::unique_ptr<Inspection::Value> Tag)
		{
			auto Result = Tag.get();
			
			_Tags.push_back(std::move(Tag));
			
			return Result;
		}
		
		Inspection::Value * AddTag(const std::string & Name)
		{
			auto Tag = std::make_unique<Inspection::Value>();
			
			Tag->SetName(Name);
			
			return AddTag(std::move(Tag));
		}
		
		template<typename DataType>
		Inspection::Value * AddTag(const std::string & Name, const DataType & Data)
		{
			auto Tag = AddTag(Name);
			
			Tag->SetData(Data);
			
			return Tag;
		}
		
		const std::any & GetData(void) const
		{
			return _Data;
		}
		
		std::uint32_t GetFieldCount(void) const
		{
			return _Fields.size();
		}
		
		const std::string & GetName(void) const
		{
			return _Name;
		}
		
		const std::list<std::unique_ptr<Inspection::Value>> & GetTags(void) const
		{
			return _Tags;
		}
		
		Inspection::Value * GetTag(const std::string & Name)
		{
			for(auto & Tag : _Tags)
			{
				if(Tag->GetName() == Name)
				{
					return Tag.get();
				}
			}
			throw std::invalid_argument("Could not find a tag named \"" + Name + "\".");
		}
		
		Inspection::Value * GetField(const std::string & Name)
		{
			for(auto & Field : _Fields)
			{
				if(Field->GetName() == Name)
				{
					return Field.get();
				}
			}
			throw std::invalid_argument("Could not find a field named \"" + Name + "\".");
		}
		
		const std::list<std::unique_ptr<Inspection::Value>> & GetFields(void) const
		{
			return _Fields;
		}
		
		bool HasTag(const std::string & Name)
		{
			return std::find_if(std::begin(_Tags), std::end(_Tags), [&Name](auto & Tag) { return Tag->GetName() == Name; }) != std::end(_Tags);
		}
		
		bool HasField(const std::string & Name)
		{
			return std::find_if(std::begin(_Fields), std::end(_Fields), [&Name](auto & Field) { return Field->GetName() == Name; }) != std::end(_Fields);
		}
		
		void SetData(const std::any & Data)
		{
			_Data = Data;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
		
		void ClearFields(void)
		{
			_Fields.clear();
		}
	private:
		std::any _Data;
		std::string _Name;
		std::list<std::unique_ptr<Inspection::Value>> _Tags;
		std::list<std::unique_ptr<Inspection::Value>> _Fields;
	};
}

#endif
