#ifndef COMMON_RESULTS_H
#define COMMON_RESULTS_H

#include <algorithm>
#include <cstdint>
#include <experimental/any>
#include <list>
#include <memory>
#include <string>

namespace Results
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
		
		void Append(const std::string & Name, std::shared_ptr< Value > Value)
		{
			Value->SetName(Name);
			_Values.push_back(Value);
		}
		
		void Append(const std::string & Name, const std::experimental::any & Value)
		{
			_Values.push_back(std::make_shared< Results::Value >(Name, Value));
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
			
			throw new std::exception();
		}
		
		std::uint32_t GetCount(void) const
		{
			return _Values.size();
		}
		
		std::shared_ptr< Results::Value > GetValue(const std::string & Name)
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
		
		const std::list< std::shared_ptr< Value > > & GetValues(void)
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
		std::list< std::shared_ptr< Value > > _Values;
	};

	class Result
	{
	public:
		Result(void)
		{
		}
		
		Result(bool Success, std::uint64_t Length, std::shared_ptr< Value > Value) :
			_Length(Length),
			_Success(Success),
			_Value(Value)
		{
		}
		
		~Result(void)
		{
		}
		
		std::uint64_t GetLength(void) const
		{
			return _Length;
		}
		
		bool GetSuccess(void) const
		{
			return _Success;
		}
		
		std::shared_ptr< Value > & GetValue(void)
		{
			return _Value;
		}
		
		const std::shared_ptr< Value > & GetValue(const std::string & Name)
		{
			for(auto & Value : _Value->GetValues())
			{
				if(Value->GetName() == Name)
				{
					return Value;
				}
			}
			
			throw new std::exception();
		}
		
		const std::experimental::any & GetAny(void)
		{
			return _Value->GetAny();
		}
		
		const std::experimental::any & GetAny(const std::string & Name)
		{
			for(auto & Value : _Value->GetValues())
			{
				if(Value->GetName() == Name)
				{
					return Value->GetAny();
				}
			}
			
			throw new std::exception();
		}
		
		void SetLength(std::uint64_t Length)
		{
			_Length = Length;
		}
		
		void SetSuccess(bool Success)
		{
			_Success = Success;
		}
		
		void SetValue(std::shared_ptr< Value > Value)
		{
			_Value = Value;
		}
	private:
		std::uint64_t _Length;
		bool _Success;
		std::shared_ptr< Value > _Value;
	};
}

#endif
