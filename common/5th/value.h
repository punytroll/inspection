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
		
		void Append(std::shared_ptr< Value > Value)
		{
			_Values.push_back(Value);
		}
		
		void Append(const std::string & Name, std::shared_ptr< Inspection::Value > Value)
		{
			Value->SetName(Name);
			_Values.push_back(Value);
		}
		
		void Append(const std::list< std::shared_ptr< Inspection::Value > > & Values)
		{
			for(auto Value : Values)
			{
				_Values.push_back(Value);
			}
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
		
		const Inspection::Length & GetLength(void) const
		{
			return _Length;
		}
		
		const std::string & GetName(void) const
		{
			return _Name;
		}
		
		const Inspection::Length & GetOffset(void) const
		{
			return _Offset;
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
		
		bool Has(const std::string & Name)
		{
			return std::find_if(std::begin(_Values), std::end(_Values), [&Name](const std::shared_ptr< Value > & Value) { return Value->GetName() == Name; }) != std::end(_Values);
		}
		
		void SetAny(const std::experimental::any & Any)
		{
			_Any = Any;
		}
		
		void SetLength(const Inspection::Length & Length)
		{
			_Length = Length;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
		
		void SetOffset(const Inspection::Length & Offset)
		{
			_Offset = Offset;
		}
	private:
		std::experimental::any _Any;
		Inspection::Length _Length;
		std::string _Name;
		Inspection::Length _Offset;
		std::list< std::shared_ptr< Inspection::Value > > _Values;
	};
	
	inline std::shared_ptr< Value > MakeValue(const std::string & Name)
	{
		auto Result{std::make_shared< Inspection::Value >()};
		
		Result->SetName(Name);
		
		return Result;
	}
	
	template< typename ValueType >
	inline std::shared_ptr< Value > MakeValue(const std::string & Name, const ValueType & Value)
	{
		auto Result{std::make_shared< Inspection::Value >()};
		
		Result->SetName(Name);
		Result->SetAny(Value);
		
		return Result;
	}
}

#endif
