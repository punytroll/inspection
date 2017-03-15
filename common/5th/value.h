#ifndef COMMON_5TH_VALUE_H
#define COMMON_5TH_VALUE_H

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
		
		Value(const std::string & Name) :
			_Name(Name)
		{
		}
		
		Value(const std::experimental::any & Any) :
			_Any(Any)
		{
		}
		
		Value(const std::string & Name, const std::experimental::any & Any) :
			_Any(Any),
			_Name(Name)
		{
		}
		
		void Append(std::shared_ptr< Value > Value)
		{
			_Values.push_back(Value);
		}
		
		void Append(const std::string & Name, std::shared_ptr< Inspection::Value > Value)
		{
			Value->SetName(Name);
			_Values.push_back(Value);
		}
		
		void Append(const std::string & Name, const std::experimental::any & Value)
		{
			_Values.push_back(std::make_shared< Inspection::Value >(Name, Value));
		}
		
		void Append(const std::experimental::any & Any)
		{
			_Values.push_back(std::make_shared< Inspection::Value >(Any));
		}
		
		const std::string & GetName(void) const
		{
			return _Name;
		}
		
		bool Has(const std::string & Name)
		{
			return std::find_if(std::begin(_Values), std::end(_Values), [&Name](const std::shared_ptr< Value > & Value) { return Value->GetName() == Name; }) != std::end(_Values);
		}
		
		const std::experimental::any & GetAny(void)
		{
			return _Any;
		}
		
		const std::experimental::any & GetAny(const std::string & Name)
		{
			for(auto & Value : _Values)
			{
				if(Value->GetName() == Name)
				{
					return Value->GetAny();
				}
			}
			
			throw new std::invalid_argument("Could not find any value named \"" + Name + "\".");
		}
		
		std::uint32_t GetCount(void) const
		{
			return _Values.size();
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
			
			throw new std::exception();
		}
		
		const std::list< std::shared_ptr< Inspection::Value > > & GetValues(void)
		{
			return _Values;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
		
		void SetAny(const std::experimental::any & Any)
		{
			_Any = Any;
		}
	private:
		std::experimental::any _Any;
		std::string _Name;
		std::list< std::shared_ptr< Inspection::Value > > _Values;
	};
}

#endif