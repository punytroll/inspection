#ifndef INSPECTION_COMMON_VALUE_H
#define INSPECTION_COMMON_VALUE_H

#include <algorithm>
#include <any>
#include <list>
#include <memory>
#include <stdexcept>

namespace Inspection
{
	class Value
	{
	public:
		Value(void)
		{
		}
		
		std::shared_ptr<Inspection::Value> AppendField(std::shared_ptr<Inspection::Value> Field)
		{
			_Fields.push_back(Field);
			
			return Field;
		}
		
		std::shared_ptr<Inspection::Value> AppendField(const std::string & Name, std::shared_ptr<Inspection::Value> Field)
		{
			Field->SetName(Name);
			
			return AppendField(Field);
		}
		
		std::shared_ptr<Inspection::Value> AppendField(const std::string & Name)
		{
			auto Field{std::make_shared<Inspection::Value>()};
			
			Field->SetName(Name);
			
			return AppendField(Field);
		}
		
		template<typename DataType>
		std::shared_ptr<Inspection::Value> AppendField(const std::string & Name, const DataType & Data)
		{
			auto Result{AppendField(Name)};
			
			Result->SetData(Data);
			
			return Result;
		}
		
		void AppendFields(const std::list<std::shared_ptr<Inspection::Value>> & Fields)
		{
			for(auto Field : Fields)
			{
				_Fields.push_back(Field);
			}
		}
		
		std::shared_ptr<Inspection::Value> AddTag(std::shared_ptr<Inspection::Value> Tag)
		{
			_Tags.push_back(Tag);
			
			return Tag;
		}
		
		std::shared_ptr<Inspection::Value> AddTag(const std::string & Name)
		{
			auto Tag{std::make_shared<Inspection::Value>()};
			
			Tag->SetName(Name);
			
			return AddTag(Tag);
		}
		
		template<typename DataType>
		std::shared_ptr<Inspection::Value> AddTag(const std::string & Name, const DataType & Data)
		{
			auto Tag{AddTag(Name)};
			
			Tag->SetData(Data);
			
			return Tag;
		}
		
		void AddTags(const std::list<std::shared_ptr<Inspection::Value>> & Tags)
		{
			for(auto Tag : Tags)
			{
				_Tags.push_back(Tag);
			}
		}
		
		const std::any & GetData(void)
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
		
		const std::list<std::shared_ptr<Inspection::Value>> & GetTags(void) const
		{
			return _Tags;
		}
		
		std::shared_ptr<Inspection::Value> GetTag(const std::string & Name)
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
		
		std::shared_ptr<Inspection::Value> GetField(const std::string & Name)
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
		
		const std::list<std::shared_ptr<Inspection::Value>> & GetFields(void)
		{
			return _Fields;
		}
		
		bool HasTag(const std::string & Name)
		{
			return std::find_if(std::begin(_Tags), std::end(_Tags), [&Name](const std::shared_ptr<Inspection::Value> & Tag) { return Tag->GetName() == Name; }) != std::end(_Tags);
		}
		
		bool HasField(const std::string & Name)
		{
			return std::find_if(std::begin(_Fields), std::end(_Fields), [&Name](const std::shared_ptr<Inspection::Value> & Field) { return Field->GetName() == Name; }) != std::end(_Fields);
		}
		
		void SetData(const std::any & Data)
		{
			_Data = Data;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
	private:
		std::any _Data;
		std::string _Name;
		std::list<std::shared_ptr<Inspection::Value>> _Tags;
		std::list<std::shared_ptr<Inspection::Value>> _Fields;
	};
}

#endif
