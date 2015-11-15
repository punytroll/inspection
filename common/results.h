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
	class ValueBase
	{
	public:
		ValueBase(void)
		{
		}
		
		ValueBase(const std::string & Name) :
			_Name(Name)
		{
		}
		
		virtual ~ValueBase(void)
		{
		}
		
		const std::string & GetName(void) const
		{
			return _Name;
		}
		
		void SetName(const std::string & Name)
		{
			_Name = Name;
		}
		
		virtual std::experimental::any & GetAny(void) = 0;
		
		virtual std::experimental::any & GetAny(const std::string & Name) = 0;
		
		virtual std::shared_ptr< Results::ValueBase > GetValue(const std::string & Name) = 0;
		
		virtual std::list< std::shared_ptr< ValueBase > > & GetValues(void) = 0;
	protected:
		std::string _Name;
	};

	class Value : public ValueBase
	{
	public:
		Value(void)
		{
		}
		
		Value(const std::string & Name) :
			ValueBase(Name)
		{
		}
		
		Value(const std::experimental::any & Value) :
			_Value(Value)
		{
		}
		
		Value(const std::string & Name, const std::experimental::any & Value) :
			ValueBase(Name),
			_Value(Value)
		{
		}
		
		virtual ~Value(void)
		{
		}
		
		void SetValue(std::experimental::any & Value)
		{
			_Value = Value;
		}
		
		virtual std::experimental::any & GetAny(void) override
		{
			return _Value;
		}
		
		virtual std::experimental::any & GetAny(const std::string & Name) override
		{
			if(_Name == Name)
			{
				return _Value;
			}
			
			throw new std::exception();
		}
		
		virtual std::shared_ptr< Results::ValueBase > GetValue(const std::string & Name) override
		{
			throw new std::exception();
		}
		
		virtual std::list< std::shared_ptr< ValueBase > > & GetValues(void)
		{
			throw new std::exception();
		}
	private:
		std::experimental::any _Value;
	};

	class Values : public ValueBase
	{
	public:
		Values(void)
		{
		}
		
		Values(const std::string & Name) :
			ValueBase(Name)
		{
		}
		
		virtual ~Values(void)
		{
		}
		
		void Append(std::shared_ptr< ValueBase > Value)
		{
			_Values.push_back(Value);
		}
		
		void Append(const std::string & Name, std::shared_ptr< ValueBase > Value)
		{
			Value->SetName(Name);
			_Values.push_back(Value);
		}
		
		void Append(const std::string & Name, std::experimental::any & Value)
		{
			_Values.push_back(std::make_shared< Results::Value >(Name, Value));
		}
		
		std::uint32_t GetCount(void) const
		{
			return _Values.size();
		}
		
		bool Has(const std::string & Name)
		{
			return std::find_if(std::begin(_Values), std::end(_Values), [&Name](const std::shared_ptr< ValueBase > & Value) { return Value->GetName() == Name; }) != std::end(_Values);
		}
		
		virtual std::shared_ptr< Results::ValueBase > GetValue(const std::string & Name) override
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
		
		virtual std::experimental::any & GetAny(void) override
		{
			throw new std::exception();
		}
		
		virtual std::experimental::any & GetAny(const std::string & Name)
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
		
		virtual std::list< std::shared_ptr< ValueBase > > & GetValues(void)
		{
			return _Values;
		}
	private:
		std::list< std::shared_ptr< ValueBase > > _Values;
	};

	class Result
	{
	public:
		Result(void)
		{
		}
		
		Result(bool Success, std::uint64_t Length, std::shared_ptr< ValueBase > Value) :
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
		
		std::shared_ptr< ValueBase > & GetValue(void)
		{
			return _Value;
		}
		
		std::shared_ptr< ValueBase > & GetValue(const std::string & Name)
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
		
		std::experimental::any & GetAny(void)
		{
			return _Value->GetAny();
		}
		
		std::experimental::any & GetAny(const std::string & Name)
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
		
		void SetValue(std::shared_ptr< ValueBase > Value)
		{
			_Value = Value;
		}
	private:
		std::uint64_t _Length;
		bool _Success;
		std::shared_ptr< ValueBase > _Value;
	};
}

#endif
