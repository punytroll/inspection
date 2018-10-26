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
		
		std::shared_ptr< Value > AppendValue(std::shared_ptr< Value > Value)
		{
			_Values.push_back(Value);
			
			return Value;
		}
		
		template< typename AnyType >
		std::shared_ptr< Value > AppendValue(const std::string & Name, const AnyType & Any)
		{
			auto Result{std::make_shared< Inspection::Value >()};
			
			Result->SetName(Name);
			Result->SetAny(Any);
			_Values.push_back(Result);
			
			return Result;
		}
		
		std::shared_ptr< Value > AppendValue(const std::string & Name, std::shared_ptr< Inspection::Value > Value)
		{
			Value->SetName(Name);
			_Values.push_back(Value);
			
			return Value;
		}
		
		void AppendValues(const std::list< std::shared_ptr< Inspection::Value > > & Values)
		{
			for(auto Value : Values)
			{
				_Values.push_back(Value);
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
		
		std::shared_ptr< Value > AddTag(const std::string & Name, const std::experimental::any & Any)
		{
			auto Result{std::make_shared< Inspection::Value >()};
			
			Result->SetAny(Any);
			Result->SetName(Name);
			_Tags.push_back(Result);
			
			return Result;
		}
		
		const std::experimental::any & GetAny(void)
		{
			return _Any;
		}
		
		const std::experimental::any & GetTagAny(const std::string & Name)
		{
			for(auto & Tag : _Tags)
			{
				if(Tag->GetName() == Name)
				{
					return Tag->GetAny();
				}
			}
			throw std::invalid_argument("Could not find any value named \"" + Name + "\".");
		}
		
		std::uint32_t GetCount(void) const
		{
			return _Values.size();
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
			for(auto & Value : _Values)
			{
				if(Value->GetName() == Name)
				{
					return Value;
				}
			}
			throw std::invalid_argument("Could not find a value named \"" + Name + "\".");
		}
		
		const std::list< std::shared_ptr< Inspection::Value > > & GetValues(void)
		{
			return _Values;
		}
		
		bool HasTag(const std::string & Name)
		{
			return std::find_if(std::begin(_Tags), std::end(_Tags), [&Name](const std::shared_ptr< Inspection::Value > & Tag) { return Tag->GetName() == Name; }) != std::end(_Tags);
		}
		
		bool HasValue(const std::string & Name)
		{
			return std::find_if(std::begin(_Values), std::end(_Values), [&Name](const std::shared_ptr< Value > & Value) { return Value->GetName() == Name; }) != std::end(_Values);
		}
		
		void SetAny(const std::experimental::any & Any)
		{
			_Any = Any;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
	private:
		std::experimental::any _Any;
		std::string _Name;
		std::list< std::shared_ptr< Inspection::Value > > _Tags;
		std::list< std::shared_ptr< Inspection::Value > > _Values;
	};
}

#endif
