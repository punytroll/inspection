#ifndef INSPECTION_COMMON_VALUE_H
#define INSPECTION_COMMON_VALUE_H

#include <algorithm>
#include <experimental/any>
#include <list>
#include <memory>

namespace Inspection
{
	class Value
	{
	public:
		Value(void)
		{
		}
		
		std::shared_ptr< Inspection::Value > AppendField(std::shared_ptr< Inspection::Value > Field)
		{
			_Fields.push_back(Field);
			
			return Field;
		}
		
		std::shared_ptr< Inspection::Value > AppendField(const std::string & Name, std::shared_ptr< Inspection::Value > Field)
		{
			Field->SetName(Name);
			
			return AppendField(Field);
		}
		
		template< typename DataType >
		std::shared_ptr< Inspection::Value > AppendField(const std::string & Name, const DataType & Data)
		{
			auto Field{std::make_shared< Inspection::Value >()};
			
			Field->SetData(Data);
			
			return AppendField(Name, Field);
		}
		
		void AppendFields(const std::list< std::shared_ptr< Inspection::Value > > & Fields)
		{
			for(auto Field : Fields)
			{
				_Fields.push_back(Field);
			}
		}
		
		std::shared_ptr< Value > AddTag(std::shared_ptr< Value > Value)
		{
			_Tags.push_back(Value);
			
			return Value;
		}
		
		std::shared_ptr< Value > AddTag(const std::string & Name)
		{
			auto Result{std::make_shared< Inspection::Value >()};
			
			Result->SetName(Name);
			_Tags.push_back(Result);
			
			return Result;
		}
		
		template< typename DataType >
		std::shared_ptr< Value > AddTag(const std::string & Name, const DataType & Data)
		{
			auto Result{std::make_shared< Inspection::Value >()};
			
			Result->SetData(Data);
			Result->SetName(Name);
			_Tags.push_back(Result);
			
			return Result;
		}
		
		const std::experimental::any & GetData(void)
		{
			return _Data;
		}
		
		std::uint32_t GetCount(void) const
		{
			return _Fields.size();
		}
		
		const std::string & GetName(void) const
		{
			return _Name;
		}
		
		const std::list< std::shared_ptr< Inspection::Value > > & GetTags(void) const
		{
			return _Tags;
		}
		
		std::shared_ptr< Inspection::Value > GetTag(const std::string & Name)
		{
			for(auto & Tag : _Tags)
			{
				if(Tag->GetName() == Name)
				{
					return Tag;
				}
			}
			throw std::invalid_argument("Could not find a tag named \"" + Name + "\".");
		}
		
		std::shared_ptr< Inspection::Value > GetValue(const std::string & Name)
		{
			for(auto & Field : _Fields)
			{
				if(Field->GetName() == Name)
				{
					return Field;
				}
			}
			throw std::invalid_argument("Could not find a field named \"" + Name + "\".");
		}
		
		const std::list< std::shared_ptr< Inspection::Value > > & GetFields(void)
		{
			return _Fields;
		}
		
		bool HasTag(const std::string & Name)
		{
			return std::find_if(std::begin(_Tags), std::end(_Tags), [&Name](const std::shared_ptr< Inspection::Value > & Tag) { return Tag->GetName() == Name; }) != std::end(_Tags);
		}
		
		bool HasField(const std::string & Name)
		{
			return std::find_if(std::begin(_Fields), std::end(_Fields), [&Name](const std::shared_ptr< Inspection::Value > & Field) { return Field->GetName() == Name; }) != std::end(_Fields);
		}
		
		void SetData(const std::experimental::any & Data)
		{
			_Data = Data;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
	private:
		std::experimental::any _Data;
		std::string _Name;
		std::list< std::shared_ptr< Inspection::Value > > _Tags;
		std::list< std::shared_ptr< Inspection::Value > > _Fields;
	};
}

#endif
