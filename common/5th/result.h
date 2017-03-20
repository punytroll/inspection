#ifndef COMMON_5TH_RESULT_H
#define COMMON_5TH_RESULT_H

#include "length.h"
#include "value.h"

namespace Inspection
{
	class Result
	{
	public:
		Result(void) :
			_Success(false)
		{
		}
		
		Result(bool Success, std::shared_ptr< Value > Value) :
			_Success(Success),
			_Value(Value)
		{
		}
		
		~Result(void)
		{
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
		
		void SetSuccess(bool Success)
		{
			_Success = Success;
		}
		
		void SetValue(std::shared_ptr< Value > Value)
		{
			_Value = Value;
		}
	private:
		bool _Success;
		std::shared_ptr< Value > _Value;
	};

	inline std::unique_ptr< Inspection::Result > MakeFailure()
	{
		return std::make_unique< Inspection::Result >();
	}

	template< typename ValueType >
	inline std::unique_ptr< Result > MakeResult(bool Success, const ValueType & Value)
	{
		return std::make_unique< Inspection::Result >(Success, Inspection::MakeValue("", Value));
	}

	inline std::unique_ptr< Result > MakeResult(bool Success, std::shared_ptr< Inspection::Value > Value)
	{
		return std::make_unique< Inspection::Result >(Success, Value);
	}
}

#endif
